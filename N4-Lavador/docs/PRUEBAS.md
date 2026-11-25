# Pruebas de N4-Lavador

Entorno: Wokwi, ESP32, HiveMQ público, prefijo aislado. Evidencias bajo `evidencias/pruebas/N4-Lavador/` del repositorio. Los números indican el prefijo del nombre de captura en INDICE.md. Los resultados se limitan a lo observado y no equivalen a aprobación de PR.

| Prueba | Regla | Resultado observado | Evidencia | Estado |
|---|---|---|---|---|
| GPIO y contacto COM-NO | R4 | GPIO26; LED apagado en OFF (0 pasos), encendido en ON (123 pasos) | F13_Rele/01, 28–31 | Verificada; 28–29 son diagnóstico intermedio, no versión heredada |
| P6: 60 s sin contacto | R6 | OFF a 56,044 s, ON a 64,391 s | F14_Modo_seguro/04–05 | Verificada en etapa de corrección; antes controlado pendiente |
| P5: ausencia de confirmación | R5 | Alerta falla del lavador, también en registro local sin MQTT | Confirmacion_alertas/06–07 | Local verificada; ventilación N1 pendiente |
| Confirmación y pérdida | R5 | Confirmación local observada; pérdida y recuperación | Confirmacion_alertas/08–09,12–13 | Local verificada |
| P10: orden sin firma | R10 | Rechazada, lavador OFF | F15_Seguridad/16–17 | Verificada |
| Timestamp fuera de margen | R9 | Timestamp inválido, orden rechazada | MQTT_NTP/19 y logs del chat | Verificada para adelanto; caducidad >60 s no ensayada específicamente |
| ON firmado | R10 | ON por orden gateway, diferencia 6 s | F15_Seguridad/21 | Verificada |
| Replay | R10 | Orden repetida rechazada a 35 y 41 s | Registro-serial-F15-completo.md | Verificada en misma ejecución, no tras reinicio |
| OFF sin concentración baja vigente | R7 | Apagado rechazado, sigue ON | F15_Seguridad/22, registro | Verificada |
| P4: 1500 ppm y OFF firmado | R4/R7 | 1500 ppm recibidos; OFF; motor detenido en 11697 pasos | F15_Seguridad/23–25 | Verificada con gateway simulado |
| Firma incorrecta | R10 | Firma HMAC inválida; orden rechazada, OFF | F15_Seguridad/25 | Verificada |
| P7: 2500 ppm y OFF | R7 | 2500 ppm recibidos; ON; OFF rechazado tres veces | F15_Seguridad/26–27 y registro | Verificada con gateway simulado |
| P8: publicaciones periódicas | R8 | Estados periódicos observados; intermitencias de red | MQTT_NTP/14–15 y registro | Parcial; medir intervalo y ThingSpeak conjuntamente |
| P3: dos nodos y viento 135° | R4 | No ejecutada con N5 real | Pendiente | Pendiente |
| P9: datos inválidos rechazados por N5 | R9 | No ejecutada con N5 real | Pendiente | Pendiente |

## Extracto de la prueba de 2500 ppm

Registro serial: concentración recibida con reloj 1795633482; ON a 1795633492; OFF rechazado a 1795633498, 1795633503 y 1795633512. No se observó apagado. Referencia: `F15_Seguridad/Registro-serial-F15-completo.md`.

## Regresión pendiente antes de entrega integrada

Repetir ON/OFF autenticados, modo seguro, confirmación, alerta, concentración alta/baja y ataque tras el ajuste final de polaridad. Ejecutar prueba con N5 y N1 para confirmar R4/R5, verificar intervalos y registrar resultado real. Obtener evidencia del firmware heredado con escenarios equivalentes; no confundir capturas de diagnóstico intermedio con el antes original. No alterar originales de Wokwi. Registrar fecha, versión, responsable, evidencia y commit.

## Incidencias del entorno

MQTT presentó desconexiones y errores -4, además de un reinicio por assert durante un intento previo de conexión. Se ajustó reconexión y arranque NTP; no se ha confirmado estabilidad prolongada. El reloj del simulador presentó desfases frente al navegador; se empleó el reloj ESP32 para generar mensajes de prueba sin eliminar las validaciones. No generalizar ese procedimiento a la integración real. Las capturas deben revisarse para ocultar credenciales antes de publicar.
