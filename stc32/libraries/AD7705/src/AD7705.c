/*
 * AD7705.c - ADC AD7705 / TM7705 por SPI (ver AD7705.h).
 * Registro de comunicacion: [/DRDY RS2 RS1 RS0 R/W STBY CH1 CH0]
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <SPI.h>
#include "AD7705.h"

#define R_COM    0x00
#define R_SETUP  0x10
#define R_CLOCK  0x20
#define R_DATO   0x30
#define LEER     0x08

static uint8_t pin_cs, pin_drdy = AD7705_SIN_DRDY, reloj, ultimo;

static void sel(void)
{
	SPI_beginTransaction(1000000UL, MSBFIRST, SPI_MODE3);
	digitalWrite(pin_cs, LOW);
}

static void desel(void)
{
	digitalWrite(pin_cs, HIGH);
}

static void escribir(uint8_t com, uint8_t v)
{
	sel();
	SPI_transfer(com);
	SPI_transfer(v);
	desel();
}

static uint8_t leer8(uint8_t com)
{
	uint8_t v;
	sel();
	SPI_transfer(com);
	v = SPI_transfer(0xFF);
	desel();
	return v;
}

uint8_t ad7705_ready(uint8_t canal)
{
	if (pin_drdy != AD7705_SIN_DRDY) return digitalRead(pin_drdy) == LOW;
	return (leer8((uint8_t)(R_COM | LEER | (canal & 1))) & 0x80) == 0;
}

static uint8_t esperar(uint8_t canal)
{
	unsigned long t0 = millis();
	while (!ad7705_ready(canal))
		if (millis() - t0 > 500) return 1;
	return 0;
}

uint8_t ad7705_begin(uint8_t cs, uint8_t drdy, uint8_t velocidad)
{
	uint8_t i;
	pin_cs = cs;
	pin_drdy = drdy;
	pinMode(cs, OUTPUT);
	digitalWrite(cs, HIGH);
	if (drdy != AD7705_SIN_DRDY) pinMode(drdy, INPUT);
	sel();                                       /* reinicio: 32 unos seguidos */
	for (i = 0; i < 5; i++) SPI_transfer(0xFF);
	desel();
	delay(2);
	/* reloj: CLKDIV = 1, CLK = 1 (cristal de 4,9152 MHz) + velocidad */
	reloj = (uint8_t)(0x0C | (velocidad & 3));
	escribir(R_CLOCK, reloj);
	return (leer8(R_CLOCK | LEER) & 0x1F) == reloj ? 0 : 1;
}

uint8_t ad7705_setup(uint8_t canal, uint8_t ganancia, uint8_t unipolar)
{
	canal &= 1;
	escribir((uint8_t)(R_CLOCK | canal), reloj);
	/* autocalibracion (MD = 01) + ganancia + B/U, sin bufer ni FSYNC */
	escribir((uint8_t)(R_SETUP | canal),
	         (uint8_t)(0x40 | ((ganancia & 7) << 3) | (unipolar ? 0x04 : 0)));
	ultimo = canal;
	return esperar(canal);
}

static unsigned int dato(uint8_t canal)
{
	unsigned int v;
	if (esperar(canal)) return 0;
	sel();
	SPI_transfer((uint8_t)(R_DATO | LEER | canal));
	v = (unsigned int)SPI_transfer(0xFF) << 8;
	v |= SPI_transfer(0xFF);
	desel();
	return v;
}

unsigned int ad7705_read(uint8_t canal)
{
	canal &= 1;
	if (canal != ultimo) {
		/* cambio de canal: el filtro necesita unas conversiones para asentarse */
		ultimo = canal;
		leer8((uint8_t)(R_COM | LEER | canal));
		dato(canal);
		dato(canal);
	}
	return dato(canal);
}

long ad7705_toMicrovolts(unsigned int valor, unsigned int vref_mv, uint8_t ganancia, uint8_t unipolar)
{
	long x, uv;
	if (unipolar) x = (long)valor;                       /* 0..65535 = 0..Vref */
	else x = ((long)valor - 32768L) * 2;                 /* -65536..65534 = -Vref..Vref */
	/* x * vref_mv * 1000 / 65536 en dos pasos para no desbordar un long */
	uv = x * (long)vref_mv;                             /* hasta 65536 * 5000 */
	uv = (uv / 65536L) * 1000L + ((uv % 65536L) * 1000L) / 65536L;
	return uv / (1L << (ganancia & 7));
}
