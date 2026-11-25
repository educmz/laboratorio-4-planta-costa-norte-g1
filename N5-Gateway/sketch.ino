// Planta Costa Norte - Nodo 5: Gateway (fog)
// Instalacion heredada del proveedor anterior, corregida por el grupo G1 (fallas F17 a F20)
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

// ---------------- Ordenes firmadas hacia los nodos ----------------
// Formato acordado con N4 (N4-Lavador/docs/INTEGRACION-N5.md):
//   gateway|ts|cmd:ON|firma      gateway|ts|conc:2450.5|firma
// firma = HMAC-SHA256("gateway|ts|contenido"), primeros 16 hex.
// Cada receptor rechaza ts repetidos, por eso cada topico lleva su propio
// contador estrictamente creciente.
const char* T_CMD_LAVADOR = "upc/ga/lavador/cmd";
const char* T_CMD_POZAS   = "upc/ga/pozas/cmd";
const char* T_CONC        = "upc/ga/gateway/conc";
const char* T_ALERTA_GW   = "upc/ga/gateway/alerta";
long tsEnviadoLavador = 0, tsEnviadoPozas = 0, tsEnviadoConc = 0;

bool publicarFirmado(const char* topico, const String& contenido, long& ultimoTs) {
  long ts = time(nullptr);
  if (ts < TS_MINIMO) {
    Serial.println("NO ENVIADO (gateway sin hora NTP): " + contenido);
    return false;
  }
  if (ts <= ultimoTs) ts = ultimoTs + 1;
  ultimoTs = ts;
  String base = "gateway|" + String(ts) + "|" + contenido;
  return mqtt.publish(topico, (base + "|" + firmar(base)).c_str());   // sin retain
}

// ---------------- Lazo cerrado del lavador (R5) ----------------
const unsigned long T_CONFIRMACION = 10000;   // ms
unsigned long tOrdenOn = 0;
bool esperandoConfirmacion = false, lavadorConfirmado = false, ventMaxActiva = false;
unsigned long tReafirmar = 0;

void respaldoFallaLavador(const String& origen) {
  if (ventMaxActiva) return;
  alerta = true;
  ventMaxActiva = true;
  esperandoConfirmacion = false;
  Serial.println("ALERTA: falla del lavador (" + origen + "). Ventilacion de pozas al maximo");
  mqtt.publish(T_ALERTA_GW, "{\"alerta\":\"falla del lavador\"}");
  publicarFirmado(T_CMD_POZAS, "cmd:VENT:MAX", tsEnviadoPozas);
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
    respaldoFallaLavador("alerta de N4");
    return;
  }
  if (topico.endsWith("/estado")) {
    Serial.println("Estado lavador: " + msg);
    bool confirma = msg.indexOf("\"CONFIRMADO\"") >= 0 || msg.indexOf("\"confirmado\":true") >= 0;
    if (confirma && esperandoConfirmacion) {
      esperandoConfirmacion = false;
      lavadorConfirmado = true;
      Serial.println("[LAZO] Lavador confirmado en " + String(millis() - tOrdenOn) + " ms desde la deteccion");
    }
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
    // N4 necesita la concentracion firmada para permitir el apagado (R7) y
    // como senal de vida del gateway (R6).
    if (vigentes > 0) publicarFirmado(T_CONC, "conc:" + String(conc, 1), tsEnviadoConc);

    // Regla combinada (fog)
    if (!lavadorOn && confirmaciones >= 2 && haciaComunidad) {
      publicarFirmado(T_CMD_LAVADOR, "cmd:ON", tsEnviadoLavador);
      lavadorOn = true;
      tOrdenOn = millis();
      tReafirmar = millis();
      esperandoConfirmacion = true;
      lavadorConfirmado = false;
      Serial.println("[LAZO] Deteccion: " + String(confirmaciones) + " nodos sobre umbral y viento " +
                     String(viento) + " grados. Orden: lavador ON");
    } else if (lavadorOn && vigentes > 0 && conc < UMBRAL_GAS * 0.8) {   // sin datos vigentes no se apaga (R7)
      publicarFirmado(T_CMD_LAVADOR, "cmd:OFF", tsEnviadoLavador);
      lavadorOn = false;
      esperandoConfirmacion = false;
      lavadorConfirmado = false;
      Serial.println("Orden: lavador OFF");
      if (ventMaxActiva) {
        ventMaxActiva = false;
        publicarFirmado(T_CMD_POZAS, "cmd:VENT:NORMAL", tsEnviadoPozas);
      }
    } else if (lavadorOn && millis() - tReafirmar >= 10000) {
      // Reafirma la orden: si N4 perdio el mensaje o se reinicio, vuelve a encender
      tReafirmar = millis();
      publicarFirmado(T_CMD_LAVADOR, "cmd:ON", tsEnviadoLavador);
    }

    // Lazo cerrado: sin confirmacion en 10 s, alerta y ventilacion maxima (R5)
    if (esperandoConfirmacion && millis() - tOrdenOn > T_CONFIRMACION) {
      respaldoFallaLavador("sin confirmacion en 10 s");
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
