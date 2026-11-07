/*
 * MCP4725.c - DAC MCP4725 por I2C (ver MCP4725.h).
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <Wire.h>
#include "MCP4725.h"

static uint8_t dir = 0x60;
static unsigned int ultimo;

uint8_t mcp4725_begin(uint8_t addr)
{
	dir = addr;
	Wire_beginTransmission(dir);
	return Wire_endTransmission() == 0 ? 0 : 1;
}

/* escritura rapida: [0 0 PD1 PD0 D11..D8] [D7..D0] */
static uint8_t rapido(uint8_t pd, unsigned int v)
{
	Wire_beginTransmission(dir);
	Wire_write((uint8_t)((pd << 4) | ((v >> 8) & 0x0F)));
	Wire_write((uint8_t)v);
	return Wire_endTransmission();
}

uint8_t mcp4725_setValue(unsigned int v)
{
	if (v > 4095) v = 4095;
	ultimo = v;
	return rapido(0, v);
}

uint8_t mcp4725_powerDown(uint8_t modo)
{
	return rapido((uint8_t)(modo & 3), ultimo);
}

int mcp4725_getValue(void)
{
	unsigned int v;
	if (Wire_requestFrom2(dir, 3) != 3) return -1;
	Wire_read();                              /* estado */
	v = (unsigned int)(uint8_t)Wire_read() << 4;
	v |= (uint8_t)Wire_read() >> 4;
	return (int)v;
}

uint8_t mcp4725_setValueEEPROM(unsigned int v)
{
	unsigned long t0;
	uint8_t r;
	if (v > 4095) v = 4095;
	ultimo = v;
	Wire_beginTransmission(dir);
	Wire_write(0x60);                          /* DAC + EEPROM */
	Wire_write((uint8_t)(v >> 4));
	Wire_write((uint8_t)(v << 4));
	r = Wire_endTransmission();
	if (r) return r;
	t0 = millis();                             /* RDY (bit 7 del estado) = 1: termino */
	while (millis() - t0 < 100) {
		if (Wire_requestFrom2(dir, 1) == 1 && (Wire_read() & 0x80)) return 0;
		delay(2);
	}
	return 4;
}
