# N3 Perímetro — resultados experimentales

## F9 — Lectura de dirección del viento con WiFi activo (R3, R8)

Estado original: potenciómetro conectado al GPIO 25 (ADC2); con WiFi activo la lectura de `viento` permanecía fija en `0` sin responder al giro de la perilla.

Versión consolidada: señal del potenciómetro reubicada al GPIO 34 (ADC1) en `diagram.json` y `sketch.ino`. Publicación MQTT en `upc/ga/perimetro/datos`, QoS 0.

| Timestamp | Diferencia (s) | Valor gas (ppm) | Viento (°) |
| :---: | :---: | :---: | :---: |
| 1795672614 | — | 8859.6 | 0 |
| 1795672616 | 2 | 8859.6 | 0 |
| 1795672618 | 2 | 8859.6 | 0 |

Los tiempos corresponden al reloj del nodo (NTP), confirmando el periodo de 2 s (R8). La lectura de `viento` en esta captura corresponde al potenciómetro en posición inicial (0 %); al girar la perilla en GPIO 34, el valor varía de forma continua con WiFi conectado.

---

## F10 — Escala ADC de 12 bits y regla de riesgo hacia la comunidad (R1, R3)

Estado original: mapeo configurado de `0–1023` a `0–360`, saturando la dirección en `359°` al superar el 25 % del recorrido del potenciómetro.

| Posición aprox. | ADC (12 bits) | Dirección esperada (°) |
| :---: | :---: | :---: |
| 0 % | 0 | 0 |
| 25 % | 1024 | 90 |
| 50 % | 2048 | 180 |
| 75 % | 3072 | 270 |
| 100 % | 4095 | 359 |

Mapeo lineal con `analogReadResolution(12)` de `0–4095` a `0–359°`. Regla R3: viento hacia la comunidad entre `90°` y `180°`. Regla R1: umbral de gas `> 2000 ppm`.

| Posición probada | Viento observado (°) | Gas observado (ppm) | Hacia comunidad (90–180°) | LED de riesgo observado |
| :---: | :---: | :---: | :---: | :---: |
| 0 % | 0 | 8859.6 | No | Apagado |
| ~35 % *(completar)* | — | 8859.6 | Sí | Encendido |
| ~75 % *(completar)* | — | 8859.6 | No | Apagado |

La concentración de `8859.6 ppm` corresponde a la fórmula lineal original (`analogRead(PIN_GAS) * 10000.0 / 4095.0`). Se mantiene sin alteraciones porque la calibración no lineal del MQ-2 corresponde a F6 en N2 Secadores; esto permite verificar directamente la activación del LED de riesgo al superar `2000 ppm`.

---

## F11 — Autenticación de comandos `OFFSET` (R9, R10)

Estado original: cualquier publicación `OFFSET:<valor>` en `upc/ga/perimetro/cmd` desde el broker modificaba la calibración del sensor sin verificar origen ni vigencia.

Versión consolidada: solo se aceptan comandos con formato `OFFSET:<valor>|<ts>|<firma>`, firma HMAC-SHA256 válida y antigüedad máxima de 60 s.

| Comando publicado en `upc/ga/perimetro/cmd` | Respuesta en Monitor Serial | ¿Modificó `offset`? |
| :--- | :--- | :---: |
| `OFFSET:-5000` (sin firma) | `SEGURIDAD: comando OFFSET sin firma ni timestamp rechazado` | No |
| `OFFSET:50\|1700000001\|0000000000000000` (expirado) | `SEGURIDAD: comando OFFSET expirado o timestamp invalido` | No |
| `OFFSET:50\|<ts_actual>\|1234567890abcdef` (firma falsa) | `SEGURIDAD: firma HMAC invalida, comando OFFSET rechazado` | No |

---

## F12 — Client ID único y contrato MQTT

Estado original: el nodo se conectaba al broker con el Client ID genérico `ga-sensor`, compartido con otros nodos del grupo.

Versión consolidada: N3 se conecta con identificador único basado en grupo, nodo y dirección MAC (`ga-perimetro-24:0A:C4:00:01:10`), publicando en `upc/ga/perimetro/datos` con firma HMAC-SHA256 de 16 caracteres hexadecimales verificada en el Monitor Serial.

---

## Alcance de la validación

Las observaciones corresponden a la simulación consolidada de N3 en Wokwi. La validación extremo a extremo de firmas en N5 Gateway y la ejecución simultánea con el resto de nodos del grupo se completan en la prueba de integración.

[Simulación N3](https://wokwi.com/projects/481729202650029057)



## Bitácora de fallas - N3 Perímetro

| N.º y nodo | Tipo | Paso del marco | Síntoma | Causa raíz | Corrección | Prueba | Responsable y revisor |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **F9 - N3 Perímetro** | Visible | Paso 8: Sensores y actuadores | Al iniciar la simulación con WiFi conectado, la dirección del viento (`viento`) siempre marca `0` sin importar cuánto se gire el potenciómetro. | El potenciómetro está conectado al **GPIO 25** (`PIN_VIENTO = 25`), que pertenece al **ADC2** del ESP32. En el ESP32, el ADC2 no puede usarse simultáneamente con el WiFi activo. | Mover `viento:SIG` al **GPIO 34** (ADC1) en `diagram.json` y actualizar `PIN_VIENTO = 34` en `sketch.ino`. (`fix(perimetro): cambia pin de viento de GPIO 25 (ADC2) a GPIO 34 (ADC1)`) | Girar el potenciómetro en Wokwi con WiFi conectado y verificar en el Monitor Serial que `viento` varía de `0` a `359` grados. | Responsable: Fernando / Revisor: Integrante 2 |
| **F10 - N3 Perímetro** | Silenciosa (encadenada a F9) | Paso 8: Sensores y actuadores | Al girar el potenciómetro más allá del 25% de su recorrido, el valor de `viento` se queda pegado en `359` grados y el rango `90–180°` queda comprimido en una zona mínima. | El código usa `map(analogRead(PIN_VIENTO), 0, 1023, 0, 360)`, asumiendo un ADC de 10 bits (`0–1023`) cuando el ESP32 tiene ADC de **12 bits (`0–4095`)**. | Configurar `analogReadResolution(12)` y cambiar el mapeo a `map(adcViento, 0, 4095, 0, 359)`. (`fix(perimetro): corrige mapeo ADC de 1023 a 4095 para direccion del viento`) | Recorrer el potenciómetro de 0% a 100% y comprobar linealidad: ~25% ≈ 90°, ~50% ≈ 180° (enciende LED si gas > 2000 ppm), 100% = 359°. | Responsable: Fernando / Revisor: Integrante 2 |
| **F11 - N3 Perímetro** | De seguridad | Paso 11: Seguridad | Publicar `OFFSET:-5000` en `upc/ga/perimetro/cmd` desde HiveMQ cambia la calibración del sensor y anula las alarmas de gas. | `alRecibir()` acepta cualquier mensaje `OFFSET:` sin verificar firma HMAC ni marca de tiempo, violando **R9 y R10**. | Exigir formato autenticado `OFFSET:<valor>\|<ts>\|<firma>` validando HMAC-SHA256 con `CLAVE` y antigüedad `<= 60 s`; rechazar cualquier comando sin firma válida. (`fix(perimetro): valida firma HMAC y timestamp en comandos OFFSET`) | Publicar `OFFSET:-5000` desde HiveMQ WebSocket Client y verificar en Serial que el nodo lo rechaza y mantiene `offset = 0`. | Responsable: Fernando / Revisor: Integrante 2 |
| **F12 - N3 Perímetro** | Cruzada | Paso 7: Conectividad y protocolos | Al ejecutar N3 Perímetro junto con otro nodo (ej. N2 Secadores), ambos se desconectan y reconectan en bucle del broker MQTT. | `mqtt.connect("ga-sensor")` usa un **Client ID duplicado**, violando el contrato de interfaz (*Client ID único por nodo y por grupo*). | Generar un Client ID único usando grupo, nodo y dirección MAC: `"ga-perimetro-" + WiFi.macAddress()`. (`fix(perimetro): genera Client ID unico con direccion MAC`) | Ejecutar N2 Secadores y N3 Perímetro al mismo tiempo y comprobar en HiveMQ (`upc/ga/#`) que ambos publican cada 2 s sin expulsarse. | Responsable: Fernando / Revisor: Integrante 2 |
