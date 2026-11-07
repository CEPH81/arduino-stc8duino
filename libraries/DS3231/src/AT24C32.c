/*
 * AT24C32.c - EEPROM AT24C32 por I2C (ver AT24C32.h).
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <Wire.h>
#include "AT24C32.h"

#define TROZO 16          /* divide la pagina de 32 y cabe en el bufer de Wire */

static uint8_t dir = 0x57;

/* mientras escribe, la memoria no responde a su direccion */
static uint8_t esperar(void)
{
	unsigned long t0 = millis();
	do {
		Wire_beginTransmission(dir);
		if (Wire_endTransmission() == 0) return 0;
	} while (millis() - t0 < 30);
	return 2;
}

uint8_t at24c32_begin(uint8_t addr)
{
	dir = addr;
	Wire_beginTransmission(dir);
	return Wire_endTransmission() == 0 ? 0 : 1;
}

uint8_t at24c32_write(unsigned int pos, const uint8_t *p, unsigned int n)
{
	uint8_t k, i, r;
	while (n) {
		k = (uint8_t)(TROZO - (pos % TROZO));
		if (k > n) k = (uint8_t)n;
		Wire_beginTransmission(dir);
		Wire_write((uint8_t)(pos >> 8));
		Wire_write((uint8_t)pos);
		for (i = 0; i < k; i++) Wire_write(p[i]);
		r = Wire_endTransmission();
		if (r) return r;
		r = esperar();
		if (r) return r;
		pos += k; p += k; n -= k;
	}
	return 0;
}

uint8_t at24c32_read(unsigned int pos, uint8_t *p, unsigned int n)
{
	uint8_t k, i;
	while (n) {
		k = n > TROZO ? TROZO : (uint8_t)n;
		Wire_beginTransmission(dir);
		Wire_write((uint8_t)(pos >> 8));
		Wire_write((uint8_t)pos);
		if (Wire_endTransmission() != 0) return 1;
		if (Wire_requestFrom2(dir, k) != k) return 1;
		for (i = 0; i < k; i++) p[i] = (uint8_t)Wire_read();
		pos += k; p += k; n -= k;
	}
	return 0;
}

uint8_t at24c32_writeByte(unsigned int pos, uint8_t v)
{
	return at24c32_write(pos, &v, 1);
}

int at24c32_readByte(unsigned int pos)
{
	uint8_t v;
	if (at24c32_read(pos, &v, 1)) return -1;
	return v;
}
