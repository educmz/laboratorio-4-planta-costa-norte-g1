/**
 * Planta Costa Norte - G1
 * Nodo 2: Secadores
 * Laboratorio 4 - Desarrollo de Soluciones IoT
 *
 * F5: Correccion del periodo de publicacion.
 * F6: Calibracion experimental del sensor MQ-2.
 * F7: Implementacion de firma HMAC-SHA256.
 * F8: Incompatibilidad de topicos con Gateway.
 * EXTRA: Correccion del Client ID duplicado.
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <time.h>
#include "esp_sntp.h"
#include <math.h>
#include "mbedtls/md.h"

// ==================================================
// CONFIGURACION MQTT Y SEGURIDAD
// ==================================================

const char* T_DATOS = "upc/ga/secadores/datos";

// F7 - CORRECCION DE SEGURIDAD:
// Se utiliza la misma clave de laboratorio del Gateway.
// No publicar la clave en el repositorio publico.
#include "secrets.h"

// F8 - FALLA CRUZADA:
// El topico de Secadores es correcto.
// El Gateway debe cambiar su suscripcion de:
// upc/ga/+/dato
// a:
// upc/ga/+/datos

// ==================================================
// CONFIGURACION DE PINES
// ==================================================

const int PIN_GAS = 34;
const int PIN_DHT = 15;
const int PIN_R = 25;
const int PIN_G = 26;
const int PIN_B = 27;

// Umbrales de concentracion de gas en ppm.
const float UMBRAL_AMARILLO = 1500.0;
const float UMBRAL_ROJO = 2000.0;

// F5 - CORRECCION VISIBLE:
// Antes: 100 ms entre publicaciones.
// Ahora: 2000 ms entre publicaciones.
// Cumple la regla R8 del laboratorio.
const unsigned long PERIODO = 2000;

// ==================================================
// OBJETOS Y VARIABLES
// ==================================================

WiFiClient net;
PubSubClient mqtt(net);
DHTesp dht;

unsigned long tPub = 0;

// ==================================================
// F6 - CORRECCION SILENCIOSA
// ==================================================

/**
 * Calibracion experimental del sensor MQ-2.
 *
 * Se utilizaron siete concentraciones configuradas
 * en Wokwi y sus respectivas lecturas ADC.
 *
 * La respuesta del sensor no es lineal.
 * Por ello se utiliza interpolacion por segmentos
 * en lugar de escalar directamente el ADC.
 */

const int NUM_PUNTOS = 7;

const float PPM_REF[NUM_PUNTOS] = {
  100.0,
  400.0,
  1000.0,
  1500.0,
  2000.0,
  2500.0,
  5000.0
};

const int ADC_REF[NUM_PUNTOS] = {
  3337,
  3635,
  3762,
  3811,
  3839,
  3861,
  3916
};

/**
 * Convierte una lectura ADC a ppm mediante
 * interpolacion entre puntos de calibracion.
 *
 * Devuelve NAN cuando la lectura se encuentra
 * fuera del rango calibrado.
 */
float convertirGas(int adc) {

  // Verificar la integridad de la calibracion.
  for (int i = 0; i < NUM_PUNTOS; i++) {

    if (ADC_REF[i] < 0 || ADC_REF[i] > 4095) {
      return NAN;
    }

    if (i > 0 && ADC_REF[i] <= ADC_REF[i - 1]) {
      return NAN;
    }
  }

  // No generar concentraciones sin calibracion.
  if (adc < ADC_REF[0] ||
      adc > ADC_REF[NUM_PUNTOS - 1]) {
    return NAN;
  }

  // Buscar el segmento de calibracion correspondiente.
  for (int i = 0; i < NUM_PUNTOS - 1; i++) {

    if (adc >= ADC_REF[i] &&
        adc <= ADC_REF[i + 1]) {

      float proporcion =
        float(adc - ADC_REF[i]) /
        float(ADC_REF[i + 1] - ADC_REF[i]);

      return PPM_REF[i] +
        proporcion *
        (PPM_REF[i + 1] - PPM_REF[i]);
    }
  }

  return NAN;
}

// ==================================================
// F7 - CORRECCION DE SEGURIDAD
// ==================================================

/**
 * Genera una firma HMAC-SHA256 utilizando
 * la clave compartida del grupo G1.
 *
 * Retorna los primeros 16 caracteres hexadecimales.
 *
 * Formato requerido:
 * id|ts|valor
 */
String firmar(const String& mensaje) {

  unsigned char out[32];

  int resultado = mbedtls_md_hmac(
    mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
    (const unsigned char*)CLAVE,
    strlen(CLAVE),
    (const unsigned char*)mensaje.c_str(),
    mensaje.length(),
    out
  );

  if (resultado != 0) {
    Serial.println("Error al generar HMAC");
    return "";
  }

  // Primeros 8 bytes del hash:
  // equivalen a 16 caracteres hexadecimales.
  char firma[17];

  for (int i = 0; i < 8; i++) {

    snprintf(
      firma + 2 * i,
      3,
      "%02x",
      out[i]
    );
  }

  firma[16] = '\0';

  return String(firma);
}

// ==================================================
// CONTROL DE BALIZA RGB
// ==================================================

/**
 * Controla la baliza RGB de catodo comun.
 */
void baliza(int r, int g, int b) {

  digitalWrite(PIN_R, r);
  digitalWrite(PIN_G, g);
  digitalWrite(PIN_B, b);
}

/**
 * Actualiza el indicador local segun la
 * concentracion de gas detectada.
 */
void actualizarBaliza(float gas) {

  if (gas > UMBRAL_ROJO) {

    // Rojo: concentracion peligrosa.
    baliza(1, 0, 0);

  } else if (gas > UMBRAL_AMARILLO) {

    // Amarillo: nivel de advertencia.
    baliza(1, 1, 0);

  } else {

    // Verde: concentracion normal.
    baliza(0, 1, 0);
  }
}

// ==================================================
// CONEXION MQTT
// ==================================================

/**
 * Establece la conexion con el broker MQTT.
 *
 * CORRECCION ADICIONAL:
 * El Client ID anterior era "ga-sensor",
 * compartido con N3 Perimetro.
 *
 * Se utiliza un identificador unico compuesto
 * por grupo, nodo y direccion MAC.
 */
void conectar() {

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (mqtt.connected()) {
    return;
  }

  String id = "ga-secadores-" + WiFi.macAddress();

  if (mqtt.connect(id.c_str())) {

    Serial.println("MQTT conectado");
    Serial.println("Client ID: " + id);

  } else {

    Serial.println(
      "Error MQTT: " + String(mqtt.state())
    );
  }
}

// ==================================================
// CONFIGURACION INICIAL
// ==================================================

void setup() {

  Serial.begin(115200);

  // Configuracion de entradas y salidas.
  pinMode(PIN_GAS, INPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  // ADC del ESP32 de 12 bits: 0 a 4095.
  analogReadResolution(12);

  // Inicializacion del sensor DHT22.
  dht.setup(PIN_DHT, DHTesp::DHT22);

  // Azul: nodo inicializando.
  baliza(0, 0, 1);

  // Conexion a la red WiFi de Wokwi.
  WiFi.begin("Wokwi-GUEST", "");

  Serial.println("Conectando WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }

  Serial.println("WiFi conectado");

  // Sincronizacion de tiempo real mediante NTP.
  // Requisito R9 del laboratorio.
  sntp_set_sync_interval(15000);
  configTime(0, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");

  for (
    int i = 0;
    i < 50 && time(nullptr) < 1700000000;
    i++
  ) {
    delay(200);
  }

  // Configuracion del broker MQTT.
  mqtt.setServer("broker.hivemq.com", 1883);
  mqtt.setKeepAlive(60);

  // MEJORA ADICIONAL:
  // Aumentar el buffer para enviar el JSON
  // completo, incluyendo la firma HMAC.
  mqtt.setBufferSize(512);

  conectar();

  Serial.println("================================");
  Serial.println("Nodo N2 Secadores inicializado");
  Serial.println("Periodo: 2000 ms");
  Serial.println("Calibracion MQ-2: 7 puntos");
  Serial.println("Seguridad: HMAC-SHA256");
  Serial.println("================================");
}

// ==================================================
// BUCLE PRINCIPAL
// ==================================================

void loop() {

  // Mantener la conexion MQTT.
  // Se limita la frecuencia de reconexion
  // para evitar intentos continuos.
  static unsigned long tConexion = 0;

  if (!mqtt.connected() &&
      millis() - tConexion >= 5000) {

    tConexion = millis();
    conectar();
  }

  if (mqtt.connected()) {
    mqtt.loop();
  }

  // F5 - CORRECCION VISIBLE:
  // Control del intervalo de publicacion.
  if (millis() - tPub < PERIODO) {
    return;
  }

  tPub = millis();

  // Lectura analogica del MQ-2.
  int adcGas = analogRead(PIN_GAS);

  // F6 - CORRECCION SILENCIOSA:
  // Se reemplaza la conversion original:
  // analogRead(PIN_GAS) * 10000.0 / 4095.0
  //
  // Ahora se utiliza la curva calibrada.
  float gas = convertirGas(adcGas);

  // Lectura de temperatura y humedad.
  TempAndHumidity d = dht.getTempAndHumidity();

  // Mostrar lectura ADC para diagnostico.
  Serial.println(
    "ADC MQ-2: " + String(adcGas)
  );

  // Validar la concentracion calculada.
  if (isnan(gas)) {

    // Si supera el rango superior calibrado,
    // mantener la advertencia de peligro.
    // No inventar una concentracion numerica.
    if (adcGas > ADC_REF[NUM_PUNTOS - 1]) {

      baliza(1, 0, 0);

      Serial.println(
        "ALERTA: gas superior al rango calibrado"
      );

    } else {

      // Azul: lectura fuera del rango inferior.
      baliza(0, 0, 1);

      Serial.println(
        "Gas fuera del rango calibrado"
      );
    }

    return;
  }

  // Procesamiento local en la capa Edge.
  actualizarBaliza(gas);

  Serial.println(
    "Gas calibrado: " + String(gas, 1) + " ppm"
  );

  // MEJORA ADICIONAL:
  // Evitar enviar lecturas DHT22 invalidas.
  if (isnan(d.temperature) ||
      isnan(d.humidity)) {

    Serial.println("Error de lectura DHT22");
    return;
  }

  // Obtener timestamp sincronizado por NTP.
  long ts = time(nullptr);

  if (ts < 1700000000) {

    Serial.println("Timestamp NTP no valido");
    return;
  }

  // Compensacion temporal del desfase en Wokwi.
  ts += AJUSTE_WOKWI_SEGUNDOS;

  // F7 - CORRECCION DE SEGURIDAD:
  // Generar la firma utilizando exactamente
  // el mismo valor numerico del JSON.

  String valor = String(gas, 1);

  String contenidoFirma =
    "secadores|" +
    String(ts) +
    "|" +
    valor;

  String firma = firmar(contenidoFirma);

  // Verificar que la firma fue generada.
  if (firma.length() != 16) {

    Serial.println("Error: firma HMAC no valida");
    return;
  }

  // Construccion del mensaje JSON.
  // El campo valor se envia como numero.
  // La firma HMAC ya no se encuentra vacia.
  String msg =
    "{\"id\":\"secadores\",\"ts\":" +
    String(ts) +

    ",\"valor\":" + valor +

    ",\"unidad\":\"ppm\"" +

    ",\"temp\":" +
    String(d.temperature, 1) +

    ",\"hum\":" +
    String(d.humidity, 1) +

    ",\"equipo\":\"Secador rotatorio SR-01\"" +

    ",\"zona\":\"Linea de secado de harina - Planta Chimbote\"" +

    ",\"responsable\":\"Jefe de turno de produccion\"" +

    ",\"calidad\":\"ok\"" +

    ",\"firma\":\"" + firma + "\"}";

  // MEJORA ADICIONAL:
  // Verificar el resultado de mqtt.publish().
  if (mqtt.connected() &&
      mqtt.publish(T_DATOS, msg.c_str())) {

    Serial.println(msg);
    Serial.println("Publicacion MQTT exitosa");

  } else {

    Serial.println("Error al publicar por MQTT");
  }

  Serial.println("--------------------------------");
}
