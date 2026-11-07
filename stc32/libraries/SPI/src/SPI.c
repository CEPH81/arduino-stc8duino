/*
 * SPI.c - SPI maestro por hardware (ver SPI.h). Por consulta, sin interrupcion.
 * Registros SPSTAT (CDh), SPCTL (CEh), SPDAT (CFh); patas con P_SW1 bits 3:2.
 * Parte de stc8duino (version stc32duino). LGPL 2.1.
 */
#include <Arduino.h>
#include "SPI.h"

sfr SPSTAT = 0xCD;
sfr SPCTL  = 0xCE;
sfr SPDAT  = 0xCF;

#define SPIF 0x80
#define WCOL 0x40

static uint8_t spi_pins;
/* MOSI, MISO, SCLK de cada opcion */
static const uint8_t spi_patas[4][3] = {
	{ P1_3, P1_4, P1_5 },
	{ P2_3, P2_4, P2_5 },
	{ P7_5, P7_6, P7_7 },
	{ P3_4, P3_3, P3_2 },
};

void SPI_setPins(uint8_t opcion)
{
	spi_pins = opcion & 3;
}

void SPI_begin(void)
{
	pinMode(spi_patas[spi_pins][0], OUTPUT);
	pinMode(spi_patas[spi_pins][1], INPUT_PULLUP);
	pinMode(spi_patas[spi_pins][2], OUTPUT);
	P_SW1 = (P_SW1 & ~0x0C) | (spi_pins << 2);
	SPCTL = 0x80 | 0x40 | 0x10;       /* SSIG (SS libre), SPEN, maestro; modo 0, MSB, F/4 */
	SPSTAT = SPIF | WCOL;
}

void SPI_end(void)
{
	SPCTL = 0;
}

void SPI_setBitOrder(uint8_t orden)
{
	if (orden == LSBFIRST) SPCTL |= 0x20;   /* DORD */
	else SPCTL &= ~0x20;
}

void SPI_setDataMode(uint8_t modo)
{
	SPCTL = (SPCTL & ~0x0C) | (modo & 0x0C);
}

/* SPR1:0 = 00 F/4, 01 F/8, 10 F/16, 11 F/2 */
void SPI_setClockDivider(uint8_t divisor)
{
	uint8_t spr;
	if (divisor <= 2) spr = 3;
	else if (divisor <= 4) spr = 0;
	else if (divisor <= 8) spr = 1;
	else spr = 2;
	SPCTL = (SPCTL & ~0x03) | spr;
}

void SPI_beginTransaction(unsigned long reloj, uint8_t orden, uint8_t modo)
{
	uint8_t div;
	if (reloj >= F_CPU / 2) div = 2;
	else if (reloj >= F_CPU / 4) div = 4;
	else if (reloj >= F_CPU / 8) div = 8;
	else div = 16;
	SPI_setClockDivider(div);
	SPI_setBitOrder(orden);
	SPI_setDataMode(modo);
}

void SPI_endTransaction(void)
{
}

uint8_t SPI_transfer(uint8_t b)
{
	SPDAT = b;
	while (!(SPSTAT & SPIF))
		;
	SPSTAT = SPIF | WCOL;              /* se borran escribiendo 1 */
	return SPDAT;
}

uint16_t SPI_transfer16(uint16_t w)
{
	uint8_t alto, bajo;
	if (SPCTL & 0x20) {                /* LSB primero: byte bajo primero */
		bajo = SPI_transfer((uint8_t)w);
		alto = SPI_transfer((uint8_t)(w >> 8));
	} else {
		alto = SPI_transfer((uint8_t)(w >> 8));
		bajo = SPI_transfer((uint8_t)w);
	}
	return ((uint16_t)alto << 8) | bajo;
}

void SPI_transferBuf(void *buf, unsigned int n)
{
	uint8_t *p = (uint8_t *)buf;
	while (n--) {
		*p = SPI_transfer(*p);
		p++;
	}
}
