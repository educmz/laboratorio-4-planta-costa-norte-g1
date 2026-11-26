# Laboratorio 4 — Instalación heredada: Planta Costa Norte (G1)

Curso: 1ASI0572 Desarrollo de Soluciones IoT.

## Objetivo
Auditar, diagnosticar, corregir y verificar la instalación heredada de cinco nodos, vinculando cada causa raíz con los 12 pasos de IoT System Design Steps. Una falla solo se considera resuelta con prueba y revisión.

## Integrantes y roles
El integrador coordina el tablero y las pruebas integradas. Nadie aprueba su propio cambio.

| Nodo | Responsable | Revisor | Wokwi original | Copia corregida |
|---|---|---|---|---|
| [N1-Pozas](N1-Pozas/) | Pedro Nanfuñay — Integrante 1 | Gonzalo Aliaga | [Original](https://wokwi.com/projects/481589788453665793) | [Copia corregida](https://wokwi.com/projects/481597967387496449) |
| [N2-Secadores](N2-Secadores/) | Eduardo Chacaliaza — Integrante 2 | Pedro Nanfuñay | [Original](https://wokwi.com/projects/481589874625672193) | [Copia corregida](https://wokwi.com/projects/481603469063491585) |
| [N3-Perimetro](N3-Perimetro/) | Integrante 3 | Eduardo Chacaliaza | [Original](https://wokwi.com/projects/481589950905406465) | [Copia de trabajo](https://wokwi.com/projects/481729202650029057) |
| [N4-Lavador](N4-Lavador/) | Fabricio Fabian Quispe Barzola — Integrante 4 | Integrante 3 | [Original](https://wokwi.com/projects/481590024229204993) | [Copia corregida](https://wokwi.com/projects/481668300518825985) |
| [N5-Gateway](N5-Gateway/) | Gonzalo Aliaga — Integrante 5 (integrador) | Integrante 4 | [Original](https://wokwi.com/projects/481590098239821825) | [Copia corregida](https://wokwi.com/projects/481701580182887425) |

La rotación de revisión vincula a cada responsable con un integrante distinto.

## Contrato MQTT
- Broker: broker.hivemq.com, puerto 1883; WebSocket 8884.
- Prefijo G1: `ga`; observación: `upc/ga/#`.
- Datos: `upc/ga/{nodo}/datos` (pozas, secadores, perimetro).
- Órdenes: `upc/ga/{nodo}/cmd`; estado y alerta: `upc/ga/{nodo}/estado` y `upc/ga/{nodo}/alerta`.
- Concentración máxima: `upc/ga/gateway/conc`.
- Datos: id, ts (NTP), valor numérico, unidad, calidad, firma. HMAC-SHA256 de id|ts|valor, primeros 16 caracteres hexadecimales. Client ID único por nodo y grupo.
- No publicar claves reales ni credenciales.

## Reglas de referencia
| Regla | Criterio |
|---|---|
| R1 | Gas: 2000 ppm; temperatura de pozas: 35 °C |
| R2 | Pozas ventilan localmente si gas o temperatura superan umbral |
| R3 | Viento hacia comunidad: 90–180 grados |
| R4 | Lavador enciende con al menos dos nodos sobre umbral y viento hacia comunidad; apaga bajo 1600 ppm |
| R5 | Confirmación en máximo 10 s; si falta, alerta y ventilación máxima |
| R6 | Lavador enciende tras 60 s sin mensajes del gateway |
| R7 | No apagar lavador con concentración alta |
| R8 | Publicación cada 2 s; ThingSpeak cada 15 s o más |
| R9 | HMAC válido y tiempo real; rechazo de datos con antigüedad superior a 60 s |
| R10 | Solo gateway autoriza órdenes y configuración |

## Bitácora de fallas
Registro de diagnóstico, correcciones y pruebas de la instalación. La numeración de los pasos sigue el material [IoT Basic Solution Design – Part 1 (UPC)](https://drive.google.com/file/d/1vPF15-RvpXWGbC3gflAwg-Ui0xZDKPaj/view). Los pasos se asignan según la causa raíz de cada incidencia.

| ID y nodo | Tipo | Paso del marco | Síntoma | Causa raíz | Corrección y commit | Prueba | Responsable | Revisor |
|---|---|---|---|---|---|---|---|---|
| F1 — N1-Pozas | Visible | Paso 10: Definición del procesamiento de datos en cada nodo y en la nube (R1) | Temperatura en Fahrenheit en vez de Celsius | Uso de `getTempFByIndex(0)` con umbral en °C | `getTempCByIndex(0)`; [PR #17](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/17) · `57186d9` | Lectura de 22.0 °C observada. [Evidencia](N1-Pozas/pruebas/F1-temperatura.png) | Pedro Nanfuñay | Integrante 5 |
| F2 — N1-Pozas | Silenciosa | Paso 10: Definición del procesamiento de datos en cada nodo y en la nube (R1, R2) | Ventilación puede apagarse con temperatura alta | El else del gas sobrescribía la decisión por temperatura | Evaluación conjunta de gas, temperatura y orden de ventilación máxima; [PR #18](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/18) · `ad16783` | `Ventilacion: ON` observado; no se ha aislado la condición de temperatura alta con gas bajo. [Evidencia](N1-Pozas/pruebas/F2-ventilacion.png) | Pedro Nanfuñay | Integrante 5 |
| F3 — N1-Pozas | Seguridad | Paso 4: Definición de los requisitos de la capa de intercambio (R9, R10) | Aceptación de comandos sin autenticación | Ausencia de verificación de firma, origen y vigencia | HMAC, validación temporal y protección contra repetición; [PR #19](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/19) · `855c1d1` | Rechazo de comando sin firma observado. [Evidencia](N1-Pozas/pruebas/F3-seguridad.png) | Pedro Nanfuñay | Integrante 5 |
| F4 — N1-Pozas | Cruzada | Paso 7: Selección de las arquitecturas de intercambio de datos e integración de información (R8, R9) | Telemetría incompatible con N5 | Valor como cadena con sufijo ppm y firma sobre ese texto | Valor numérico, unidad independiente y firma coherente; [PR #20](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/20) · `93ba419` | JSON corregido documentado en N1; recepción de lecturas de Pozas observada durante prueba integrada de N2–N5. [Registro](evidencias/pruebas/N2-Secadores.md) | Pedro Nanfuñay | Integrante 5 |
| F5 — N2-Secadores | Visible | Paso 4: Definición de los requisitos de la capa de intercambio (R8) | Mensajes repetidos en el mismo segundo | PERIODO = 100 ms | 2000 ms; [PR #6](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/6) · `c063b1d` | HiveMQ: seis timestamps consecutivos 1795643206–1795643216, separados por 2 s de reloj del nodo, en versión consolidada · [Ensayo del 26/11](evidencias/pruebas/N2-Secadores/INDICE.md) | Eduardo Chacaliaza — Integrante 2 | Pedro Nanfuñay |
| F6 — N2-Secadores | Silenciosa | Paso 10: Definición del procesamiento de datos en cada nodo y en la nube (R1) | Lectura original 8859.6 ppm con referencia de 400 ppm | Escalado lineal de ADC a ppm sin calibración | Interpolación de siete puntos; [PR #7](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/7) · `c126a2d` | Versión consolidada: 393.0, 1839.3 y 2500.0 ppm; balizas verde, amarilla y roja; referencias declaradas de 400, 1800 y 2500 ppm · [Ensayo del 26/11](evidencias/pruebas/N2-Secadores/INDICE.md) | Eduardo Chacaliaza — Integrante 2 | Pedro Nanfuñay |
| F7 — N2-Secadores | Seguridad | Paso 4: Definición de los requisitos de la capa de intercambio (R9) | JSON con firma vacía | Ausencia de HMAC-SHA256 | Firma de id, ts y valor; [PR #8](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/8) · `bd5c1df` | HiveMQ recibió firmas de 16 caracteres; N5 aceptó lecturas legítimas y registró `RECHAZADO (firma invalida): secadores`. [Resultados](evidencias/pruebas/N2-Secadores.md) · [Ensayo del 26/11](evidencias/pruebas/N2-Secadores/INDICE.md) | Eduardo Chacaliaza — Integrante 2 | Pedro Nanfuñay |
| F8 — N2-Secadores | Cruzada | Paso 7: Selección de las arquitecturas de intercambio de datos e integración de información | Suscripción N5 incompatible con publicación N2 | N5 usa `upc/ga/+/dato` en vez de `upc/ga/+/datos` | Corrección en N5: suscripción en plural (ver F17, [commit 9ebf24c](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/commit/9ebf24c)) | N5 recibió `OK secadores: 400.0 ppm` y `OK secadores: 2500.0 ppm`; integración comprobada. [Resultados](evidencias/pruebas/N2-Secadores.md) · [Ensayo del 26/11](evidencias/pruebas/N2-Secadores/INDICE.md) | Eduardo Chacaliaza — Integrante 2 y Gonzalo Aliaga (Gateway) | Pedro Nanfuñay (bitácora N2) |
| F9 — N3-Perimetro | Visible | — | — | — | — | Sin ejecutar | Integrante 3 | Eduardo Chacaliaza |
| F10 — N3-Perimetro | Silenciosa | — | — | — | — | Sin ejecutar | Integrante 3 | Eduardo Chacaliaza |
| F11 — N3-Perimetro | Seguridad | — | — | — | — | Sin ejecutar | Integrante 3 | Eduardo Chacaliaza |
| F12 — N3-Perimetro | Cruzada | — | — | — | — | Sin ejecutar | Integrante 3 | Eduardo Chacaliaza |
| F13 — N4-Lavador | Visible | Paso 8: Selección de sensores y actuadores | Funcionamiento incorrecto del relé | GPIO27 en firmware y GPIO26 en circuito | Corrección de GPIO y polaridad; indicador LED. [21927c9](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/commit/21927c9) | LED y motor responden correctamente a los estados ON/OFF. [Evidencias](evidencias/pruebas/N4-Lavador/F13_Rele/) | Fabricio Fabian Quispe Barzola | Integrante 3 |
| F14 — N4-Lavador | Silenciosa | Paso 10: Definición del procesamiento de datos en cada nodo y en la nube (R6) | No se activa el modo seguro | Reinicio del temporizador mediante mqtt.connected() | Control de último contacto y activación preventiva tras 60 s. [a69050e](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/commit/a69050e) | OFF a 56,044 s y ON a 64,391 s. [Evidencias](evidencias/pruebas/N4-Lavador/F14_Modo_seguro/) | Fabricio Fabian Quispe Barzola | Integrante 3 |
| F15 — N4-Lavador | Seguridad | Paso 4: Definición de los requisitos de la capa de intercambio (R9, R10) | Aceptación de órdenes no autenticadas | Ausencia de validación de firma y vigencia | HMAC-SHA256, NTP, protección contra replay y restricciones de apagado. [5388fdd](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/commit/5388fdd) | Rechazo de órdenes inválidas; apagado permitido a 1500 ppm y rechazado a 2500 ppm. [Evidencias](evidencias/pruebas/N4-Lavador/F15_Seguridad/) | Fabricio Fabian Quispe Barzola | Integrante 3 |
| F16 — N4-Lavador | Cruzada | Paso 7: Selección de las arquitecturas de intercambio de datos e integración de información | Riesgo de incompatibilidad entre N4 y N5 | Diferencias identificadas en mecanismos de comunicación heredados | Diseño del contrato MQTT autenticado y diferenciación entre activación y confirmación | Comunicación comprobada con gateway simulado. [Contrato técnico](N4-Lavador/docs/INTEGRACION-N5.md) | Fabricio Fabian Quispe Barzola | Integrante 3 |
| F17 — N5-Gateway | Visible | Paso 7: Selección de las arquitecturas de intercambio de datos e integración de información | El gateway no muestra datos y el semáforo queda siempre verde | Suscripción a `upc/ga/+/dato`; los nodos publican en `datos` (misma causa que F8) | Suscripción a `upc/ga/+/datos`; rama `bugfix/gateway-topico-datos` · `9ebf24c` | Simulador de nodos: el serie muestra `OK pozas: 400.0 ppm` (captura 01). [Evidencias](evidencias/pruebas/N5-Gateway/) | Gonzalo Aliaga — Integrante 5 (integrador) | Integrante 4 |
| F18 — N5-Gateway | Silenciosa | Paso 10: Definición del procesamiento de datos en cada nodo y en la nube (R4, R7) | Con un nodo desconectado, el gateway sigue usando su último valor y el lavador no se apaga | Las lecturas no tenían vigencia | Vigencia de 10 s por nodo; sin datos vigentes no se apaga; rama `bugfix/gateway-datos-vencidos` · `dd7e69d` | Secadores y perímetro dejan de publicar: `Nodos con datos vigentes: 1` y la concentración solo usa datos vigentes (capturas 08 y 12). [Evidencias](evidencias/pruebas/N5-Gateway/) | Gonzalo Aliaga — Integrante 5 (integrador) | Integrante 4 |
| F19 — N5-Gateway | Seguridad | Paso 4: Definición de los requisitos de la capa de intercambio (R9) | Un dato firmado antiguo reenviado desde el broker se acepta; una alerta falsa dispara `VENT:MAX` | Sin `configTime` (NTP) el filtro de 60 s nunca rechaza; sin control de repetición ni validación de alerta | NTP, rechazo de antiguos, futuros y repetidos, `id` coherente con el tópico, alerta solo con lavador encendido, clave en `secrets.h`; rama `bugfix/gateway-ntp-antirreplay` · `4f97976`, `04b0d77` | Firma inválida rechazada (captura 09); desfase de reloj en Wokwi detectado y corregido (captura 10); mensaje de hace 120 s rechazado y alerta falsa ignorada (captura 11). [Evidencias](evidencias/pruebas/N5-Gateway/) | Gonzalo Aliaga — Integrante 5 (integrador) | Integrante 4 |
| F20 — N5-Gateway | Cruzada | Paso 7: Selección de las arquitecturas de intercambio de datos e integración de información (R5, R6, R10) | N4 rechaza las órdenes del gateway y entra en modo seguro a los 60 s | El gateway enviaba `ON`/`OFF` y la concentración sin firma y con retain; N4 exige `gateway\|ts\|contenido\|firma` | Órdenes y concentración firmadas, reafirmación cada 10 s, control de confirmación en 10 s con alerta y `cmd:VENT:MAX`; rama `bugfix/gateway-ordenes-firmadas` · `a13f921` | Con N4 simulado: `cmd:ON` firmado, confirmación en 2582 ms, sin confirmación a los 10 s alerta y `cmd:VENT:MAX` (capturas 02 a 07). Pendiente: integración con N4 real en `upc/ga/`. [Evidencias](evidencias/pruebas/N5-Gateway/) | Gonzalo Aliaga — Integrante 5 (integrador) | Integrante 4 |

### Hallazgo adicional N2

**Integración de N2:** [PR #23](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/23) incorporado a `main`; contiene el firmware consolidado y las evidencias. Los PR individuales conservan la trazabilidad de las correcciones sin aprobación formal registrada en GitHub.

EXTRA-N2-01: N2 y N3 comparten `ga-sensor`. Se establece un identificador por grupo, nodo y MAC ([PR #9](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/9), `ccbf176`). N5 recibió lecturas de N2 y N3 durante la misma ejecución; tres nodos con datos vigentes. Paso 7. [Prueba aislada de sesiones](evidencias/pruebas/N2-Secadores/EXTRA_Client_ID/ensayo-local.md).

## Flujo de trabajo
1. Guardar copia de Wokwi; subir código original a main antes de corregir, sin secretos.
2. Observar síntoma y guardar evidencia inicial.
3. Registrar incidencia con responsable y estado en Trello/Jira.
4. Diagnosticar causa raíz y paso del marco.
5. Crear rama `bugfix/<nodo>-<descripcion>` desde main actualizado.
6. Corregir, probar y registrar evidencia de regresión; commit `fix(secadores): descripcion` (adaptar al nodo).
7. Abrir PR a main, solicitar revisión independiente, integrar tras aprobación y actualizar bitácora.

## Evidencias y entrega
- [Evidencias de sesión 1](evidencias/sesion-1/)
- [Evidencias de sesión 2](evidencias/sesion-2/)
- [Evidencias de pruebas](evidencias/pruebas/)
- [Plan de pruebas y medición del lazo cerrado](docs/pruebas.md)
- Tablero Trello/Jira: por registrar.
- Exposición en vivo de 10 minutos: apertura, falla difícil por integrante (25 s), demostración integrada (pico, viento, lavador confirmado, ataque rechazado), preguntas.
- Captura del tablero al cierre de cada sesión, una rama y PR aprobado por falla, enlaces corregidos y medición antes/después.
