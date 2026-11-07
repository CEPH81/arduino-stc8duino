/*
 * HardwareSerial.c - Serial (UART1) con interrupcion. Ver HardwareSerial.h.
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

static volatile unsigned char rx_buf[SERIAL_RX_BUFFER_SIZE];
static volatile unsigned char tx_buf[SERIAL_TX_BUFFER_SIZE];
static volatile unsigned char rx_head, rx_tail, tx_head, tx_tail;
static volatile unsigned char tx_ocupado;
static unsigned char isp_pos;
static const char isp_clave[] = "@STCISP#";

void serial_isr(void) interrupt 4
{
	unsigned char c, sig;
	if (RI) {
		RI = 0;
		c = SBUF;
		if (c == isp_clave[isp_pos]) {
			if (++isp_pos == sizeof(isp_clave) - 1)
				IAP_CONTR = 0x60;      /* reinicio en el cargador de la ROM */
		} else {
			isp_pos = (c == isp_clave[0]);
		}
		sig = (rx_head + 1) & (SERIAL_RX_BUFFER_SIZE - 1);
		if (sig != rx_tail) {
			rx_buf[rx_head] = c;
			rx_head = sig;
		}
	}
	if (TI) {
		TI = 0;
		if (tx_head != tx_tail) {
			SBUF = tx_buf[tx_tail];
			tx_tail = (tx_tail + 1) & (SERIAL_TX_BUFFER_SIZE - 1);
		} else {
			tx_ocupado = 0;
		}
	}
}

void Serial_begin(unsigned long baud)
{
	unsigned int recarga = (unsigned int)(65536UL - (F_CPU / 4 + baud / 2) / baud);

	ES = 0;
	/* P3.0 (RX) y P3.1 (TX) cuasi-bidireccionales */
	P3M1 &= ~0x03;
	P3M0 &= ~0x03;
	P_SW1 &= 0x3F;              /* UART1 en P3.0/P3.1 */
	SCON = 0x50;                /* modo 1 (8 bits), recepcion activa */
	AUXR &= ~0x10;              /* T2R: parar el temporizador 2 */
	T2L = (unsigned char)recarga;
	T2H = (unsigned char)(recarga >> 8);
	AUXR |= 0x04 | 0x01 | 0x10; /* T2 en 1T, UART1 con T2 (S1BRT), en marcha */
	rx_head = rx_tail = tx_head = tx_tail = 0;
	tx_ocupado = 0;
	isp_pos = 0;
	ES = 1;
	EA = 1;
}

void Serial_end(void)
{
	Serial_flush();
	ES = 0;
}

int Serial_available(void)
{
	return (unsigned char)(rx_head - rx_tail) & (SERIAL_RX_BUFFER_SIZE - 1);
}

int Serial_peek(void)
{
	if (rx_head == rx_tail) return -1;
	return rx_buf[rx_tail];
}

int Serial_read(void)
{
	unsigned char c;
	if (rx_head == rx_tail) return -1;
	c = rx_buf[rx_tail];
	rx_tail = (rx_tail + 1) & (SERIAL_RX_BUFFER_SIZE - 1);
	return c;
}

size_t Serial_write(unsigned char c)
{
	unsigned char sig;
	for (;;) {
		ES = 0;
		if (!tx_ocupado) {          /* UART libre: se envia directamente */
			tx_ocupado = 1;
			SBUF = c;
			ES = 1;
			return 1;
		}
		sig = (tx_head + 1) & (SERIAL_TX_BUFFER_SIZE - 1);
		if (sig != tx_tail) break;
		ES = 1;                     /* bufer lleno: esperar a que salga un byte */
		while (sig == tx_tail)
			;
	}
	tx_buf[tx_head] = c;
	tx_head = sig;
	ES = 1;
	return 1;
}

size_t Serial_writeStr(const char *s)
{
	size_t n = 0;
	while (*s) {
		Serial_write(*s++);
		n++;
	}
	return n;
}

size_t Serial_writeBuf(const unsigned char *b, unsigned int n)
{
	unsigned int i;
	for (i = 0; i < n; i++)
		Serial_write(b[i]);
	return n;
}

size_t Serial_newline(void)
{
	Serial_write('\r');
	Serial_write('\n');
	return 2;
}

void Serial_flush(void)
{
	while (tx_ocupado)
		;
}
