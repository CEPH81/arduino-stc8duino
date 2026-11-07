/*
 * PCF8574.h - expansor de 8 patas por I2C (PCF8574: 0x20-0x27, PCF8574A: 0x38-0x3F).
 *
 * El PCF8574 no tiene registro de direccion: cada pata es "cuasi-bidireccional".
 *   - Salida en 0: la pata se pone a GND con fuerza (puede encender un LED a VCC).
 *   - Salida en 1 / entrada: la pata queda alta con una corriente pequena (unos
 *     100 uA); un pulsador a GND la lleva a 0. Por eso las entradas se escriben en 1.
 * Este modulo recuerda lo escrito en cada expansor para cambiar una pata sin tocar
 * las demas, y mantiene siempre en 1 las patas declaradas como entrada.
 *
 *   Wire.begin();
 *   pcf8574_begin(0x20);                       // todas como entrada
 *   pcf8574_pinMode(0x20, 0, OUTPUT);          // P0 salida
 *   pcf8574_digitalWrite(0x20, 0, LOW);        // P0 a GND (LED a VCC encendido)
 *   v = pcf8574_digitalRead(0x20, 7);          // lee P7
 *   pcf8574_write(0x20, 0x0F);  b = pcf8574_read(0x20);   // las 8 a la vez
 *
 * Hasta 8 expansores (direcciones con la misma parte baja comparten el estado:
 * no mezcle 0x20 y 0x38 en el mismo programa).
 * Las funciones devuelven/usan -1 o 0xFF si el expansor no responde.
 *
 * Parte de stc8duino (version stc32duino). LGPL 2.1.
 */
#ifndef PCF8574_h
#define PCF8574_h

#include <Arduino.h>
#include <Wire.h>

uint8_t pcf8574_begin(uint8_t addr);                     /* 0 si responde */
uint8_t pcf8574_write(uint8_t addr, uint8_t valor);      /* las 8 patas; 0 si bien */
int     pcf8574_read(uint8_t addr);                      /* 0..255, -1 si falla */
void    pcf8574_pinMode(uint8_t addr, uint8_t pata, uint8_t modo);   /* INPUT / OUTPUT */
void    pcf8574_digitalWrite(uint8_t addr, uint8_t pata, uint8_t valor);
int     pcf8574_digitalRead(uint8_t addr, uint8_t pata);
void    pcf8574_toggle(uint8_t addr, uint8_t pata);

#endif
