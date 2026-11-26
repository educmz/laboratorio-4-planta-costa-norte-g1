# N2 — Secadores

Nodo de monitoreo ambiental de la línea de secado de harina de Planta Costa Norte (G1).

**Responsable:** Eduardo Chacaliaza — Integrante 2.

## Simulación

- [Instalación original](https://wokwi.com/projects/481589874625672193)
- [Instalación corregida y validada](https://wokwi.com/projects/481603469063491585)
- [Registro experimental e integración](../evidencias/pruebas/N2-Secadores.md)

## Hardware

ESP32, MQ-2 (GPIO34), DHT22 (GPIO15) y baliza RGB de cátodo común (GPIO25, 26 y 27).

## Comunicación

Broker MQTT: `broker.hivemq.com:1883`. Tópico: `upc/ga/secadores/datos`. Publicación cada 2000 ms. El mensaje contiene concentración numérica en ppm, temperatura, humedad, timestamp NTP y HMAC-SHA256 truncado a 16 caracteres hexadecimales. La clave compartida se configura localmente mediante `secrets.h`, a partir de `secrets.example.h` de la rama de seguridad.

## Auditoría

| ID | Tipo | Paso del marco IoT | Corrección | Trazabilidad |
|---|---|---|---|---|
| F5 | Visible | Paso 4: Definición de los requisitos de la capa de intercambio (R8) | Periodo de 100 a 2000 ms | [PR 6](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/6) |
| F6 | Silenciosa | Paso 10: Definición del procesamiento de datos en cada nodo y en la nube (R1) | Interpolación de siete puntos experimentales ADC/ppm | [PR 7](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/7) |
| F7 | Seguridad | Paso 4: Definición de los requisitos de la capa de intercambio (R9) | Firma HMAC de id, timestamp y valor | [PR 8](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/8) |
| F8 | Cruzada | Paso 7: Selección de las arquitecturas de intercambio de datos e integración de información | Suscripción N5 a datos en plural | [Commit N5](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/commit/9ebf24c) |
| EXTRA-N2-01 | Adicional | Paso 7: Selección de las arquitecturas de intercambio de datos e integración de información | Client ID diferenciado por nodo y MAC | [PR 9](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/9) |

## Validación

La versión consolidada de Wokwi publicó seis mensajes consecutivos con diferencias de 2 s en sus timestamps. Las pruebas de calibración produjeron 393.0, 1839.3 y 2500.0 ppm con balizas verde, amarilla y roja, respectivamente. El Gateway aceptó lecturas de Secadores de 400.0 y 2500.0 ppm y rechazó un mensaje con firma inválida. Los registros experimentales describen el alcance de la prueba.

La integración al código principal se gestiona mediante los PR individuales y la revisión independiente. El firmware original permanece disponible en el historial de Git.
