# Evidencias N4-Lavador – Grupo G1

31 capturas originales del chat, renombradas y agrupadas. No se alteraron sus pixeles.
**Privacidad:** varias capturas de sketch.ino muestran la clave HMAC; ocultarla antes de publicar y coordinar su rotación.
Las capturas de polaridad incorrecta pertenecen al proceso de corrección, no al firmware heredado. F16 e integración con N5 siguen pendientes. Las pruebas F15 se hicieron antes del ajuste final de polaridad. El registro conserva las pruebas de replay (35 y 41 s) y bloqueo a 2500 ppm; no hay captura independiente de replay identificada.

| Archivo | Descripción |
|---|---|
| `F13_Rele/01-F13-heredado-GPIO27.png` | Código heredado con GPIO27; evidencia estática. |
| `Diagnostico/02-ThingSpeak-configuracion-canal.png` | Pantalla de creación de canal. |
| `F14_Modo_seguro/03-F14-corregido-OFF-35s.png` | Estado OFF a 35,686 s. |
| `F14_Modo_seguro/04-F14-corregido-antes-60s.png` | OFF a 56,044 s. |
| `F14_Modo_seguro/05-F14-corregido-modo-seguro.png` | ON por modo seguro a 64,391 s. |
| `Confirmacion_alertas/06-N4-alerta-sin-confirmacion.png` | Alerta de falla publicada por MQTT. |
| `Confirmacion_alertas/07-N4-alerta-local-sin-conexion.png` | Alerta local sin MQTT. |
| `Confirmacion_alertas/08-N4-confirmacion-perdida.png` | Confirmación de 4 ms y pérdida posterior. |
| `Confirmacion_alertas/09-N4-confirmacion-local-recuperada.png` | Recuperación de confirmación en 12 ms. |
| `MQTT_NTP/10-HiveMQ-configuracion-inicial.png` | Configuración del cliente. |
| `MQTT_NTP/11-HiveMQ-conectado-suscripcion.png` | Cliente conectado y suscripción. |
| `Confirmacion_alertas/12-N4-confirmacion-local-sostenida.png` | Confirmado localmente a 76,940 s. |
| `Confirmacion_alertas/13-N4-confirmacion-local-sin-internet.png` | Confirmado localmente a 74,772 s. |
| `MQTT_NTP/14-N4-reconexion-MQTT.png` | Recuperación de MQTT. |
| `MQTT_NTP/15-N4-comunicacion-MQTT.png` | Recepción externa en HiveMQ. |
| `F15_Seguridad/16-F15-despues-orden-rechazada.png` | Orden sin firma rechazada. |
| `F15_Seguridad/17-F15-HiveMQ-ON-sin-firma-modo-seguro.png` | ON sin firma y posterior modo seguro. |
| `MQTT_NTP/18-HiveMQ-configuracion-reinicio.png` | Configuración antes de conectar. |
| `MQTT_NTP/19-N4-diagnostico-desfase-reloj.png` | Captura de simulación durante diagnóstico de reloj; el desfase consta en el log compartido. |
| `Diagnostico/20-Generador-HMAC-timestamp-invalido.png` | Generador con valor 46 no válido. |
| `F15_Seguridad/21-F15-firma-valida.png` | ON autenticado; 1234 pasos. |
| `F15_Seguridad/22-F15-apagado-bloqueado.png` | OFF rechazado; no demuestra por sí solo 2500 ppm. |
| `F15_Seguridad/23-F15-concentracion-baja.png` | 1500 ppm recibidos. |
| `F15_Seguridad/24-F15-apagado-permitido.png` | OFF autenticado; 11697 pasos. |
| `F15_Seguridad/25-F15-firma-invalida.png` | Firma incorrecta rechazada; motor sigue en 11697 pasos. |
| `F15_Seguridad/26-F15-concentracion-alta.png` | 2500 ppm recibidos con lavador OFF. |
| `F15_Seguridad/27-F15-bloqueo-2500ppm.png` | OFF rechazado con lavador ON; correlacionar con registro. |
| `F13_Rele/28-F13-polaridad-incorrecta-OFF.png` | LED encendido con lavador OFF, durante corrección. |
| `F13_Rele/29-F13-polaridad-incorrecta-ON.png` | LED apagado con lavador ON, durante corrección. |
| `F13_Rele/30-F13-corregido-rele-OFF.png` | LED apagado, motor detenido. |
| `F13_Rele/31-F13-corregido-rele-ON.png` | LED encendido, motor en movimiento. |
