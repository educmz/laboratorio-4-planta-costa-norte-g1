// Planta Costa Norte - Nodo 2: Secadores
// Instalacion heredada del proveedor anterior
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <time.h>
#include <math.h>

const char* T_DATOS = "upc/ga/secadores/datos";

const int PIN_GAS = 34, PIN_DHT = 15, PIN_R = 25, PIN_G = 26, PIN_B = 27;
const float UMBRAL_AMARILLO = 1500, UMBRAL_ROJO = 2000;   // ppm
const unsigned long PERIODO = 100;                         // ms entre publicaciones

WiFiClient net;
PubSubClient mqtt(net);
DHTesp dht;

// F6: calibracion experimental ADC/ppm en Wokwi.
const int NUM_PUNTOS = 7;
const float PPM_REF[NUM_PUNTOS] = {100, 400, 1000, 1500, 2000, 2500, 5000};
const int ADC_REF[NUM_PUNTOS] = {3337, 3635, 3762, 3811, 3839, 3861, 3916};

float convertirGas(int adc) {
  if (adc < ADC_REF[0] || adc > ADC_REF[NUM_PUNTOS - 1]) return NAN;
  for (int i = 0; i < NUM_PUNTOS - 1; i++) {
    if (adc >= ADC_REF[i] && adc <= ADC_REF[i + 1]) {
      float p = float(adc - ADC_REF[i]) / float(ADC_REF[i + 1] - ADC_REF[i]);
      return PPM_REF[i] + p * (PPM_REF[i + 1] - PPM_REF[i]);
    }
  }
  return NAN;
}

void baliza(int r, int g, int b) {
  digitalWrite(PIN_R, r);
  digitalWrite(PIN_G, g);
  digitalWrite(PIN_B, b);
}

void conectar() {
  while (WiFi.status() != WL_CONNECTED) delay(200);
  while (!mqtt.connected()) {
    if (mqtt.connect("ga-sensor")) {
      Serial.println("MQTT conectado");
    } else {
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  analogReadResolution(12);
  dht.setup(PIN_DHT, DHTesp::DHT22);
  WiFi.begin("Wokwi-GUEST", "");
  while (WiFi.status() != WL_CONNECTED) delay(200);
  configTime(0, 0, "pool.ntp.org");
  for (int i = 0; i < 50 && time(nullptr) < 1700000000; i++) delay(200);
  mqtt.setServer("broker.hivemq.com", 1883);
  mqtt.setKeepAlive(60);
  conectar();
}

unsigned long tPub = 0;

void loop() {
  if (!mqtt.connected()) conectar();
  mqtt.loop();
  if (millis() - tPub < PERIODO) return;
  tPub = millis();

  int adcGas = analogRead(PIN_GAS);
  float gas = convertirGas(adcGas);
  Serial.println("ADC MQ-2: " + String(adcGas));
  if (isnan(gas)) {
    if (adcGas > ADC_REF[NUM_PUNTOS - 1]) baliza(1, 0, 0);
    else baliza(0, 0, 1);
    Serial.println("Gas fuera del rango calibrado");
    return;
  }
  Serial.println("Gas calibrado: " + String(gas, 1) + " ppm");
  TempAndHumidity d = dht.getTempAndHumidity();

  // Baliza local (edge)
  if (gas > UMBRAL_ROJO) baliza(1, 0, 0);
  else if (gas > UMBRAL_AMARILLO) baliza(1, 1, 0);
  else baliza(0, 1, 0);

  long ts = time(nullptr);
  String msg = "{\"id\":\"secadores\",\"ts\":" + String(ts) +
               ",\"valor\":" + String(gas, 1) +
               ",\"unidad\":\"ppm\",\"temp\":" + String(d.temperature, 1) +
               ",\"hum\":" + String(d.humidity, 1) +
               ",\"equipo\":\"Secador rotatorio SR-01\"" +
               ",\"zona\":\"Linea de secado de harina - Planta Chimbote\"" +
               ",\"responsable\":\"Jefe de turno de produccion\"" +
               ",\"calidad\":\"ok\",\"firma\":\"\"}";
  mqtt.publish(T_DATOS, msg.c_str());
  Serial.println(msg);
}
