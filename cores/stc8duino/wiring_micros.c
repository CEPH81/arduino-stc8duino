/*
 * wiring_micros.c - micros() y delayMicroseconds()
 * (en un modulo aparte para que solo se enlace si se usa).
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

#define T0_TICKS (F_CPU / 1000)
#define T0_RELOAD (65536UL - T0_TICKS)
extern volatile unsigned long stc_ms_count;

unsigned long micros(void)
{
	unsigned long m;
	unsigned int t;
	uint8_t h, l;

	EA = 0;
	m = stc_ms_count;
	do {
		h = TH0;
		l = TL0;
	} while (h != TH0);
	t = ((unsigned int)h << 8) | l;
	if (TF0 && t < (unsigned int)(T0_RELOAD + T0_TICKS / 2)) m++;   /* desborde aun sin atender */
	EA = 1;
	t -= (unsigned int)T0_RELOAD;
	return m * 1000UL + t / (F_CPU / 1000000UL);
}

/* espera activa: unos 4 ciclos por vuelta interna en los STC 1T */
void delayMicroseconds(unsigned int us)
{
	if (us > 50) {
		unsigned long start = micros();
		while (micros() - start < us)
			;
		return;
	}
	while (us--) {
		uint8_t n = (F_CPU / 1000000UL + 2) / 4;
		while (--n)
			;
	}
}

