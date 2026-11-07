/*
 * ADS1115.c - ADC ADS1115 por I2C (ver ADS1115.h).
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <Wire.h>
#include "ADS1115.h"

#define REG_CONV   0x00
#define REG_CONFIG 0x01

static uint8_t dir = 0x48;
static uint8_t ganancia = ADS1115_2V048;
static uint8_t velocidad = ADS1115_128SPS;

/* escala completa en microvoltios / 256 (asi valor * k cabe en un long) */
static const unsigned int escala[6] = { 24000, 16000, 8000, 4000, 2000, 1000 };

static uint8_t escribir(uint8_t reg, unsigned int v)
{
	Wire_beginTransmission(dir);
	Wire_write(reg);
	Wire_write((uint8_t)(v >> 8));
	Wire_write((uint8_t)v);
	return Wire_endTransmission();
}

static long leer(uint8_t reg)             /* -1 si no responde */
{
	unsigned int v;
	Wire_beginTransmission(dir);
	Wire_write(reg);
	if (Wire_endTransmission() != 0) return -1;
	if (Wire_requestFrom2(dir, 2) != 2) return -1;
	v = (unsigned int)Wire_read() << 8;
	v |= (uint8_t)Wire_read();
	return v;
}

uint8_t ads1115_begin(uint8_t addr)
{
	dir = addr;
	return leer(REG_CONFIG) < 0 ? 1 : 0;
}

void ads1115_setGain(uint8_t g)     { if (g <= ADS1115_0V256) ganancia = g; }
void ads1115_setRate(uint8_t r)     { velocidad = r & 7; }

/* mux: campo MUX (bits 14:12) del registro de configuracion */
static int convertir(uint8_t mux)
{
	long v;
	unsigned long t0;
	unsigned int cfg = 0x8000                       /* OS: empezar */
	                 | ((unsigned int)mux << 12)
	                 | ((unsigned int)ganancia << 9)
	                 | 0x0100                       /* una sola conversion */
	                 | ((unsigned int)velocidad << 5)
	                 | 0x0003;                      /* comparador apagado */
	if (escribir(REG_CONFIG, cfg) != 0) return 0;
	t0 = millis();
	do {                                            /* OS = 1: termino */
		v = leer(REG_CONFIG);
		if (v < 0) return 0;
		if (millis() - t0 > 200) return 0;
	} while (!(v & 0x8000));
	v = leer(REG_CONV);
	return (int)(unsigned int)v;
}

int ads1115_readChannel(uint8_t canal)
{
	return convertir((uint8_t)(4 + (canal & 3)));    /* 100..111: AINx contra GND */
}

int ads1115_readDiff(uint8_t par)
{
	return convertir((uint8_t)(par & 3));            /* 000..011 */
}

long ads1115_toMicrovolts(int valor)
{
	return (long)valor * escala[ganancia] / 128;
}
