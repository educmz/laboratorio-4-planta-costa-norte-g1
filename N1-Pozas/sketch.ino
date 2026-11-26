// Planta Costa Norte - Nodo 1: Pozas de materia prima
// Instalacion heredada del proveedor anterior
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>
#include <Preferences.h>
#include <stdlib.h>
#include <string.h>
#include "mbedtls/md.h"

const char* CLAVE   = "costa-norte-ga-4817";
const char* T_DATOS = "upc/ga/pozas/datos";
const char* T_CMD   = "upc/ga/pozas/cmd";

const int PIN_GAS = 34, PIN_TEMP = 4, PIN_BUZZER = 25, PIN_VENT = 26;
float umbralGas = 2000;       // ppm
const float UMBRAL_T = 35;    // grados C
bool ventMax = false;

WiFiClient net;
PubSubClient mqtt(net);
OneWire bus(PIN_TEMP);
DallasTemperature sensor(&bus);
Preferences prefs;
bool nvsOK = false;
uint32_t ultimoTsComando = 0;

String firmar(const String& m) {
  byte out[32];
  mbedtls_md_context_t c;
  mbedtls_md_init(&c);
  mbedtls_md_setup(&c, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
  mbedtls_md_hmac_starts(&c, (const unsigned char*)CLAVE, strlen(CLAVE));
  mbedtls_md_hmac_update(&c, (const unsigned char*)m.c_str(), m.length());
  mbedtls_md_hmac_finish(&c, out);
  mbedtls_md_free(&c);
  char h[17];
  for (int i = 0; i < 8; i++) sprintf(h + 2 * i, "%02x", out[i]);
  return String(h);
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

void conectar() {
  while (WiFi.status() != WL_CONNECTED) delay(200);
  while (!mqtt.connected()) {
    String id = "ga-pozas-" + WiFi.macAddress();
    if (mqtt.connect(id.c_str())) {
      mqtt.subscribe(T_CMD);
      Serial.println("MQTT conectado");
    } else {
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_VENT, OUTPUT);
  sensor.begin();
  nvsOK = prefs.begin("ga-pozas", false);
  if (nvsOK) ultimoTsComando = prefs.getUInt("lastCmdTs", 0);
  WiFi.begin("Wokwi-GUEST", "");
  while (WiFi.status() != WL_CONNECTED) delay(200);
  configTime(0, 0, "pool.ntp.org");
  for (int i = 0; i < 50 && time(nullptr) < 1700000000; i++) delay(200);
  mqtt.setServer("broker.hivemq.com", 1883);
  mqtt.setBufferSize(512);
  mqtt.setKeepAlive(60);
  mqtt.setCallback(alRecibir);
  conectar();
}

unsigned long tPub = 0;

void loop() {
  if (!mqtt.connected()) conectar();
  mqtt.loop();
  if (millis() - tPub < 2000) return;
  tPub = millis();

  float gas = analogRead(PIN_GAS) * 10000.0 / 4095.0;
  sensor.requestTemperatures();
  float temp = sensor.getTempCByIndex(0);

  // Accion local (edge)
  bool gasAlto = gas > umbralGas;
  bool temperaturaAlta = temp > UMBRAL_T;
  bool ventilar = gasAlto || temperaturaAlta || ventMax;
  digitalWrite(PIN_VENT, ventilar ? HIGH : LOW);
  if (gasAlto) tone(PIN_BUZZER, 1000); else noTone(PIN_BUZZER);

  long ts = time(nullptr);
  String v = String(gas, 1) + "ppm";
  String msg = "{\"id\":\"pozas\",\"ts\":" + String(ts) + ",\"valor\":\"" + v +
               "\",\"unidad\":\"ppm\",\"temp\":" + String(temp, 1) +
               ",\"calidad\":\"ok\",\"firma\":\"" + firmar("pozas|" + String(ts) + "|" + v) + "\"}";
  mqtt.publish(T_DATOS, msg.c_str());
  Serial.println(msg);
  Serial.println(String("Ventilacion: ") + (ventilar ? "ON" : "OFF"));
}
