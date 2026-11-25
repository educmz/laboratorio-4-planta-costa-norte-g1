# Evidencias N5-Gateway

Responsable: Gonzalo Aliaga. Fecha: 2026-11-25.
Copia corregida en Wokwi: https://wokwi.com/projects/481701580182887425

Entorno: gateway corregido en Wokwi (izquierda) y [simulador de nodos](../../../N5-Gateway/tests/n5_simulador_nodos.html) publicando en `upc/ga/#` de HiveMQ (derecha). El simulador reemplaza a N1, N2, N3 y N4 con mensajes firmados según el contrato.

| Captura | Falla o regla | Qué demuestra |
|---|---|---|
| [01](01-F17-datos-recibidos.png) | F17 | Con la suscripción a `upc/ga/+/datos` el gateway recibe y valida los datos: `OK pozas: 400.0 ppm` |
| [02](02-F20-deteccion-orden-on-firmada.png) | F20, R4 | 3 nodos sobre 2000 ppm y viento 135°: `[LAZO] Deteccion … Orden: lavador ON`; en el broker `gateway\|ts\|cmd:ON\|firma`; semáforo rojo |
| [03](03-lazo-cerrado-confirmado-2582ms.png) | R5, lazo cerrado | Confirmación del lavador recibida: `[LAZO] Lavador confirmado en 2582 ms desde la deteccion` |
| [04](04-lavador-off.png) | R4 | Concentración bajo 1600 ppm: `Orden: lavador OFF`, semáforo verde |
| [05](05-R5-sin-confirmacion-alerta-ventmax.png) | R5 | Sin confirmación en 10 s: `ALERTA: falla del lavador`, publica `gateway/alerta` y `cmd:VENT:MAX` firmado a pozas |
| [06](06-R5-alerta-reconocida-operador.png) | Pulsador | `Alerta reconocida por el operador` |
| [07](07-R5-lavador-off-vent-normal.png) | R4, R5 | Al bajar la concentración: `cmd:OFF` y `cmd:VENT:NORMAL` firmados |
| [08](08-F18-nodos-caidos-sin-vigencia.png) | F18 | Secadores y perímetro dejan de publicar: `Nodos con datos vigentes: 1 (pozas si, secadores NO, perimetro NO)` |
| [09](09-F19-firma-invalida-rechazada.png) | F19, R9 | Mensajes firmados con otra clave: `RECHAZADO (firma invalida)` |
| [10](10-F19-desfase-reloj-wokwi.png) | F19 | Hallazgo durante la prueba: el reloj simulado se atrasa y los datos válidos se rechazaban como `marca de tiempo en el futuro`. Corregido resincronizando NTP cada 15 s (`04b0d77`) |
| [11](11-F19-mensaje-antiguo-y-alerta-falsa-rechazados.png) | F19, R9 | Dato firmado de hace 120 s: `RECHAZADO (mensaje antiguo)`; alerta falsa con el lavador apagado: `ALERTA IGNORADA` |
| [12](12-F18-solo-pozas-vigente.png) | F18 | Con secadores y perímetro caídos, solo pozas cuenta: `Nodos con datos vigentes: 1` y la concentración publicada (`conc:400.0`) ya no usa los valores viejos |

## Medición del lazo cerrado

| Etapa | Versión | Resultado |
|---|---|---|
| Antes | Código original (`5b1a535`) | Sin confirmación: el gateway no recibía datos (F17) y sus órdenes sin firma eran rechazadas por N4 (F20) |
| Después | `docs/gateway-n5` | 2582 ms desde la detección hasta la confirmación (N4 simulado con confirmación a 3 s) |

## Pendiente

- Prueba de integración con los nodos reales N1 a N4 (requiere que N1 publique `valor` numérico, N3 corrija la escala del viento y N4 use el prefijo `upc/ga/`).
