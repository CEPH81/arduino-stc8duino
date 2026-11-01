/*
 * stc_reset.c - stcReset(): reinicio por programa
 * (en un modulo aparte para que solo se enlace si se usa).
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

void stcReset(uint8_t isp)
{
	IAP_CONTR = isp ? 0x60 : 0x20;   /* SWBS = arrancar desde la ROM de carga */
}
