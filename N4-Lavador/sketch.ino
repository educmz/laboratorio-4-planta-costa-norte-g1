// Planta Costa Norte - Nodo 4: Lavador de gases
// Instalacion heredada del proveedor anterior
#include <WiFi.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const char* BROKER    = "broker.hivemq.com";

const char* T_CMD    = "upc/ga/lavador/cmd";
const char* T_CONC   = "upc/ga/gateway/conc";
const char* T_ESTADO = "upc/ga/lavador/estado";
const char* T_ALERTA = "upc/ga/lavador/alerta";

const int PIN_RELE   = 26;
const int PIN_CONF   = 14;
const int PIN_STEP   = 18;
const int PIN_DIR    = 19;

const unsigned long T_CONFIRMACION = 10000;
const unsigned long T_MODO_SEGURO  = 60000;
const float CONC_ALTA = 2000.0;   // ppm

WiFiClient net;
PubSubClient mqtt(net);

bool lavadorOn = false;
int intensidad = 1;
float concentracion = 0;
unsigned long tOrden = 0;
unsigned long ultimoContacto = 0;
unsigned long tPaso = 0;
bool esperandoConf = false;
bool modoSeguro = false;

void publicar(const char* topico, String msg) {
  mqtt.publish(topico, msg.c_str());
  Serial.println(String(topico) + " -> " + msg);
}

void encender(const char* motivo) {
  lavadorOn = true;
  digitalWrite(PIN_RELE, HIGH);
  tOrden = millis();
  esperandoConf = true;
  publicar(T_ESTADO, String("{\"lavador\":\"ON\",\"motivo\":\"") + motivo + "\"}");
}

void apagar(const char* motivo) {
  lavadorOn = false;
  digitalWrite(PIN_RELE, LOW);
  esperandoConf = false;
  publicar(T_ESTADO, String("{\"lavador\":\"OFF\",\"motivo\":\"") + motivo + "\"}");
}

void alRecibir(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  String t = String(topic);

  if (t == T_CONC) {
    concentracion = msg.toFloat();
  } else if (t == T_CMD) {
    if (msg == "ON") {
      encender("orden gateway");
    } else if (msg == "OFF") {
      apagar("orden gateway");
    } else if (msg.startsWith("INT:")) {
      intensidad = constrain(msg.substring(4).toInt(), 1, 5);
      publicar(T_ESTADO, "{\"intensidad\":" + String(intensidad) + "}");
    }
  }
}

void conectar() {
  while (WiFi.status() != WL_CONNECTED) { delay(200); }
  while (!mqtt.connected()) {
    String id = "ga-lavador-" + String(random(0xffff), HEX);
    if (mqtt.connect(id.c_str())) {
      mqtt.subscribe(T_CMD);
      mqtt.subscribe(T_CONC);
      publicar(T_ESTADO, "{\"nodo\":\"lavador\",\"estado\":\"online\"}");
    } else {
      delay(1000);
    }
  }
}

void girarMotor() {
  if (!lavadorOn) return;
  unsigned long periodo = 4000 / intensidad;   // microsegundos entre pasos
  if (micros() - tPaso >= periodo) {
    tPaso = micros();
    digitalWrite(PIN_STEP, !digitalRead(PIN_STEP));
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RELE, OUTPUT);
  pinMode(PIN_CONF, INPUT);
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  digitalWrite(PIN_RELE, LOW);
  digitalWrite(PIN_DIR, HIGH);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  mqtt.setServer(BROKER, 1883);
  mqtt.setCallback(alRecibir);
  mqtt.setKeepAlive(60);
  conectar();
  ultimoContacto = millis();
}

void loop() {
  if (!mqtt.connected()) conectar();
  mqtt.loop();
  if (mqtt.connected()) ultimoContacto = millis();

  // Lazo cerrado: confirmacion de funcionamiento
  if (esperandoConf) {
    if (digitalRead(PIN_CONF) == HIGH) {
      esperandoConf = false;
      publicar(T_ESTADO, "{\"lavador\":\"CONFIRMADO\"}");
    } else if (millis() - tOrden > T_CONFIRMACION) {
      esperandoConf = false;
      publicar(T_ALERTA, "{\"alerta\":\"falla del lavador\"}");
    }
  }

  // Modo seguro: sin contacto con el gateway
  if (!modoSeguro && millis() - ultimoContacto > T_MODO_SEGURO) {
    modoSeguro = true;
    encender("modo seguro");
  }

  girarMotor();
}
