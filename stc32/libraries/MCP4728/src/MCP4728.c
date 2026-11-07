/*
 * MCP4728.c - DAC MCP4728 por I2C (ver MCP4728.h).
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <Wire.h>
#include "MCP4728.h"

static uint8_t dir = 0x60;
static uint8_t vref, gan;                 /* 1 bit por canal (A = bit 3 ... D = bit 0) */
static unsigned int valores[4];

uint8_t mcp4728_begin(uint8_t addr)
{
	dir = addr;
	Wire_beginTransmission(dir);
	return Wire_endTransmission() == 0 ? 0 : 1;
}

/* primer byte de un canal: VREF PD1 PD0 GAIN D11..D8 */
static uint8_t alto(uint8_t canal, unsigned int v)
{
	uint8_t b = (uint8_t)((v >> 8) & 0x0F);
	if (vref & (8 >> canal)) b |= 0x80;
	if (gan & (8 >> canal)) b |= 0x10;
	return b;
}

uint8_t mcp4728_setChannel(uint8_t canal, unsigned int v)
{
	canal &= 3;
	if (v > 4095) v = 4095;
	valores[canal] = v;
	Wire_beginTransmission(dir);
	Wire_write((uint8_t)(0x40 | (canal << 1)));     /* escritura multiple, UDAC = 0 */
	Wire_write(alto(canal, v));
	Wire_write((uint8_t)v);
	return Wire_endTransmission();
}

uint8_t mcp4728_setAll(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
	uint8_t i;
	valores[0] = a; valores[1] = b; valores[2] = c; valores[3] = d;
	Wire_beginTransmission(dir);
	for (i = 0; i < 4; i++) {                      /* escritura rapida: 0 0 PD1 PD0 D11..D8 */
		if (valores[i] > 4095) valores[i] = 4095;
		Wire_write((uint8_t)((valores[i] >> 8) & 0x0F));
		Wire_write((uint8_t)valores[i]);
	}
	return Wire_endTransmission();
}

uint8_t mcp4728_setVref(uint8_t ref)
{
	vref = ref ? 0x0F : 0;
	Wire_beginTransmission(dir);
	Wire_write((uint8_t)(0x80 | vref));
	return Wire_endTransmission();
}

uint8_t mcp4728_setGain(uint8_t g)
{
	gan = g ? 0x0F : 0;
	Wire_beginTransmission(dir);
	Wire_write((uint8_t)(0xC0 | gan));
	return Wire_endTransmission();
}

uint8_t mcp4728_saveEEPROM(void)
{
	uint8_t i, r;
	Wire_beginTransmission(dir);
	Wire_write(0x50);                              /* escritura secuencial A-D + EEPROM */
	for (i = 0; i < 4; i++) {
		Wire_write(alto(i, valores[i]));
		Wire_write((uint8_t)valores[i]);
	}
	r = Wire_endTransmission();
	delay(60);                                     /* escritura de la EEPROM */
	return r;
}
