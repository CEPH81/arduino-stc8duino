/* wiring_shift.c - shiftOut(), shiftIn() y pulseIn(). Parte de stc8duino. LGPL 2.1. */
#include "Arduino.h"

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
	uint8_t i;
	for (i = 0; i < 8; i++) {
		if (bitOrder == LSBFIRST) {
			digitalWrite(dataPin, val & 1);
			val >>= 1;
		} else {
			digitalWrite(dataPin, (val & 0x80) != 0);
			val <<= 1;
		}
		digitalWrite(clockPin, HIGH);
		digitalWrite(clockPin, LOW);
	}
}

uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder)
{
	uint8_t value = 0, i;
	for (i = 0; i < 8; i++) {
		digitalWrite(clockPin, HIGH);
		if (bitOrder == LSBFIRST)
			value |= digitalRead(dataPin) << i;
		else
			value |= digitalRead(dataPin) << (7 - i);
		digitalWrite(clockPin, LOW);
	}
	return value;
}

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
	unsigned long start = micros(), t0;
	while (digitalRead(pin) == state)
		if (micros() - start > timeout) return 0;
	while (digitalRead(pin) != state)
		if (micros() - start > timeout) return 0;
	t0 = micros();
	while (digitalRead(pin) == state)
		if (micros() - start > timeout) return 0;
	return micros() - t0;
}
