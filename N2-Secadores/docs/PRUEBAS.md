# Pruebas de aceptación de Secadores

| Caso | Estímulo | Criterio observado y alcance |
|---|---|---|
| F5 | Ejecutar original y corregido | En el original se repitió el timestamp. La copia corregida publicó cuatro mensajes con pasos de 2 s en el reloj del emisor. El intervalo de llegada al navegador es una magnitud distinta. |
| F6 | Variar el mando de gas en Wokwi | Se registraron control, baliza y lectura serial. Las tres consignas reales y sus errores figuran en la [calibración](CALIBRACION-MQ2.md). |
| F7 | Comparar firma vacía, lectura auténtica y una firma alterada | En la copia aislada N5 registró aceptación, rechazo y una segunda aceptación. El ensayo criptográfico independiente conservó el decimal del JSON. |
| F8 | Observar dos receptores con distinto filtro en el mismo prefijo de ensayo | Gateway singular conectado sin lectura. Gateway plural recibió y validó Secadores. |
| EXTRA-N2-01 | Conectar dos clientes propios a un broker local y observar sesiones y tópicos | Con el mismo ID se desconectó el primero. Con IDs distintos se recibieron las dos publicaciones mientras ambos seguían conectados. |
| Tiempo | Escalonar el inicio y después reiniciar conjuntamente | Se registraron −71, +2 y +1 s entre mensaje y Gateway. N5 mantuvo sus límites en las tres observaciones. |

## Reproducción de las pruebas locales

Desde la raíz del repositorio, con Python 3, compilador C++ y Node.js 20 o posterior:

```sh
python3 N2-Secadores/tests/verificar.py
node --test N2-Secadores/tests/firma.test.cjs
```

La primera prueba compara el diagrama con los pines y compila las funciones reales de conversión y baliza para verificar anclas, monotonía, límites y colores. La segunda ejecuta la función criptográfica y el generador de JSON del HTML con Web Crypto y un vector conocido.

## Cliente manual

Servir la carpeta de pruebas con `python3 -m http.server 8000 --bind 127.0.0.1 --directory N2-Secadores/tests`. Abrir `http://localhost:8000/mqtt.html`. La clave se mantiene en memoria y la vista previa conserva un decimal. Seleccionar la hora, verificar el vector, generar el JSON y publicar una sola muestra durante la ventana coordinada del grupo. Comprobar el resultado en el monitor de N5. El cliente usa WSS y descarga Paho con integridad de recurso fijada.

Los ESP32 transmitieron a HiveMQ por MQTT. El cliente web de HiveMQ devolvió `AMQJS0007E` al conectar a `broker.hivemq.com:8884`; por tanto, esa sesión no acredita recepción mediante dicho cliente. La aceptación de telemetría se verificó mediante el monitor serial de N5. [Evidencias](../../evidencias/pruebas/N2-Secadores/INDICE.md).

Se ejercitó además el botón de publicación con Paho 1.1.0 y un broker Aedes 0.51.3 por WSS de bucle local. El broker recibió una única muestra de 400 ppm, QoS 0 y retain falso. Se utilizó una clave temporal de ensayo.
