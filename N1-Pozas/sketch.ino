// Planta Costa Norte - Nodo 1: Pozas de materia prima
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Preferences.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "mbedtls/md.h"

const char* CLAVE   = "costa-norte-ga-4817";
const char* T_DATOS = "upc/ga/pozas/datos";
const char* T_CMD   = "upc/ga/pozas/cmd";

const int PIN_GAS = 34;
const int PIN_TEMP = 4;
const int PIN_BUZZER = 25;
const int PIN_VENT = 26;

float umbralGas = 2000.0f;
const float UMBRAL_T = 35.0f;

bool ventMax = false;

const unsigned long PUBLICAR_MS = 2000;
const unsigned long MQTT_RETRY_MS = 5000;
const unsigned long WIFI_RETRY_MS = 30000;

WiFiClient net;
PubSubClient mqtt(net);
OneWire bus(PIN_TEMP);
DallasTemperature sensor(&bus);
Preferences prefs;

unsigned long tPub = 0;
unsigned long tMqtt = 0;
unsigned long tWifi = 0;

bool intentoMqttRealizado = false;
bool nvsOK = false;

uint32_t ultimoTsComando = 0;

String firmar(const String& mensaje) {
  const mbedtls_md_info_t* info =
    mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

  if (!info) return "";

  unsigned char digest[32];

  int resultado = mbedtls_md_hmac(
    info,
    (const unsigned char*)CLAVE,
    strlen(CLAVE),
    (const unsigned char*)mensaje.c_str(),
    mensaje.length(),
    digest
  );

  if (resultado != 0) return "";

  char hex[17];

  for (int i = 0; i < 8; i++) {
    sprintf(hex + i * 2, "%02x", digest[i]);
  }

  hex[16] = '\0';

  return String(hex);
}

bool firmaIgual(const String& a, const String& b) {
  if (a.length() != 16 || b.length() != 16) {
    return false;
  }

  uint8_t diferencia = 0;

  for (int i = 0; i < 16; i++) {
    diferencia |= (uint8_t)(a[i] ^ b[i]);
  }

  return diferencia == 0;
}

float interpolarGas(
  int adc,
  int adcA,
  float ppmA,
  int adcB,
  float ppmB
) {
  float proporcion =
    float(adc - adcA) / float(adcB - adcA);

  float logPPM =
    log10f(ppmA) +
    proporcion * (log10f(ppmB) - log10f(ppmA));

  float resultado = powf(10.0f, logPPM);

  return fmaxf(0.1f, fminf(resultado, 100000.0f));
}

float convertirGasPPM(int adc) {
  const int adcTabla[] = {
    3382, 3790, 3830, 3906
  };

  const float ppmTabla[] = {
    120.0f, 1000.0f, 2000.0f, 4000.0f
  };

  const int cantidad = 4;

  if (adc <= adcTabla[0]) {
    return interpolarGas(
      adc,
      adcTabla[0], ppmTabla[0],
      adcTabla[1], ppmTabla[1]
    );
  }

  for (int i = 0; i < cantidad - 1; i++) {
    if (adc <= adcTabla[i + 1]) {
      return interpolarGas(
        adc,
        adcTabla[i], ppmTabla[i],
        adcTabla[i + 1], ppmTabla[i + 1]
      );
    }
  }

  return interpolarGas(
    adc,
    adcTabla[2], ppmTabla[2],
    adcTabla[3], ppmTabla[3]
  );
}

void alRecibir(
  char* topic,
  byte* payload,
  unsigned int longitud
) {
  if (strcmp(topic, T_CMD) != 0) return;

  if (longitud == 0 || longitud > 160) {
    Serial.println(
      "[SEGURIDAD] RECHAZADO: tamano invalido"
    );
    return;
  }

  String mensaje;
  mensaje.reserve(longitud);

  for (unsigned int i = 0; i < longitud; i++) {
    mensaje += (char)payload[i];
  }

  int p1 = mensaje.indexOf('|');
  int p2 = mensaje.indexOf('|', p1 + 1);
  int p3 = mensaje.lastIndexOf('|');

  if (p1 <= 0 || p2 <= p1 + 1 || p3 <= p2 + 1) {
    Serial.println(
      "[SEGURIDAD] RECHAZADO: formato invalido"
    );
    return;
  }

  String id = mensaje.substring(0, p1);
  String textoTs = mensaje.substring(p1 + 1, p2);
  String contenido = mensaje.substring(p2 + 1, p3);
  String firma = mensaje.substring(p3 + 1);

  if (id != "gateway") {
    Serial.println(
      "[SEGURIDAD] RECHAZADO: identidad invalida"
    );
    return;
  }

  char* fin = nullptr;
  long ts = strtol(textoTs.c_str(), &fin, 10);

  if (fin == textoTs.c_str() ||
      *fin != '\0' ||
      ts < 1700000000L) {
    Serial.println(
      "[SEGURIDAD] RECHAZADO: timestamp invalido"
    );
    return;
  }

  long ahora = (long)time(nullptr);

  if (ahora < 1700000000L ||
      ahora - ts > 60 ||
      ts - ahora > 15) {
    Serial.println(
      "[SEGURIDAD] RECHAZADO: mensaje vencido o sin NTP"
    );
    return;
  }

  String material = mensaje.substring(0, p3);

  if (!firmaIgual(firma, firmar(material))) {
    Serial.println(
      "[SEGURIDAD] RECHAZADO: HMAC invalida"
    );
    return;
  }

  if (contenido != "cmd:VENT:MAX" &&
      contenido != "cmd:VENT:NORMAL") {
    Serial.println(
      "[SEGURIDAD] RECHAZADO: comando desconocido"
    );
    return;
  }

  if (ts <= (long)ultimoTsComando) {
    Serial.println(
      "[SEGURIDAD] RECHAZADO: Replay"
    );
    return;
  }

  if (!nvsOK ||
      prefs.putUInt("lastCmdTs", (uint32_t)ts) == 0) {
    Serial.println(
      "[SEGURIDAD] RECHAZADO: error de persistencia"
    );
    return;
  }

  ultimoTsComando = (uint32_t)ts;

  if (contenido == "cmd:VENT:MAX") {
    ventMax = true;

    digitalWrite(PIN_VENT, HIGH);

    Serial.println(
      "[GATEWAY] VENT:MAX autenticado y ejecutado"
    );
  } else {
    ventMax = false;

    Serial.println(
      "[GATEWAY] VENT:NORMAL autenticado"
    );
  }
}

void mantenerConexion() {
  unsigned long ahora = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (ahora - tWifi >= WIFI_RETRY_MS) {
      tWifi = ahora;

      Serial.println(
        "[WiFi] Reintentando conexion"
      );

      WiFi.reconnect();
    }

    return;
  }

  if (mqtt.connected()) return;

  if (intentoMqttRealizado &&
      ahora - tMqtt < MQTT_RETRY_MS) {
    return;
  }

  intentoMqttRealizado = true;
  tMqtt = ahora;

  String clientId =
    "ga-pozas-" + String((uint32_t)esp_random(), HEX);

  if (mqtt.connect(clientId.c_str())) {
    bool suscrito = mqtt.subscribe(T_CMD);

    Serial.println("MQTT conectado");

    Serial.println(
      suscrito
        ? "[MQTT] Suscrito a comandos"
        : "[MQTT] Error de suscripcion"
    );
  } else {
    Serial.printf(
      "[MQTT] Conexion fallida, estado=%d\n",
      mqtt.state()
    );
  }
}

void leerYActuar() {
  int adcGas = analogRead(PIN_GAS);
  float gas = convertirGasPPM(adcGas);

  sensor.requestTemperatures();
  float temp = sensor.getTempCByIndex(0);

  bool temperaturaValida =
    temp != DEVICE_DISCONNECTED_C &&
    isfinite(temp);

  bool gasAlto = gas > umbralGas;

  bool temperaturaAlta =
    temperaturaValida &&
    temp > UMBRAL_T;

  bool ventilar =
    gasAlto ||
    temperaturaAlta ||
    ventMax ||
    !temperaturaValida;

  digitalWrite(
    PIN_VENT,
    ventilar ? HIGH : LOW
  );

  if (gasAlto) {
    tone(PIN_BUZZER, 1000);
  } else {
    noTone(PIN_BUZZER);
  }

  Serial.printf(
    "[MQ2] ADC=%d | Gas estimado=%.1f ppm | Temp=%.1f C\n",
    adcGas,
    gas,
    temp
  );

  Serial.println(
    String("Ventilacion: ") +
    (ventilar ? "ON" : "OFF")
  );

  if (!mqtt.connected()) return;

  time_t tiempo = time(nullptr);

  if (tiempo < 1700000000LL) {
    Serial.println(
      "[MQTT] Sin NTP: no publicar datos sin hora valida"
    );
    return;
  }

  uint32_t ts = (uint32_t)tiempo;
  String v = String(gas, 1);

  String material =
    "pozas|" + String(ts) + "|" + v;

  String firma = firmar(material);

  if (firma.length() != 16) {
    Serial.println(
      "[MQTT] No se pudo generar HMAC"
    );
    return;
  }

  String msg =
    "{\"id\":\"pozas\",\"ts\":" + String(ts) +
    ",\"valor\":" + v +
    ",\"unidad\":\"ppm\",\"temp\":" +
    String(temp, 1) +
    ",\"calidad\":\"" +
    String(temperaturaValida ? "ok" : "error_sensor") +
    "\",\"firma\":\"" + firma + "\"}";

  bool enviado = mqtt.publish(
    T_DATOS,
    msg.c_str()
  );

  if (!enviado) {
    Serial.printf(
      "[MQTT] Error al publicar, estado=%d\n",
      mqtt.state()
    );
  }

  Serial.println(msg);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_GAS, INPUT);
  pinMode(PIN_VENT, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_VENT, LOW);
  noTone(PIN_BUZZER);
  analogReadResolution(12);
  sensor.begin();
  nvsOK = prefs.begin("ga-pozas", false);
  if (nvsOK) {
    ultimoTsComando =
      prefs.getUInt("lastCmdTs", 0);
  } else {
    Serial.println(
      "[SEGURIDAD] NVS no disponible"
    );
  }

  mqtt.setServer(
    "broker.hivemq.com",
    1883
  );

  mqtt.setCallback(alRecibir);
  mqtt.setBufferSize(1024);
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(3);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  WiFi.begin("Wokwi-GUEST", "", 6);

  tWifi = millis();

  configTime(
    0, 0,
    "pool.ntp.org",
    "time.google.com"
  );

  Serial.println(
    "[N1] Pozas - Laboratorio 4"
  );

  Serial.println(
    "[N1] Lectura Edge, HMAC y MQTT iniciados"
  );
}

void loop() {
  unsigned long ahora = millis();

  static unsigned long tNtp = 0;
  if (WiFi.status() == WL_CONNECTED &&
      ahora - tNtp >= 15000) {
    tNtp = ahora;
    configTime(0, 0, "pool.ntp.org", "time.google.com");
  }

  if (tPub == 0 ||
      ahora - tPub >= PUBLICAR_MS) {
    tPub = ahora;
    leerYActuar();
  }

  if (mqtt.connected()) {
    mqtt.loop();
  }

  mantenerConexion();

  delay(10);
}
