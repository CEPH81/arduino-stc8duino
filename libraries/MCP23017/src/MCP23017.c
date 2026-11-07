/*
 * MCP23017.c - expansor MCP23017 por I2C (ver MCP23017.h). Registros en el
 * orden de fabrica (IOCON.BANK = 0): el de B esta en la direccion de A + 1.
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <Wire.h>
#include "MCP23017.h"

#define IODIR 0x00
#define GPPU  0x0C
#define GPIO  0x12
#define OLAT  0x14

static uint8_t escribir(uint8_t addr, uint8_t reg, uint8_t v)
{
	Wire_beginTransmission(addr);
	Wire_write(reg);
	Wire_write(v);
	return Wire_endTransmission();
}

static int leer(uint8_t addr, uint8_t reg)
{
	Wire_beginTransmission(addr);
	Wire_write(reg);
	if (Wire_endTransmission() != 0) return -1;
	if (Wire_requestFrom2(addr, 1) != 1) return -1;
	return (uint8_t)Wire_read();
}

/* cambia un bit de un registro (lee, modifica y escribe) */
static uint8_t bit_reg(uint8_t addr, uint8_t reg, uint8_t pata, uint8_t valor)
{
	int v;
	uint8_t m = (uint8_t)(1 << (pata & 7));
	reg = (uint8_t)(reg + ((pata >> 3) & 1));
	v = leer(addr, reg);
	if (v < 0) return 2;
	return escribir(addr, reg, (uint8_t)(valor ? (v | m) : (v & ~m)));
}

uint8_t mcp23017_begin(uint8_t addr)
{
	uint8_t r = mcp23017_portMode(addr, MCP23017_A, 0xFF, 0);
	if (r == 0) r = mcp23017_portMode(addr, MCP23017_B, 0xFF, 0);
	return r;
}

uint8_t mcp23017_pinMode(uint8_t addr, uint8_t pata, uint8_t modo)
{
	uint8_t r = bit_reg(addr, IODIR, pata, modo != OUTPUT);
	if (r == 0) r = bit_reg(addr, GPPU, pata, modo == INPUT_PULLUP);
	return r;
}

uint8_t mcp23017_digitalWrite(uint8_t addr, uint8_t pata, uint8_t valor)
{
	return bit_reg(addr, OLAT, pata, valor);
}

int mcp23017_digitalRead(uint8_t addr, uint8_t pata)
{
	int v = leer(addr, (uint8_t)(GPIO + ((pata >> 3) & 1)));
	if (v < 0) return -1;
	return (v >> (pata & 7)) & 1;
}

uint8_t mcp23017_portMode(uint8_t addr, uint8_t puerto, uint8_t entradas, uint8_t pullups)
{
	uint8_t r = escribir(addr, (uint8_t)(IODIR + (puerto & 1)), entradas);
	if (r == 0) r = escribir(addr, (uint8_t)(GPPU + (puerto & 1)), pullups);
	return r;
}

uint8_t mcp23017_writePort(uint8_t addr, uint8_t puerto, uint8_t valor)
{
	return escribir(addr, (uint8_t)(OLAT + (puerto & 1)), valor);
}

int mcp23017_readPort(uint8_t addr, uint8_t puerto)
{
	return leer(addr, (uint8_t)(GPIO + (puerto & 1)));
}
