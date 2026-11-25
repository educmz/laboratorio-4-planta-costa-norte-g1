# Integración N4–N5 (propuesta pendiente de acuerdo)

## Contrato del enunciado

Broker `broker.hivemq.com`, MQTT 1883, WebSocket 8884; Client ID único. Prefijo G1 `upc/ga/`. Datos de sensores JSON con id, ts, valor numérico, unidad, calidad y firma HMAC-SHA256 de `id|ts|valor` truncada a 16 caracteres hexadecimales. Reglas R4–R10 aplicables. El documento no prescribe el formato exacto de los comandos autenticados que implementamos: se debe acordar con N5.

| Topic oficial | Publica | Consume |
|---|---|---|
| upc/ga/lavador/cmd | N5 | N4 |
| upc/ga/gateway/conc | N5 | N4 |
| upc/ga/lavador/estado | N4 | N5 |
| upc/ga/lavador/alerta | N4 | N5 |

## Formato probado con gateway simulado

`gateway|<timestamp Unix en segundos>|cmd:ON|<firma>`; análogo para `cmd:OFF`, `cmd:INT:1` a `cmd:INT:5` (intensidad no probada en esta sesión) y `conc:2500.0` / `conc:1500.0` en el topic de concentración. Firmar los primeros tres campos exactos con HMAC-SHA256 y enviar los primeros 16 hexadecimales. Ventana implementada: máximo 60 s de antigüedad y tolerancia de adelanto de 5 s. Timestamps crecientes por flujo según firmware de prueba; la protección demostrada no garantiza persistencia tras reinicio. Confirmar el código definitivo antes de publicar el contrato.

Estados observados: JSON `{"nodo":"lavador","lavador":"ON","intensidad":1,"modoSeguro":false,"confirmado":false}` y eventos `{"lavador":"ON","motivo":"orden gateway"}`, `{"lavador":"CONFIRMADO"}`. Alerta `{"alerta":"falla del lavador"}` o `{"alerta":"sin contacto con gateway"}`. Los estados y alertas observados no están firmados; revisar alcance de R9 con el grupo y protegerlos antes de considerar seguridad integral. No confundir ON ordenado con CONFIRMADO.

## F16 y aceptación conjunta

Hipótesis a verificar en N5 actual: el gateway puede considerar ON al emitir una orden sin esperar confirmación real; la versión heredada también emplea órdenes sin firma. Determinar cuál es la causa raíz cruzada que explica el síntoma, registrar incidencia y coordinar cambios con integrante 5. No asignar la misma causa a dos fallas sin evidencia.

1. Acordar formatos y clave de simulación nueva, fuera de Git. Sincronizar relojes y comprobar estabilidad MQTT.
2. Ejecutar en entorno aislado conjunto: dos nodos >2000 ppm y viento 135°, orden autenticada N5, N4 ON y confirmación en <=10 s; registrar t0 y t1.
3. Repetir sin confirmación: alerta N4 recibida por N5 y ventilación máxima de N1 demostrada.
4. Verificar OFF con concentración baja, rechazo con alta, pérdida de contacto, ataque y regresión. Restaurar prefijo oficial solo coordinadamente.
5. Registrar PR de la falla cruzada, revisión independiente y evidencia. F16 permanece abierta hasta entonces.
