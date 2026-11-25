# Sustentación individual — N4-Lavador

## Información general

- Responsable: Fabricio Fabian Quispe Barzola.
- Grupo: G1.
- Rol: Integrante 4.
- Nodo: N4-Lavador.
- Curso: Desarrollo de Soluciones IoT.
- Laboratorio 4: Instalación heredada.

## Objetivo

Diagnosticar y corregir las fallas funcionales y de seguridad del nodo responsable del lavado de gases de la Planta Costa Norte.

## Falla más difícil: F15

### Síntoma

El lavador aceptaba órdenes MQTT sin comprobar su autenticidad.

### Causa raíz

El firmware heredado procesaba comandos de texto sin validar su firma ni vigencia temporal.

### Corrección

Se implementaron mecanismos de autenticación HMAC-SHA256, sincronización NTP, validación temporal, protección contra mensajes repetidos y restricciones de apagado.

### Resultados

- Rechazo de órdenes sin autenticación.
- Rechazo de firmas incorrectas.
- Aceptación de órdenes autenticadas.
- Rechazo de mensajes repetidos.
- Bloqueo de apagado con concentración elevada.

Commit: `5388fdd`.

## Guion de exposición — 25 segundos

Mi responsabilidad fue N4-Lavador. La falla más compleja fue F15, porque el sistema aceptaba órdenes MQTT sin autenticación. Identifiqué que el firmware no verificaba la firma ni la vigencia de los mensajes. Implementé HMAC-SHA256, NTP y protección contra repetición. Finalmente, comprobé el rechazo de órdenes inválidas y el bloqueo del apagado con concentración elevada.

## Resumen de correcciones

### F13 — Relé

Se corrigió GPIO27 por GPIO26 y se ajustó la polaridad del relé.

La incorporación de un LED permitió comprobar visualmente la conmutación del contacto normalmente abierto.

### F14 — Modo seguro

Se corrigió la lógica de último contacto para activar preventivamente el lavador después de 60 segundos sin mensajes del gateway.

### F15 — Seguridad

Se incorporaron mecanismos de autenticación y validación temporal para controlar las órdenes MQTT.

### F16 — Diseño de integración

Se documentó la interfaz de comunicación de N4 con N5, diferenciando la activación ordenada de la confirmación efectiva.

## Preguntas técnicas

### ¿Por qué mqtt.connected() no era suficiente?

Porque indica que existe conexión con el broker, pero no demuestra que el gateway se encuentre enviando mensajes.

### ¿Por qué se utiliza HMAC-SHA256?

Para verificar la autenticidad e integridad de los mensajes mediante una clave compartida.

### ¿Cuál es la función de NTP?

Proporcionar una referencia temporal para comprobar la vigencia de los mensajes recibidos.

### ¿Qué sucede después de 60 segundos sin contacto?

El lavador activa el modo seguro como medida preventiva.

### ¿Qué ocurre si se recibe una orden OFF con 2500 ppm?

El firmware rechaza el apagado debido a la condición de concentración elevada.

### ¿Cómo se verifica la actuación del relé?

Mediante un LED conectado al contacto normalmente abierto y la observación del funcionamiento del motor.

### ¿Cuál es la diferencia entre activación y confirmación?

La activación corresponde a la orden de funcionamiento. La confirmación corresponde a la respuesta del mecanismo de realimentación.

## Referencias

- [Instalación heredada](https://wokwi.com/projects/481590024229204993).
- [Instalación corregida](https://wokwi.com/projects/481668300518825985).
- [Pruebas realizadas](PRUEBAS.md).
- [Evidencias](../../evidencias/pruebas/N4-Lavador/).

## Conclusión

Las correcciones implementadas mejoran el control del actuador, la tolerancia ante pérdida de comunicación y la seguridad de las órdenes MQTT.

El trabajo desarrollado se encuentra respaldado mediante pruebas individuales, evidencias experimentales, commits y pull requests publicados en el repositorio del Grupo G1.