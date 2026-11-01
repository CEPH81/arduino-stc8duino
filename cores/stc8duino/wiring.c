/*
 * wiring.c - init(), millis(), micros(), delay() y reinicio.
 *
 * El temporizador 0 (modo 1T, 16 bits con recarga automatica) interrumpe cada
 * milisegundo. El reloj del chip lo deja calibrado el cargador (stcgal -t) a la
 * frecuencia del menu "Reloj", que llega aqui como F_CPU.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

#define T0_TICKS (F_CPU / 1000)              /* cuentas por milisegundo */
#define T0_RELOAD (65536UL - T0_TICKS)

volatile unsigned long stc_ms_count;

void stc_timer0_isr(void) __interrupt(1)
{
	stc_ms_count++;
}

void init(void)
{
#ifdef STC8
	P_SW2 |= 0x80;          /* acceso a los registros extendidos */
#endif
	TMOD &= 0xF0;           /* T0 modo 0: 16 bits con recarga */
	AUXR |= AUXR_T0x12;     /* 1T: cuenta a F_CPU */
	TL0 = T0_RELOAD & 0xFF;
	TH0 = T0_RELOAD >> 8;
	TF0 = 0;
	ET0 = 1;
	TR0 = 1;
	EA = 1;
}

unsigned long millis(void)
{
	unsigned long m;
	EA = 0;
	m = stc_ms_count;
	EA = 1;
	return m;
}

void delay(unsigned long ms)
{
	unsigned long start = millis();
	while (millis() - start < ms)
		;
}

