/* main.c - arranque del sketch. Parte de stc8duino. LGPL 2.1. */
#include "Arduino.h"

void main(void)
{
	init();
	setup();
	for (;;)
		loop();
}
