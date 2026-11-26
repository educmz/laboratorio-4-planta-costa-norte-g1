# Evidencias de Secadores · 26 de noviembre de 2026

| Ensayo | Evidencia | Qué permite comprobar |
|---|---|---|
| F5, F6 y F7 originales | [Monitor y consigna de 400 ppm](F5_Periodo/original-400ppm.jpg) · [secuencia serial](REGISTRO-SERIAL.md) | Lectura de 8859.6 ppm, marca de tiempo repetida y firma vacía. Se aislaron el tópico y el Client ID de esta corrida. |
| F5 corregida | [Cuatro publicaciones consecutivas](REGISTRO-SERIAL.md) | Tres intervalos de 2 s en el reloj del emisor. |
| F6, tres rangos | [400 ppm](F6_Calibracion/400ppm-verde.jpg) · [1828 ppm](F6_Calibracion/1828ppm-amarilla.jpg) · [2524 ppm](F6_Calibracion/2524ppm-roja.jpg) | Consigna visible, concentración en el JSON del monitor y baliza. |
| F7 | [Rechazo](F7_Seguridad/firma-invalida.jpg) · [Continuidad](F7_Seguridad/continuidad.jpg) | N5 valida la primera muestra, rechaza la firma alterada y procesa la siguiente muestra legítima. |
| F8 | [Emisor aislado](F8_Integracion/publicacion-aislada.jpg) · [Filtro singular](F8_Integracion/filtro-singular.jpg) · [Filtro plural](F8_Integracion/filtro-plural.jpg) | Se contrasta el silencio del receptor heredado con el registro de aceptación del corregido. |
| Tiempo y compilación | [Rechazo por antigüedad](MQTT_NTP/n5-mensaje-antiguo.jpg) · [Arranque del candidato](MQTT_NTP/arranque-candidato.jpg) | Los límites de N5 siguen activos. El sketch de la rama compiló y arrancó con una clave de ensayo en un encabezado temporal. |
| EXTRA-N2-01 | [Ensayo local de sesiones](EXTRA_Client_ID/ensayo-local.md) | Desconexión por identificador duplicado y recepción simultánea con identificadores diferenciados. |

Los registros previos del grupo se conservan en la [bitácora histórica](../N2-Secadores.md). Las capturas de cierre de las sesiones del grupo corresponden al tablero y tienen un propósito distinto.
