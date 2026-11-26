# Convivencia de dos clientes MQTT

El 26 de noviembre de 2026 se ejecutó un broker Aedes 0.51.3 en la interfaz de bucle local y tres clientes Paho 2.2.0. Se desactivó la reconexión automática. Con el mismo identificador, la llegada de la segunda sesión provocó la notificación de desconexión de la primera. Con identificadores diferentes, un tercer cliente recibió una muestra de Secadores y otra de Perímetro mientras ambos emisores seguían conectados.

Salida de las aserciones sobre las sesiones y los mensajes recibidos:

```text
Dos clientes de prueba con ga-sensor: el primero perdió la sesión.
IDs distintos: ambas sesiones activas. Suscriptor recibió upc/ga/perimetro/datos, upc/ga/secadores/datos.
```

El ensayo aísla el mecanismo del protocolo. La observación previa con los nodos del grupo está en el [registro integrado](../../N2-Secadores.md).
