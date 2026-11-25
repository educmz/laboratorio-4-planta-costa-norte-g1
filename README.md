# Laboratorio 4 — Instalación heredada: Planta Costa Norte (G1)

Curso: 1ASI0572 Desarrollo de Soluciones IoT.

## Objetivo
Auditar, diagnosticar, corregir y verificar la instalación heredada de cinco nodos, vinculando cada causa raíz con los 12 pasos de IoT System Design Steps. Una falla solo se considera resuelta con prueba y revisión.

## Integrantes y roles
Completar los nombres y confirmar la rotación de revisión antes del minuto 30 de la sesión 1. El integrador coordina el tablero y las pruebas integradas. Nadie aprueba su propio cambio.

| Nodo | Responsable | Revisor | Wokwi original | Copia corregida |
|---|---|---|---|---|
| [N1-Pozas](N1-Pozas/) | Integrante 1 | Integrante 5 (propuesto) | [Original](https://wokwi.com/projects/481589788453665793) | Por registrar |
| [N2-Secadores](N2-Secadores/) | Integrante 2 (@educmz) | Integrante 1 | [Original](https://wokwi.com/projects/481589874625672193) | [Copia de trabajo](https://wokwi.com/projects/481603469063491585) (sin corregir) |
| [N3-Perimetro](N3-Perimetro/) | Integrante 3 | Integrante 2 | [Original](https://wokwi.com/projects/481589950905406465) | Por registrar |
| [N4-Lavador](N4-Lavador/) | Fabricio Fabian Quispe Barzola — Integrante 4 | Integrante 3 | [Original](https://wokwi.com/projects/481590024229204993) | [Copia corregida](https://wokwi.com/projects/481668300518825985) |
| [N5-Gateway](N5-Gateway/) | Integrante 5 (integrador) | Integrante 4 | [Original](https://wokwi.com/projects/481590098239821825) | Por registrar |

La revisión de N1 por el integrante 5 es una propuesta de cierre de rotación, por confirmar con el grupo.

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
Las filas son espacios de diagnóstico, no hallazgos confirmados. Registrar incidencia en Trello/Jira y evidencias antes/después.

| ID y nodo | Tipo | Paso del marco | Síntoma | Causa raíz | Corrección y commit | Prueba | Responsable | Revisor |
|---|---|---|---|---|---|---|---|---|
| F1 — N1-Pozas | Visible | — | — | — | — | Sin ejecutar | Integrante 1 | Integrante 5 (propuesto) |
| F2 — N1-Pozas | Silenciosa | — | — | — | — | Sin ejecutar | Integrante 1 | Integrante 5 (propuesto) |
| F3 — N1-Pozas | Seguridad | — | — | — | — | Sin ejecutar | Integrante 1 | Integrante 5 (propuesto) |
| F4 — N1-Pozas | Cruzada | — | — | — | — | Sin ejecutar | Integrante 1 | Integrante 5 (propuesto) |
| F5 — N2-Secadores | Visible | R8: periodicidad de datos | Mensajes repetidos en el mismo segundo | PERIODO = 100 ms | 2000 ms; [PR #6](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/6) · `c063b1d` | HiveMQ: seis timestamps consecutivos 1795643206–1795643216, separados por 2 s de reloj del nodo, en versión consolidada | Integrante 2 (@educmz) | Integrante 1 |
| F6 — N2-Secadores | Silenciosa | Paso 8: sensores y actuadores; R1 | Lectura original 8859.6 ppm con referencia de 400 ppm | Escalado lineal de ADC a ppm sin calibración | Interpolación de siete puntos; [PR #7](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/7) · `c126a2d` | Versión consolidada: 393.0, 1839.3 y 2500.0 ppm; balizas verde, amarilla y roja; referencias declaradas de 400, 1800 y 2500 ppm | Integrante 2 (@educmz) | Integrante 1 |
| F7 — N2-Secadores | Seguridad | R9: autenticación e integridad | JSON con firma vacía | Ausencia de HMAC-SHA256 | Firma de id, ts y valor; [PR #8](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/8) · `bd5c1df` | HiveMQ recibió firmas de 16 caracteres en versión consolidada; aceptación y rechazo en N5 no evaluados | Integrante 2 (@educmz) | Integrante 1 |
| F8 — N2-Secadores | Cruzada | Contrato de interfaz MQTT | Suscripción N5 incompatible con publicación N2 | N5 usa `upc/ga/+/dato` en vez de `upc/ga/+/datos` | Corrección en N5: suscripción en plural | HiveMQ confirma recepción en tópico de N2; prueba extremo a extremo no ejecutada | Integrantes 2 y 5 | Integrante 1 (bitácora N2) |
| F9 — N3-Perimetro | Visible | — | — | — | — | Sin ejecutar | Integrante 3 | Integrante 2 |
| F10 — N3-Perimetro | Silenciosa | — | — | — | — | Sin ejecutar | Integrante 3 | Integrante 2 |
| F11 — N3-Perimetro | Seguridad | — | — | — | — | Sin ejecutar | Integrante 3 | Integrante 2 |
| F12 — N3-Perimetro | Cruzada | — | — | — | — | Sin ejecutar | Integrante 3 | Integrante 2 |
| F13 — N4-Lavador | Visible | 8: sensores y actuadores | Funcionamiento incorrecto del relé | GPIO27 en firmware y GPIO26 en circuito | Corrección de GPIO y polaridad; indicador LED. [21927c9](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/commit/21927c9) | LED y motor responden correctamente a los estados ON/OFF. [Evidencias](evidencias/pruebas/N4-Lavador/F13_Rele/) | Fabricio Fabian Quispe Barzola | Integrante 3 |
| F14 — N4-Lavador | Silenciosa | Diseño de lógica de servicio y temporización | No se activa el modo seguro | Reinicio del temporizador mediante mqtt.connected() | Control de último contacto y activación preventiva tras 60 s. [a69050e](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/commit/a69050e) | OFF a 56,044 s y ON a 64,391 s. [Evidencias](evidencias/pruebas/N4-Lavador/F14_Modo_seguro/) | Fabricio Fabian Quispe Barzola | Integrante 3 |
| F15 — N4-Lavador | Seguridad | Diseño de seguridad y comunicación | Aceptación de órdenes no autenticadas | Ausencia de validación de firma y vigencia | HMAC-SHA256, NTP, protección contra replay y restricciones de apagado. [5388fdd](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/commit/5388fdd) | Rechazo de órdenes inválidas; apagado permitido a 1500 ppm y rechazado a 2500 ppm. [Evidencias](evidencias/pruebas/N4-Lavador/F15_Seguridad/) | Fabricio Fabian Quispe Barzola | Integrante 3 |
| F16 — N4-Lavador | Cruzada | Diseño de integración entre componentes | Riesgo de incompatibilidad entre N4 y N5 | Diferencias identificadas en mecanismos de comunicación heredados | Diseño del contrato MQTT autenticado y diferenciación entre activación y confirmación | Comunicación comprobada con gateway simulado. [Contrato técnico](N4-Lavador/docs/INTEGRACION-N5.md) | Fabricio Fabian Quispe Barzola | Integrante 3 |
| F17 — N5-Gateway | Visible | — | — | — | — | Sin ejecutar | Integrante 5 (integrador) | Integrante 4 |
| F18 — N5-Gateway | Silenciosa | — | — | — | — | Sin ejecutar | Integrante 5 (integrador) | Integrante 4 |
| F19 — N5-Gateway | Seguridad | — | — | — | — | Sin ejecutar | Integrante 5 (integrador) | Integrante 4 |
| F20 — N5-Gateway | Cruzada | — | — | — | — | Sin ejecutar | Integrante 5 (integrador) | Integrante 4 |

### Hallazgo adicional N2

EXTRA-N2-01: N2 y N3 comparten `ga-sensor`. Se establece un identificador por grupo, nodo y MAC ([PR #9](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/9), `ccbf176`). Conexión individual observada; ejecución simultánea no evaluada.

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
