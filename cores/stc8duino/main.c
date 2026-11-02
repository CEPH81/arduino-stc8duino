/*
 * main.c - arranque del programa Arduino.
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>

volatile __data unsigned char runSerialEvent;

void main(void)
{
	init();
	initVariant();
	setup();
	for (;;) {
		loop();
		if (runSerialEvent) serialEvent();
	}
}
