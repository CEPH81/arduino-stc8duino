/*
 * WMath.c - random(), randomSeed() y map().
 * Generador congruencial lineal de 32 bits (rapido; suficiente para efectos).
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

static unsigned long semilla_actual = 1;

void randomSeed(unsigned long semilla)
{
	if (semilla != 0) semilla_actual = semilla;
}

long stc_random(long min, long max)
{
	unsigned long r;
	if (max <= min) return min;
	semilla_actual = semilla_actual * 1103515245UL + 12345UL;
	r = semilla_actual >> 8;                 /* los bits bajos son poco aleatorios */
	return min + (long)(r % (unsigned long)(max - min));
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
	if (in_max == in_min) return out_min;
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
