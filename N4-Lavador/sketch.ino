// Planta Costa Norte - Nodo 4: Lavador de gases
// Version corregida - Integrante 4 - Grupo G1

#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <math.h>
#include <mbedtls/md.h>

// ================= CONFIGURACION =================

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const char* BROKER = "broker.hivemq.com";

// Reemplazar localmente por la clave acordada con N5.
// No publicar la clave real en GitHub.
const char* CLAVE = "costa-norte-ga-4817";

// F16: integracion con N5-Gateway
const char* T_CMD    = "upc/ga/lavador/cmd";
const char* T_CONC   = "upc/ga/gateway/conc";
const char* T_ESTADO = "upc/ga/lavador/estado";
const char* T_ALERTA = "upc/ga/lavador/alerta";

// ================= HARDWARE =================

// F13: corregido GPIO 27 por GPIO 26.
const int PIN_RELE = 26;
const int PIN_CONF = 14;
const int PIN_STEP = 18;
const int PIN_DIR  = 19;

// Modulo Wokwi original con transistor NPN.
// LOW conecta COM con NO.
const int RELE_ON  = HIGH;
const int RELE_OFF = LOW;

// ================= REGLAS =================

const unsigned long T_CONFIRMACION = 10000;
const unsigned long T_MODO_SEGURO  = 60000;
const unsigned long T_RECONEXION   = 5000;
const unsigned long T_PUBLICACION  = 2000;

const float CONC_APAGADO = 1600.0;

// ================= ESTADO =================

WiFiClient net;
PubSubClient mqtt(net);

bool lavadorOn = false;
bool esperandoConf = false;
bool confirmado = false;
bool modoSeguro = false;
bool fallaLavador = false;

bool concentracionValida = false;
float concentracion = 0;

int intensidad = 1;

unsigned long tOrden = 0;
unsigned long ultimoContacto = 0;
unsigned long ultimaConcentracion = 0;
unsigned long ultimoIntento = 0;
unsigned long ultimaPublicacion = 0;
unsigned long tPaso = 0;

long ultimaOrdenTs = 0;
long ultimaConcTs = 0;

String ultimoComando = "";

// ================= COMUNICACION =================

void publicar(const char* topico, const String& msg) {
  bool enviado = false;

  if (mqtt.connected()) {
    enviado = mqtt.publish(topico, msg.c_str());
  }

  Serial.print(enviado ? "[MQTT] " : "[LOCAL] ");
  Serial.println(String(topico) + " -> " + msg);
}

void alerta(const String& mensaje) {
  publicar(
    T_ALERTA,
    String("{\"alerta\":\"") + mensaje + "\"}"
  );
}

// ================= SEGURIDAD HMAC =================

String firmar(const String& mensaje) {
  unsigned char resultado[32];

  const mbedtls_md_info_t* info =
    mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

  if (info == nullptr) return "";

  int resultadoHmac = mbedtls_md_hmac(
                        info,
                        (const unsigned char*) CLAVE,
                        strlen(CLAVE),
                        (const unsigned char*) mensaje.c_str(),
                        mensaje.length(),
                        resultado
                      );

  if (resultadoHmac != 0) return "";

  char hex[17];

  for (int i = 0; i < 8; i++) {
    sprintf(hex + i * 2, "%02x", resultado[i]);
  }

  hex[16] = '\0';

  return String(hex);
}

bool firmaIgual(const String& a, const String& b) {
  if (a.length() != 16 || b.length() != 16) {
    return false;
  }

  unsigned char diferencia = 0;

  for (int i = 0; i < 16; i++) {
    diferencia |= a[i] ^ b[i];
  }

  return diferencia == 0;
}

bool esNumero(const String& texto) {
  if (texto.length() == 0) return false;

  char* fin = nullptr;
  double valor = strtod(texto.c_str(), &fin);

  return fin != texto.c_str()
         && *fin == '\0'
         && isfinite(valor);
}

// Formato propuesto:
// gateway|timestamp|cmd:ON|firma
// gateway|timestamp|conc:2450.5|firma
//
// Firma = HMAC-SHA256 de los primeros
// tres campos, truncada a 16 caracteres hex.

bool autenticar(
  const String& mensaje,
  const String& prefijo,
  String& contenido,
  long& timestamp
) {
  if (strcmp(CLAVE, "REEMPLAZAR_CLAVE_GRUPO") == 0) {
    Serial.println("[SEGURIDAD] Falta configurar clave");
    return false;
  }

  int p1 = mensaje.indexOf('|');
  int p2 = mensaje.indexOf('|', p1 + 1);
  int p3 = mensaje.indexOf('|', p2 + 1);

  if (p1 < 0 || p2 < 0 || p3 < 0) {
    Serial.println("[SEGURIDAD] Mensaje sin firma");
    return false;
  }

  if (mensaje.indexOf('|', p3 + 1) >= 0) {
    return false;
  }

  String origen = mensaje.substring(0, p1);
  String tsTexto = mensaje.substring(p1 + 1, p2);

  contenido = mensaje.substring(p2 + 1, p3);

  String firmaRecibida = mensaje.substring(p3 + 1);
  String mensajeBase = mensaje.substring(0, p3);

  if (origen != "gateway") return false;
  if (!contenido.startsWith(prefijo)) return false;

  if (tsTexto.length() == 0) return false;

  for (unsigned int i = 0; i < tsTexto.length(); i++) {
    if (tsTexto[i] < '0' || tsTexto[i] > '9') {
      return false;
    }
  }

  timestamp = tsTexto.toInt();

  long ahora = (long) time(nullptr);

  if (ahora < 1700000000L) {
    Serial.println("[SEGURIDAD] NTP no sincronizado");
    return false;
  }

  long diferencia = ahora - timestamp;

  Serial.printf(
    "[NTP] ESP32: %ld | Mensaje: %ld | Diferencia: %ld s\n",
    ahora, timestamp, diferencia
  );

  // F16: tolerancia temporal compatible con N5.
  if (timestamp < 1700000000L ||
      diferencia < -15 ||
      diferencia > 60) {

    Serial.printf(
      "[SEGURIDAD] Timestamp invalido. Desfase: %ld s\n",
      diferencia
    );

    return false;
  }

  if (!firmaIgual(firmar(mensajeBase), firmaRecibida)) {
    Serial.println("[SEGURIDAD] Firma HMAC invalida");
    return false;
  }

  contenido = contenido.substring(prefijo.length());

  return true;
}

// ================= ACTUADORES =================

void encender(const char* motivo) {
  // No reiniciar el temporizador con ordenes repetidas.
  if (lavadorOn) return;

  lavadorOn = true;
  confirmado = false;
  fallaLavador = false;
  esperandoConf = true;

  tOrden = millis();

  digitalWrite(PIN_RELE, RELE_ON);

  Serial.println(
    String("[LAVADOR] ON: ") + motivo
  );

  publicar(
    T_ESTADO,
    String("{\"lavador\":\"ON\",\"motivo\":\"")
    + motivo + "\"}"
  );
}

bool apagadoPermitido() {
  if (!concentracionValida) return false;

  if (millis() - ultimaConcentracion >= T_MODO_SEGURO) {
    return false;
  }

  if (modoSeguro) return false;

  return concentracion < CONC_APAGADO;
}

void apagar(const char* motivo) {
  // R4 y R7: impedir apagado inseguro.
  if (!apagadoPermitido()) {
    Serial.println("[SEGURIDAD] Apagado rechazado");
    return;
  }

  if (!lavadorOn) return;

  lavadorOn = false;
  confirmado = false;
  esperandoConf = false;
  fallaLavador = false;

  digitalWrite(PIN_RELE, RELE_OFF);
  digitalWrite(PIN_STEP, LOW);

  publicar(
    T_ESTADO,
    String("{\"lavador\":\"OFF\",\"motivo\":\"")
    + motivo + "\"}"
  );
}

void registrarContacto() {
  ultimoContacto = millis();

  if (modoSeguro) {
    modoSeguro = false;
    Serial.println("[SEGURIDAD] Gateway recuperado");
  }
}

// ================= RECEPCION MQTT =================

void alRecibir(
  char* topic,
  byte* payload,
  unsigned int len
) {
  String msg;

  for (unsigned int i = 0; i < len; i++) {
    msg += (char) payload[i];
  }

  String topico(topic);

  Serial.println("[RX MQTT] " + topico);

  String contenido;
  long ts = 0;

  if (topico == T_CONC) {

    if (!autenticar(msg, "conc:", contenido, ts)) {
      return;
    }

    // Rechazar concentraciones antiguas o repetidas.
    if (ts <= ultimaConcTs) return;

    if (!esNumero(contenido)) return;

    float valor = contenido.toFloat();

    if (!isfinite(valor) || valor < 0) return;

    ultimaConcTs = ts;
    concentracion = valor;
    concentracionValida = true;
    ultimaConcentracion = millis();

    registrarContacto();

    Serial.println(
      "[CONCENTRACION] " + String(concentracion, 1)
      + " ppm"
    );

    return;
  }

  if (topico != T_CMD) return;

  // F15: nunca ejecutar ordenes sin autenticacion.
  if (!autenticar(msg, "cmd:", contenido, ts)) {
    Serial.println("[SEGURIDAD] Orden rechazada");
    return;
  }

  // Validar comando antes de actualizar el contacto.
  bool comandoValido =
    contenido == "ON" ||
    contenido == "OFF" ||
    contenido == "INT:1" ||
    contenido == "INT:2" ||
    contenido == "INT:3" ||
    contenido == "INT:4" ||
    contenido == "INT:5";

  if (!comandoValido) return;

  // Proteccion contra repeticion durante la ejecucion.
  if (ts <= ultimaOrdenTs) {
    Serial.println("[SEGURIDAD] Orden repetida");
    return;
  }

  ultimaOrdenTs = ts;
  ultimoComando = contenido;

  registrarContacto();

  if (contenido == "ON") {

    encender("orden gateway");

  } else if (contenido == "OFF") {

    apagar("orden gateway");

  } else if (contenido.startsWith("INT:")) {

    intensidad = contenido.substring(4).toInt();

    publicar(
      T_ESTADO,
      "{\"intensidad\":" + String(intensidad) + "}"
    );
  }
}

// ================= CONEXION =================
void mantenerConexion() {
  static bool ntpListo = false;
  static bool ntpIniciado = false;
  static unsigned long ultimoNTP = 0;
  static unsigned long ultimoWiFi = 0;

  if (WiFi.status() != WL_CONNECTED) {

    ntpIniciado = false;
    ntpListo = false;

    if (millis() - ultimoWiFi >= 10000) {
      ultimoWiFi = millis();

      Serial.println("[WIFI] Intentando reconectar");
      WiFi.reconnect();
    }

    return;
  }

  // F16: sincronizacion periodica de NTP.
  if (!ntpIniciado ||
      millis() - ultimoNTP >= 15000) {

    ntpIniciado = true;
    ultimoNTP = millis();

    configTime(
      0, 0,
      "pool.ntp.org",
      "time.google.com",
      "time.nist.gov"
    );

    Serial.printf(
      "[NTP] Resincronizando. Reloj: %ld\n",
      (long) time(nullptr)
    );
  }

  if (time(nullptr) >= 1700000000L && !ntpListo) {
    ntpListo = true;

    Serial.printf(
      "[NTP] Reloj disponible: %ld\n",
      (long) time(nullptr)
    );
  }

  if (mqtt.connected()) return;

  if (millis() - ultimoIntento < T_RECONEXION) {
    return;
  }

  ultimoIntento = millis();

  net.stop();

  String id = "ga-lavador-" + WiFi.macAddress();

  Serial.println("[MQTT] Intentando conectar");

  if (mqtt.connect(id.c_str())) {

    bool subCmd = mqtt.subscribe(T_CMD);
    bool subConc = mqtt.subscribe(T_CONC);

    Serial.printf(
      "[MQTT] Suscripcion CMD: %s\n",
      subCmd ? "OK" : "ERROR"
    );

    Serial.printf(
      "[MQTT] Suscripcion CONC: %s\n",
      subConc ? "OK" : "ERROR"
    );

    if (!subCmd || !subConc) {
      mqtt.disconnect();
      return;
    }

    publicar(
      T_ESTADO,
      "{\"nodo\":\"lavador\",\"estado\":\"online\"}"
    );

    Serial.println("[MQTT] Conectado");

  } else {

    Serial.printf(
      "[MQTT] Error: %d\n",
      mqtt.state()
    );
  }
}
// ================= LAZO CERRADO =================

void verificarConfirmacion() {
  if (!lavadorOn) return;

  bool sensorActivo = digitalRead(PIN_CONF) == HIGH;

  if (esperandoConf) {

    if (sensorActivo &&
        millis() - tOrden <= T_CONFIRMACION) {

      esperandoConf = false;
      confirmado = true;
      fallaLavador = false;

      publicar(
        T_ESTADO,
        "{\"lavador\":\"CONFIRMADO\"}"
      );

      Serial.println(
        "[CONFIRMACION] Tiempo: "
        + String(millis() - tOrden) + " ms"
      );

    } else if (millis() - tOrden >= T_CONFIRMACION) {

      esperandoConf = false;
      confirmado = false;
      fallaLavador = true;

      alerta("falla del lavador");
    }

  } else if (confirmado && !sensorActivo) {

    confirmado = false;
    esperandoConf = true;
    tOrden = millis();

    Serial.println("[LAVADOR] Confirmacion perdida");
  }
}

// ================= MODO SEGURO =================

void verificarModoSeguro() {
  // F14: el temporizador depende de mensajes
  // autenticados, no de la conexion al broker.

  if (!modoSeguro &&
      millis() - ultimoContacto >= T_MODO_SEGURO) {

    modoSeguro = true;
    concentracionValida = false;

    intensidad = 5;

    Serial.println("[SEGURIDAD] MODO SEGURO ACTIVADO");

    encender("modo seguro");

    alerta("sin contacto con gateway");
  }
}

// ================= MOTOR =================

void girarMotor() {
  if (!lavadorOn) return;

  unsigned long periodo = 4000UL / intensidad;

  if (micros() - tPaso >= periodo) {
    tPaso = micros();

    digitalWrite(
      PIN_STEP,
      !digitalRead(PIN_STEP)
    );
  }
}

// ================= TELEMETRIA =================

void publicarEstadoPeriodico() {
  if (millis() - ultimaPublicacion < T_PUBLICACION) {
    return;
  }

  ultimaPublicacion = millis();

  String estado = lavadorOn ? "ON" : "OFF";

  if (lavadorOn && confirmado) {
    estado = "CONFIRMADO";
  }

  String mensaje =
    String("{\"nodo\":\"lavador\",\"lavador\":\"")
    + estado + "\",\"intensidad\":"
    + String(intensidad)
    + ",\"modoSeguro\":"
    + (modoSeguro ? "true" : "false")
    + ",\"confirmado\":"
    + (confirmado ? "true" : "false")
    + "}";

  publicar(T_ESTADO, mensaje);

  // Reenviar alerta si no pudo entregarse inicialmente.
  if (fallaLavador) {
    alerta("falla del lavador");
  }
}

// ================= SETUP =================

void setup() {
  Serial.begin(115200);

  pinMode(PIN_RELE, OUTPUT);
  pinMode(PIN_CONF, INPUT);
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);

  digitalWrite(PIN_RELE, RELE_OFF);
  digitalWrite(PIN_STEP, LOW);
  digitalWrite(PIN_DIR, HIGH);

  ultimoContacto = millis();
  ultimoIntento = millis() - T_RECONEXION;

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS, 6);



  mqtt.setServer(BROKER, 1883);
  mqtt.setCallback(alRecibir);
  mqtt.setBufferSize(512);
  mqtt.setKeepAlive(60);
  mqtt.setSocketTimeout(3);

  Serial.println("==============================");
  Serial.println("N4-LAVADOR - GRUPO G1");
  Serial.println("Version corregida");
  Serial.println("==============================");
}

// ================= LOOP =================

void loop() {

  static unsigned long ultimoReloj = 0;

  if (millis() - ultimoReloj >= 5000) {
    ultimoReloj = millis();

    Serial.printf(
      "[RELOJ ESP32] %ld\n",
      (long) time(nullptr)
    );
  }

  verificarModoSeguro();
  verificarConfirmacion();

  mantenerConexion();

  if (mqtt.connected()) {
    mqtt.loop();
  }

  girarMotor();
  publicarEstadoPeriodico();
}
