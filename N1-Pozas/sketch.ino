// Planta Costa Norte - Nodo 1: Pozas de materia prima
// Instalacion heredada del proveedor anterior
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>
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

void alRecibir(char* t, byte* p, unsigned int n) {
  String msg;
  for (unsigned int i = 0; i < n; i++) msg += (char)p[i];
  if (msg.startsWith("UMBRAL:")) {
    umbralGas = msg.substring(7).toFloat();
    Serial.println("Nuevo umbral de gas: " + String(umbralGas));
  } else if (msg == "VENT:MAX") {
    ventMax = true;
  } else if (msg == "VENT:NORMAL") {
    ventMax = false;
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
