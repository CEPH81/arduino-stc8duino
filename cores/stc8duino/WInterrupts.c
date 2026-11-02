/*
 * WInterrupts.c - attachInterrupt() para INT0 (P3.2) e INT1 (P3.3).
 *   attachInterrupt(0, funcion, FALLING)   INT0, P3.2
 *   attachInterrupt(1, funcion, CHANGE)    INT1, P3.3
 * Los STC solo detectan flanco de bajada o cualquier flanco (CHANGE).
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

extern void (*stc_int0_func)(void);
extern void (*stc_int1_func)(void);

void attachInterrupt(uint8_t num, void (*func)(void), uint8_t mode)
{
	uint8_t falling = (mode == FALLING);
	if (num == 0) {
		stc_int0_func = func;
		IT0 = falling;          /* 1 = solo flanco de bajada, 0 = ambos flancos */
		IE0 = 0;
		EX0 = 1;
	} else if (num == 1) {
		stc_int1_func = func;
		IT1 = falling;
		IE1 = 0;
		EX1 = 1;
	}
	EA = 1;
}

void detachInterrupt(uint8_t num)
{
	if (num == 0) { EX0 = 0; stc_int0_func = 0; }
	else if (num == 1) { EX1 = 0; stc_int1_func = 0; }
}
