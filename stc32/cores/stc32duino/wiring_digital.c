/*
 * wiring_digital.c - pinMode, digitalWrite y digitalRead.
 * Modos del STC32G (PxM1:PxM0): 00 cuasi-bidireccional, 01 salida push-pull,
 * 10 solo entrada, 11 drenador abierto. Pull-up interna en PxPU.
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

#define MODO(M1, M0, PU) \
	switch (mode) { \
	case OUTPUT:            M1 &= ~m; M0 |= m;  PU &= ~m; break; \
	case OUTPUT_OPEN_DRAIN: M1 |= m;  M0 |= m;  PU &= ~m; break; \
	case INPUT_PULLUP:      M1 |= m;  M0 &= ~m; PU |= m;  break; \
	default:                M1 |= m;  M0 &= ~m; PU &= ~m; break; \
	}

void pinMode(uint8_t pin, uint8_t mode)
{
	uint8_t m = 1 << (pin & 7);
	switch (pin >> 4) {
	case 0: MODO(P0M1, P0M0, P0PU) break;
	case 1: MODO(P1M1, P1M0, P1PU) break;
	case 2: MODO(P2M1, P2M0, P2PU) break;
	case 3: MODO(P3M1, P3M0, P3PU) break;
	case 4: MODO(P4M1, P4M0, P4PU) break;
	case 5: MODO(P5M1, P5M0, P5PU) break;
	case 6: MODO(P6M1, P6M0, P6PU) break;
	case 7: MODO(P7M1, P7M0, P7PU) break;
	}
}

#define ESCRIBIR(P) if (val) P |= m; else P &= ~m;

void digitalWrite(uint8_t pin, uint8_t val)
{
	uint8_t m = 1 << (pin & 7);
	switch (pin >> 4) {
	case 0: ESCRIBIR(P0) break;
	case 1: ESCRIBIR(P1) break;
	case 2: ESCRIBIR(P2) break;
	case 3: ESCRIBIR(P3) break;
	case 4: ESCRIBIR(P4) break;
	case 5: ESCRIBIR(P5) break;
	case 6: ESCRIBIR(P6) break;
	case 7: ESCRIBIR(P7) break;
	}
}

int digitalRead(uint8_t pin)
{
	uint8_t m = 1 << (pin & 7);
	uint8_t v = 0;
	switch (pin >> 4) {
	case 0: v = P0; break;
	case 1: v = P1; break;
	case 2: v = P2; break;
	case 3: v = P3; break;
	case 4: v = P4; break;
	case 5: v = P5; break;
	case 6: v = P6; break;
	case 7: v = P7; break;
	}
	return (v & m) ? HIGH : LOW;
}
