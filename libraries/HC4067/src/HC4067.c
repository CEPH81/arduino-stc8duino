/*
 * HC4067.c - multiplexor 74HC4067 (ver HC4067.h).
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include "HC4067.h"

static uint8_t pins[4], pin_sig, pin_en = HC4067_SIN_EN, actual = 0xFF;

void hc4067_begin(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t sig, uint8_t en)
{
	uint8_t i;
	pins[0] = s0; pins[1] = s1; pins[2] = s2; pins[3] = s3;
	pin_sig = sig;
	pin_en = en;
	for (i = 0; i < 4; i++) pinMode(pins[i], OUTPUT);
	if (en != HC4067_SIN_EN) {
		pinMode(en, OUTPUT);
		digitalWrite(en, LOW);               /* activo en bajo */
	}
	actual = 0xFF;
	hc4067_select(0);
}

void hc4067_select(uint8_t canal)
{
	uint8_t i;
	canal &= 15;
	if (canal == actual) return;
	actual = canal;
	for (i = 0; i < 4; i++) digitalWrite(pins[i], (uint8_t)((canal >> i) & 1));
	delayMicroseconds(5);                    /* que se asiente la entrada */
}

void hc4067_enable(uint8_t si)
{
	if (pin_en != HC4067_SIN_EN) digitalWrite(pin_en, si ? LOW : HIGH);
}

void hc4067_sigMode(uint8_t modo) { pinMode(pin_sig, modo); }

int hc4067_analogRead(uint8_t canal)
{
	hc4067_select(canal);
	return analogRead(pin_sig);
}

uint8_t hc4067_digitalRead(uint8_t canal)
{
	hc4067_select(canal);
	return (uint8_t)digitalRead(pin_sig);
}

void hc4067_digitalWrite(uint8_t canal, uint8_t valor)
{
	hc4067_select(canal);
	digitalWrite(pin_sig, valor);
}
