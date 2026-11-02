/*
 * serial_isr.c - interrupcion de la UART1 y bufer de recepcion. Va aparte de
 * HardwareSerial.c para que las funciones de Serial solo se enlacen si el
 * sketch las usa (la interrupcion se enlaza siempre: su vector esta en main.c).
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"
#include "serial_private.h"

volatile __xdata uint8_t stc_rx_buf[SERIAL_RX_BUFFER_SIZE];
volatile uint8_t stc_rx_head, stc_rx_tail;
volatile __bit stc_tx_busy;
#ifndef STC_NO_AUTO_ISP
static uint8_t isp_pos;
static __code const char isp_magic[] = "@STCISP#";
#endif

void stc_uart1_isr(void) __interrupt(4)
{
	if (RI) {
		uint8_t c = SBUF;
		uint8_t next = (stc_rx_head + 1) & (SERIAL_RX_BUFFER_SIZE - 1);
		RI = 0;
#ifndef STC_NO_AUTO_ISP
		if (c == isp_magic[isp_pos]) {
			if (++isp_pos == sizeof(isp_magic) - 1)
				IAP_CONTR = 0x60;          /* reinicio en el cargador de la ROM */
		} else {
			isp_pos = (c == isp_magic[0]);
		}
#endif
		if (next != stc_rx_tail) {
			stc_rx_buf[stc_rx_head] = c;
			stc_rx_head = next;
			runSerialEvent = 1;
		}
	}
	if (TI) {
		TI = 0;
		stc_tx_busy = 0;
	}
}

