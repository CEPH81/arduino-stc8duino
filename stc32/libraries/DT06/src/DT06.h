/*
 * DT06.h - modulo TTL-WiFi DT-06 (Doit, ESP8285) por Serial3, para stc8duino
 * (STC8H, STC8A) y stc32duino (STC32G).
 * Activar antes: Herramientas > Serial3 (UART3) > Si.
 *
 * El DT-06 trae un programa de "puerto serie a WiFi transparente": lo que
 * entra por su RX sale por la conexion de red (TCP/UDP) y al reves. Todo se
 * configura en su pagina web:
 *   1. Al encenderlo crea una red WiFi abierta (DOIT_xxxx). Conectese a ella.
 *   2. Abra http://192.168.4.1 y elija:
 *        - modo AP o STA (STA = se conecta a su router: nombre y clave),
 *        - servidor o cliente TCP / UDP, puerto e IP del otro equipo,
 *        - la velocidad del puerto serie (la misma que en dt06_begin).
 *   3. Guarde y reinicie el modulo.
 *
 * Conexiones (VCC 3,3 V; varios modulos aceptan 5 V en VCC, ver su placa):
 *   TX del modulo -> P0.0 (RX3)       RX del modulo -> P0.1 (TX3)       GND
 *
 *   dt06_begin(115200);
 *   Serial3.print("temperatura=25\r\n");     // sale por la red
 *   if (Serial3.available()) c = Serial3.read();
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef DT06_h
#define DT06_h

#include <Arduino.h>

#ifndef STC_SERIAL3
#error "El DT-06 usa Serial3: active Herramientas > Serial3 (UART3) > Si"
#endif

#define DT06_BAUD_FABRICA 115200

#define dt06_begin(baud) Serial3_begin(baud)

#endif
