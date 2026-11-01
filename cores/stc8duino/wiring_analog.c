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
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

#if defined(STC8H) || defined(STC8A)
#define ADC_BITS 12
#else
#define ADC_BITS 10
#endif

static uint8_t read_bits = 10;

void analogReadResolution(uint8_t bits)
{
	if (bits >= 8 && bits <= 16) read_bits = bits;
}

static uint8_t pin_to_channel(uint8_t pin)
{
	uint8_t port = digitalPinToPort(pin), b = digitalPinToBit(pin);
	if (port == 1) return b;
#if defined(STC8H) || defined(STC8A)
	if (port == 0 && b < 7) return 8 + b;
#endif
	return 0xFF;
}

int analogRead(uint8_t pin)
{
	uint8_t ch = pin_to_channel(pin);
	uint16_t v;

	if (ch == 0xFF) return 0;
	pinMode(pin, INPUT);

#ifdef STC8
	ADCCFG = 0x2F;                        /* resultado alineado a la derecha, reloj mas lento */
	if (!(ADC_CONTR & 0x80)) {
		ADC_CONTR = 0x80;                 /* encender el ADC y esperar a que se estabilice */
		delayMicroseconds(200);
	}
	ADC_CONTR = 0x80 | ch;
	ADC_CONTR |= 0x40;                    /* ADC_START */
	__asm__("nop"); __asm__("nop");
	while (!(ADC_CONTR & 0x20))           /* ADC_FLAG */
		;
	ADC_CONTR &= ~0x20;
	v = ((uint16_t)ADC_RES << 8) | ADC_RESL;
#else
	P1ASF |= 1 << ch;                     /* pata como entrada analogica */
	PCON2 |= 0x20;                        /* ADRJ: 2 bits altos en ADC_RES */
	if (!(ADC_CONTR & 0x80)) {
		ADC_CONTR = 0x80;
		delay(2);
	}
	ADC_CONTR = 0x80 | 0x00 | ch;         /* velocidad mas lenta, mas precisa */
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
