/*
 * serial3.c - Serial3 (UART3) de STC8H y STC8A (ver Serial3.h).
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"

#if (defined(STC8H) || defined(STC8A)) && defined(STC_SERIAL3)

extern volatile __idata uint8_t stc_rx3_buf[SERIAL3_RX_BUFFER_SIZE];
extern volatile __data uint8_t stc_rx3_head, stc_rx3_tail;
extern volatile __bit stc_tx3_busy;

void Serial3_begin(unsigned long baud)
{
	/* temporizador 3 en modo 1T con la frecuencia real del chip */
	__data unsigned int recarga = (unsigned int)(65536UL - (STC_FREQ() / 4 + baud / 2) / baud);

	IE2 &= ~0x08;                         /* ES3 */
	P_SW2 = (P_SW2 & ~0x02) | 0x80;       /* UART3 en P0.0 (RX) / P0.1 (TX) */
	P0M1 &= ~0x03;                        /* cuasi-bidireccionales */
	P0M0 &= ~0x03;
	T4T3M &= ~0x0F;                       /* parar T3 */
	T3L = (uint8_t)recarga;
	T3H = (uint8_t)(recarga >> 8);
	T4T3M |= 0x02 | 0x08;                 /* T3x12, T3R */
	S3CON = 0x50;                         /* 8 bits, velocidad con T3, recepcion activa */
	stc_rx3_head = stc_rx3_tail = 0;
	stc_tx3_busy = 0;
	IE2 |= 0x08;
	EA = 1;
}

void Serial3_end(void)
{
	Serial3_flush();
	IE2 &= ~0x08;
	S3CON = 0;
}

int Serial3_available(void)
{
	return (uint8_t)(stc_rx3_head - stc_rx3_tail) & (SERIAL3_RX_BUFFER_SIZE - 1);
}

int Serial3_peek(void)
{
	if (stc_rx3_head == stc_rx3_tail) return -1;
	return stc_rx3_buf[stc_rx3_tail];
}

int Serial3_read(void)
{
	__data uint8_t c;
	if (stc_rx3_head == stc_rx3_tail) return -1;
	c = stc_rx3_buf[stc_rx3_tail];
	stc_rx3_tail = (stc_rx3_tail + 1) & (SERIAL3_RX_BUFFER_SIZE - 1);
	return c;
}

size_t Serial3_write(uint8_t c)
{
	while (stc_tx3_busy)
		;
	stc_tx3_busy = 1;
	S3BUF = c;
	return 1;
}

void Serial3_flush(void)
{
	while (stc_tx3_busy)
		;
}

#endif
