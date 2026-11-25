# Medición antes y después del lazo cerrado

El enunciado exige medir desde la detección hasta la confirmación del lavador, antes y después. El plan grupal `docs/pruebas.md` solicita fecha, versión, t0, t1, diferencia y evidencia. La confirmación local no es la medición integral.

| Etapa | Fecha | Versión/commit | t0 detección | t1 confirmación | t1−t0 | Evidencia |
|---|---|---|---|---|---|---|
| Antes (heredado) | Pendiente | Pendiente | No medido | No medido | No disponible | Reproducir escenario original sin alterar proyecto fuente |
| Después (integrado) | Pendiente | Pendiente | No medido | No medido | No disponible | Ejecutar N1–N5 coordinadamente |

## Observaciones locales disponibles

- Se observó confirmación local de 25 ms en monitor serial, tras activación por modo seguro; no corresponde a detección en sensores.
- Se observaron confirmación de 4 ms y recuperación en 12 ms, con pérdida intermedia. No son mediciones de latencia integral ni sustituyen el antes/después.
- La alerta por ausencia de confirmación se observó localmente y por publicación MQTT; la actuación N1 ordenada por N5 sigue pendiente.

## Protocolo de medición

Usar las mismas concentraciones (dos nodos por encima de 2000 ppm), dirección de viento (135°), posición inicial y procedimiento de confirmación en ambas versiones. Registrar t0 al detectar el escenario, t1 cuando se recibe/observa confirmación, en una referencia temporal común y documentada. Si no se confirma, registrar fallo o tiempo de espera observado, no inventar t1. Guardar logs/capturas, versión y condiciones. Medir adicionalmente desde orden ON hasta confirmación para comprobar R5 (máximo 10 s), distinguiéndolo de la latencia integral. Considerar el desfase entre simulaciones Wokwi y no restar relojes no comparables.
