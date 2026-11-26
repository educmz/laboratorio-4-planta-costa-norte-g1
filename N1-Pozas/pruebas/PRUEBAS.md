# Registro de pruebas — N1-Pozas

## Información general

- **Responsable:** Pedro Nanfuñay (Nodo 1 — Pozas de materia prima).
- **Grupo:** G1.
- **Plataforma:** Wokwi.
- **Microcontrolador:** ESP32.
- **Comunicación:** MQTT mediante HiveMQ (`broker.hivemq.com`).
- **Correcciones:** ramas `bugfix/pozas-temperatura-celsius`, `bugfix/pozas-ventilacion-edge`, `bugfix/pozas-autenticacion-hmac` y `bugfix/pozas-formato-telemetria`.

## Objetivo

Verificar las correcciones realizadas en el Nodo 1 de la instalación heredada: lectura de temperatura en grados Celsius, decisión local de ventilación, rechazo de órdenes MQTT no autenticadas y publicación de telemetría conforme al contrato del grupo.

Las pruebas se efectuaron en Wokwi con el monitor serial del ESP32 y, para la prueba de seguridad, mediante mensajes enviados desde el cliente MQTT de HiveMQ. Se conservan capturas posteriores a las correcciones de F1, F2 y F3. El formato corregido de F4 aparece en una de esas capturas.

## Resultados experimentales

| ID | Regla | Prueba | Resultado observado | Estado de evidencia |
|---|---|---|---|---|
| N1-P01 | R1 | Lectura del DS18B20 en Celsius (F1) | En el monitor serial se observó una temperatura de `22.0 °C`, coherente con el valor configurado en Wokwi. | Captura posterior disponible |
| N1-P02 | R2 | Evaluación de ventilación local (F2) | El código corregido utiliza la condición lógica `gasAlto \|\| temperaturaAlta \|\| ventMax`; en la simulación se observó el estado `Ventilacion: ON`. | Captura posterior disponible; no aísla todavía la causa por temperatura |
| N1-P03 | R10 | Envío de `UMBRAL:5000` sin autenticación (F3) | El receptor registró el rechazo del comando en el monitor serial. | Captura posterior disponible |
| N1-P04 | R9 | Formato JSON de telemetría (F4) | Se observó `valor` como número JSON, `unidad` separada en `ppm` y una firma hexadecimal de 16 caracteres. | Visible en una captura posterior; validación por N5 pendiente |
| N1-P05 | R8 | Periodicidad de mensajes | En registros seriales del diagnóstico aparecieron timestamps consecutivos con incrementos de 2 segundos. | Observación de consola; sin captura específica del firmware final |

## F1 — Lectura de temperatura en Celsius

**Rama:** `bugfix/pozas-temperatura-celsius`.

**Commit:** por registrar.

El firmware heredado utilizaba `getTempFByIndex(0)` y presentaba la temperatura en Fahrenheit aunque la regla R1 establece el umbral en grados Celsius. Se reemplazó por `getTempCByIndex(0)`.

| Condición | Resultado observado |
|---|---|
| Lectura posterior con temperatura ambiente configurada alrededor de 22 °C | `temp: 22.0` o lectura equivalente en Celsius |
| Comparación con el umbral de R1 | Se utiliza el umbral `35 °C` en la misma unidad que la lectura |

### Evidencias

- [F1 - Temperatura corregida](./F1-temperatura.png)

No se adjunta una imagen del firmware heredado para F1; la diferencia de unidades se describe a partir del código original y de la prueba posterior.

## F2 — Activación local de ventilación

**Rama:** `bugfix/pozas-ventilacion-edge`.

**Commit:** por registrar.

La lógica heredada podía anular una activación por temperatura cuando la condición de gas era falsa. Se corrigió la decisión para que la ventilación se active si se supera el umbral de gas **o** el de temperatura, sin exigir autorización del gateway.

| Condición | Resultado |
|---|---|
| Condición corregida en el firmware | `gasAlto \|\| temperaturaAlta \|\| ventMax` |
| Estado registrado en la captura posterior | `Ventilacion: ON` |
| Gas por debajo de 2000 ppm y temperatura superior a 35 °C | **Pendiente de evidencia específica** que demuestre la activación exclusivamente por temperatura |
| Gas bajo y temperatura normal | **Pendiente de evidencia específica** de `Ventilacion: OFF` |

### Evidencias

- [F2 - Ventilación local](./F2-ventilacion.png)

La captura demuestra la ejecución de la versión modificada, pero por sí sola no permite atribuir la activación a temperatura alta cuando el gas está por debajo del umbral. La validación completa de R2 requiere esa combinación de entradas.

## F3 — Rechazo de comandos MQTT no autorizados

**Rama:** `bugfix/pozas-autenticacion-hmac`.

**Commit:** por registrar.

En el firmware heredado se aceptaban órdenes recibidas en `upc/ga/pozas/cmd` sin comprobar su autenticidad. En el diagnóstico se logró modificar el umbral enviando `UMBRAL:5000` desde un cliente MQTT externo. Posteriormente, el receptor fue corregido para rechazar comandos en texto sin la firma requerida.

| Entrada | Resultado observado |
|---|---|
| `UMBRAL:5000` sin firma HMAC | Mensaje rechazado en el monitor serial |
| Orden válida `gateway\|ts\|cmd:VENT:MAX\|firma` | **Pendiente de prueba integrada con N5** |
| Reenvío de una orden firmada previamente aceptada | **Pendiente de evidencia experimental** |

La versión final de N1 usa el formato firmado de órdenes establecido en el código compartido de N5: `gateway|ts|contenido|firma`, con HMAC-SHA256 truncada a 16 caracteres hexadecimales, control temporal y rechazo de repeticiones.

### Evidencias

- [F3 - Comando no autorizado rechazado](evidencias/F3-seguridad.png).

Esta captura respalda el rechazo del mensaje sin firma. No demuestra todavía que el Nodo 1 acepte y ejecute correctamente una orden legítima emitida por N5.

## F4 — Formato de telemetría y firma MQTT

**Rama:** `bugfix/pozas-formato-telemetria`.

**Commit:** por registrar.

El firmware heredado enviaba el campo `valor` como texto con el sufijo `ppm`. Se corrigió la publicación para transmitir un número JSON, mantener `unidad` como campo independiente y calcular la firma a partir de `pozas|ts|valor`, como establece el contrato del grupo.

| Campo | Resultado observado después de la corrección |
|---|---|
| `id` | `"pozas"` |
| `ts` | Número entero correspondiente al timestamp del nodo |
| `valor` | Número JSON, sin comillas ni sufijo `ppm` |
| `unidad` | `"ppm"` |
| `firma` | Cadena hexadecimal de 16 caracteres |

## Comunicación MQTT y NTP

El Nodo 1 utiliza el broker `broker.hivemq.com`, puerto `1883` para el ESP32 y el tópico `upc/ga/pozas/datos` para publicar telemetría. Los comandos se reciben mediante `upc/ga/pozas/cmd`. El monitor externo HiveMQ puede conectarse por WebSocket TLS en el puerto `8884`.

Durante el diagnóstico se observaron mensajes con timestamps sucesivos separados por aproximadamente dos segundos. El firmware utiliza NTP y una firma HMAC para proteger la telemetría. La observación del JSON en el serial no sustituye la aceptación por N5 ni una medición integral de latencia entre nodos.

## Pruebas adicionales pendientes

| ID | Regla | Escenario por completar | Resultado esperado |
|---|---|---|---|
| N1-P06 | R2 | Gas inferior a 2000 ppm y temperatura superior a 35 °C | Ventilación `ON` por temperatura |
| N1-P07 | R2 | Gas inferior a 2000 ppm y temperatura normal | Ventilación `OFF`, salvo orden forzada o falla de sensor |
| N1-P08 | R10 | Orden firmada `VENT:MAX` desde N5 | Orden aceptada y ventilación forzada |
| N1-P09 | R9/R10 | Repetición de orden firmada | Mensaje rechazado |
| N1-P10 | R9 | N5 recibe telemetría firmada de N1 | Registro de aceptación `OK pozas: ... ppm` |

Estas pruebas se consignan como pendientes y no forman parte de los resultados experimentales confirmados.

## Alcance experimental

Los resultados documentados corresponden a pruebas individuales en el simulador Wokwi y a mensajes de prueba enviados desde HiveMQ. Solo se dispone de capturas posteriores a las correcciones. Por ello, la causa raíz de cada falla se contrasta con el firmware heredado, mientras que las imágenes respaldan el comportamiento visible después del cambio.

La conversión del MQ-2 utilizada durante algunas pruebas fue evaluada a partir de lecturas de ADC del simulador y no representa una calibración física certificada. Las capturas no demuestran todavía la validación extremo a extremo con N5 ni todas las combinaciones de entrada exigidas para la actuación local.

## Conclusión

Las evidencias disponibles muestran la lectura de temperatura en Celsius, la ejecución de la lógica de ventilación modificada, el rechazo de un comando MQTT no autenticado y la generación de telemetría con `valor` numérico y firma en el formato previsto.

La documentación permite relacionar cada corrección con su comportamiento observado sin atribuir resultados a pruebas que todavía no se han ejecutado. Para cerrar la verificación integral del Nodo 1 quedan pendientes la prueba de ventilación causada únicamente por temperatura alta y la aceptación de mensajes y órdenes firmadas en conjunto con N5.
