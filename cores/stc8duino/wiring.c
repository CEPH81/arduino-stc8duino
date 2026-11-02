/*
 * wiring.c - init(), millis() y delay().
 *
 * El temporizador 0 (modo 1T, 16 bits con recarga automatica) interrumpe cada
 * milisegundo. F_CPU es la frecuencia del menu "Reloj" (la que deja el cargador).
 *
 * Los contadores van en la RAM interna (__data): la interrupcion solo usa el
 * acumulador y dura alrededor de 1 us a 24 MHz.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"

volatile __data unsigned long stc_ms_count;   /* milisegundos */
volatile __data unsigned long stc_us_count;   /* microsegundos al inicio del milisegundo actual */

/* stc_ms_count += 1 y stc_us_count += 1000, en ensamblador: unos 20 ciclos */
void stc_timer0_isr(void) __interrupt(1) __naked
{
	__asm
	push	psw
	push	acc
	inc	_stc_ms_count
	mov	a,_stc_ms_count
	jnz	00001$
	inc	(_stc_ms_count + 1)
	mov	a,(_stc_ms_count + 1)
	jnz	00001$
	inc	(_stc_ms_count + 2)
	mov	a,(_stc_ms_count + 2)
	jnz	00001$
	inc	(_stc_ms_count + 3)
00001$:
	mov	a,_stc_us_count
	add	a,#0xe8
	mov	_stc_us_count,a
	mov	a,(_stc_us_count + 1)
	addc	a,#0x03
	mov	(_stc_us_count + 1),a
	jnc	00002$
	inc	(_stc_us_count + 2)
	mov	a,(_stc_us_count + 2)
	jnz	00002$
	inc	(_stc_us_count + 3)
00002$:
	pop	acc
	pop	psw
	reti
	__endasm;
}

void init(void)
{
#ifdef STC8
	P_SW2 |= 0x80;          /* acceso a los registros extendidos */
#endif
	TR0 = 0;
	TMOD &= 0xF0;           /* T0 modo 0: 16 bits con recarga */
	AUXR |= AUXR_T0x12;     /* 1T: cuenta a F_CPU */
	TL0 = T0_RELOAD & 0xFF;
	TH0 = T0_RELOAD >> 8;
	TF0 = 0;
	PT0 = 1;                /* prioridad alta: millis() no se atrasa */
	ET0 = 1;
	TR0 = 1;
	EA = 1;
}

/* solo se bloquea la interrupcion del temporizador 0 (no todas) mientras se copia */
unsigned long millis(void)
{
	__data unsigned long m;
	ET0 = 0;
	m = stc_ms_count;
	ET0 = 1;
	return m;
}

/* espera ms milisegundos exactos desde la llamada: cuenta milisegundos enteros
 * y despues espera a que el temporizador llegue a la misma posicion dentro del
 * milisegundo en que empezo (con solo millis() podia quedar hasta 1 ms corto) */
static __data unsigned long d_m;
static __data unsigned int d_t;

/* lectura coherente del contador de ms y del temporizador 0 (en d_m y d_t) */
static void leer_ms_t0(void)
{
	__data uint8_t h, l;
	ET0 = 0;
	do {
		h = TH0;
		l = TL0;
	} while (h != TH0);
	d_t = ((unsigned int)h << 8) | l;
	d_m = stc_ms_count;
	/* desborde que la interrupcion aun no atendio */
	if (TF0 && d_t < (unsigned int)(T0_RELOAD + T0_TICKS / 2)) d_m++;
	ET0 = 1;
}

void delay(unsigned long ms)
{
	__data unsigned long fin;
	__data unsigned int t0;

	if (ms == 0) return;
	leer_ms_t0();
	fin = d_m + ms;
	t0 = d_t;
	do {
		leer_ms_t0();
	} while ((long)(d_m - fin) < 0);
	while (d_m == fin && d_t < t0)
		leer_ms_t0();
}
