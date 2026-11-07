/*
 * wiring_pwm.c - analogWrite(): PWM por hardware con el modulo PWMB del STC32G.
 *
 * Canales PWM5-PWM8; cada uno puede salir por una de cuatro patas (PWMB_PS),
 * solo una a la vez:
 *   PWM5: P2.0  P1.7  P0.0  P7.4      (P2.0: LED_BUILTIN de la placa verde)
 *   PWM6: P2.1  P5.4  P0.1  P7.5
 *   PWM7: P2.2  P3.3  P0.2  P7.6
 *   PWM8: P2.3  P3.4  P0.3  P7.7      (P3.4: LED de la placa negra)
 * Valores de 0 a 255 como en Arduino, a unos 1000 Hz. En las demas patas,
 * analogWrite pone la pata en HIGH si el valor es 128 o mas.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

static const unsigned char pwmb_patas[4][4] = {
	{ P2_0, P1_7, P0_0, P7_4 },
	{ P2_1, P5_4, P0_1, P7_5 },
	{ P2_2, P3_3, P0_2, P7_6 },
	{ P2_3, P3_4, P0_3, P7_7 },
};

static unsigned char pwmb_listo;

static void pwmb_iniciar(void)
{
	/* contador a F_CPU/(PSCR+1), periodo de 256 cuentas: unos 1000 Hz */
	unsigned int pscr = (unsigned int)(F_CPU / 1000UL / 256UL - 1);
	EAXFR = 1;
	PWMB_PSCRH = (unsigned char)(pscr >> 8);
	PWMB_PSCRL = (unsigned char)pscr;
	PWMB_ARRH = 0;
	PWMB_ARRL = 255;
	PWMB_BKR = 0x80;                   /* MOE: salidas habilitadas */
	PWMB_EGR = 0x01;                   /* UG: cargar el preescalador y el periodo */
	PWMB_CR1 = 0x81;                   /* ARPE y CEN: en marcha */
	pwmb_listo = 1;
}

void analogWrite(uint8_t pin, int val)
{
	unsigned char c, ps;
	unsigned int ccr;

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

	EAXFR = 1;
	PWMB_PS = (PWMB_PS & ~(3 << (2 * c))) | (ps << (2 * c));
	XFR(PWMB_CCMR1_DIR + c) = 0x68;              /* modo PWM 1 con precarga */
	XFR(PWMB_CCR5H_DIR + 2 * c) = (unsigned char)(ccr >> 8);   /* primero el alto */
	XFR(PWMB_CCR5H_DIR + 2 * c + 1) = (unsigned char)ccr;
	if (c < 2) PWMB_CCER1 |= 1 << (4 * c);        /* CC5E / CC6E */
	else PWMB_CCER2 |= 1 << (4 * (c - 2));        /* CC7E / CC8E */
	PWMB_ENO |= 1 << (2 * c);                     /* ENO5P..ENO8P */
}
