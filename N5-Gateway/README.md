# N5-Gateway

Responsable: Gonzalo Aliaga, integrante 5 (integrador). Revisor: integrante 4.

- [Wokwi original](https://wokwi.com/projects/481590098239821825)
- Copia corregida: por registrar.

## Qué hace el gateway

Recibe los datos firmados de pozas, secadores y perímetro, decide en la capa fog si enciende el lavador (al menos 2 nodos sobre 2000 ppm y viento entre 90 y 180 grados), lo apaga bajo 1600 ppm, vigila que el lavador confirme en 10 s y, si no confirma, alerta y lleva la ventilación de las pozas al máximo. Muestra el estado general en el semáforo NeoPixel (verde, ámbar o rojo), el pulsador reconoce la alerta y cada 20 s envía los datos a ThingSpeak.

## Fallas encontradas

| ID | Tipo | Síntoma | Causa raíz | Rama |
|---|---|---|---|---|
| F17 | Visible | El gateway no muestra ningún dato y el semáforo queda siempre verde | Suscripción a `upc/ga/+/dato`; el contrato y los nodos usan `datos` (misma causa que F8 de N2) | `bugfix/gateway-topico-datos` |
| F18 | Silenciosa | Si un nodo se desconecta, el gateway sigue usando su último valor y el lavador puede quedarse encendido para siempre | Las lecturas no tenían vigencia: se guardaba el último valor sin hora de llegada | `bugfix/gateway-datos-vencidos` |
| F19 | Seguridad | Un mensaje firmado antiguo, reenviado desde el broker, se acepta como nuevo; una alerta falsa en `lavador/alerta` dispara `VENT:MAX` | Faltaba `configTime` (NTP): `time()` contaba segundos desde el arranque y `ahora - ts` era negativo, así el filtro de 60 s nunca rechazaba. Tampoco había control de repetición ni validación de la alerta | `bugfix/gateway-ntp-antirreplay` |
| F20 | Cruzada | N4 rechaza todas las órdenes del gateway y a los 60 s entra en modo seguro | El gateway enviaba `ON`, `OFF` y la concentración en texto plano (y con retain); N4 exige `gateway\|ts\|contenido\|firma` | `bugfix/gateway-ordenes-firmadas` |

Las ramas están apiladas en ese orden (cada una parte de la anterior), así los PR se integran uno tras otro sin conflictos.

## Configuración en Wokwi

1. En la copia de Wokwi, reemplazar `sketch.ino` por el de este directorio.
2. Crear una pestaña nueva `secrets.h` con el contenido de `secrets.example.h` y la clave HMAC del grupo (la misma de los cinco nodos). Ese archivo no se sube al repositorio.
3. Ejecutar y verificar en el monitor serie: `NTP sincronizado` y `Gateway conectado`.

## Contrato que usa el gateway

| Dirección | Tópico | Formato |
|---|---|---|
| Recibe | `upc/ga/{pozas,secadores,perimetro}/datos` | JSON del contrato; `valor` numérico; perímetro agrega `"viento"` |
| Recibe | `upc/ga/lavador/estado` | JSON de N4; confirma con `"CONFIRMADO"` o `"confirmado":true` |
| Recibe | `upc/ga/lavador/alerta` | `{"alerta":"falla del lavador"}` (solo se acepta con el lavador encendido) |
| Envía | `upc/ga/lavador/cmd` | `gateway\|ts\|cmd:ON\|firma` y `cmd:OFF` |
| Envía | `upc/ga/gateway/conc` | `gateway\|ts\|conc:2500.0\|firma` cada 2 s |
| Envía | `upc/ga/pozas/cmd` | `gateway\|ts\|cmd:VENT:MAX\|firma` y `cmd:VENT:NORMAL` |
| Envía | `upc/ga/gateway/alerta` | `{"alerta":"falla del lavador"}` |

Firma: HMAC-SHA256 de `gateway|ts|contenido`, primeros 16 caracteres hexadecimales (mismo formato que acordó N4 en [INTEGRACION-N5](../N4-Lavador/docs/INTEGRACION-N5.md)).

## Pruebas

Abrir [tests/n5_simulador_nodos.html](tests/n5_simulador_nodos.html) servido por HTTP local (por ejemplo `python -m http.server` dentro de `tests/` y abrir `http://localhost:8000/n5_simulador_nodos.html`). Simula N1, N2, N3 y N4 con mensajes firmados y lanza ataques desde el broker.

| Prueba | Pasos en el simulador | Salida esperada en el monitor serie del gateway |
|---|---|---|
| F17 | Publicar cualquier nodo | `OK pozas: 400.0 ppm` (antes: nada) |
| F18 | Pico, luego desmarcar "publicar cada 2 s" en dos nodos | A los 10 s `Nodos con datos vigentes: 1`; el lavador ya no cuenta esos valores |
| F19 | Dato firmado de hace 120 s / Replay / Alerta falsa con lavador apagado | `RECHAZADO (mensaje antiguo)`, `RECHAZADO (mensaje repetido)`, `ALERTA IGNORADA` |
| F20 | Pico con viento a la comunidad | En HiveMQ `gateway\|ts\|cmd:ON\|firma`; N4 lo acepta y confirma |
| R5 | Pico con la confirmación automática desmarcada | A los 10 s `ALERTA: falla del lavador` y `cmd:VENT:MAX` en `pozas/cmd` |
| Lazo cerrado | Pico con confirmación automática | `[LAZO] Lavador confirmado en N ms desde la deteccion` (medición antes y después) |

## Pendiente con otros nodos

- N1 debe publicar `valor` como número sin "ppm" ni comillas; hoy el gateway rechaza su firma. También debe aceptar solo órdenes firmadas del gateway (`cmd:VENT:MAX`, `cmd:VENT:NORMAL`).
- N3 debe mantener el campo `"viento"` y convertir el ADC del ESP32 con la escala 0 a 4095.
- N4 debe volver del prefijo de pruebas `upc/ga-n4-test-…` a `upc/ga/`.
- Todos: misma clave HMAC en `secrets.h`.
- Riesgo residual: los estados y alertas de N4 no están firmados; el gateway solo acepta una alerta si coincide con su propio estado.
