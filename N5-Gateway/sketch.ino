// Planta Costa Norte - Nodo 5: Gateway (fog)
// Instalacion heredada del proveedor anterior
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>
#include "mbedtls/md.h"

#include "secrets.h"                     // CLAVE (HMAC del grupo) y TS_KEY (ThingSpeak), fuera del repositorio

const int PIN_PIXEL = 5, PIN_ACK = 13;
const float UMBRAL_GAS = 2000;           // ppm

Adafruit_NeoPixel pixel(1, PIN_PIXEL, NEO_GRB + NEO_KHZ800);
WiFiClient net;
PubSubClient mqtt(net);

float gasPozas = 0, gasSecadores = 0, gasPerimetro = 0;
// Momento (millis) del ultimo dato valido de cada nodo; 0 = nunca recibido
unsigned long tPozas = 0, tSecadores = 0, tPerimetro = 0;
const unsigned long VIGENCIA_DATO = 10000;   // ms: 5 publicaciones perdidas (cada 2 s)

bool vigente(unsigned long t) { return t != 0 && millis() - t <= VIGENCIA_DATO; }
int viento = 0;
bool lavadorOn = false, alerta = false;

// Seguridad de datos (R9): hora real y proteccion contra repeticion
const long TS_MINIMO = 1700000000;   // por debajo, el reloj no esta sincronizado con NTP
const long MAX_ANTIGUEDAD = 60;      // s
const long MAX_ADELANTO = 15;        // s de tolerancia: en Wokwi el reloj simulado se atrasa respecto al real
long ultimoTsPozas = 0, ultimoTsSecadores = 0, ultimoTsPerimetro = 0;

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
    // La alerta del lavador no viene firmada: solo se acepta si coincide con
    // el estado del propio gateway (lavador encendido y esperando confirmacion).
    if (!lavadorOn || msg.indexOf("falla del lavador") < 0) {
      Serial.println("ALERTA IGNORADA (no coincide con el estado del gateway): " + msg);
      return;
    }
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
  if (ahora < TS_MINIMO) {
    Serial.println("RECHAZADO (gateway sin hora NTP): " + id);
    return;
  }
  if (ahora - ts > MAX_ANTIGUEDAD) {
    Serial.println("RECHAZADO (mensaje antiguo): " + id);
    return;
  }
  if (ts - ahora > MAX_ADELANTO) {
    Serial.println("RECHAZADO (marca de tiempo en el futuro): " + id);
    return;
  }
  // El topico debe corresponder al id firmado
  if (!topico.endsWith("/" + id + "/datos")) {
    Serial.println("RECHAZADO (id no corresponde al topico): " + id);
    return;
  }
  // Anti-replay: cada nodo publica con marcas de tiempo crecientes
  long* ultimoTs = nullptr;
  if (id == "pozas") ultimoTs = &ultimoTsPozas;
  else if (id == "secadores") ultimoTs = &ultimoTsSecadores;
  else if (id == "perimetro") ultimoTs = &ultimoTsPerimetro;
  if (ultimoTs == nullptr) {
    Serial.println("RECHAZADO (nodo desconocido): " + id);
    return;
  }
  if (ts <= *ultimoTs) {
    Serial.println("RECHAZADO (mensaje repetido): " + id);
    return;
  }
  *ultimoTs = ts;

  float v = valor.toFloat();
  if (id == "pozas") { gasPozas = v; tPozas = millis(); }
  else if (id == "secadores") { gasSecadores = v; tSecadores = millis(); }
  else if (id == "perimetro") { gasPerimetro = v; viento = campo(msg, "viento").toInt(); tPerimetro = millis(); }
  Serial.println("OK " + id + ": " + String(v, 1) + " ppm");
}

void conectar() {
  while (WiFi.status() != WL_CONNECTED) delay(200);
  while (!mqtt.connected()) {
    String id = "ga-gateway-" + WiFi.macAddress();
    if (mqtt.connect(id.c_str())) {
      mqtt.subscribe("upc/ga/+/datos");   // contrato: upc/<p>/<nodo>/datos
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
  // Hora real para validar la antiguedad de los mensajes (R9)
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  for (int i = 0; i < 50 && time(nullptr) < TS_MINIMO; i++) delay(200);
  Serial.println(time(nullptr) >= TS_MINIMO ? "NTP sincronizado" : "NTP sin sincronizar: se rechazaran los datos");
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

  // Resincroniza NTP cada 15 s: en Wokwi la simulacion corre mas lenta que el
  // tiempo real y el reloj se va atrasando; sin esto, a los pocos minutos los
  // datos de los nodos parecen venir "del futuro" y se rechazan.
  static unsigned long tNtp = 0;
  if (millis() - tNtp >= 15000) {
    tNtp = millis();
    configTime(0, 0, "pool.ntp.org", "time.google.com");
  }

  if (digitalRead(PIN_ACK) == LOW && alerta) {
    alerta = false;
    Serial.println("Alerta reconocida por el operador");
  }

  if (millis() - tDecision >= 2000) {
    tDecision = millis();
    // Solo cuentan los datos vigentes: un nodo desconectado no puede dejar
    // congelado su ultimo valor en la decision (falla silenciosa).
    bool okPozas = vigente(tPozas), okSecadores = vigente(tSecadores), okPerimetro = vigente(tPerimetro);
    int vigentes = okPozas + okSecadores + okPerimetro;
    static int vigentesAntes = -1;
    if (vigentes != vigentesAntes) {
      vigentesAntes = vigentes;
      Serial.println(String("Nodos con datos vigentes: ") + vigentes +
                     " (pozas " + (okPozas ? "si" : "NO") + ", secadores " + (okSecadores ? "si" : "NO") +
                     ", perimetro " + (okPerimetro ? "si" : "NO") + ")");
    }
    bool haciaComunidad = okPerimetro && (viento >= 90 && viento <= 180);
    int confirmaciones = (okPozas && gasPozas > UMBRAL_GAS) + (okSecadores && gasSecadores > UMBRAL_GAS) +
                         (okPerimetro && gasPerimetro > UMBRAL_GAS);
    float conc = max(okPozas ? gasPozas : 0, max(okSecadores ? gasSecadores : 0, okPerimetro ? gasPerimetro : 0));
    if (vigentes > 0) mqtt.publish("upc/ga/gateway/conc", String(conc, 1).c_str());

    // Regla combinada (fog)
    if (!lavadorOn && confirmaciones >= 2 && haciaComunidad) {
      mqtt.publish("upc/ga/lavador/cmd", "ON", true);
      lavadorOn = true;
      Serial.println("Orden: lavador ON");
    } else if (lavadorOn && vigentes > 0 && conc < UMBRAL_GAS * 0.8) {   // sin datos vigentes no se apaga (R7)
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
