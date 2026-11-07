/*
 * wiring_analog.c - analogRead() con el ADC de 12 bits del STC32G.
 *
 * Canales 0-7 = P1.0-P1.7 (A0-A7), 8-14 = P0.0-P0.6 (A8-A14).
 * Por defecto da 10 bits (0-1023) como Arduino; analogReadResolution(12)
 * da el valor completo (0-4095). La referencia es la alimentacion (Vcc).
 * Reloj del ADC cerca de 1,5 MHz: una lectura tarda unos 20 us.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

#define ADC_BITS 12
/* ADCCFG: SPEED = reloj del ADC F_CPU / 2 / (SPEED + 1), cerca de 1,5 MHz */
#define ADC_SPEED_RAW ((F_CPU / 3000000UL) > 0 ? (F_CPU / 3000000UL) - 1 : 0)
#define ADC_SPEED ((unsigned char)(ADC_SPEED_RAW > 15 ? 15 : ADC_SPEED_RAW))

static unsigned char read_bits = 10;

void analogReadResolution(uint8_t bits)
{
	if (bits >= 8 && bits <= 16) read_bits = bits;
}

int analogRead(uint8_t pin)
{
	unsigned char ch, m;
	unsigned int v;

	m = 1 << (pin & 7);
	ch = pin & 7;
	/* pata en alta impedancia (entrada analogica) sin pasar por pinMode() */
	switch (pin >> 4) {
	case 1:
		P1M1 |= m; P1M0 &= ~m;
		break;
	case 0:
		if (ch == 7) return 0;
		P0M1 |= m; P0M0 &= ~m;
		ch += 8;
		break;
	default:
		return 0;
	}

	if (!(ADC_CONTR & 0x80)) {
		ADCCFG = 0x20 | ADC_SPEED;        /* resultado alineado a la derecha */
		ADC_CONTR = 0x80;                 /* encender el ADC y esperar a que se estabilice */
		delay(1);
	}
	ADC_CONTR = 0x80 | 0x40 | ch;         /* canal y ADC_START */
	v = ADC_CONTR;                        /* pausa de unos ciclos */
	v = ADC_CONTR;
	while (!(ADC_CONTR & 0x20))           /* ADC_FLAG */
		;
	ADC_CONTR &= ~0x20;
	v = ((unsigned int)ADC_RES << 8) | ADC_RESL;
	if (read_bits > ADC_BITS) return v << (read_bits - ADC_BITS);
	return v >> (ADC_BITS - read_bits);
}
