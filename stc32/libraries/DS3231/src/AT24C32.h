/*
 * AT24C32.h - memoria EEPROM AT24C32 / AT24C64 (4 u 8 KB) por I2C, la del
 * modulo ZS-042 del DS3231. Para stc8duino y stc32duino.
 *
 *   at24c32_begin(0x57);                     // 0x57 en el ZS-042 (0x50-0x57)
 *   at24c32_write(100, datos, n);            // cualquier largo y posicion
 *   at24c32_read(100, datos, n);
 *   at24c32_writeByte(0, 55);  b = at24c32_readByte(0);
 *
 * La escritura se parte en trozos que no cruzan una pagina de 32 bytes y
 * espera a que la memoria termine cada uno (unos 10 ms). Cada byte aguanta
 * un millon de escrituras: no escriba en cada vuelta de loop().
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef AT24C32_h
#define AT24C32_h

#include <Arduino.h>
#include <Wire.h>

uint8_t at24c32_begin(uint8_t addr);                                    /* 0 si responde */
uint8_t at24c32_write(unsigned int pos, const uint8_t *p, unsigned int n);  /* 0 si bien */
uint8_t at24c32_read(unsigned int pos, uint8_t *p, unsigned int n);        /* 0 si bien */
uint8_t at24c32_writeByte(unsigned int pos, uint8_t valor);
int     at24c32_readByte(unsigned int pos);                             /* -1 si error */

#endif
