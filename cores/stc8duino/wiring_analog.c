/*
 * wiring_analog.c - analogRead().
 *
 * ADC de cada familia:
 *   STC8H, STC8A  12 bits   canales 0-7 = P1.0-P1.7 (A0-A7), 8-14 = P0.0-P0.6 (A8-A14)
 *   STC8G         10 bits   canales 0-7 = P1.0-P1.7
 *   STC15         10 bits   canales 0-7 = P1.0-P1.7
 * Por defecto analogRead() da 10 bits (0-1023) como Arduino;
 * analogReadResolution(12) da el valor completo en los ADC de 12 bits.
 * La referencia es la alimentacion (Vcc).
 *
 * Una lectura tarda unos 20 us a 24 MHz (reloj del ADC de unos 1,5 MHz).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

#if defined(STC8H) || defined(STC8A)
#define ADC_BITS 12
#else
#define ADC_BITS 10
#endif

#ifdef STC8
/* ADCCFG: SPEED = reloj del ADC F_CPU / 2 / (SPEED + 1), cerca de 1,5 MHz */
#define ADC_SPEED_RAW ((F_CPU / 3000000UL) > 0 ? (F_CPU / 3000000UL) - 1 : 0)
#define ADC_SPEED (ADC_SPEED_RAW > 15 ? 15 : ADC_SPEED_RAW)
#endif

static __data uint8_t read_bits = 10;

void analogReadResolution(uint8_t bits)
{
	if (bits >= 8 && bits <= 16) read_bits = bits;
}

int analogRead(uint8_t pin)
{
	__data uint8_t ch, m;
	__data uint16_t v;

	m = digitalPinToBitMask(pin);
	ch = digitalPinToBit(pin);
	/* pata en alta impedancia (entrada analogica) sin pasar por pinMode() */
	switch (digitalPinToPort(pin)) {
	case 1:
		P1M1 |= m; P1M0 &= ~m;
#ifndef STC8
		P1ASF |= m;
#endif
		break;
#if defined(STC8H) || defined(STC8A)
	case 0:
		if (ch == 7) return 0;
		P0M1 |= m; P0M0 &= ~m;
		ch += 8;
		break;
#endif
	default:
		return 0;
	}

#ifdef STC8
	if (!(ADC_CONTR & 0x80)) {
		ADCCFG = 0x20 | ADC_SPEED;        /* resultado alineado a la derecha */
		ADC_CONTR = 0x80;                 /* encender el ADC y esperar a que se estabilice */
		delay(1);
	}
	ADC_CONTR = 0x80 | 0x40 | ch;         /* canal y ADC_START */
	__asm__("nop"); __asm__("nop");
	while (!(ADC_CONTR & 0x20))           /* ADC_FLAG */
		;
	ADC_CONTR &= ~0x20;
	v = ((uint16_t)ADC_RES << 8) | ADC_RESL;
#else
	if (!(ADC_CONTR & 0x80)) {
		PCON2 |= 0x20;                    /* ADRJ: 2 bits altos en ADC_RES */
		ADC_CONTR = 0x80;
		delay(2);
	}
	ADC_CONTR = 0x80 | 0x20 | ch;         /* 360 ciclos por conversion */
	ADC_CONTR |= 0x08;                    /* ADC_START */
	__asm__("nop"); __asm__("nop"); __asm__("nop"); __asm__("nop");
	while (!(ADC_CONTR & 0x10))           /* ADC_FLAG */
		;
	ADC_CONTR &= ~0x10;
	v = ((uint16_t)(ADC_RES & 0x03) << 8) | ADC_RESL;
#endif
	if (read_bits > ADC_BITS) return v << (read_bits - ADC_BITS);
	return v >> (ADC_BITS - read_bits);
}
