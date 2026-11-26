# Registros seriales del 26 de noviembre de 2026

Extractos consecutivos del monitor en las corridas indicadas. Las líneas de arranque de la ROM se omiten.

## Original con transporte aislado

```text
{"id":"secadores","ts":1795724795,"valor":8859.6,"unidad":"ppm","temp":24.0,"hum":40.0,"equipo":"Secador rotatorio SR-01","zona":"Linea de secado de harina - Planta Chimbote","responsable":"Jefe de turno de produccion","calidad":"ok","firma":""}
{"id":"secadores","ts":1795724795,"valor":8859.6,"unidad":"ppm","temp":24.0,"hum":40.0,"equipo":"Secador rotatorio SR-01","zona":"Linea de secado de harina - Planta Chimbote","responsable":"Jefe de turno de produccion","calidad":"ok","firma":""}
{"id":"secadores","ts":1795724796,"valor":8859.6,"unidad":"ppm","temp":24.0,"hum":40.0,"equipo":"Secador rotatorio SR-01","zona":"Linea de secado de harina - Planta Chimbote","responsable":"Jefe de turno de produccion","calidad":"ok","firma":""}
{"id":"secadores","ts":1795724796,"valor":8859.6,"unidad":"ppm","temp":24.0,"hum":40.0,"equipo":"Secador rotatorio SR-01","zona":"Linea de secado de harina - Planta Chimbote","responsable":"Jefe de turno de produccion","calidad":"ok","firma":""}
{"id":"secadores","ts":1795724796,"valor":8859.6,"unidad":"ppm","temp":24.0,"hum":40.0,"equipo":"Secador rotatorio SR-01","zona":"Linea de secado de harina - Planta Chimbote","responsable":"Jefe de turno de produccion","calidad":"ok","firma":""}
```

## N2 corregido publicado

```text
Conectando WiFi...
WiFi conectado
MQTT conectado
Client ID: ga-secadores-24:0A:C4:00:01:10
================================
Nodo N2 Secadores inicializado
Periodo: 2000 ms
Calibracion MQ-2: 7 puntos
Seguridad: HMAC-SHA256
================================
ADC MQ-2: 3628
Gas calibrado: 393.0 ppm
{"id":"secadores","ts":1795724593,"valor":393.0,"unidad":"ppm","temp":24.0,"hum":40.0,"equipo":"Secador rotatorio SR-01","zona":"Linea de secado de harina - Planta Chimbote","responsable":"Jefe de turno de produccion","calidad":"ok","firma":"d0960dad43273d3c"}
Publicacion MQTT exitosa
--------------------------------
ADC MQ-2: 3636
Gas calibrado: 404.7 ppm
{"id":"secadores","ts":1795724595,"valor":404.7,"unidad":"ppm","temp":24.0,"hum":40.0,"equipo":"Secador rotatorio SR-01","zona":"Linea de secado de harina - Planta Chimbote","responsable":"Jefe de turno de produccion","calidad":"ok","firma":"1930bee9004c0c42"}
Publicacion MQTT exitosa
--------------------------------
ADC MQ-2: 3861
Gas calibrado: 2500.0 ppm
{"id":"secadores","ts":1795724597,"valor":2500.0,"unidad":"ppm","temp":24.0,"hum":40.0,"equipo":"Secador rotatorio SR-01","zona":"Linea de secado de harina - Planta Chimbote","responsable":"Jefe de turno de produccion","calidad":"ok","firma":"0dd1e091663e19ce"}
Publicacion MQTT exitosa
--------------------------------
ADC MQ-2: 3831
Gas calibrado: 1857.1 ppm
{"id":"secadores","ts":1795724599,"valor":1857.1,"unidad":"ppm","temp":24.0,"hum":40.0,"equipo":"Secador rotatorio SR-01","zona":"Linea de secado de harina - Planta Chimbote","responsable":"Jefe de turno de produccion","calidad":"ok","firma":"50749ea77afcc07d"}
Publicacion MQTT exitosa
--------------------------------
```

## Candidato tras arranque conjunto

```text
Conectando WiFi...
WiFi conectado
MQTT conectado
Client ID: ga-n2-qa-c-24:0A:C4:00:01:10
================================
Nodo N2 Secadores inicializado
Periodo: 2000 ms
Calibracion MQ-2: 7 puntos
Seguridad: HMAC-SHA256
================================
ADC MQ-2: 3628
Gas calibrado: 393.0 ppm
{"id":"secadores","ts":1795725001,"valor":393.0,"unidad":"ppm","temp":24.0,"hum":40.0,"equipo":"Secador rotatorio SR-01","zona":"Linea de secado de harina - Planta Chimbote","responsable":"Jefe de turno de produccion","calidad":"ok","firma":"98a251abcc17eb26"}
Publicacion MQTT exitosa
--------------------------------
ADC MQ-2: 3628
Gas calibrado: 393.0 ppm
{"id":"secadores","ts":1795725003,"valor":393.0,"unidad":"ppm","temp":24.0,"hum":40.0,"equipo":"Secador rotatorio SR-01","zona":"Linea de secado de harina - Planta Chimbote","responsable":"Jefe de turno de produccion","calidad":"ok","firma":"f1f549fd3bdd7f08"}
Publicacion MQTT exitosa
--------------------------------
```

## N5 con el candidato, firma alterada y recuperación

```text
NTP sincronizado
Gateway conectado
Nodos con datos vigentes: 0 (pozas NO, secadores NO, perimetro NO)
N5=1795724999 ts=1795725001 delta=2 s
OK secadores: 393.0 ppm
RECHAZADO (firma invalida): secadores
Nodos con datos vigentes: 1 (pozas NO, secadores si, perimetro NO)
N5=1795725002 ts=1795725003 delta=1 s
OK secadores: 393.0 ppm
```
