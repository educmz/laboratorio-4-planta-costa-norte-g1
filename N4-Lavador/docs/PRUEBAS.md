# Registro de pruebas — N4-Lavador

## Información general

- Responsable: Fabricio Fabian Quispe Barzola.
- Grupo: G1.
- Plataforma: Wokwi.
- Microcontrolador: ESP32.
- Comunicación: MQTT mediante HiveMQ.
- Instalación heredada: `bbc3ce1`.
- Correcciones publicadas: `21927c9`, `a69050e` y `5388fdd`.

## Objetivo

Verificar el funcionamiento individual del lavador mediante pruebas de actuación, temporización, confirmación y seguridad.

Las pruebas se realizaron utilizando mensajes MQTT en un entorno aislado, simulando la comunicación del gateway.

## Resultados experimentales

| ID | Regla | Prueba | Resultado observado |
|---|---|---|---|
| N4-P01 | R4 | Activación del relé | LED encendido y motor funcionando |
| N4-P02 | R4 | Desactivación del relé | LED apagado y motor detenido |
| N4-P03 | R6 | Pérdida de contacto | Activación del modo seguro tras superar 60 s |
| N4-P04 | R5 | Ausencia de confirmación | Generación de alerta local y MQTT |
| N4-P05 | R5 | Confirmación de funcionamiento | Confirmación registrada localmente |
| N4-P06 | R5 | Pérdida de confirmación | Pérdida y recuperación detectadas |
| N4-P07 | R10 | Comando sin firma | Orden rechazada |
| N4-P08 | R10 | Firma incorrecta | Orden rechazada |
| N4-P09 | R9 | Timestamp fuera del margen probado | Orden rechazada |
| N4-P10 | R10 | Orden ON autenticada | Lavador activado |
| N4-P11 | R10 | Repetición de mensaje | Repetición rechazada durante la ejecución |
| N4-P12 | R7 | OFF sin concentración baja válida | Apagado bloqueado |
| N4-P13 | R4/R7 | Concentración 1500 ppm y OFF autenticado | Apagado permitido |
| N4-P14 | R7 | Concentración 2500 ppm y OFF autenticado | Apagado rechazado |
| N4-P15 | R8 | Publicación periódica MQTT | Publicaciones observadas |

## F13 — Control del relé

**Commit:** `21927c9`.

La corrección del GPIO y de la polaridad permitió obtener una correspondencia entre el estado lógico del lavador y la conmutación física simulada.

| Estado | LED | Motor |
|---|---|---|
| OFF | Apagado | Detenido |
| ON | Encendido | Funcionando |

### Evidencias

- [GPIO incorrecto en el firmware heredado](../../evidencias/pruebas/N4-Lavador/F13_Rele/01-F13-heredado-GPIO27.png).
- [Relé corregido OFF](../../evidencias/pruebas/N4-Lavador/F13_Rele/30-F13-corregido-rele-OFF.png).
- [Relé corregido ON](../../evidencias/pruebas/N4-Lavador/F13_Rele/31-F13-corregido-rele-ON.png).

## F14 — Modo seguro

**Commit:** `a69050e`.

Se observó el comportamiento del lavador sin recepción de órdenes del gateway.

| Tiempo de simulación | Estado |
|---|---|
| 35,686 s | OFF |
| 56,044 s | OFF |
| 64,391 s | ON mediante modo seguro |

La activación preventiva se produjo después de superar el umbral configurado de 60 segundos.

### Evidencias

- [Estado anterior al umbral](../../evidencias/pruebas/N4-Lavador/F14_Modo_seguro/04-F14-corregido-antes-60s.png).
- [Modo seguro activado](../../evidencias/pruebas/N4-Lavador/F14_Modo_seguro/05-F14-corregido-modo-seguro.png).

## F15 — Seguridad MQTT

**Commit:** `5388fdd`.

Se evaluó la respuesta del firmware frente a diferentes tipos de mensajes.

| Entrada | Resultado |
|---|---|
| ON sin firma | Rechazado |
| Firma incorrecta | Rechazada |
| Timestamp inválido | Rechazado |
| ON autenticado | Aceptado |
| Mensaje repetido | Rechazado |
| OFF sin concentración baja válida | Rechazado |
| OFF autenticado con 1500 ppm | Aceptado |
| OFF autenticado con 2500 ppm | Rechazado |

En la prueba de apagado permitido, el motor permaneció detenido en 11697 pasos.

La protección contra mensajes repetidos fue observada durante una misma ejecución del firmware.

### Evidencias

- [Pruebas de seguridad](../../evidencias/pruebas/N4-Lavador/F15_Seguridad/).
- [Registro serial](../../evidencias/pruebas/N4-Lavador/F15_Seguridad/Registro-serial-F15-completo.md).

## Confirmación y alertas

El interruptor conectado al GPIO14 permitió verificar la detección de confirmación, pérdida de confirmación y recuperación.

| Evento | Tiempo local observado |
|---|---|
| Confirmación tras activación preventiva | 25 ms |
| Confirmación registrada | 4 ms |
| Recuperación de confirmación | 12 ms |

[Evidencias de confirmación y alertas](../../evidencias/pruebas/N4-Lavador/Confirmacion_alertas/).

## Comunicación MQTT y NTP

Se verificó la comunicación entre el ESP32 y el cliente MQTT externo.

Durante el diagnóstico se observaron desconexiones y diferencias entre el reloj del navegador y el reloj del ESP32. La implementación incorpora mecanismos de reconexión y sincronización temporal.

[Evidencias MQTT y NTP](../../evidencias/pruebas/N4-Lavador/MQTT_NTP/).

## Alcance experimental

Los resultados corresponden a pruebas individuales realizadas durante las distintas etapas de corrección del nodo N4.

Las pruebas de seguridad utilizaron un gateway simulado. Las mediciones de confirmación corresponden a eventos locales y no representan la latencia integral de los cinco nodos.

## Conclusión

Los resultados experimentales demuestran el funcionamiento de las correcciones del relé, la activación preventiva del modo seguro y los mecanismos de validación de órdenes MQTT.

La documentación y las evidencias permiten reproducir los escenarios utilizados durante el diagnóstico y verificar el comportamiento individual del lavador.