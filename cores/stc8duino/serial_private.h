/* serial_private.h - estado compartido entre serial_isr.c y HardwareSerial.c. Parte de stc8duino. LGPL 2.1. */
#ifndef serial_private_h
#define serial_private_h
extern volatile __xdata uint8_t stc_rx_buf[SERIAL_RX_BUFFER_SIZE];
extern volatile uint8_t stc_rx_head, stc_rx_tail;
extern volatile __bit stc_tx_busy;
#endif
