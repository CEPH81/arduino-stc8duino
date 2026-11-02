/* wiring_private.h - constantes del temporizador 0 (millis/micros). Parte de stc8duino. LGPL 2.1. */
#ifndef wiring_private_h
#define wiring_private_h

#define T0_TICKS  ((unsigned int)(F_CPU / 1000UL))       /* cuentas por milisegundo */
#define T0_RELOAD ((unsigned int)(65536UL - T0_TICKS))
/* cuentas por microsegundo, redondeado (11 a 11,0592 MHz) */
#define T0_TPU    ((unsigned int)((F_CPU + 500000UL) / 1000000UL))

extern volatile __data unsigned long stc_ms_count;
extern volatile __data unsigned long stc_us_count;

#endif
