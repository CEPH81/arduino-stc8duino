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

/* microsegundos dentro del milisegundo: t * 1000 / T0_TICKS sin dividir.
 * Se calcula (t * K) >> 16 con K = 65536000 / T0_TICKS usando solo
 * multiplicaciones de 8 x 8 bits (la instruccion MUL AB del 8051). */
#define US_K  ((unsigned int)((65536000UL + T0_TICKS / 2) / T0_TICKS))
#define US_KL ((uint8_t)(US_K & 0xFF))
#define US_KH ((uint8_t)(US_K >> 8))

static __data uint8_t us_kl = US_KL, us_kh = US_KH;

/* devuelve los 16 bits altos de t * K (entrada y salida en dpl/dph) */
static unsigned int mul_k_alto(unsigned int t) __naked
{
	(void) t;
	__asm
	mov	a,dpl
	mov	b,_us_kl
	mul	ab              ; tl*kl
	mov	r2,b
	mov	a,dpl
	mov	b,_us_kh
	mul	ab              ; tl*kh
	add	a,r2
	mov	r2,a
	clr	a
	addc	a,b
	mov	r3,a
	mov	r4,#0
	mov	a,dph
	mov	b,_us_kl
	mul	ab              ; th*kl
	add	a,r2
	mov	a,b
	addc	a,r3
	mov	r3,a
	clr	a
	addc	a,r4
	mov	r4,a
	mov	a,dph
	mov	b,_us_kh
	mul	ab              ; th*kh
	add	a,r3
	mov	dpl,a
	mov	a,b
	addc	a,r4
	mov	dph,a
	ret
	__endasm;
}

static unsigned int frac_us(unsigned int t)
{
	__data unsigned int d = mul_k_alto(t);
	return d > 999 ? 999 : d;
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
	return u + frac_us(t - T0_RELOAD);    /* cuentas dentro del milisegundo -> us */
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
