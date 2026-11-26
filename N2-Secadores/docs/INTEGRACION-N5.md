# Integración de Secadores con el Gateway

N2 publica en `upc/ga/secadores/datos` con QoS 0 y sin retención. En N5 el filtro `upc/ga/+/datos` acepta exactamente un nivel de nodo. El filtro heredado acababa en `/dato`. La corrección del integrador forma parte del historial de main. [Commit de N5](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/commit/9ebf24c) · [PR #11](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/11).

## Ensayo paralelo

Se ejecutaron copias temporales del [Gateway heredado](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/commit/5b1a535) y del Gateway corregido con un prefijo exclusivo de ensayo, IDs de cliente distintos y escritura en ThingSpeak desactivada. Las copias compartieron el emisor y una clave de ensayo. El Gateway heredado quedó conectado sin registrar lecturas de Secadores. El corregido recibió el mensaje dirigido a la terminación plural. Una publicación manual con un dígito HMAC alterado produjo el rechazo y la siguiente lectura legítima volvió a procesarse. [Capturas y registro de las tres etapas](../../evidencias/pruebas/N2-Secadores/INDICE.md).

## Medida del desfase

Se registró $\Delta=t_{mensaje}-t_{N5}$ al recibir en el Gateway de ensayo, después de validar la firma. N5 exige $-60\leq\Delta\leq15$ segundos y marcas estrictamente crecientes por nodo.

| Corrida | Configuración | Observación |
|---|---|---|
| Copias publicadas, arranque escalonado | N2 con −20 s | N5 rechazó por antigüedad. El monitor original no imprimía ambos relojes. |
| Candidato, arranque escalonado | Ajuste 0 s, SNTP periódico | N5=1795724943, mensaje=1795724872, Δ=−71 s. Rechazo correcto. |
| Candidato, reinicio conjunto | Ajuste 0 s, SNTP periódico | Δ=+2 s y +1 s. N5 aceptó ambas lecturas. |

La configuración de entrega parte de UTC sin compensación, tres servidores NTP y una solicitud de resincronización cada 15 s de tiempo del ESP32. Restar 20 s en el caso de −71 s habría aumentado la antigüedad aparente a 91 s. La prueba corta con arranque conjunto acredita aceptación puntual. La estabilidad durante sesiones prolongadas, con velocidades de simulación distintas, sigue abierta. El ajuste local −20 permite reproducir la configuración anterior. En todos los casos se calcula un solo timestamp y se usa tanto en la firma como en el JSON.
