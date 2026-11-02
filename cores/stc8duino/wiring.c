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

void delay(unsigned long ms)
{
	__data unsigned long start;
	if (ms == 0) return;
	start = millis();
	while (millis() - start < ms)
		;
}
