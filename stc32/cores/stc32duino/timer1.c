/*
 * timer1.c - temporizador 1 con interrupcion periodica.
 *
 *   timer1Begin(1000, funcion);   // llama a funcion() cada 1000 us
 *   timer1End();
 *
 * Periodo de unos pocos us a mas de una hora. Hasta 65536 ciclos (2,7 ms a
 * 24 MHz) cuenta a F_CPU; por encima, a F_CPU/12 con un divisor por programa.
 * La funcion se ejecuta dentro de la interrupcion: debe ser corta y las
 * variables que comparta con loop() deben ser volatile.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

#define T1_TICKS_MS (F_CPU / 1000UL)    /* ciclos por ms */

static void (*t1_func)(void);
static volatile unsigned int t1_div, t1_cnt;

void timer1_isr(void) interrupt 3
{
	if (--t1_cnt == 0) {
		t1_cnt = t1_div;
		if (t1_func) t1_func();
	}
}

void timer1Begin(unsigned long periodo_us, void (*func)(void))
{
	unsigned long ciclos;
	unsigned int div = 1;
	unsigned char x12 = 1;

	if (periodo_us > 100000UL)
		ciclos = (periodo_us / 1000UL) * T1_TICKS_MS;
	else
		ciclos = (periodo_us * T1_TICKS_MS) / 1000UL;
	if (ciclos < 48) ciclos = 48;             /* minimo razonable para la interrupcion */
	if (ciclos > 65536UL) {
		x12 = 0;
		ciclos /= 12;
		div = (unsigned int)((ciclos + 65535UL) / 65536UL);
		ciclos /= div;
	}
	TR1 = 0;
	ET1 = 0;
	t1_func = func;
	t1_div = t1_cnt = div;
	TMOD &= 0x0F;                             /* T1 modo 0: 16 bits con recarga */
	if (x12) AUXR |= 0x40;                    /* T1x12: cuenta a F_CPU */
	else AUXR &= ~0x40;                       /* cuenta a F_CPU/12 */
	TL1 = (unsigned char)(65536UL - ciclos);
	TH1 = (unsigned char)((65536UL - ciclos) >> 8);
	TF1 = 0;
	ET1 = 1;
	TR1 = 1;
	EA = 1;
}

void timer1End(void)
{
	TR1 = 0;
	ET1 = 0;
	t1_func = 0;
}
