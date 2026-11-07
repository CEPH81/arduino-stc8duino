/*
 * main.c - variables del arranque. main() va en stc_main.h, que sdcc.sh
 * agrega al final del sketch: asi la tabla de interrupciones se arma con lo
 * que el sketch realmente usa (por ejemplo Serial3 con JDY40 o DT06).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>

unsigned char runSerialEvent;
