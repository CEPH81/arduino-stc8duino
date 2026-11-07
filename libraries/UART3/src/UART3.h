/*
 * UART3.h - Serial3 (UART3: RX3 = P0.0, TX3 = P0.1) de STC8H y STC8A con
 * solo incluir este archivo, sin tocar Herramientas > Serial3.
 *
 *   #include <UART3.h>
 *   Serial3.begin(9600);
 *   Serial3.print("hola");
 *
 * Los programas que no lo incluyen no gastan memoria en Serial3 (ni en su
 * vector de interrupcion).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef UART3_h
#define UART3_h

#include <Arduino.h>

#if !defined(STC8H) && !defined(STC8A)
#error "Este chip no tiene UART3 (Serial3): solo STC8H y STC8A"
#endif

#ifndef STC_SERIAL3
#define STC_SERIAL3
#include <Serial3.h>
#endif

#endif
