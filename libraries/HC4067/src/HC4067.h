/*
 * HC4067.h - multiplexor analogico de 16 canales 74HC4067 / CD74HC4067,
 * para stc8duino (STC8H...) y stc32duino (STC32G).
 *
 * Conexiones del modulo: VCC 3,3 o 5 V (igual que la placa), GND,
 *   S0-S3 -> 4 patas a eleccion, SIG -> una entrada analogica (p. ej. A0 =
 *   P1.0), EN a GND (o a una pata: HC4067_SIN_EN si va a GND).
 *   C0-C15: las 16 entradas o salidas.
 *
 *   hc4067_begin(S0, S1, S2, S3, A0, HC4067_SIN_EN);
 *   v = hc4067_analogRead(5);        // canal C5
 *   b = hc4067_digitalRead(7);       // canal C7 (con INPUT_PULLUP en SIG)
 *
 * El multiplexor conecta un canal a la vez (unos 70 ohm), en las dos
 * direcciones: tambien sirve para escribir con hc4067_digitalWrite.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef HC4067_h
#define HC4067_h

#include <Arduino.h>

#define HC4067_SIN_EN 0xFF

void    hc4067_begin(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t sig, uint8_t en);
void    hc4067_select(uint8_t canal);        /* 0-15 */
void    hc4067_enable(uint8_t si);           /* 0: desconecta todos (con EN) */
int     hc4067_analogRead(uint8_t canal);
uint8_t hc4067_digitalRead(uint8_t canal);
void    hc4067_digitalWrite(uint8_t canal, uint8_t valor);
void    hc4067_sigMode(uint8_t modo);        /* INPUT, INPUT_PULLUP u OUTPUT en SIG */

#endif
