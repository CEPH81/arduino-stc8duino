/* timer1_isr.c - interrupcion del temporizador 1 (timer1Begin). Va aparte de
 * timer1.c porque su vector esta siempre en main.c. Parte de stc8duino. LGPL 2.1. */
#include "Arduino.h"

void (* __data stc_t1_func)(void);
__data unsigned int stc_t1_div, stc_t1_cnt;

void stc_timer1_isr(void) __interrupt(3)
{
	if (--stc_t1_cnt == 0) {
		stc_t1_cnt = stc_t1_div;
		if (stc_t1_func) stc_t1_func();
	}
}
