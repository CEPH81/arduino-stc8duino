/*
 * HX711.c - amplificador HX711 (ver HX711.h).
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include "HX711.h"

static uint8_t pin_dt, pin_sck, pulsos = HX711_128;
static long cero;
static float escala = 1.0f;

void hx711_begin(uint8_t dt, uint8_t sck, uint8_t ganancia)
{
	pin_dt = dt;
	pin_sck = sck;
	pulsos = ganancia;
	pinMode(dt, INPUT);
	pinMode(sck, OUTPUT);
	digitalWrite(sck, LOW);
	hx711_read();                    /* fija la ganancia para la proxima lectura */
}

uint8_t hx711_ready(void)
{
	return digitalRead(pin_dt) == LOW;
}

long hx711_read(void)
{
	unsigned long v = 0;
	unsigned long t0 = millis();
	uint8_t i;
	while (!hx711_ready())
		if (millis() - t0 > 1000) return 0;
	noInterrupts();
	for (i = 0; i < 24; i++) {
		digitalWrite(pin_sck, HIGH);
		v <<= 1;
		digitalWrite(pin_sck, LOW);
		if (digitalRead(pin_dt)) v |= 1;
	}
	for (i = 0; i < pulsos; i++) {   /* 1-3 pulsos mas: ganancia de la siguiente */
		digitalWrite(pin_sck, HIGH);
		digitalWrite(pin_sck, LOW);
	}
	interrupts();
	if (v & 0x800000UL) v |= 0xFF000000UL;   /* signo de 24 bits */
	return (long)v;
}

long hx711_readAverage(uint8_t n)
{
	long suma = 0;
	uint8_t i;
	if (n == 0) n = 1;
	for (i = 0; i < n; i++) suma += hx711_read();
	return suma / n;
}

void  hx711_tare(uint8_t n)           { cero = hx711_readAverage(n); }
void  hx711_setOffset(long c)         { cero = c; }
long  hx711_getOffset(void)           { return cero; }
long  hx711_getValue(uint8_t n)       { return hx711_readAverage(n) - cero; }
void  hx711_setScale(float e)         { if (e != 0.0f) escala = e; }
float hx711_getScale(void)            { return escala; }
float hx711_getUnits(uint8_t n)       { return (float)hx711_getValue(n) / escala; }

void hx711_powerDown(void)
{
	digitalWrite(pin_sck, LOW);
	digitalWrite(pin_sck, HIGH);     /* mas de 60 us en alto: se duerme */
	delayMicroseconds(80);
}

void hx711_powerUp(void)
{
	digitalWrite(pin_sck, LOW);      /* despierta y vuelve al canal A, x128 */
	if (pulsos != HX711_128) hx711_read();
}
