/*
 * int_isr.c - interrupciones INT0 e INT1: llaman a la funcion de attachInterrupt().
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

void (*stc_int0_func)(void);
void (*stc_int1_func)(void);

void stc_int0_isr(void) __interrupt(0)
{
	if (stc_int0_func) stc_int0_func();
}

void stc_int1_isr(void) __interrupt(2)
{
	if (stc_int1_func) stc_int1_func();
}
