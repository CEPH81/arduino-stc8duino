/*
 * WInterrupts.c - attachInterrupt() para INT0 (P3.2) e INT1 (P3.3).
 *   attachInterrupt(digitalPinToInterrupt(P3_3), funcion, FALLING)
 * El STC32G detecta flanco de bajada (FALLING) o los dos flancos (CHANGE).
 * La funcion se ejecuta dentro de la interrupcion: debe ser corta, y las
 * variables que comparta con loop() deben ser volatile.
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

static void (*int0_func)(void);
static void (*int1_func)(void);

void int0_isr(void) interrupt 0
{
	if (int0_func) int0_func();
}

void int1_isr(void) interrupt 2
{
	if (int1_func) int1_func();
}

void attachInterrupt(uint8_t num, void (*func)(void), uint8_t mode)
{
	uint8_t bajada = (mode == FALLING);
	if (num == 0) {
		EX0 = 0;
		int0_func = func;
		IT0 = bajada;           /* 1 = solo flanco de bajada, 0 = los dos */
		IE0 = 0;
		EX0 = 1;
	} else if (num == 1) {
		EX1 = 0;
		int1_func = func;
		IT1 = bajada;
		IE1 = 0;
		EX1 = 1;
	}
	EA = 1;
}

void detachInterrupt(uint8_t num)
{
	if (num == 0) { EX0 = 0; int0_func = 0; }
	else if (num == 1) { EX1 = 0; int1_func = 0; }
}
