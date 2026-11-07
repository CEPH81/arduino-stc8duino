/*
 * serial3_isr.c - interrupcion de la UART3 (Serial3): recibe en el bufer y avisa
 * del fin de cada envio. Va aparte de serial3.c: la interrupcion se enlaza
 * siempre en STC8H/STC8A (su vector esta en main.c), asi que esta escrita en
 * ensamblador para que ocupe lo minimo (unos 40 bytes) y no guarde registros
 * de mas. Bufer de 16 bytes en la RAM interna (idata).
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

#if (defined(STC8H) || defined(STC8A)) && defined(STC_SERIAL3)

volatile __idata uint8_t stc_rx3_buf[SERIAL3_RX_BUFFER_SIZE];
volatile __data uint8_t stc_rx3_head, stc_rx3_tail;
volatile __bit stc_tx3_busy;

/* S3CON = 0xAC (S3RI bit 0, S3TI bit 1), S3BUF = 0xAD */
void stc_uart3_isr(void) __interrupt(17) __naked
{
	__asm
	push	acc
	push	psw
	push	0x00                    ; R0 del banco 0 (el del programa)
	mov	a,0xac
	jnb	acc.0,00001$
	anl	0xac,#0xfe              ; borrar S3RI
	mov	a,_stc_rx3_head
	inc	a
	anl	a,#0x0f                 ; SERIAL3_RX_BUFFER_SIZE - 1
	cjne	a,_stc_rx3_tail,00002$
	sjmp	00001$                  ; bufer lleno: se pierde el byte
00002$:
	xch	a,_stc_rx3_head         ; a = posicion libre, head = siguiente
	add	a,#_stc_rx3_buf
	mov	r0,a
	mov	@r0,0xad                ; S3BUF
00001$:
	mov	a,0xac
	jnb	acc.1,00003$
	anl	0xac,#0xfd              ; borrar S3TI
	clr	_stc_tx3_busy
00003$:
	pop	0x00
	pop	psw
	pop	acc
	reti
	__endasm;
}

#endif
