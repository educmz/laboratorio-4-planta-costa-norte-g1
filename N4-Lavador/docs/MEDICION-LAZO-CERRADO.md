# Mediciones de respuesta — N4-Lavador

## Objetivo

Documentar los tiempos de respuesta observados durante las pruebas del nodo N4-Lavador y establecer el procedimiento de medición del lazo cerrado.

## Versiones de referencia

| Versión | Commit |
|---|---|
| Instalación heredada | `bbc3ce1` |
| Corrección del relé | `21927c9` |
| Corrección del modo seguro | `a69050e` |
| Corrección de seguridad | `5388fdd` |

## Comparación funcional

| Característica | Instalación heredada | Instalación corregida |
|---|---|---|
| GPIO del relé | GPIO27 | GPIO26 |
| Seguimiento del gateway | Basado en conexión MQTT | Basado en recepción de mensajes |
| Modo seguro | Temporizador reiniciado incorrectamente | Activación preventiva tras 60 s sin contacto |
| Órdenes MQTT | Texto sin autenticación | Validación HMAC-SHA256 |
| Vigencia temporal | Sin validación | Validación mediante NTP |
| Repetición de mensajes | Sin control | Control durante la ejecución |
| Apagado inseguro | Sin validación efectiva de concentración | Restricción basada en concentración vigente |

## Mediciones del modo seguro

| Instante observado | Estado |
|---|---|
| 35,686 s | OFF |
| 56,044 s | OFF |
| 64,391 s | ON mediante modo seguro |

Los resultados muestran la activación preventiva del lavador después de superar el umbral configurado de 60 segundos.

## Mediciones de confirmación

| Evento | Tiempo local observado |
|---|---|
| Confirmación tras activación preventiva | 25 ms |
| Confirmación local | 4 ms |
| Recuperación de confirmación | 12 ms |

Los tiempos corresponden a eventos locales registrados durante la simulación de N4.

## Procedimiento de medición integral

La medición del lazo cerrado considera el tiempo transcurrido desde la detección de una condición de riesgo hasta la confirmación del funcionamiento del lavador.

El tiempo total se obtiene mediante:

`Tiempo total = t1 - t0`

Donde:

- t0 representa el instante de detección.
- t1 representa el instante de confirmación del lavador.

El escenario de referencia utiliza dos nodos con concentraciones superiores a 2000 ppm y una dirección del viento de 135°.

Para realizar una comparación entre versiones se utilizan condiciones equivalentes y una referencia temporal común.

La regla R5 establece un máximo de 10 segundos entre la orden de activación y la confirmación.

## Evidencias

- [Pruebas del modo seguro](../../evidencias/pruebas/N4-Lavador/F14_Modo_seguro/).
- [Confirmación y alertas](../../evidencias/pruebas/N4-Lavador/Confirmacion_alertas/).
- [Registro de pruebas](PRUEBAS.md).

## Alcance de las mediciones

Los valores numéricos presentados corresponden exclusivamente a los eventos locales observados en N4-Lavador.

La comparación entre la instalación heredada y la corregida presentada en este documento es funcional. No representa una comparación experimental de latencia integral entre los cinco nodos.

## Conclusión

Las mediciones obtenidas permiten caracterizar la respuesta local del mecanismo de confirmación y el comportamiento temporal del modo seguro.

Los registros experimentales demuestran que el firmware detecta eventos de confirmación, pérdida de confirmación y activación preventiva durante las pruebas individuales.