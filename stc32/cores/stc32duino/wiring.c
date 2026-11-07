/*
 * wiring.c - inicio del chip y tiempo (millis/micros/delay) con el
 * temporizador 0 en modo 1T con recarga automatica: una interrupcion por ms.
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

#define T0_TICKS  (F_CPU / 1000UL)              /* ciclos por ms */
#define T0_RELOAD ((unsigned int)(65536UL - T0_TICKS))
/* fraccion del ms en us = ciclos * T0_K >> 16, con T0_K = 65536000 / ciclos por ms.
 * Vale para cualquier frecuencia (11,0592, 22,1184, 33,1776 MHz...) y nunca
 * pasa de 999 us: micros() no retrocede al cambiar de milisegundo. */
#define T0_K      ((unsigned long)(65536000UL / T0_TICKS))

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
	return m * 1000UL + (((unsigned long)t * T0_K) >> 16);
}

/* espera 'us' microsegundos contados con micros() (hasta unos 71 minutos) */
static void espera_us(unsigned long us)
{
	unsigned long t0 = micros();
	while (micros() - t0 < us)
		;
}

/* Con micros() y no con millis(): el primer milisegundo de millis() ya puede
 * estar empezado, y delay(100) daba unos 99,2 ms. */
void delay(unsigned long ms)
{
	while (ms >= 1000) {
		espera_us(1000000UL);
		ms -= 1000;
	}
	if (ms)
		espera_us(ms * 1000UL);
}

/* DELAYUS_AJUSTE: lo que tardan la llamada y la primera lectura de micros().
 * Medido con PruebaTiempos a 24 MHz: unos 3 us (72 ciclos); a otras
 * frecuencias se escala: 12 MHz 6 us, 35 MHz 2 us. */
#define DELAYUS_AJUSTE ((unsigned int)((72000000UL + F_CPU / 2) / F_CPU))

void delayMicroseconds(unsigned int us)
{
	unsigned long t0 = micros();
	if (us <= DELAYUS_AJUSTE)
		return;
	us -= DELAYUS_AJUSTE;
	while (micros() - t0 < us)
		;
}
