/*
 * HardwareSerial.c - UART1 con interrupcion: recepcion en un bufer circular,
 * envio esperando a que termine el byte anterior.
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

#include "serial_private.h"

void HardwareSerial_begin_config(unsigned long baud, uint8_t config)
{
	unsigned int reload = (unsigned int)(65536UL - (F_CPU / 4 + baud / 2) / baud);
	(void) config;

	ES = 0;
	SCON = 0x50;                         /* modo 1 (8 bits), recepcion activa */
	AUXR &= ~AUXR_T2R;
	T2L = reload & 0xFF;
	T2H = reload >> 8;
	AUXR |= AUXR_T2x12 | AUXR_S1ST2 | AUXR_T2R;
	P_SW1 &= 0x3F;                       /* UART1 en P3.0/P3.1 */
	stc_rx_head = stc_rx_tail = 0;
	stc_tx_head = stc_tx_tail = 0;
	stc_tx_busy = 0;
	runSerialEvent = 0;
	ES = 1;
	EA = 1;
}

void HardwareSerial_begin(unsigned long baud)
{
	HardwareSerial_begin_config(baud, SERIAL_8N1);
}

void HardwareSerial_end(void)
{
	HardwareSerial_flush();
	ES = 0;
	REN = 0;
}

int HardwareSerial_available(void)
{
	return (uint8_t)(stc_rx_head - stc_rx_tail) & (SERIAL_RX_BUFFER_SIZE - 1);
}

int HardwareSerial_availableForWrite(void)
{
	return (SERIAL_TX_BUFFER_SIZE - 1) - ((uint8_t)(stc_tx_head - stc_tx_tail) & (SERIAL_TX_BUFFER_SIZE - 1));
}

int HardwareSerial_peek(void)
{
	if (stc_rx_head == stc_rx_tail) return -1;
	return stc_rx_buf[stc_rx_tail];
}

int HardwareSerial_read(void)
{
	uint8_t c;
	if (stc_rx_head == stc_rx_tail) return -1;
	c = stc_rx_buf[stc_rx_tail];
	stc_rx_tail = (stc_rx_tail + 1) & (SERIAL_RX_BUFFER_SIZE - 1);
	runSerialEvent = (stc_rx_head != stc_rx_tail);
	return c;
}

size_t HardwareSerial_write(uint8_t c)
{
	__data uint8_t next;
	for (;;) {
		ES = 0;
		if (!stc_tx_busy) {              /* UART libre: se envia directamente */
			stc_tx_busy = 1;
			SBUF = c;
			ES = 1;
			return 1;
		}
		next = (stc_tx_head + 1) & (SERIAL_TX_BUFFER_SIZE - 1);
		if (next != stc_tx_tail) break;
		ES = 1;                          /* bufer lleno: esperar a que salga un byte */
		while (next == stc_tx_tail)
			;
	}
	stc_tx_buf[stc_tx_head] = c;
	stc_tx_head = next;
	ES = 1;
	return 1;
}

/* espera a que se haya enviado todo */
void HardwareSerial_flush(void)
{
	while (stc_tx_busy)
		;
}
