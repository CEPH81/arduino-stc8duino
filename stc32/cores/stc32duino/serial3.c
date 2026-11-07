/*
 * serial3.c - Serial3 (UART3) del STC32G (ver Serial3.h).
 * Solo se compila con Herramientas > Serial3 > Si (STC_SERIAL3).
 * Parte de stc8duino (version stc32duino). LGPL 2.1.
 */
#include "Arduino.h"

#ifdef STC_SERIAL3

static volatile unsigned char rx3_buf[SERIAL3_RX_BUFFER_SIZE];
static volatile unsigned char rx3_head, rx3_tail, tx3_ocupado;

void serial3_isr(void) interrupt 17
{
	unsigned char c, sig;
	if (S3CON & 0x01) {                   /* S3RI */
		c = S3BUF;
		S3CON &= ~0x01;
		sig = (rx3_head + 1) & (SERIAL3_RX_BUFFER_SIZE - 1);
		if (sig != rx3_tail) {
			rx3_buf[rx3_head] = c;
			rx3_head = sig;
		}
	}
	if (S3CON & 0x02) {                   /* S3TI */
		S3CON &= ~0x02;
		tx3_ocupado = 0;
	}
}

void Serial3_begin(unsigned long baud)
{
	unsigned int recarga = (unsigned int)(65536UL - (F_CPU / 4 + baud / 2) / baud);

	IE2 &= ~0x08;                         /* ES3 */
	P_SW2 = (P_SW2 & ~0x02) | 0x80;       /* UART3 en P0.0 (RX) / P0.1 (TX) */
	P0M1 &= ~0x03;                        /* cuasi-bidireccionales */
	P0M0 &= ~0x03;
	T4T3M &= ~0x0F;                       /* parar T3 */
	T3L = (unsigned char)recarga;
	T3H = (unsigned char)(recarga >> 8);
	T4T3M |= 0x02 | 0x08;                 /* T3x12, T3R */
	S3CON = 0x50;                         /* 8 bits, velocidad con T3, recepcion activa */
	rx3_head = rx3_tail = 0;
	tx3_ocupado = 0;
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
	return (unsigned char)(rx3_head - rx3_tail) & (SERIAL3_RX_BUFFER_SIZE - 1);
}

int Serial3_peek(void)
{
	if (rx3_head == rx3_tail) return -1;
	return rx3_buf[rx3_tail];
}

int Serial3_read(void)
{
	unsigned char c;
	if (rx3_head == rx3_tail) return -1;
	c = rx3_buf[rx3_tail];
	rx3_tail = (rx3_tail + 1) & (SERIAL3_RX_BUFFER_SIZE - 1);
	return c;
}

size_t Serial3_write(unsigned char c)
{
	while (tx3_ocupado)
		;
	tx3_ocupado = 1;
	S3BUF = c;
	return 1;
}

size_t Serial3_writeStr(const char *s)
{
	size_t n = 0;
	while (*s) {
		Serial3_write(*s++);
		n++;
	}
	return n;
}

size_t Serial3_writeBuf(const unsigned char *b, unsigned int n)
{
	unsigned int i;
	for (i = 0; i < n; i++)
		Serial3_write(b[i]);
	return n;
}

void Serial3_flush(void)
{
	while (tx3_ocupado)
		;
}

#endif
