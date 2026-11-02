/*
 * int_isr.c - interrupciones INT0-INT4 (attachInterrupt) y del temporizador 1
 * (timer1Begin).
 *
 * Cada vector es un trozo minimo en ensamblador que salta a una rutina comun:
 * guarda los registros una sola vez y llama a la funcion del usuario. Asi las
 * seis interrupciones ocupan unos 150 bytes en lugar de mas de 500.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

void (* __data stc_int_func[5])(void);
void (* __data stc_t1_func)(void);
__data unsigned int stc_t1_div, stc_t1_cnt;

/* entrada: acc ya guardado en la pila, A = direccion (RAM interna) del puntero a la funcion */
void stc_int_comun(void) __naked
{
	__asm
	push	bits
	push	b
	push	dpl
	push	dph
	push	(0+7)
	push	(0+6)
	push	(0+5)
	push	(0+4)
	push	(0+3)
	push	(0+2)
	push	(0+1)
	push	(0+0)
	push	psw
	mov	psw,#0x00
	mov	r0,a
	mov	dpl,@r0
	inc	r0
	mov	dph,@r0
	mov	a,dpl
	orl	a,dph
	jz	00001$
	lcall	__sdcc_call_dptr
00001$:
	pop	psw
	pop	(0+0)
	pop	(0+1)
	pop	(0+2)
	pop	(0+3)
	pop	(0+4)
	pop	(0+5)
	pop	(0+6)
	pop	(0+7)
	pop	dph
	pop	dpl
	pop	b
	pop	bits
	pop	acc
	reti
	; registro de bits de SDCC (b0-b7), igual que en las interrupciones que genera el compilador
	.area BIT_BANK	(REL,OVR,DATA)
bits:
	.ds 1
	.area CSEG    (CODE)
	__endasm;
}

void stc_int0_isr(void) __interrupt(0) __naked
{
	__asm
	push	acc
	mov	a,#(_stc_int_func + 0)
	ljmp	_stc_int_comun
	__endasm;
}

void stc_int1_isr(void) __interrupt(2) __naked
{
	__asm
	push	acc
	mov	a,#(_stc_int_func + 2)
	ljmp	_stc_int_comun
	__endasm;
}

/* en los STC8 las banderas de INT2-INT4 estan en AUXINTIF (0xEF) */
void stc_int2_isr(void) __interrupt(10) __naked
{
	__asm
#ifdef STC8
	anl	0xef,#0xef
#endif
	push	acc
	mov	a,#(_stc_int_func + 4)
	ljmp	_stc_int_comun
	__endasm;
}

void stc_int3_isr(void) __interrupt(11) __naked
{
	__asm
#ifdef STC8
	anl	0xef,#0xdf
#endif
	push	acc
	mov	a,#(_stc_int_func + 6)
	ljmp	_stc_int_comun
	__endasm;
}

void stc_int4_isr(void) __interrupt(16) __naked
{
	__asm
#ifdef STC8
	anl	0xef,#0xbf
#endif
	push	acc
	mov	a,#(_stc_int_func + 8)
	ljmp	_stc_int_comun
	__endasm;
}

/* temporizador 1: divisor por programa de 16 bits (stc_t1_cnt) para periodos largos */
void stc_timer1_isr(void) __interrupt(3) __naked
{
	__asm
	push	acc
	push	psw
	mov	a,_stc_t1_cnt
	jnz	00001$
	dec	(_stc_t1_cnt + 1)
00001$:
	dec	_stc_t1_cnt
	mov	a,_stc_t1_cnt
	orl	a,(_stc_t1_cnt + 1)
	jz	00002$
	pop	psw
	pop	acc
	reti
00002$:
	mov	_stc_t1_cnt,_stc_t1_div
	mov	(_stc_t1_cnt + 1),(_stc_t1_div + 1)
	pop	psw
	mov	a,#_stc_t1_func
	ljmp	_stc_int_comun
	__endasm;
}
