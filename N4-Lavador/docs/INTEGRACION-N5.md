# Contrato de integración — N4-Lavador y N5-Gateway

## Objetivo

Definir la comunicación entre N4-Lavador y N5-Gateway para el intercambio de órdenes autenticadas, concentraciones, estados, confirmaciones y alertas.

## Arquitectura

N5-Gateway centraliza las decisiones de activación del lavador.

N4-Lavador ejecuta las órdenes autorizadas, controla los actuadores y comunica su estado de funcionamiento.

La comunicación utiliza el protocolo MQTT mediante un broker compartido.

## Configuración

| Parámetro | Valor |
|---|---|
| Broker | broker.hivemq.com |
| Puerto MQTT | 1883 |
| Puerto WebSocket | 8884 |
| Prefijo oficial | upc/ga/ |
| Prefijo experimental | upc/ga-n4-test-481668300518825985/ |

## Topics

| Topic | Publicador | Suscriptor |
|---|---|---|
| upc/ga/lavador/cmd | N5 | N4 |
| upc/ga/gateway/conc | N5 | N4 |
| upc/ga/lavador/estado | N4 | N5 |
| upc/ga/lavador/alerta | N4 | N5 |

## Formato de órdenes autenticadas

La interfaz implementada en N4 utiliza mensajes firmados mediante HMAC-SHA256.

### Activación

`gateway|timestamp|cmd:ON|firma`

### Desactivación

`gateway|timestamp|cmd:OFF|firma`

### Regulación de intensidad

`gateway|timestamp|cmd:INT:5|firma`

La intensidad admite valores del 1 al 5.

### Concentración

`gateway|timestamp|conc:2500.0|firma`

La concentración se expresa en partes por millón.

## Generación de firma

La firma se calcula mediante HMAC-SHA256 utilizando una clave compartida y los tres primeros campos exactos del mensaje.

Se utilizan los primeros 16 caracteres hexadecimales del resultado.

El timestamp se expresa en segundos Unix.

La implementación contempla:

- Antigüedad máxima de 60 segundos.
- Tolerancia de adelanto de 5 segundos.
- Validación de autenticidad.
- Control de timestamps repetidos durante la ejecución.

El archivo secrets.h contiene la configuración privada utilizada en el entorno de simulación.

Este formato corresponde a la interfaz de órdenes implementada en N4 y no sustituye el contrato JSON de los sensores definido por el enunciado.

## Estados del lavador

Ejemplo de estado periódico:

```json
{
  "nodo": "lavador",
  "lavador": "ON",
  "intensidad": 1,
  "modoSeguro": false,
  "confirmado": false
}
```

Ejemplo de confirmación:

```json
{
  "lavador": "CONFIRMADO"
}
```

Ejemplo de alerta:

```json
{
  "alerta": "falla del lavador"
}
```

Ejemplo de pérdida de contacto:

```json
{
  "alerta": "sin contacto con gateway"
}
```

La interfaz distingue entre la activación ordenada y la confirmación efectiva.

Los mensajes de estado y alerta observados utilizan JSON sin firma HMAC; la autenticación implementada en esta versión se concentra en las órdenes y concentraciones recibidas por N4.

## Reglas de funcionamiento

| Regla | Comportamiento especificado |
|---|---|
| R4 | Activación bajo condiciones de concentración y viento; apagado bajo 1600 ppm |
| R5 | Confirmación en máximo 10 s y generación de alerta ante ausencia de confirmación |
| R6 | Activación preventiva después de 60 s sin contacto |
| R7 | Bloqueo de apagado con concentración elevada |
| R8 | Publicación periódica de estados |
| R9 | Validación de autenticidad y vigencia |
| R10 | Procesamiento de órdenes autorizadas |

## Secuencia de integración

1. Los nodos sensores publican sus mediciones.
2. N5 evalúa las concentraciones y dirección del viento.
3. N5 emite una orden autenticada.
4. N4 valida el mensaje.
5. N4 activa el relé y el motor.
6. El sensor de confirmación detecta el funcionamiento.
7. N4 comunica la confirmación.
8. N5 utiliza el estado recibido para gestionar el lazo de control.

Esta secuencia describe el comportamiento especificado del sistema integrado.

## Validación experimental de la interfaz

Durante las pruebas individuales se utilizaron mensajes externos para simular las órdenes del gateway.

Se observó la aceptación de órdenes autenticadas, rechazo de mensajes inválidos, recepción de concentraciones y publicación de estados y alertas.

## Conclusión

El contrato documentado establece la interfaz de comunicación de N4-Lavador y describe su relación funcional con N5-Gateway.

La implementación proporciona mecanismos de autenticación, control del actuador, confirmación y respuesta preventiva ante pérdida de comunicación.