/*
 * JDY40.h - radio serie JDY-40 (2,4 GHz, transparente) por Serial3, para
 * stc8duino (STC8H, STC8A) y stc32duino (STC32G).
 * Activar antes: Herramientas > Serial3 (UART3) > Si.
 *
 * Conexiones (el JDY-40 funciona a 2,2-3,6 V: VCC a 3,3 V):
 *   TXD del modulo -> P0.0 (RX3)     RXD del modulo -> P0.1 (TX3)
 *   SET -> pata a eleccion (bajo = comandos AT, alto = transmision)
 *   CS  -> pata a eleccion (bajo = activo, alto = dormido), o directo a GND
 *
 *   jdy40_begin(SET, CS, 9600);       // 9600 es la velocidad de fabrica
 *   jdy40_setChannel(1);              // 1-128; los modulos que se hablan deben
 *   jdy40_setRFID(0x8899);            //   tener el mismo canal, RFID y DVID
 *   Serial3.print("hola");            // llega a los otros modulos
 *   if (Serial3.available()) c = Serial3.read();
 *
 * Las funciones de configuracion devuelven 0 si el modulo respondio OK.
 * Lo configurado queda guardado en el modulo.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef JDY40_h
#define JDY40_h

#include <Arduino.h>

#ifndef STC_SERIAL3
#error "El JDY-40 usa Serial3: active Herramientas > Serial3 (UART3) > Si"
#endif

#define JDY40_SIN_PATA 0xFF       /* CS conectado directo a GND */

uint8_t jdy40_begin(uint8_t pin_set, uint8_t pin_cs, unsigned long baud);
uint8_t jdy40_command(const char *orden, char *respuesta, uint8_t max);  /* largo de la respuesta */
uint8_t jdy40_setChannel(uint8_t canal);       /* 1-128 */
uint8_t jdy40_setRFID(unsigned int id);        /* identificador de red, 0000-FFFF */
uint8_t jdy40_setDVID(unsigned int id);        /* identificador del equipo, 0000-FFFF */
uint8_t jdy40_setPower(uint8_t nivel);         /* 0 (-25 dBm) a 9 (+12 dBm) */
void    jdy40_sleep(void);
void    jdy40_wake(void);

#endif
