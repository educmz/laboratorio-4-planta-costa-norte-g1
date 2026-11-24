# Plan de pruebas (no ejecutadas)

| ID | Escenario | Esperado |
|---|---|---|
| P1 | Pozas gas > 2000 ppm | Ventilación local |
| P2 | Pozas temperatura > 35 °C | Ventilación local |
| P3 | Dos nodos > 2000 ppm y viento 135° | Lavador encendido |
| P4 | Concentración máxima < 1600 ppm | Lavador apagado |
| P5 | Sin confirmación en 10 s | Alerta y ventilación máxima |
| P6 | Sin mensajes gateway por 60 s | Lavador encendido por precaución |
| P7 | Concentración alta y orden de apagado | Lavador no se apaga |
| P8 | Observar intervalos MQTT y ThingSpeak | 2 s y >= 15 s |
| P9 | Firma inválida o dato > 60 s | Rechazo por gateway |
| P10 | Orden no autorizada en entorno de prueba del grupo | Rechazo |
| P11 | N2 variar gas, temperatura, humedad | Lecturas coherentes y baliza según lógica validada |

## Lazo cerrado
Medir desde detección del escenario hasta confirmación del lavador, en las mismas condiciones antes y después. Registrar fecha, versión, t0, t1, diferencia, evidencia y si no hubo confirmación.

| Etapa | Commit | t0 | t1 | Diferencia | Evidencia |
|---|---|---|---|---|---|
| Antes | — | — | — | — | — |
| Después | — | — | — | — | — |

## Regresión
Repetir las pruebas pertinentes tras cada cambio y todas al final; registrar resultado real, responsable y evidencia. No declarar aprobada una prueba sin ejecutarla.
