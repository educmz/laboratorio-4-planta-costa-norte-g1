```
ets Jul 29 2019 12:21:46

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:1156
load:0x40078000,len:11456
ho 0 tail 12 room 4
load:0x40080400,len:2972
entry 0x400805dc
==============================
N4-LAVADOR - GRUPO G1
Version corregida
==============================
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[RELOJ ESP32] 3
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[WIFI] Intentando reconectar
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[RELOJ ESP32] 8
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[RELOJ ESP32] 13
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[WIFI] Intentando reconectar
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[RELOJ ESP32] 18
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[RELOJ ESP32] 23
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[LOCAL] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[NTP] Resincronizando. Reloj: 27
[MQTT] Intentando conectar
[MQTT] Suscripcion CMD: OK
[MQTT] Suscripcion CONC: OK
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","estado":"online"}
[MQTT] Conectado
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[RELOJ ESP32] 30
[NTP] Reloj disponible: 1795720885
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[RELOJ ESP32] 1795720888
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"OFF","intensidad":1,"modoSeguro":false,"confirmado":false}
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720891 | Mensaje: 1795720885 | Diferencia: 6 s
[CONCENTRACION] 2500.0 ppm
[RX MQTT] upc/ga/lavador/cmd
[NTP] ESP32: 1795720891 | Mensaje: 1795720885 | Diferencia: 6 s
[LAVADOR] ON: orden gateway
[MQTT] upc/ga/lavador/estado -> {"lavador":"ON","motivo":"orden gateway"}
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"ON","intensidad":1,"modoSeguro":false,"confirmado":false}
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720893 | Mensaje: 1795720897 | Diferencia: -4 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"ON","intensidad":1,"modoSeguro":false,"confirmado":false}
[RELOJ ESP32] 1795720893
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 2460 ms
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720895 | Mensaje: 1795720899 | Diferencia: -4 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"CONFIRMADO","intensidad":1,"modoSeguro":false,"confirmado":true}
[NTP] Resincronizando. Reloj: 1795720896
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720897 | Mensaje: 1795720901 | Diferencia: -4 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"CONFIRMADO","intensidad":1,"modoSeguro":false,"confirmado":true}
[RELOJ ESP32] 1795720898
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720899 | Mensaje: 1795720903 | Diferencia: -4 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"CONFIRMADO","intensidad":1,"modoSeguro":false,"confirmado":true}
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 4 ms
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 1298 ms
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 4 ms
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720912 | Mensaje: 1795720905 | Diferencia: 7 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"CONFIRMADO","intensidad":1,"modoSeguro":false,"confirmado":true}
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720914 | Mensaje: 1795720907 | Diferencia: 7 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"CONFIRMADO","intensidad":1,"modoSeguro":false,"confirmado":true}
[RELOJ ESP32] 1795720914
[RX MQTT] upc/ga/lavador/cmd
[NTP] ESP32: 1795720914 | Mensaje: 1795720907 | Diferencia: 7 s
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 5 ms
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 766 ms
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 5 ms
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"CONFIRMADO","intensidad":1,"modoSeguro":false,"confirmado":true}
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720916 | Mensaje: 1795720909 | Diferencia: 7 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"CONFIRMADO","intensidad":1,"modoSeguro":false,"confirmado":true}
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720918 | Mensaje: 1795720923 | Diferencia: -5 s
[CONCENTRACION] 2500.0 ppm
[RELOJ ESP32] 1795720919
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 5 ms
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"ON","intensidad":1,"modoSeguro":false,"confirmado":false}
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720920 | Mensaje: 1795720925 | Diferencia: -5 s
[CONCENTRACION] 2500.0 ppm
[NTP] Resincronizando. Reloj: 1795720921
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"ON","intensidad":1,"modoSeguro":false,"confirmado":false}
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720922 | Mensaje: 1795720927 | Diferencia: -5 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 3532 ms
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"CONFIRMADO","intensidad":1,"modoSeguro":false,"confirmado":true}
[RELOJ ESP32] 1795720932
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 5 ms
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"ON","intensidad":1,"modoSeguro":false,"confirmado":false}
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 634 ms
[RX MQTT] upc/ga/lavador/cmd
[NTP] ESP32: 1795720935 | Mensaje: 1795720932 | Diferencia: 3 s
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720935 | Mensaje: 1795720932 | Diferencia: 3 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"CONFIRMADO","intensidad":1,"modoSeguro":false,"confirmado":true}
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 4 ms
[LAVADOR] Confirmacion perdida
[RELOJ ESP32] 1795720937
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 864 ms
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"CONFIRMADO","intensidad":1,"modoSeguro":false,"confirmado":true}
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720939 | Mensaje: 1795720936 | Diferencia: 3 s
[CONCENTRACION] 2500.0 ppm
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"lavador":"CONFIRMADO"}
[CONFIRMACION] Tiempo: 6 ms
[LAVADOR] Confirmacion perdida
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"ON","intensidad":1,"modoSeguro":false,"confirmado":false}
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720941 | Mensaje: 1795720947 | Diferencia: -6 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"ON","intensidad":1,"modoSeguro":false,"confirmado":false}
[RELOJ ESP32] 1795720942
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720943 | Mensaje: 1795720949 | Diferencia: -6 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"ON","intensidad":1,"modoSeguro":false,"confirmado":false}
[NTP] Resincronizando. Reloj: 1795720944
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720944 | Mensaje: 1795720951 | Diferencia: -7 s
[CONCENTRACION] 2500.0 ppm
[MQTT] upc/ga/lavador/estado -> {"nodo":"lavador","lavador":"ON","intensidad":1,"modoSeguro":false,"confirmado":false}
[RX MQTT] upc/ga/gateway/conc
[NTP] ESP32: 1795720946 | Mensaje: 1795720953 | Diferencia: -7 s
[CONCENTRACION] 2500.0 ppm
[RX MQTT] upc/ga/lavador/cmd
[NTP] ESP32: 1795720947 | Mensaje: 1795720953 | Diferencia: -6 s
[RELOJ ESP32] 1795720947

```
