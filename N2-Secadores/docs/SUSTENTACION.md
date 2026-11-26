# Sustentación individual de Eduardo Chacaliaza

## Intervención de 25 segundos

«Mi nodo mide el gas de los secadores. La falla más difícil daba una lectura numérica creíble, pero con el sensor en 400 ppm el monitor indicaba 8859.6 y encendía la alarma roja. Caractericé la respuesta simulada con siete puntos. Con interpolación por tramos, la misma entrada dio 393 ppm y baliza verde. La prueba también confirmó amarillo y rojo al elevar la concentración».

## Intervención de un minuto

«En Secadores encontré problemas de ritmo, medición, autenticación e integración. El programa publicaba cada cien milisegundos. Al fijar dos segundos, los timestamps consecutivos pasaron a diferir en dos. El error más importante para el operador era el gas: una entrada normal disparaba la alarma roja. Los siete puntos ADC/ppm permitieron interpolar según el tramo y comprobar las tres balizas. En la comunicación, el Gateway necesita recibir exactamente el valor decimal que se firmó junto al identificador y al timestamp. Además, su filtro original terminaba en dato, mientras el emisor publicaba en datos. Otro hallazgo fue que dos nodos usaban el mismo identificador de cliente. Por último, una lectura puede llegar con firma correcta y quedar fuera de la ventana temporal. El reloj del simulador y el del Gateway deben medirse durante la misma ejecución».

## Preguntas de contraste

- **¿Qué decide el nodo sin esperar una orden?** En cada muestra válida selecciona verde hasta 1500 ppm, amarillo hasta 2000 inclusive y rojo por encima. La decisión colectiva del lavador corresponde a N5.
- **¿Por qué no basta una recta entre los extremos?** Los segmentos tienen pendientes distintas. Cerca del extremo alto, una sola cuenta equivale a unos 45 ppm.
- **¿Qué autentica la firma?** Solo la secuencia exacta de id, timestamp y valor. Temperatura y humedad quedan fuera del acuerdo firmado. Ocho bytes producen 16 dígitos hexadecimales.
- **¿Qué ocurre si se escribe 400 en lugar de 400.0?** Si se firma un texto y se publica el otro, N5 obtiene otra firma aunque ambos representen el mismo número.
- **¿Por qué no basta observar el broker?** El encaminamiento MQTT, la firma y el control temporal se verifican en etapas distintas. La evidencia de aceptación es el registro de N5.
- **¿Por qué aparece el rechazo por antigüedad?** El tiempo simulado puede avanzar a otro ritmo. Restar 20 segundos desplaza toda muestra hacia el límite de antigüedad. N5 también descarta fechas adelantadas y repeticiones.
- **¿Qué distingue al Client ID del campo id?** El primero identifica una conexión en el broker. El segundo identifica al nodo dentro del mensaje firmado.
- **¿Se puede certificar con esto un detector industrial?** La caracterización corresponde al modelo MQ-2 virtual y al intervalo de 100 a 5000 ppm.

## Demostración en vivo

1. Abrir N2 y N5 en la misma sesión de prueba. Mostrar sincronización, tópico y Client ID sin exponer la configuración privada.
2. Abrir el control del MQ-2, mostrar la referencia y esperar una publicación completa. Repetir en cada zona de la baliza.
3. Comparar dos timestamps consecutivos y localizar en N5 la lectura correspondiente.
4. Durante el turno del grupo, emitir una sola prueba con firma alterada y observar el rechazo seguido de una lectura válida.
5. Mostrar la comparación de filtros y el registro de relojes si N5 descarta un paquete. Cerrar con las limitaciones de la medición.
