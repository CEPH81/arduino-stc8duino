/*
 * timer1.c - temporizador 1 con interrupcion periodica.
 *
 *   timer1Begin(1000, funcion);   // llama a funcion() cada 1000 us
 *   timer1End();
 *
 * Periodo de 1 us a mas de una hora. Hasta 65536 ciclos (2,7 ms a 24 MHz) cuenta
 * a F_CPU; por encima, a F_CPU/12 con un divisor por programa.
 * La funcion se ejecuta dentro de la interrupcion: debe ser corta y las
 * variables que comparta con loop() deben ser volatile.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"

extern void (* __data stc_t1_func)(void);
extern __data unsigned int stc_t1_div, stc_t1_cnt;   /* en timer1_isr.c */

void timer1Begin(unsigned long periodo_us, void (*func)(void))
{
	unsigned long ciclos;
	unsigned int div = 1;
	uint8_t x12 = 1;

	/* T0_TICKS = cuentas por ms con la frecuencia real del chip */
	if (periodo_us > 100000UL)
		ciclos = (periodo_us / 1000UL) * T0_TICKS;
	else
		ciclos = (periodo_us * T0_TICKS) / 1000UL;
	if (ciclos < 24) ciclos = 24;             /* minimo razonable para la interrupcion */
	if (ciclos > 65536UL) {
		x12 = 0;
		ciclos /= 12;
		div = (unsigned int)((ciclos + 65535UL) / 65536UL);
		ciclos /= div;
	}
	TR1 = 0;
	ET1 = 0;
	stc_t1_func = func;
	stc_t1_div = stc_t1_cnt = div;
	TMOD &= 0x0F;                             /* T1 modo 0: 16 bits con recarga */
	if (x12) AUXR |= AUXR_T1x12;
	else AUXR &= ~AUXR_T1x12;
	TL1 = (uint8_t)(65536UL - ciclos);
	TH1 = (uint8_t)((65536UL - ciclos) >> 8);
	TF1 = 0;
	ET1 = 1;
	TR1 = 1;
	EA = 1;
}

void timer1End(void)
{
	TR1 = 0;
	ET1 = 0;
	stc_t1_func = 0;
}
