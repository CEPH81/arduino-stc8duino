/*
 * wiring_digital.c - pinMode(), digitalWrite(), digitalRead().
 *
 * Modos de cada pata (registros PxM1:PxM0):
 *   00 cuasi-bidireccional, 01 push-pull, 10 alta impedancia, 11 drenador abierto.
 * Los SFR no se pueden direccionar de forma indirecta en el 8051: se elige el
 * puerto con un switch.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

/* En el modelo "large" las variables van en la xdata (lentas): las de estas
 * funciones, que se usan mucho, se ponen en la RAM interna (__data). */
static __data uint8_t g_port, g_mask, g_v1, g_v0;

/* modo de la pata g_port/g_mask: g_v1, g_v0 = valores de PxM1 y PxM0 (0 o g_mask) */
static void set_mode(void)
{
	__data uint8_t n = ~g_mask;
#define MODE(M1, M0) do { M1 = (M1 & n) | g_v1; M0 = (M0 & n) | g_v0; } while (0)
	switch (g_port) {
	case 0: MODE(P0M1, P0M0); break;
	case 1: MODE(P1M1, P1M0); break;
	case 2: MODE(P2M1, P2M0); break;
	case 3: MODE(P3M1, P3M0); break;
	case 4: MODE(P4M1, P4M0); break;
	case 5: MODE(P5M1, P5M0); break;
	case 6: MODE(P6M1, P6M0); break;
	case 7: MODE(P7M1, P7M0); break;
	}
#undef MODE
}

#ifdef STC8
static void set_pullup(uint8_t on)
{
	volatile __xdata uint8_t *pu = (volatile __xdata uint8_t *)(0xFE10 + g_port);
	P_SW2 |= 0x80;
	if (on) *pu |= g_mask;
	else *pu &= ~g_mask;
}
#endif

void pinMode(uint8_t pin, uint8_t mode)
{
	__data uint8_t p = pin;
	g_port = digitalPinToPort(p);
	g_mask = digitalPinToBitMask(p);
	g_v1 = g_v0 = 0;

	switch (mode) {
	case OUTPUT:            g_v0 = g_mask; break;
	case OUTPUT_OPEN_DRAIN: g_v1 = g_v0 = g_mask; break;
	case QUASI_BIDIRECTIONAL:
		digitalWrite(p, HIGH);
		break;
	case INPUT_PULLUP:
#ifdef STC8
		g_v1 = g_mask;
		set_mode();
		set_pullup(1);
#else
		/* STC15: sin resistencias de pull-up propias: cuasi-bidireccional en alto */
		set_mode();
		digitalWrite(p, HIGH);
#endif
		return;
	default:                g_v1 = g_mask; break;   /* INPUT */
	}
	set_mode();
#ifdef STC8
	set_pullup(0);
#endif
}

void digitalWrite(uint8_t pin, uint8_t val)
{
	__data uint8_t mask = digitalPinToBitMask(pin);
	if (val) {
		switch (digitalPinToPort(pin)) {
		case 0: P0 |= mask; break;
		case 1: P1 |= mask; break;
		case 2: P2 |= mask; break;
		case 3: P3 |= mask; break;
		case 4: P4 |= mask; break;
		case 5: P5 |= mask; break;
		case 6: P6 |= mask; break;
		case 7: P7 |= mask; break;
		}
	} else {
		mask = ~mask;
		switch (digitalPinToPort(pin)) {
		case 0: P0 &= mask; break;
		case 1: P1 &= mask; break;
		case 2: P2 &= mask; break;
		case 3: P3 &= mask; break;
		case 4: P4 &= mask; break;
		case 5: P5 &= mask; break;
		case 6: P6 &= mask; break;
		case 7: P7 &= mask; break;
		}
	}
}

void digitalToggle(uint8_t pin)
{
	__data uint8_t mask = digitalPinToBitMask(pin);
	switch (digitalPinToPort(pin)) {
	case 0: P0 ^= mask; break;
	case 1: P1 ^= mask; break;
	case 2: P2 ^= mask; break;
	case 3: P3 ^= mask; break;
	case 4: P4 ^= mask; break;
	case 5: P5 ^= mask; break;
	case 6: P6 ^= mask; break;
	case 7: P7 ^= mask; break;
	}
}

uint8_t digitalRead(uint8_t pin)
{
	__data uint8_t v;
	switch (digitalPinToPort(pin)) {
	case 0: v = P0; break;
	case 1: v = P1; break;
	case 2: v = P2; break;
	case 3: v = P3; break;
	case 4: v = P4; break;
	case 5: v = P5; break;
	case 6: v = P6; break;
	case 7: v = P7; break;
	default: return LOW;
	}
	return (v & digitalPinToBitMask(pin)) ? HIGH : LOW;
}
