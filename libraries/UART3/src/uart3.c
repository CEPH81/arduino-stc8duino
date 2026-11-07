/*
 * uart3.c - Con el menu Serial3 en "Automatico" el nucleo no compila Serial3:
 * se compila aqui, una sola vez, el mismo codigo. El vector de la
 * interrupcion lo pone main() en el sketch (stc_main.h).
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef STC_SERIAL3
#define STC_SERIAL3
#include "serial3.c"
#include "serial3_isr.c"
#endif
