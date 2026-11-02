/*
 * WInterrupts.c - attachInterrupt() para las interrupciones externas.
 *
 *   numero  pata   flancos
 *   0       P3.2   FALLING o CHANGE
 *   1       P3.3   FALLING o CHANGE
 *   2       P3.6   solo FALLING
 *   3       P3.7   solo FALLING
 *   4       P3.0   solo FALLING (es el RX de Serial)
 *
 * attachInterrupt(digitalPinToInterrupt(P3_2), funcion, FALLING) o attachInterrupt(0, ...).
 * Los STC no tienen RISING: se trata como CHANGE en INT0/INT1 (la funcion puede
 * leer la pata con digitalRead para saber que flanco fue).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

extern void (* __data stc_int_func[5])(void);

void attachInterrupt(uint8_t num, void (*func)(void), uint8_t mode)
{
	if (num > 4) return;
	stc_int_func[num] = func;
	switch (num) {
	case 0: IT0 = (mode == FALLING); IE0 = 0; EX0 = 1; break;
	case 1: IT1 = (mode == FALLING); IE1 = 0; EX1 = 1; break;
#ifdef STC8
	case 2: AUXINTIF &= ~0x10; INTCLKO |= 0x10; break;
	case 3: AUXINTIF &= ~0x20; INTCLKO |= 0x20; break;
	case 4: AUXINTIF &= ~0x40; INTCLKO |= 0x40; break;
#else
	case 2: INTCLKO |= 0x10; break;
	case 3: INTCLKO |= 0x20; break;
	case 4: INTCLKO |= 0x40; break;
#endif
	}
	EA = 1;
}

void detachInterrupt(uint8_t num)
{
	switch (num) {
	case 0: EX0 = 0; break;
	case 1: EX1 = 0; break;
	case 2: INTCLKO &= ~0x10; break;
	case 3: INTCLKO &= ~0x20; break;
	case 4: INTCLKO &= ~0x40; break;
	default: return;
	}
	stc_int_func[num] = 0;
}
