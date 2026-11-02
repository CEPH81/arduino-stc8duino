/*
 * pins_arduino.h - STC8H8K64U (LQFP64): P0-P7 completos, USB en P3.0/P3.1. LED en P3.4, pulsador en P3.3 (INT1).
 * Patas: P1_0 = 0x10, P3_2 = 0x32... Serial: RX P3.0, TX P3.1.
 * ADC de 12 bits: A0-A7 = P1.0-P1.7, A8-A14 = P0.0-P0.6.
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#define NUM_DIGITAL_PINS 64
#define LED_BUILTIN P3_4
#define BOTON_BUILTIN P3_3   /* pulsador de la placa, a GND */
#define A0 P1_0
#define A1 P1_1
#define A2 P1_2
#define A3 P1_3
#define A4 P1_4
#define A5 P1_5
#define A6 P1_6
#define A7 P1_7
#define A8 P0_0
#define A9 P0_1
#define A10 P0_2
#define A11 P0_3
#define A12 P0_4
#define A13 P0_5
#define A14 P0_6
#endif
