// Planta Costa Norte - Nodo 5: Gateway (fog)
// Instalacion heredada del proveedor anterior
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>
#include "mbedtls/md.h"

const char* CLAVE  = "REEMPLAZAR_CLAVE_GRUPO";
const char* TS_KEY = "TU_WRITE_KEY";     // clave de escritura de ThingSpeak

const int PIN_PIXEL = 5, PIN_ACK = 13;
const float UMBRAL_GAS = 2000;           // ppm

Adafruit_NeoPixel pixel(1, PIN_PIXEL, NEO_GRB + NEO_KHZ800);
WiFiClient net;
PubSubClient mqtt(net);

float gasPozas = 0, gasSecadores = 0, gasPerimetro = 0;
int viento = 0;
bool lavadorOn = false, alerta = false;

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

// Extrae el texto de un campo del JSON
String campo(const String& m, const char* k) {
  String key = "\"" + String(k) + "\":";
  int i = m.indexOf(key);
  if (i < 0) return "";
  i += key.length();
  int j = i;
  while (j < (int)m.length() && m[j] != ',' && m[j] != '}') j++;
  return m.substring(i, j);
}

String sinComillas(String s) { s.replace("\"", ""); return s; }

void alRecibir(char* t, byte* p, unsigned int n) {
  String msg;
  for (unsigned int i = 0; i < n; i++) msg += (char)p[i];
  String topico(t);

  if (topico.endsWith("/alerta")) {
    alerta = true;
    Serial.println("ALERTA: " + msg);
    mqtt.publish("upc/ga/pozas/cmd", "VENT:MAX");   // respuesta de respaldo
    return;
  }
  if (topico.endsWith("/estado")) {
    Serial.println("Estado lavador: " + msg);
    return;
  }

  String id    = sinComillas(campo(msg, "id"));
  long ts      = campo(msg, "ts").toInt();
  String valor = campo(msg, "valor");
  String firma = sinComillas(campo(msg, "firma"));

  if (firma != firmar(id + "|" + String(ts) + "|" + valor)) {
    Serial.println("RECHAZADO (firma invalida): " + id);
    return;
  }
  long ahora = time(nullptr);
  if (ahora - ts > 60) {
    Serial.println("RECHAZADO (mensaje antiguo): " + id);
    return;
  }

  float v = valor.toFloat();
  if (id == "pozas") gasPozas = v;
  else if (id == "secadores") gasSecadores = v;
  else if (id == "perimetro") { gasPerimetro = v; viento = campo(msg, "viento").toInt(); }
  Serial.println("OK " + id + ": " + String(v, 1) + " ppm");
}

void conectar() {
  while (WiFi.status() != WL_CONNECTED) delay(200);
  while (!mqtt.connected()) {
    String id = "ga-gateway-" + WiFi.macAddress();
    if (mqtt.connect(id.c_str())) {
      mqtt.subscribe("upc/ga/+/dato");
      mqtt.subscribe("upc/ga/lavador/estado");
      mqtt.subscribe("upc/ga/lavador/alerta");
      Serial.println("Gateway conectado");
    } else {
      delay(1000);
    }
  }
}

void enviarThingSpeak() {
  if (String(TS_KEY) == "TU_WRITE_KEY") return;
  HTTPClient http;
  http.begin("http://api.thingspeak.com/update?api_key=" + String(TS_KEY) +
             "&field1=" + String(gasPozas) + "&field2=" + String(gasSecadores) +
             "&field3=" + String(gasPerimetro) + "&field4=" + String(viento) +
             "&field5=" + String(lavadorOn ? 1 : 0));
  http.GET();
  http.end();
}

void setup() {
  Serial.begin(115200);
  pixel.begin();
  pinMode(PIN_ACK, INPUT_PULLUP);
  WiFi.begin("Wokwi-GUEST", "");
  while (WiFi.status() != WL_CONNECTED) delay(200);
  mqtt.setServer("broker.hivemq.com", 1883);
  mqtt.setBufferSize(512);
  mqtt.setKeepAlive(60);
  mqtt.setCallback(alRecibir);
  conectar();
}

unsigned long tDecision = 0, tNube = 0;

void loop() {
  if (!mqtt.connected()) conectar();
  mqtt.loop();

  if (digitalRead(PIN_ACK) == LOW && alerta) {
    alerta = false;
    Serial.println("Alerta reconocida por el operador");
  }

  if (millis() - tDecision >= 2000) {
    tDecision = millis();
    bool haciaComunidad = (viento >= 90 && viento <= 180);
    int confirmaciones = (gasPozas > UMBRAL_GAS) + (gasSecadores > UMBRAL_GAS) + (gasPerimetro > UMBRAL_GAS);
    float conc = max(gasPozas, max(gasSecadores, gasPerimetro));
    mqtt.publish("upc/ga/gateway/conc", String(conc, 1).c_str());

    // Regla combinada (fog)
    if (!lavadorOn && confirmaciones >= 2 && haciaComunidad) {
      mqtt.publish("upc/ga/lavador/cmd", "ON", true);
      lavadorOn = true;
      Serial.println("Orden: lavador ON");
    } else if (lavadorOn && conc < UMBRAL_GAS * 0.8) {
      mqtt.publish("upc/ga/lavador/cmd", "OFF", true);
      lavadorOn = false;
      Serial.println("Orden: lavador OFF");
    }

    // Semaforo general de la planta
    if (alerta || lavadorOn) pixel.setPixelColor(0, pixel.Color(255, 0, 0));
    else if (confirmaciones >= 1) pixel.setPixelColor(0, pixel.Color(255, 150, 0));
    else pixel.setPixelColor(0, pixel.Color(0, 255, 0));
    pixel.show();
  }

  if (millis() - tNube >= 20000) {
    tNube = millis();
    enviarThingSpeak();
  }
}
