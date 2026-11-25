# N4-Lavador — Planta Costa Norte (G1)

Curso: 1ASI0572 Desarrollo de Soluciones IoT · Laboratorio 4.

- Responsable: integrante 4 (nombre pendiente de registrar).
- Revisor: integrante 3 (nombre y aprobación pendientes).
- [Wokwi heredado](https://wokwi.com/projects/481590024229204993) · [Copia de trabajo corregida](https://wokwi.com/projects/481668300518825985).
- Prefijo oficial: `upc/ga/`; pruebas aisladas: `upc/ga-n4-test-481668300518825985/`.

## Propósito y hardware

El nodo acciona el lavador de gases por orden del gateway y verifica el funcionamiento por medio de un interruptor. Implementa un modo seguro ante pérdida de contacto y evita apagados cuando no dispone de concentración baja vigente. ESP32: GPIO26 relé (HIGH activo, LOW inactivo, comprobado en simulación), GPIO14 confirmación, GPIO18 STEP, GPIO19 DIR. Se añadió LED con resistencia de 220 Ω entre el contacto NO y tierra, alimentando COM desde 3V3, para verificar la conmutación.

## Bitácora individual

La bitácora oficial debe actualizarse también en el README raíz. Los commits y aprobaciones todavía no están registrados. Los números de pasos distintos del 8 deben contrastarse con el marco de 12 pasos del curso.

| ID y nodo | Tipo | Paso del marco | Síntoma | Causa raíz | Corrección y commit | Prueba | Responsable | Revisor |
|---|---|---|---|---|---|---|---|---|
| F13 — N4-Lavador | Visible | 8: sensores y actuadores | Relé no corresponde a la orden | GPIO27 en código heredado frente a GPIO26 del circuito | GPIO26; ajuste HIGH/LOW tras prueba COM-NO; commit pendiente | LED OFF con motor detenido y ON con motor en marcha; capturas 30 y 31 | Integrante 4 | Integrante 3 |
| F14 — N4-Lavador | Silenciosa | Por confirmar: lógica de servicio y temporización | Modo seguro no se activa al perder mensajes del gateway con broker conectado | El código heredado actualizaba ultimoContacto por mqtt.connected() | Actualizar al recibir mensajes válidos; activar ON tras 60 s; commit pendiente | OFF a 56,044 s y ON a 64,391 s; capturas 04 y 05. Reproducción controlada del antes pendiente | Integrante 4 | Integrante 3 |
| F15 — N4-Lavador | Seguridad | Por confirmar: seguridad y contrato de mensajes | Órdenes no autenticadas podían actuar sobre el lavador | Manejo de comandos de texto sin verificar firma ni vigencia en código heredado | HMAC-SHA256 truncado, NTP, ventana temporal, protección contra repetición, bloqueo OFF sin concentración baja vigente; commit pendiente | 8 pruebas planificadas de la etapa de seguridad; registro serial y capturas 16–27. Regresión final tras ajuste de relé pendiente | Integrante 4 | Integrante 3 |
| F16 — N4-Lavador | Cruzada | Por confirmar: integración y contrato | Hipótesis: N5 no procesa confirmación real o emite comandos incompatibles | Pendiente de confirmar en versión actual de N5 | Acordar protocolo y verificar lazo cerrado con integrante 5; sin commit | No ejecutada con N5 real | Integrantes 4 y 5 | Integrante 3 para N4; revisión de N5 según rotación |

## Ejecución y seguridad

Abrir la copia de Wokwi, conservar topics aislados para pruebas, esperar MQTT y reloj NTP disponible. Usar la misma clave de simulación en el generador local y el firmware. No publicar claves; extraer a `secrets.h` (ignorado por Git) y proporcionar `secrets.example.h` sin valores reales. No utilizar el prefijo compartido hasta coordinar con N5. La firma de órdenes y concentración aquí probada es una extensión del contrato, no un acuerdo ya confirmado por el grupo. El broker público y la clave compartida no constituyen seguridad de producción.

## Documentación y evidencias

- [Resultados de pruebas](docs/PRUEBAS.md)
- [Contrato propuesto e integración](docs/INTEGRACION-N5.md)
- [Medición del lazo cerrado](docs/MEDICION-LAZO-CERRADO.md)
- [Sustentación](docs/SUSTENTACION.md)
- [Índice de evidencias](../evidencias/pruebas/N4-Lavador/INDICE.md) (extraer el ZIP de evidencias en esa ubicación y ocultar credenciales antes de publicar).

Estado: F13, F14 y F15 cuentan con pruebas de sus correcciones; F16, regresión final, medición integral y revisión Git pendientes. No se declara completa la instalación integrada.

## Medición requerida del lazo cerrado


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

## Entrega y revisión

Registrar F13–F16 en Trello/Jira con responsable y estado; solicitar revisión independiente al integrante 3. Las ramas propuestas son `bugfix/lavador-pin-rele`, `bugfix/lavador-modo-seguro`, `bugfix/lavador-autenticacion-hmac` y `bugfix/lavador-integracion-gateway` (esta última pendiente de diagnóstico). El integrante 4 revisa N5. Guardar capturas reales del tablero al cierre de cada sesión.

Fuente: enunciado Laboratorio 4, secciones 3, 4, 6, 7 y 9; diapositivas 5, 8, 9 y 10.
