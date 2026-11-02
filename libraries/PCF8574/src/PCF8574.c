/* PCF8574.c - expansor PCF8574 / PCF8574A (ver PCF8574.h). Parte de stc8duino. LGPL 2.1. */
#include <Arduino.h>
#include <Wire.h>
#include "PCF8574.h"

static __xdata uint8_t salida[8];     /* ultimo valor escrito en cada expansor */
static __xdata uint8_t entradas[8];   /* patas declaradas como entrada (siempre en 1) */

uint8_t pcf8574_write(uint8_t addr, uint8_t valor)
{
	__data uint8_t i = addr & 7;
	salida[i] = valor;
	Wire_beginTransmission(addr);
	Wire_write(valor | entradas[i]);
	return Wire_endTransmission();
}

int pcf8574_read(uint8_t addr)
{
	if (Wire_requestFrom2(addr, 1) != 1) return -1;
	return Wire_read();
}

uint8_t pcf8574_begin(uint8_t addr)
{
	__data uint8_t i = addr & 7;
	entradas[i] = 0xFF;                  /* al encender el PCF8574 todas estan en 1 */
	return pcf8574_write(addr, 0xFF);
}

void pcf8574_pinMode(uint8_t addr, uint8_t pata, uint8_t modo)
{
	__data uint8_t i = addr & 7, m = 1 << (pata & 7);
	if (modo == OUTPUT) entradas[i] &= ~m;
	else entradas[i] |= m;
	pcf8574_write(addr, salida[i]);
}

void pcf8574_digitalWrite(uint8_t addr, uint8_t pata, uint8_t valor)
{
	__data uint8_t i = addr & 7, m = 1 << (pata & 7);
	pcf8574_write(addr, valor ? (salida[i] | m) : (salida[i] & ~m));
}

void pcf8574_toggle(uint8_t addr, uint8_t pata)
{
	__data uint8_t i = addr & 7;
	pcf8574_write(addr, salida[i] ^ (1 << (pata & 7)));
}

int pcf8574_digitalRead(uint8_t addr, uint8_t pata)
{
	int v = pcf8574_read(addr);
	if (v < 0) return -1;
	return (v >> (pata & 7)) & 1;
}
