/**
 * Planta Costa Norte - G1
 * Nodo 3: Perimetro
 * Laboratorio 4 - Desarrollo de Soluciones IoT
 *
 * F9:  Cambio de PIN_VIENTO de GPIO 25 (ADC2) a GPIO 34 (ADC1) por conflicto con WiFi.
 * F10: Correccion de escala ADC de 10 bits (1023) a 12 bits (4095) para direccion del viento.
 * F11: Validacion HMAC-SHA256 y timestamp en comandos de calibracion (R9 y R10).
 * F12: Correccion de Client ID duplicado ("ga-sensor") usando identificador unico por nodo y MAC.
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include "mbedtls/md.h"


// CONFIGURACION MQTT Y SEGURIDAD


const char* CLAVE   = "costa-norte-ga-4817";
const char* T_DATOS = "upc/ga/perimetro/datos";
const char* T_CMD   = "upc/ga/perimetro/cmd";


// CONFIGURACION DE PINES Y UMBRALES


const int PIN_GAS = 35;

// F9 - CORRECCION VISIBLE:
// Antes: GPIO 25 (ADC2, incompatible con WiFi en ESP32).
// Ahora: GPIO 34 (ADC1, funciona correctamente con WiFi activo).
const int PIN_VIENTO = 34;

const int PIN_LED = 18;

const float UMBRAL_GAS = 2000.0;        // ppm (Regla R1)
const unsigned long PERIODO = 2000;     // 2 s (Regla R8)
const long MAX_ANTIGUEDAD_CMD = 60;     // 60 s (Regla R9)

float offset = 0.0;                     // calibracion del sensor


// OBJETOS Y VARIABLES


WiFiClient net;
PubSubClient mqtt(net);

unsigned long tPub = 0;


// FIRMA HMAC-SHA256


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
  for (int i = 0; i < 8; i++) {
    snprintf(h + 2 * i, 3, "%02x", out[i]);
  }
  h[16] = '\0';
  return String(h);
}


// F11 - CORRECCION DE SEGURIDAD (R9 y R10)


/**
 * Solo acepta comandos OFFSET firmados por el gateway y vigentes (<= 60 s).
 * Formato esperado: OFFSET:<valor>|<ts>|<firma>
 * Firma esperada:   HMAC-SHA256("gateway|<ts>|OFFSET:<valor>") (16 hex)
 */
void alRecibir(char* t, byte* p, unsigned int n) {
  String msg;
  for (unsigned int i = 0; i < n; i++) {
    msg += (char)p[i];
  }
  msg.trim();

  if (!msg.startsWith("OFFSET:")) {
    Serial.println("SEGURIDAD: comando desconocido rechazado -> " + msg);
    return;
  }

  int sep1 = msg.indexOf('|');
  int sep2 = msg.indexOf('|', sep1 + 1);

  if (sep1 == -1 || sep2 == -1) {
    Serial.println("SEGURIDAD: comando OFFSET sin firma ni timestamp rechazado");
    return;
  }

  String cmdParte = msg.substring(0, sep1);           // "OFFSET:<valor>"
  String tsStr    = msg.substring(sep1 + 1, sep2);    // "<ts>"
  String firmaRec = msg.substring(sep2 + 1);          // "<firma>"
  firmaRec.toLowerCase();

  long tsCmd = tsStr.toInt();
  long ahora = time(nullptr);

  if (ahora < 1700000000 || labs(ahora - tsCmd) > MAX_ANTIGUEDAD_CMD) {
    Serial.println("SEGURIDAD: comando OFFSET expirado o timestamp invalido");
    return;
  }

  String firmaEsperada = firmar("gateway|" + tsStr + "|" + cmdParte);
  if (firmaRec != firmaEsperada) {
    Serial.println("SEGURIDAD: firma HMAC invalida, comando OFFSET rechazado");
    return;
  }

  float nuevoOffset = cmdParte.substring(7).toFloat();
  if (nuevoOffset < -500.0 || nuevoOffset > 500.0) {
    Serial.println("SEGURIDAD: offset fuera de rango permitido (-500 a 500)");
    return;
  }

  offset = nuevoOffset;
  Serial.println("Calibracion autenticada aplicada: " + String(offset, 1));
}


// F12 - CONEXION MQTT (CLIENT ID UNICO)


void conectar() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqtt.connected()) return;

  // F12 - CORRECCION CRUZADA:
  // Antes: "ga-sensor" (duplicado con otros nodos del grupo).
  // Ahora: identificador unico por grupo, nodo y MAC.
  String clientId = "ga-perimetro-" + WiFi.macAddress();

  if (mqtt.connect(clientId.c_str())) {
    mqtt.subscribe(T_CMD);
    Serial.println("MQTT conectado | Client ID: " + clientId);
  } else {
    Serial.println("Error MQTT: " + String(mqtt.state()));
  }
}


// CONFIGURACION INICIAL


void setup() {
  Serial.begin(115200);
  pinMode(PIN_GAS, INPUT);
  pinMode(PIN_VIENTO, INPUT);
  pinMode(PIN_LED, OUTPUT);

  // ADC de 12 bits (0 a 4095)
  analogReadResolution(12);

  WiFi.begin("Wokwi-GUEST", "");
  Serial.println("Conectando WiFi...");
  while (WiFi.status() != WL_CONNECTED) delay(200);
  Serial.println("WiFi conectado");

  configTime(0, 0, "pool.ntp.org");
  for (int i = 0; i < 50 && time(nullptr) < 1700000000; i++) delay(200);

  mqtt.setServer("broker.hivemq.com", 1883);
  mqtt.setBufferSize(512);
  mqtt.setKeepAlive(60);
  mqtt.setCallback(alRecibir);
  conectar();

  Serial.println("================================");
  Serial.println("Nodo N3 Perimetro inicializado");
  Serial.println("================================");
}


// BUCLE PRINCIPAL


void loop() {
  static unsigned long tConexion = 0;
  if (!mqtt.connected() && millis() - tConexion >= 5000) {
    tConexion = millis();
    conectar();
  }
  if (mqtt.connected()) {
    mqtt.loop();
  }

  if (millis() - tPub < PERIODO) return;
  tPub = millis();

  float gas = analogRead(PIN_GAS) * 10000.0 / 4095.0 + offset;
  if (gas < 0) gas = 0;

  // F10 - CORRECCION SILENCIOSA:
  // Antes: map(analogRead(PIN_VIENTO), 0, 1023, 0, 360)
  // Ahora: escala completa de 12 bits (0 a 4095) -> 0 a 359 grados.
  int adcViento = analogRead(PIN_VIENTO);
  int viento = constrain(map(adcViento, 0, 4095, 0, 359), 0, 359);

  // Regla R3: viento hacia la comunidad entre 90 y 180 grados
  bool haciaComunidad = (viento >= 90 && viento <= 180);

  // Indicador local de riesgo hacia la comunidad (edge)
  digitalWrite(PIN_LED, (gas > UMBRAL_GAS && haciaComunidad) ? HIGH : LOW);

  long ts = time(nullptr);
  if (ts < 1700000000) {
    Serial.println("Timestamp NTP no valido");
    return;
  }

  String v = String(gas, 1);
  String firma = firmar("perimetro|" + String(ts) + "|" + v);

  String msg = "{\"id\":\"perimetro\",\"ts\":" + String(ts) +
               ",\"valor\":" + v +
               ",\"unidad\":\"ppm\",\"viento\":" + String(viento) +
               ",\"calidad\":\"ok\",\"firma\":\"" + firma + "\"}";

  if (mqtt.connected() && mqtt.publish(T_DATOS, msg.c_str())) {
    Serial.println(msg);
  } else {
    Serial.println("Error al publicar por MQTT");
  }
}