# Evidencias — N4-Lavador

## Información general

- Grupo: G1.
- Responsable: Fabricio Fabian Quispe Barzola.
- Nodo: N4-Lavador.
- Plataforma: Wokwi e HiveMQ.
- Total de capturas: 30.

## F13 — Corrección del relé

Commit: `21927c9`.

| Evidencia | Descripción |
|---|---|
| [01](F13_Rele/01-F13-heredado-GPIO27.png) | GPIO27 declarado en firmware heredado |
| [28](F13_Rele/28-F13-polaridad-incorrecta-OFF.png) | Diagnóstico intermedio de polaridad OFF |
| [29](F13_Rele/29-F13-polaridad-incorrecta-ON.png) | Diagnóstico intermedio de polaridad ON |
| [30](F13_Rele/30-F13-corregido-rele-OFF.png) | Relé corregido en estado OFF |
| [31](F13_Rele/31-F13-corregido-rele-ON.png) | Relé corregido en estado ON |

## F14 — Modo seguro

Commit: `a69050e`.

| Evidencia | Descripción |
|---|---|
| [03](F14_Modo_seguro/03-F14-corregido-OFF-35s.png) | Lavador apagado a 35,686 s |
| [04](F14_Modo_seguro/04-F14-corregido-antes-60s.png) | Lavador apagado a 56,044 s |
| [05](F14_Modo_seguro/05-F14-corregido-modo-seguro.png) | Modo seguro activado a 64,391 s |

## F15 — Seguridad

Commit: `5388fdd`.

| Evidencia | Descripción |
|---|---|
| [16](F15_Seguridad/16-F15-despues-orden-rechazada.png) | Rechazo de orden sin firma |
| [17](F15_Seguridad/17-F15-HiveMQ-ON-sin-firma-modo-seguro.png) | Orden inválida y activación preventiva |
| [21](F15_Seguridad/21-F15-firma-valida.png) | Orden autenticada aceptada |
| [22](F15_Seguridad/22-F15-apagado-bloqueado.png) | Restricción de apagado |
| [23](F15_Seguridad/23-F15-concentracion-baja.png) | Recepción de 1500 ppm |
| [24](F15_Seguridad/24-F15-apagado-permitido.png) | Apagado autenticado permitido |
| [25](F15_Seguridad/25-F15-firma-invalida.png) | Firma incorrecta rechazada |
| [26](F15_Seguridad/26-F15-concentracion-alta.png) | Recepción de 2500 ppm |
| [27](F15_Seguridad/27-F15-bloqueo-2500ppm.png) | Apagado rechazado con concentración elevada |

[Registro serial de seguridad](F15_Seguridad/Registro-serial-F15-completo.md).

## Confirmación y alertas

| Evidencia | Descripción |
|---|---|
| [06](Confirmacion_alertas/06-N4-alerta-sin-confirmacion.png) | Alerta por ausencia de confirmación |
| [07](Confirmacion_alertas/07-N4-alerta-local-sin-conexion.png) | Alerta local sin conexión |
| [08](Confirmacion_alertas/08-N4-confirmacion-perdida.png) | Pérdida de confirmación |
| [09](Confirmacion_alertas/09-N4-confirmacion-local-recuperada.png) | Recuperación de confirmación |
| [12](Confirmacion_alertas/12-N4-confirmacion-local-sostenida.png) | Confirmación local sostenida |
| [13](Confirmacion_alertas/13-N4-confirmacion-local-sin-internet.png) | Confirmación local sin internet |

## Comunicación MQTT y NTP

| Evidencia | Descripción |
|---|---|
| [10](MQTT_NTP/10-HiveMQ-configuracion-inicial.png) | Configuración inicial de HiveMQ |
| [11](MQTT_NTP/11-HiveMQ-conectado-suscripcion.png) | Conexión y suscripción MQTT |
| [14](MQTT_NTP/14-N4-reconexion-MQTT.png) | Reconexión MQTT |
| [15](MQTT_NTP/15-N4-comunicacion-MQTT.png) | Comunicación MQTT con cliente externo |
| [18](MQTT_NTP/18-HiveMQ-configuracion-reinicio.png) | Configuración durante reinicio |
| [19](MQTT_NTP/19-N4-diagnostico-desfase-reloj.png) | Diagnóstico del reloj |

## Diagnóstico

| Evidencia | Descripción |
|---|---|
| [20](Diagnostico/20-Generador-HMAC-timestamp-invalido.png) | Diagnóstico de timestamp inválido |

## Resumen de evidencias

| Categoría | Cantidad |
|---|---:|
| F13 — Relé | 5 |
| F14 — Modo seguro | 3 |
| F15 — Seguridad | 9 |
| Confirmación y alertas | 6 |
| MQTT y NTP | 6 |
| Diagnóstico | 1 |
| **Total** | **30** |

## Alcance

Las evidencias documentan las distintas etapas de diagnóstico y corrección del nodo N4-Lavador.

Las capturas 28 y 29 corresponden al proceso intermedio de corrección de polaridad.

Los resultados de seguridad incluyen pruebas realizadas mediante mensajes externos que simulan la comunicación del gateway.

El registro serial complementa las evidencias gráficas de autenticación, repetición de mensajes y restricciones de apagado.

## Conclusión

Las 30 capturas y los registros experimentales respaldan las correcciones individuales de F13, F14 y F15 y permiten visualizar el comportamiento funcional del nodo N4-Lavador.