/*
 * UART3.h - Serial3 (UART3: RX3 = P0.0, TX3 = P0.1) con solo incluir este
 * archivo, sin activar Herramientas > Serial3 (UART3) > Si.
 *
 *   #include <UART3.h>
 *   Serial3.begin(9600);
 *   Serial3.print("hola");
 *
 * Si el menu ya lo activo, no hace nada mas. Los programas que no lo
 * incluyen no gastan memoria en Serial3.
 *
 * Parte de stc8duino (version stc32duino). LGPL 2.1.
 */
#ifndef UART3_h
#define UART3_h

#include <Arduino.h>

#ifndef STC_SERIAL3
#define STC_SERIAL3
#include <Serial3.h>
#endif

#endif
