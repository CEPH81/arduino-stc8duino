/*
 * stc_isr.h - prototipos de las interrupciones del nucleo.
 * SDCC exige que el prototipo de cada interrupcion se vea en el archivo que
 * tiene main(): por eso se incluyen desde Arduino.h.
 *
 * Numeros de interrupcion de STC8/STC15: INT0 0, T0 1, INT1 2, T1 3, UART1 4,
 * ADC 5, LVD 6, UART2 8, SPI 9, INT2 10, INT3 11, T2 12, INT4 16, UART3 17,
 * UART4 18, T3 19, T4 20, CMP 21, I2C 24, USB 25, PWMA 26, PWMB 27.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef STC_ISR_H
#define STC_ISR_H

void stc_timer0_isr(void) __interrupt(1);   /* millis() */
void stc_uart1_isr(void) __interrupt(4);    /* Serial */
void stc_int0_isr(void) __interrupt(0);     /* attachInterrupt(0) */
void stc_int1_isr(void) __interrupt(2);     /* attachInterrupt(1) */

#endif
