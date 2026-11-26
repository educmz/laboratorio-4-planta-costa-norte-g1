# N2 Secadores — resultados experimentales

## F5 — Intervalo de publicación (R8)

Estado original: periodo de 100 ms y múltiples mensajes seriales con el mismo timestamp.

Versión consolidada: recepción MQTT en `upc/ga/secadores/datos`, QoS 0.

| Timestamp | Diferencia (s) |
|---|---:|
| 1795643206 | — |
| 1795643208 | 2 |
| 1795643210 | 2 |
| 1795643212 | 2 |
| 1795643214 | 2 |
| 1795643216 | 2 |

Los tiempos corresponden al reloj del nodo, no al intervalo de recepción del navegador.

## F6 — Calibración del MQ-2 (R1)

| Referencia (ppm) | ADC |
|---:|---:|
| 100 | 3337 |
| 400 | 3635 |
| 1000 | 3762 |
| 1500 | 3811 |
| 2000 | 3839 |
| 2500 | 3861 |
| 5000 | 3916 |

Interpolación lineal por segmentos de la curva experimental. Rango calibrado: 100–5000 ppm.

| Referencia declarada (ppm) | Lectura (ppm) | Error absoluto (ppm) | Error relativo | Baliza observada |
|---:|---:|---:|---:|---|
| 400 | 393.0 | 7.0 | 1.75 % | Verde |
| 1800 | 1839.3 | 39.3 | 2.18 % | Amarilla |
| 2500 | 2500.0 | 0.0 | 0.00 % | Roja |

La referencia de cada escenario fue indicada durante la prueba; las capturas de salida no muestran el deslizador. La captura inicial de calibración mostró 400 ppm y ADC 3628.

## F7 — HMAC (R9)

La versión consolidada genera firmas hexadecimales de 16 caracteres, observadas tanto en el monitor serial como en HiveMQ. La recepción por el broker se complementó con la validación en N5: las lecturas legítimas fueron aceptadas y el mensaje de prueba con firma inválida fue rechazado.

## F8 — Contrato MQTT

N2 publica en `upc/ga/secadores/datos`. La suscripción original de N5 termina en `/dato`. El tópico correcto se verificó mediante recepción en HiveMQ.

## Alcance de la validación

Las observaciones corresponden a la simulación consolidada de N2. Se verificó posteriormente la recepción extremo a extremo, la aceptación de firmas legítimas y el rechazo de una firma inválida en N5. El registro también muestra recepción de N2 y N3 durante una misma ejecución. Las pruebas corresponden a la versión consolidada, no a compilaciones aisladas de cada rama.

[Simulación N2](https://wokwi.com/projects/481603469063491585)

## Registro de integración N2–N5

El 26 de noviembre se observó en el monitor serial del Gateway:

```text
NTP sincronizado
Gateway conectado
OK secadores: 400.0 ppm
OK pozas: 400.0 ppm
OK perimetro: 400.0 ppm
Nodos con datos vigentes: 3 (pozas si, secadores si, perimetro si)
```

Prueba de seguridad y concentración elevada (extracto del monitor serial):

```text
OK secadores: 400.0 ppm
OK pozas: 2500.0 ppm
OK perimetro: 2500.0 ppm
OK secadores: 2500.0 ppm
RECHAZADO (firma invalida): secadores
[LAZO] Deteccion: 3 nodos sobre umbral y viento 135 grados. Orden: lavador ON
OK pozas: 2500.0 ppm
OK perimetro: 2500.0 ppm
```

El rechazo no interrumpió el procesamiento de otros datos legítimos. La orden ON no constituye por sí sola una confirmación física del lavador.
