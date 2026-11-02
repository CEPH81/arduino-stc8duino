/*
 * wiring_pwm.c - analogWrite(): PWM por hardware.
 *
 * STC8H: modulo PWMB (16 bits), canales PWM5-PWM8. Cada canal puede salir por
 * una de cuatro patas (registro PWMB_PS); un canal solo puede estar en una pata
 * a la vez:
 *   PWM5: P2.0  P1.7  P0.0  P7.4
 *   PWM6: P2.1  P5.4  P0.1  P7.5
 *   PWM7: P2.2  P3.3  P0.2  P7.6
 *   PWM8: P2.3  P3.4  P0.3  P7.7      (P3.4 es el LED de la STC8H8K64U)
 * Valores de 0 a 255 como en Arduino, a unos 1000 Hz (calculado con la
 * frecuencia real del chip). En las demas patas, y en STC8G, STC8A y STC15
 * (todavia), analogWrite pone la pata en HIGH si el valor es 128 o mas.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"

#ifdef STC8H

#define XS(a) (*(volatile __xdata uint8_t *)(a))
#define PWMB_CR1    XS(0xFEE0)
#define PWMB_EGR    XS(0xFEE7)
#define PWMB_CCMR1  0xFEE8          /* CCMR1..4 (canales 5..8) seguidos */
#define PWMB_CCER1  XS(0xFEEC)
#define PWMB_CCER2  XS(0xFEED)
#define PWMB_PSCRH  XS(0xFEF0)
#define PWMB_PSCRL  XS(0xFEF1)
#define PWMB_ARRH   XS(0xFEF2)
#define PWMB_ARRL   XS(0xFEF3)
#define PWMB_CCR5H  0xFEF5          /* CCR5H, CCR5L, CCR6H, ... CCR8L */
#define PWMB_BKR    XS(0xFEFD)
#define PWMB_ENO    XS(0xFEB5)
#define PWMB_PS     XS(0xFEB6)

/* patas de cada canal (5..8) en el orden de PWMB_PS */
static __code const uint8_t pwmb_patas[4][4] = {
	{ P2_0, P1_7, P0_0, P7_4 },
	{ P2_1, P5_4, P0_1, P7_5 },
	{ P2_2, P3_3, P0_2, P7_6 },
	{ P2_3, P3_4, P0_3, P7_7 },
};

static __data uint8_t pwmb_listo;

static void pwmb_iniciar(void)
{
	/* contador a F/(PSCR+1), periodo de 256 cuentas: unos 1000 Hz */
	__data unsigned int pscr = T0_TICKS / 256 - 1;   /* T0_TICKS = cuentas por ms */
	P_SW2 |= 0x80;
	PWMB_PSCRH = pscr >> 8;
	PWMB_PSCRL = pscr & 0xFF;
	PWMB_ARRH = 0;
	PWMB_ARRL = 255;
	PWMB_BKR = 0x80;                   /* MOE: salidas habilitadas */
	PWMB_EGR = 0x01;                   /* UG: cargar el preescalador y el periodo */
	PWMB_CR1 = 0x81;                   /* ARPE y CEN: en marcha */
	pwmb_listo = 1;
}

void analogWrite(uint8_t pin, int val)
{
	__data uint8_t c, ps, ccr_h, ccr_l;
	__data unsigned int ccr;

	for (c = 0; c < 4; c++)
		for (ps = 0; ps < 4; ps++)
			if (pwmb_patas[c][ps] == pin) goto encontrado;
	pinMode(pin, OUTPUT);
	digitalWrite(pin, val >= 128);
	return;

encontrado:
	if (!pwmb_listo) pwmb_iniciar();
	pinMode(pin, OUTPUT);
	if (val <= 0) ccr = 0;               /* siempre en bajo */
	else if (val >= 255) ccr = 256;      /* mayor que el periodo: siempre en alto */
	else ccr = val;
	ccr_h = ccr >> 8;
	ccr_l = ccr & 0xFF;

	P_SW2 |= 0x80;
	PWMB_PS = (PWMB_PS & ~(3 << (2 * c))) | (ps << (2 * c));
	XS(PWMB_CCMR1 + c) = 0x68;           /* modo PWM 1 (alto mientras CNT < CCR) con precarga */
	XS(PWMB_CCR5H + 2 * c) = ccr_h;      /* primero el byte alto */
	XS(PWMB_CCR5H + 2 * c + 1) = ccr_l;
	if (c < 2) PWMB_CCER1 |= 1 << (4 * c);        /* CC5E / CC6E */
	else PWMB_CCER2 |= 1 << (4 * (c - 2));        /* CC7E / CC8E */
	PWMB_ENO |= 1 << (2 * c);            /* ENO5P..ENO8P */
}

#else

void analogWrite(uint8_t pin, int val)
{
	pinMode(pin, OUTPUT);
	digitalWrite(pin, val >= 128);
}

#endif
