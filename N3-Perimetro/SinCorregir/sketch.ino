// Planta Costa Norte - Nodo 3: Perimetro
// Instalacion heredada del proveedor anterior
#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include "mbedtls/md.h"

const char* CLAVE   = "costa-norte-ga-4817";
const char* T_DATOS = "upc/ga/perimetro/datos";
const char* T_CMD   = "upc/ga/perimetro/cmd";

const int PIN_GAS = 35, PIN_VIENTO = 25, PIN_LED = 18;
const float UMBRAL_GAS = 2000;   // ppm
float offset = 0;                // calibracion del sensor

WiFiClient net;
PubSubClient mqtt(net);

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
  if (msg.startsWith("OFFSET:")) {
    offset = msg.substring(7).toFloat();
    Serial.println("Nueva calibracion: " + String(offset));
  }
}

void conectar() {
  while (WiFi.status() != WL_CONNECTED) delay(200);
  while (!mqtt.connected()) {
    if (mqtt.connect("ga-sensor")) {
      mqtt.subscribe(T_CMD);
      Serial.println("MQTT conectado");
    } else {
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
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

  float gas = analogRead(PIN_GAS) * 10000.0 / 4095.0 + offset;
  if (gas < 0) gas = 0;
  int viento = constrain(map(analogRead(PIN_VIENTO), 0, 1023, 0, 360), 0, 359);  // grados
  bool haciaComunidad = (viento >= 90 && viento <= 180);

  // Indicador local de riesgo hacia la comunidad (edge)
  digitalWrite(PIN_LED, gas > UMBRAL_GAS && haciaComunidad);

  long ts = time(nullptr);
  String v = String(gas, 1);
  String msg = "{\"id\":\"perimetro\",\"ts\":" + String(ts) + ",\"valor\":" + v +
               ",\"unidad\":\"ppm\",\"viento\":" + String(viento) +
               ",\"calidad\":\"ok\",\"firma\":\"" + firmar("perimetro|" + String(ts) + "|" + v) + "\"}";
  mqtt.publish(T_DATOS, msg.c_str());
  Serial.println(msg);
}
