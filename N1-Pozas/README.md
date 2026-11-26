# N1-Pozas — Planta Costa Norte (G1)

## Información general

- **Curso:** 1ASI0572 — Desarrollo de Soluciones IoT
- **Laboratorio:** 4 — Instalación heredada
- **Grupo:** G1.
- **Responsable:** Pedro Nanfuñay
- **Rol:** Integrante 1 — N1-Pozas de materia prima
- **Revisor asignado:** Integrante 5

## Proyectos Wokwi

- [Instalación heredada](https://wokwi.com/projects/481589788453665793).
- [Copia de trabajo corregida](https://wokwi.com/projects/481597967387496449).

## 1. Descripción del nodo

El nodo N1-Pozas supervisa las condiciones de gas y temperatura en las pozas de materia prima de la Planta Costa Norte. Utiliza un ESP32 para leer un sensor MQ-2 y un sensor de temperatura DS18B20, activar una alarma sonora y controlar un relé asociado a la ventilación local.

La ventilación debe activarse cuando la concentración de gas supera **2000 ppm** o la temperatura supera **35 °C**, incluso si el gateway no envía instrucciones. El nodo también publica telemetría periódica por MQTT y puede recibir órdenes autenticadas de ventilación máxima o normal desde N5-Gateway.

Las correcciones se organizaron en cuatro fallas: lectura de temperatura, lógica de actuación local, autenticación de comandos y compatibilidad del formato de telemetría.

## 2. Componentes y conexiones

| Componente | Conexión | Función |
|---|---|---|
| ESP32 DevKit V4 | — | Procesamiento, actuación local y comunicación WiFi/MQTT |
| Sensor MQ-2, salida AOUT | GPIO34 (ADC) | Lectura analógica relacionada con la concentración de gas |
| Sensor DS18B20, DQ | GPIO4 | Medición de temperatura |
| Resistencia de 4.7 kΩ | Entre DQ y 3V3 | *Pull-up* del bus OneWire |
| Buzzer | GPIO25 | Alarma sonora cuando el gas supera el umbral |
| Módulo relé | GPIO26 (IN) | Activación de la salida de ventilación |
| Alimentación del MQ-2 | 3V3 y GND en Wokwi | Alimentación representada en el circuito heredado |
| Alimentación del relé | 5V y GND | Alimentación del módulo simulado |

El firmware utiliza `HIGH` para solicitar la activación del relé y `LOW` para desactivarlo. La captura de la salida serial permite verificar la decisión lógica de ventilación; la conmutación física del relé requiere una evidencia específica del circuito.

## 3. Bitácora de fallas

| ID | Tipo | Síntoma | Causa raíz | Corrección | Commit |
|---|---|---|---|---|---|
| F1 | Visible | Temperatura informada en una unidad distinta de la requerida | Uso de `getTempFByIndex(0)` mientras el umbral está expresado en °C | Sustitución por `getTempCByIndex(0)` | Por registrar |
| F2 | Silenciosa | La ventilación puede apagarse aunque la temperatura sea alta | El `else` de la condición de gas sobrescribía la activación por temperatura | Evaluación conjunta de gas, temperatura y orden de máxima ventilación | Por registrar |
| F3 | Seguridad | Se podían modificar parámetros con mensajes MQTT sin autenticar | Recepción de órdenes en texto sin verificar origen, firma ni vigencia | Comandos firmados con HMAC, validación temporal y protección contra repetición | Por registrar |
| F4 | Cruzada | Telemetría incompatible con el contrato esperado por N5 | `valor` era una cadena con sufijo `ppm` y la firma utilizaba ese texto | `valor` numérico, `unidad` separada y firma sobre `pozas\|ts\|valor` | Por registrar |

## 4. Diagnóstico y correcciones

### F1 — Falla visible: temperatura en Fahrenheit

El código heredado leía el DS18B20 con `getTempFByIndex(0)`, aunque el límite de R1 está definido en **35 °C**. Esta diferencia podía alterar las decisiones de ventilación y la interpretación de los datos.

Se reemplazó la lectura por `getTempCByIndex(0)` sin cambiar el umbral. En la evidencia posterior se observó una lectura de **22.0 °C**, coherente con el ajuste del simulador.

**Paso del marco IoT:** paso 8, sensores y actuadores.

**Regla relacionada:** R1.

**Rama:** `bugfix/pozas-temperatura-celsius`.

**Commit:** 57186d9d90a4da058a68b90b1e7b5e4080a7c143

**Pull request:** [PR#17](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/17)

### F2 — Falla silenciosa: lógica de ventilación Edge

En el firmware heredado, una condición independiente evaluaba el gas y utilizaba un `else` que podía volver a colocar `ventilar` en `false`, aunque la temperatura ya hubiese superado su umbral.

Se unificó la decisión local mediante `gasAlto || temperaturaAlta || ventMax`. Así, cualquiera de las condiciones puede solicitar ventilación; la alarma sonora continúa vinculada al umbral de gas.

En la captura posterior se observó la lógica corregida y la salida `Ventilacion: ON`. **Esa imagen no demuestra todavía que el encendido haya sido causado exclusivamente por temperatura alta y gas bajo**, ya que durante parte del diagnóstico el valor de gas calculado también superaba 2000 ppm.

**Etapa del marco IoT:** lectura de sensores y actuación local en el borde.

**Reglas relacionadas:** R1 y R2.

**Rama:** `bugfix/pozas-ventilacion-edge`.

**Commit:** ad167832bc91b1b8c4ebd8fcaf2a854fd09bd1b9

**Pull request:** [PR#18](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/18)

### F3 — Falla de seguridad: comandos MQTT no autenticados

El código heredado aceptaba mensajes como `UMBRAL:5000`, `VENT:MAX` y `VENT:NORMAL` publicados en `upc/ga/pozas/cmd`, sin comprobar que procedieran del gateway autorizado. Durante el diagnóstico se observó que un mensaje externo podía modificar el umbral.

La corrección adopta el formato de órdenes firmadas utilizado por N5: `gateway|ts|contenido|firma`. El receptor comprueba la identidad declarada, la firma HMAC-SHA256 truncada a 16 caracteres hexadecimales, la vigencia temporal y la repetición del timestamp, cuyo último valor aceptado se conserva mediante `Preferences`.

Se admiten las órdenes `cmd:VENT:MAX` y `cmd:VENT:NORMAL` si superan las validaciones. La orden `VENT:NORMAL` retira la ventilación forzada, pero no anula la decisión local por concentración o temperatura elevada. Las credenciales se declaran en `secrets.h`, excluido del repositorio.

**Resultado observado:** rechazo de un comando MQTT sin autenticación. **Pendiente:** comprobar con N5 la aceptación de órdenes legítimas firmadas y el rechazo de una orden repetida.

**Reglas relacionadas:** R9 y R10.

**Rama:** `bugfix/pozas-autenticacion-hmac`.

**Commit:** 855c1d1782157d330988fb8df1e6337fda7cf2e5

**Pull request:** [PR#19](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/19)

### F4 — Falla cruzada: contrato de telemetría MQTT

El firmware heredado publicaba una lectura de gas con un formato similar a `"valor":"8859.6ppm"`. El contrato del grupo requiere que `valor` sea un número JSON y que la unidad se indique por separado. Además, la firma debe calcularse con el valor numérico representado como texto, sin añadir `ppm`.

Se modificó la construcción del mensaje para publicar `"valor":8859.6` en lugar de una cadena y utilizar la secuencia `pozas|ts|8859.6` como material de firma. Se conservaron los campos `id`, `ts`, `unidad`, `temp`, `calidad` y `firma`.

**Resultado observado:** JSON corregido en el monitor serial, con `valor` numérico y firma de 16 caracteres. **Pendiente:** registrar la recepción y aceptación efectiva de la telemetría por N5.

**Reglas relacionadas:** R8 y R9; contrato MQTT entre N1 y N5.

**Rama:** `bugfix/pozas-formato-telemetria`.

**Commit:** 93ba419610b6262c0facdd8db78f0f3601ef732b

**Pull request:** [PR#20](https://github.com/educmz/laboratorio-4-planta-costa-norte-g1/pull/20)

## 5. Comunicación MQTT

| Parámetro | Configuración |
|---|---|
| Broker | `broker.hivemq.com` |
| Puerto MQTT del ESP32 | `1883` |
| Puerto WebSocket TLS para monitoreo | `8884` |
| Prefijo del grupo | `upc/ga/` |
| Periodicidad configurada de publicación | 2000 ms |
| Sincronización de tiempo | NTP |

| Topic | Función |
|---|---|
| `upc/ga/pozas/datos` | Publicación de telemetría de gas y temperatura |
| `upc/ga/pozas/cmd` | Recepción de órdenes firmadas del Gateway N5 |
| `upc/ga/#` | Suscripción de diagnóstico desde el cliente HiveMQ |

La telemetría incluye `id`, `ts`, `valor` numérico, `unidad`, `temp`, `calidad` y `firma`. La firma se obtiene mediante HMAC-SHA256 del texto `pozas|ts|valor`, utilizando los primeros 16 caracteres hexadecimales. Las órdenes de N5 utilizan el formato `gateway|ts|contenido|firma`.

## 6. Control de versiones

| Implementación | Rama | Commit | PR |
|---|---|---|---|
| Corrección F1 | `bugfix/pozas-temperatura-celsius` | 57186d9d90a4da058a68b90b1e7b5e4080a7c143 | 0d01acd69b4694a1f2842f4657abb3b46fcfb9f7 |
| Corrección F2 | `bugfix/pozas-ventilacion-edge` | ad167832bc91b1b8c4ebd8fcaf2a854fd09bd1b9 | 81fcb13bcb7e4df3ccc599020b5c75ee6e538594 |
| Corrección F3 | `bugfix/pozas-autenticacion-hmac` | 855c1d1782157d330988fb8df1e6337fda7cf2e5 | d66157486b3b4e12d0334b832d226487790a7749 |
| Corrección F4 | `bugfix/pozas-formato-telemetria` | 93ba419610b6262c0facdd8db78f0f3601ef732b | 0354cf90a9d01c7e649181361eb4475c57184375 |

Los identificadores y enlaces deben obtenerse del historial real de GitHub. La publicación de ramas no demuestra por sí misma que todos los PR hayan sido aprobados e integrados a `main`.

## 7. Configuración y ejecución

1. Abrir la [copia de trabajo de N1 en Wokwi](https://wokwi.com/projects/481597967387496449), o importar los archivos `sketch.ino`, `diagram.json` y `libraries.txt` del repositorio.
2. Crear `secrets.h` a partir de `secrets.example.h` e introducir la clave HMAC acordada con N5; no publicar credenciales reales.
3. Iniciar la simulación del ESP32 y comprobar la conexión WiFi y MQTT.
4. Observar en el monitor serial la temperatura, el valor de gas y el estado de ventilación.
5. Cambiar los valores de los sensores en Wokwi para evaluar los umbrales de **2000 ppm** y **35 °C**.
6. Conectar el cliente WebSocket de HiveMQ al broker y suscribirse a `upc/ga/pozas/datos` para observar la telemetría.
7. Para la prueba de seguridad, publicar una orden sin firma en `upc/ga/pozas/cmd` y comprobar que sea rechazada.
8. Para validar comandos legítimos y aceptación de datos, ejecutar la integración con N5 utilizando la misma clave y el contrato MQTT acordado.

El archivo `secrets.h` permanece fuera del control de versiones mediante `.gitignore`.

## 8. Documentación técnica

- [Registro de pruebas y resultados experimentales](pruebas/PRUEBAS.md).
- [Evidencia F1 — temperatura](pruebas/F1-temperatura.png).
- [Evidencia F2 — ventilación y formato de datos F4](pruebas/F2-ventilacion.png).
- [Evidencia F3 — rechazo de comando](pruebas/F3-seguridad.png).
- [Bitácora general del laboratorio](../README.md).

## 9. Resultados

Las pruebas y capturas disponibles documentan:

- Lectura de temperatura corregida a grados Celsius.
- Ejecución de la lógica de ventilación local modificada y observación de `Ventilacion: ON`.
- Rechazo de un comando MQTT sin firma.
- Construcción de un mensaje de telemetría con `valor` numérico, unidad independiente y firma HMAC de 16 caracteres.
- Registros seriales con publicaciones aproximadamente cada dos segundos durante el diagnóstico.

**Pruebas pendientes:** activación demostrada únicamente por temperatura con gas bajo, ventilación apagada bajo condiciones normales, aceptación de órdenes firmadas de N5, rechazo experimental de replay y confirmación de telemetría aceptada por el gateway.

La lectura analógica del MQ-2 presentó discrepancias respecto de los valores configurados en Wokwi. Cualquier conversión experimental adicional de ADC a ppm debe documentarse y verificarse separadamente; no constituye una calibración física certificada.

## 10. Conclusiones

El diagnóstico de N1-Pozas permitió identificar cuatro fallas relacionadas con la lectura de temperatura, la lógica de ventilación, la seguridad de los comandos MQTT y la compatibilidad de telemetría con N5.

Las correcciones implementadas organizan la actuación local del ESP32, incorporan validaciones para las órdenes remotas y ajustan el mensaje de datos al contrato del grupo. Las capturas disponibles respaldan parte del funcionamiento individual observado en Wokwi; la validación completa de R2 y la integración extremo a extremo con N5 continúan pendientes.

El código y el registro experimental se mantienen separados para facilitar la revisión de cada falla, la trazabilidad de los Pull Requests y la futura integración de los cinco nodos.
