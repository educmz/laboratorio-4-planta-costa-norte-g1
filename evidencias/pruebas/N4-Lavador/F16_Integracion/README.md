# F16 — Integración N4-Lavador / N5-Gateway

Grupo G1 · Laboratorio 4 · Desarrollo de Soluciones IoT

Responsable: integrante 4 (N4). Revisor asignado: integrante 3. Integración: integrante 5 (N5).

## Diagnóstico y corrección

En las versiones usadas para la integración, N4 utilizaba un prefijo aislado y N5 publicaba en `upc/ga/`. Al unificar los tópicos se observaron rechazos por desfase temporal entre los ESP32 simulados (263 s de adelanto y, en otra ejecución, 82 s de antigüedad). La incompatibilidad heredada de órdenes en texto plano se relaciona con F20 de N5.

En N4 se unificaron los cuatro tópicos oficiales, se configuró un identificador MQTT específico, se incorporó resincronización NTP cada 15 s, keep-alive de 60 s y diagnóstico de suscripciones y recepción. Se conservó HMAC-SHA256 y el rechazo de mensajes con más de 60 s de antigüedad; la tolerancia de adelanto utilizada en la prueba fue de 15 s. No se modificaron los GPIO.

| Tópico | Dirección |
|---|---|
| `upc/ga/lavador/cmd` | N5 → N4 |
| `upc/ga/gateway/conc` | N5 → N4 |
| `upc/ga/lavador/estado` | N4 → N5 |
| `upc/ga/lavador/alerta` | N4 → N5 |

## Entorno experimental

- [N4 corregido](https://wokwi.com/projects/481668300518825985).
- [Copia N5 utilizada en la prueba](https://wokwi.com/projects/481779426126558209).
- [Simulador de sensores](../../../../N5-Gateway/tests/n5_simulador_nodos.html).
- HiveMQ, MQTT, prefijo `ga`, clave compartida privada.
- Sensores simulados N1, N2 y N3; N4 y N5 ejecutados en Wokwi en ventanas independientes.
- Confirmación automática del HTML desactivada para utilizar la entrada GPIO14 de N4.

## Evidencias gráficas

| Archivo | Observación |
|---|---|
| [01](01-simulador-conectado.png) | HTML conectado y estados N4 visibles; configuración inicial. |
| [02](02-n4-topicos-oficiales.png) | Tópicos oficiales y N4 OFF. |
| [03](03-n5-conectado-ntp.png) | N5 conectado y NTP sincronizado; inicialmente sin sensores vigentes. |
| [04](04-sensores-publicando.png) | Tres sensores publicando 400 ppm; confirmación automática aún marcada en esta etapa. |
| [05](05-n4-estado-inicial-off.png) | N4 OFF, motor en cero pasos. |
| [06](06-desfase-ntp-modo-seguro.png) | Rechazo temporal y activación preventiva del lavador. |
| [07](07-simulador-sin-confirmacion-automatica.png) | Confirmación automática desactivada y sensores publicando. |
| [08](08-n4-diagnostico-conexion.png) | N4 publica estado, sin recepción de concentración visible en esa captura. |
| [09](09-concentracion-autenticada-400ppm.png) | N4 acepta 400 ppm con diferencia temporal de 18 s; todavía ON tras modo seguro. |
| [10](10-integracion-pico-2500ppm.png) | N5 rojo, N4 ON, concentración autenticada 2500 ppm y diferencia temporal de 0 s. |
| [11](11-retorno-normal-off.png) | N5 verde y estado OFF de N4 recibido por N5. |

Se conservan 11 capturas únicas de las 12 recibidas (una duplicada). Se ocultaron las credenciales visibles en cuatro capturas; los estados y registros técnicos se mantienen. La clave expuesta anteriormente debe rotarse con el equipo antes de publicar o reutilizarla.

## Evidencia serial de activación y confirmación

[Registro serial completo de N4](F16-Registro-serial-N4.md).

Extracto del registro (se omiten líneas intermedias):

```text
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720891 | Mensaje: 1795720885 | Diferencia: 6 s
[CONCENTRACION] 2500.0 ppm
[RX MQTT] upc/ga/lavador/cmd
[NTP] ESP32: 1795720891 | Mensaje: 1795720885 | Diferencia: 6 s
[LAVADOR] ON: orden gateway
[MQTT] upc/ga/lavador/estado -> {"lavador":"ON","motivo":"orden gateway"}
...
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 2460 ms
```

## Resultados y alcance

| Comprobación | Resultado observado |
|---|---|
| Concentración firmada desde N5 | 400 y 2500 ppm aceptados |
| Orden ON autenticada | N4 encendido por orden gateway |
| Retorno OFF | Estado OFF observado en N4 y recibido por N5 |
| Confirmación GPIO14 | 2460 ms desde la activación local |
| Publicación de confirmación | JSON `{"lavador":"CONFIRMADO"}` enviado por N4 |
| Modo seguro | Activación observada ante ausencia de mensajes válidos |

El tiempo de 2460 ms es local a N4, inferior al umbral de 10 s; no acredita por sí solo la recepción de confirmación en N5 ni la latencia integral desde los sensores. Se observaron pérdidas y recuperaciones de confirmación posteriores. El registro no determina si fueron causadas por manipulación del interruptor. Las capturas no acreditan la prueba integrada de alerta con ventilación N1, rechazo integrado de ataques ni medición integral antes/después. Los estados y alertas de N4 no están firmados en la versión probada.

## Conclusión

La evidencia demuestra interoperabilidad MQTT y autenticación de órdenes y concentraciones, activación del lavador mediante N5, retorno al estado OFF y publicación de confirmación local. El alcance de esta evidencia es N4/N5 con sensores simulados, no la aceptación integral de los cinco nodos.
