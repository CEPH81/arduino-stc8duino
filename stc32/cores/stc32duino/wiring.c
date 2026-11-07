/*
 * wiring.c - inicio del chip y tiempo (millis/micros/delay) con el
 * temporizador 0 en modo 1T con recarga automatica: una interrupcion por ms.
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

#define T0_TICKS  (F_CPU / 1000UL)              /* ciclos por ms */
#define T0_RELOAD ((unsigned int)(65536UL - T0_TICKS))
#define T0_TPU    ((unsigned int)(F_CPU / 1000000UL))   /* ciclos por us */

static volatile unsigned long ms_count;

void timer0_isr(void) interrupt 1
{
	ms_count++;
}

void init(void)
{
	WTST = 0;          /* sin esperas al leer la flash: maxima velocidad */
	EAXFR = 1;         /* acceso a los registros extendidos (XFR) */
	CKCON = 0;

	TR0 = 0;
	AUXR |= 0x80;      /* T0 en modo 1T: cuenta a F_CPU */
	TMOD &= 0xF0;      /* T0 modo 0: 16 bits con recarga */
	TL0 = (unsigned char)T0_RELOAD;
	TH0 = (unsigned char)(T0_RELOAD >> 8);
	TF0 = 0;
	TR0 = 1;
	ET0 = 1;
	EA = 1;
}

unsigned long millis(void)
{
	unsigned long m;
	ET0 = 0;
	m = ms_count;
	ET0 = 1;
	return m;
}

unsigned long micros(void)
{
	unsigned long m;
	unsigned char h, l;
	unsigned int t;

	ET0 = 0;
	h = TH0;
	l = TL0;
	if (h != TH0) {        /* TL0 dio la vuelta entre las dos lecturas */
		h = TH0;
		l = TL0;
	}
	m = ms_count;
	t = (((unsigned int)h << 8) | l) - T0_RELOAD;
	if (TF0 && t < T0_TICKS / 2)
		m++;           /* desbordo y la interrupcion aun no corrio */
	ET0 = 1;
	return m * 1000UL + t / T0_TPU;
}

void delay(unsigned long ms)
{
	unsigned long t0 = millis();
	while (millis() - t0 < ms)
		;
}

void delayMicroseconds(unsigned int us)
{
	unsigned long t0 = micros();
	while (micros() - t0 < us)
		;
}
