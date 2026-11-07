/*
 * stc_main.h - main() del programa Arduino. sdcc.sh lo agrega al final del
 * sketch: SDCC arma la tabla de interrupciones en el archivo que tiene main(),
 * con los prototipos que ve ahi. Asi la de Serial3 (vector 17, que alarga la
 * tabla unos 100 bytes) solo entra si el sketch incluyo UART3.h, JDY40.h o
 * DT06.h (o con el menu Serial3 = Siempre).
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef STC_MAIN_H
#define STC_MAIN_H

#if (defined(STC8H) || defined(STC8A)) && defined(STC_SERIAL3) && !defined(STC_ISR_UART3)
void stc_uart3_isr(void) __interrupt(17);
#endif

extern unsigned char runSerialEvent;

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

#endif
