# N4-Lavador — Planta Costa Norte (G1)

## Información general

- **Curso:** 1ASI0572 — Desarrollo de Soluciones IoT.
- **Laboratorio:** 4 — Instalación heredada.
- **Grupo:** G1.
- **Responsable:** Fabricio Fabian Quispe Barzola.
- **Rol:** Integrante 4 — N4-Lavador.
- **Revisor asignado:** Integrante 3.

## Proyectos Wokwi

- [Instalación heredada](https://wokwi.com/projects/481590024229204993)
- [Instalación corregida](https://wokwi.com/projects/481668300518825985)

## 1. Descripción del nodo

El nodo N4-Lavador controla el sistema de lavado de gases de la Planta Costa Norte mediante un ESP32 conectado a una red MQTT.

Su funcionamiento comprende la recepción de órdenes del gateway, la activación del relé, el control del motor paso a paso, la confirmación de funcionamiento y la generación de alertas.

La implementación incorpora mecanismos de autenticación, validación temporal, protección contra órdenes repetidas y activación preventiva ante pérdida de comunicación.

## 2. Componentes y conexiones

| Componente | Conexión | Función |
|---|---|---|
| ESP32 DevKit | — | Procesamiento y comunicación |
| Módulo relé | GPIO26 | Activación del lavador |
| Interruptor deslizante | GPIO14 | Confirmación de funcionamiento |
| Driver A4988 STEP | GPIO18 | Control de pasos del motor |
| Driver A4988 DIR | GPIO19 | Control de dirección |
| Motor paso a paso | A4988 | Simulación del lavador |
| LED verde | Contacto NO del relé | Indicador visual |
| Resistencia 220 Ω | En serie con LED | Protección del indicador |

El relé utiliza HIGH como estado activo y LOW como estado inactivo, de acuerdo con las pruebas realizadas en Wokwi.

## 3. Bitácora de fallas

| ID | Tipo | Síntoma | Causa raíz | Corrección | Commit |
|---|---|---|---|---|---|
| F13 | Visible | Funcionamiento incorrecto del relé | GPIO27 declarado en firmware y GPIO26 conectado en circuito | Corrección de GPIO y polaridad; incorporación de indicador LED | `21927c9` |
| F14 | Silenciosa | Ausencia de activación del modo seguro | Reinicio del temporizador mediante mqtt.connected() | Control de último contacto mediante recepción de mensajes del gateway | `a69050e` |
| F15 | Seguridad | Aceptación de órdenes no autenticadas | Ausencia de validación de firma y vigencia | HMAC-SHA256, NTP, protección contra replay y restricciones de apagado | `5388fdd` |
| F16 | Cruzada | Riesgo de incompatibilidad entre N4 y N5 | Diferencias identificadas en los mecanismos de comunicación heredados | Definición del contrato de comunicación autenticada y confirmación | Diseño de integración |

## 4. Diagnóstico y correcciones

### F13 — Falla visible

El firmware heredado declaraba GPIO27 para el control del relé, mientras que el circuito se encontraba conectado al GPIO26.

Se corrigió la asignación del pin y la polaridad de activación. Adicionalmente, se incorporó un LED verde conectado al contacto normalmente abierto del relé mediante una resistencia de 220 Ω.

Resultados obtenidos:

- Relé desactivado: LED apagado y motor detenido.
- Relé activado: LED encendido y motor en funcionamiento.

**Paso del marco IoT:** paso 8, sensores y actuadores.

**Commit:** `21927c9`.

**Pull request:** [PR #1](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/1).

### F14 — Falla silenciosa

El firmware heredado actualizaba el tiempo de último contacto mientras permanecía conectado al broker MQTT.

Esta lógica impedía distinguir entre una conexión activa con el broker y la recepción efectiva de mensajes del gateway.

La corrección establece el seguimiento del último mensaje recibido y activa el modo seguro cuando se superan los 60 segundos sin contacto.

Resultados obtenidos:

- Lavador OFF a los 56,044 segundos.
- Lavador ON mediante modo seguro a los 64,391 segundos.
- Activación preventiva con intensidad máxima.
- Generación de alerta de pérdida de contacto.

**Regla relacionada:** R6.

**Commit:** `a69050e`.

**Pull request:** [PR #2](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/2).

### F15 — Falla de seguridad

El firmware heredado procesaba órdenes MQTT sin comprobar su autenticidad.

Se implementaron los siguientes mecanismos:

- Autenticación HMAC-SHA256.
- Sincronización temporal mediante NTP.
- Validación de antigüedad de mensajes.
- Protección contra repetición de mensajes durante la ejecución.
- Rechazo de firmas incorrectas.
- Restricción de apagado ante concentración elevada.
- Restricción de apagado sin información válida de concentración baja.
- Separación de credenciales mediante secrets.h.

Resultados obtenidos:

- Rechazo de órdenes sin firma.
- Rechazo de firmas incorrectas.
- Rechazo de mensajes repetidos.
- Aceptación de órdenes autenticadas.
- Apagado permitido con concentración de 1500 ppm.
- Apagado rechazado con concentración de 2500 ppm.

**Reglas relacionadas:** R7, R9 y R10.

**Commit:** `5388fdd`.

**Pull request:** [PR #3](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/3).

### F16 — Comunicación entre nodos

El diseño de integración establece un contrato MQTT entre N4-Lavador y N5-Gateway para intercambiar órdenes autenticadas, concentraciones, estados, confirmaciones y alertas.

N4 diferencia la activación ordenada de la confirmación efectiva del funcionamiento, permitiendo que el gateway gestione ambas condiciones de manera independiente.

El protocolo y sus criterios de aceptación se describen en [INTEGRACION-N5.md](docs/INTEGRACION-N5.md).

El alcance de los resultados experimentales documentados corresponde a pruebas individuales con un gateway simulado.

## 5. Comunicación MQTT

| Parámetro | Configuración |
|---|---|
| Broker | broker.hivemq.com |
| Puerto MQTT | 1883 |
| Puerto WebSocket | 8884 |
| Prefijo oficial | upc/ga/ |
| Prefijo de pruebas | upc/ga-n4-test-481668300518825985/ |

| Topic oficial | Función |
|---|---|
| upc/ga/lavador/cmd | Recepción de órdenes |
| upc/ga/gateway/conc | Recepción de concentración |
| upc/ga/lavador/estado | Publicación de estados |
| upc/ga/lavador/alerta | Publicación de alertas |

## 6. Control de versiones

| Implementación | Rama | Commit | PR |
|---|---|---|---|
| Instalación heredada | main | `bbc3ce1` | — |
| Corrección F13 | bugfix/lavador-pin-rele | `21927c9` | #1 |
| Corrección F14 | bugfix/lavador-modo-seguro | `a69050e` | #2 |
| Corrección F15 | bugfix/lavador-autenticacion-hmac | `5388fdd` | #3 |
| Documentación | docs/lavador-n4 | Historial del repositorio | #4 y #5 |

Las correcciones fueron incorporadas a la rama principal del repositorio mediante pull requests.

## 7. Configuración y ejecución

1. Abrir el proyecto corregido de Wokwi.
2. Configurar secrets.h utilizando secrets.example.h como plantilla.
3. Introducir la clave de autenticación acordada para el entorno de simulación.
4. Iniciar la simulación.
5. Verificar la conexión WiFi, MQTT y sincronización NTP.
6. Utilizar mensajes autenticados para controlar el lavador.

El archivo secrets.h se mantiene fuera del control de versiones.

## 8. Documentación técnica

- [Registro de pruebas](docs/PRUEBAS.md).
- [Contrato de integración](docs/INTEGRACION-N5.md).
- [Mediciones de respuesta](docs/MEDICION-LAZO-CERRADO.md).
- [Sustentación individual](docs/SUSTENTACION.md).
- [Índice de evidencias](../evidencias/pruebas/N4-Lavador/INDICE.md).

## 9. Resultados

La implementación corregida demuestra:

- Control adecuado del relé mediante GPIO26.
- Activación preventiva ante pérdida de contacto.
- Validación de órdenes mediante HMAC-SHA256.
- Rechazo de órdenes no autorizadas.
- Restricción de apagado ante concentración elevada.
- Detección de confirmación, pérdida de confirmación y generación de alertas.

Los resultados corresponden al comportamiento individual observado en Wokwi.

## 10. Conclusiones

El diagnóstico de N4-Lavador permitió identificar y corregir fallas relacionadas con la configuración del hardware, la lógica de comunicación y la seguridad de las órdenes MQTT.

Las correcciones F13, F14 y F15 mejoran el funcionamiento, la disponibilidad y la protección del lavador de gases.

La implementación, las pruebas individuales y las evidencias se encuentran documentadas y versionadas en el repositorio del Grupo G1.