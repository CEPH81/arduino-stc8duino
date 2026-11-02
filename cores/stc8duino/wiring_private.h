/*
 * wiring_private.h - datos del temporizador 0 (millis/micros/delay) y del reloj.
 *
 * stc_reloj es un bloque en la memoria de programa con la frecuencia del chip y
 * los valores que dependen de ella. Se compila con F_CPU (menu "Reloj") y stcload
 * lo corrige al grabar con la frecuencia real que consiguio al ajustar el
 * oscilador (por ejemplo 24,077 MHz en lugar de 24,000), asi millis() no se
 * adelanta ni se atrasa. init() copia los valores a la RAM interna.
 *
 * Formato (13 bytes): "STCF", frecuencia en Hz (4 bytes, el menos significativo
 * primero), cuentas por ms (2), cuentas por us (1), K de micros() = 65536000 /
 * cuentas por ms (2).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef wiring_private_h
#define wiring_private_h

#include <stdint.h>

extern __code volatile const uint8_t stc_reloj[13];

extern volatile __data unsigned long stc_ms_count;
extern volatile __data unsigned long stc_us_count;
extern __data unsigned int stc_t0_ticks, stc_t0_reload;
extern __data uint8_t stc_t0_tpu, us_kl, us_kh;

#define T0_TICKS  stc_t0_ticks      /* cuentas por milisegundo */
#define T0_RELOAD stc_t0_reload     /* 65536 - T0_TICKS */
#define T0_TPU    stc_t0_tpu        /* cuentas por microsegundo, redondeado */

/* frecuencia real del chip en Hz (la de stc_reloj) */
#define STC_FREQ() ((unsigned long)stc_reloj[4] | ((unsigned long)stc_reloj[5] << 8) | \
	((unsigned long)stc_reloj[6] << 16) | ((unsigned long)stc_reloj[7] << 24))

#endif
