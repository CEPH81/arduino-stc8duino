/*
 * wiring_micros.c - micros() y delayMicroseconds()
 * (en un modulo aparte para que solo se enlace si se usa).
 *
 * Ambas leen el temporizador 0 (cuenta a F_CPU), asi que la precision no depende
 * de cuanto tarde cada instruccion ni del modelo de memoria.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"

/* lectura coherente de TH0:TL0 */
static unsigned int leer_t0(void)
{
	__data uint8_t h, l;
	do {
		h = TH0;
		l = TL0;
	} while (h != TH0);
	return ((unsigned int)h << 8) | l;
}

unsigned long micros(void)
{
	__data unsigned long u;
	__data unsigned int t;

	ET0 = 0;
	u = stc_us_count;
	t = leer_t0();
	/* desborde que la interrupcion aun no atendio */
	if (TF0 && t < (unsigned int)(T0_RELOAD + T0_TICKS / 2)) u += 1000;
	ET0 = 1;
	t -= T0_RELOAD;                       /* cuentas dentro del milisegundo */
#if (F_CPU % 1000000UL) == 0
	t /= (unsigned int)(F_CPU / 1000000UL);
#else
	t = (unsigned int)(((unsigned long)t * 1000UL) / T0_TICKS);
#endif
	return u + t;
}

/* espera de us microsegundos. Todo en 16 bits y con las variables en la RAM
 * interna: se cuenta el temporizador 0 por tramos de hasta 1000 us. */
#define LEER_T0(v) do { \
		__data uint8_t h_, l_; \
		do { h_ = TH0; l_ = TL0; } while (h_ != TH0); \
		v = ((unsigned int)h_ << 8) | l_; \
	} while (0)

void delayMicroseconds(unsigned int us)
{
	__data unsigned int prev, now, falta, hecho, n;

	LEER_T0(prev);
	n = us;
	if (n <= 2) return;                   /* la llamada ya tarda alrededor de 2 us */
	n -= 2;
	while (n) {
		if (n > 1000) {
			falta = 1000 * T0_TPU;
			n -= 1000;
		} else {
			falta = n * T0_TPU;
			n = 0;
		}
		hecho = 0;
		while (hecho < falta) {
			LEER_T0(now);
			/* el temporizador va de T0_RELOAD a 65535 y vuelve a T0_RELOAD */
			hecho += (now >= prev) ? now - prev : now - prev - T0_RELOAD;
			prev = now;
		}
		prev -= hecho - falta;            /* lo que se paso cuenta para el tramo siguiente */
	}
}
