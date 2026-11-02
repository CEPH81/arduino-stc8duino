/*
 * Wire.c - I2C maestro (ver Wire.h).
 * STC8: modulo I2C (registros I2CCFG..I2CRXD en FE80H-FE87H, manual STC8H cap. 19).
 * STC15: por programa en P1.5 (SCL) y P1.4 (SDA).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include "wiring_private.h"
#include "Wire.h"

static __xdata uint8_t tx_buf[WIRE_BUFFER_SIZE], rx_buf[WIRE_BUFFER_SIZE];
static __data uint8_t tx_addr, tx_len, rx_len, rx_pos, wire_pins;
static __data uint8_t wire_speed = 0xFF;      /* 0xFF = calcular para 100 kHz */
static __bit tx_err;

#define ERR_OK      0
#define ERR_NACK_A  2
#define ERR_NACK_D  3
#define ERR_TIMEOUT 5

#ifdef STC8
/* ------------------------------------------------ modulo I2C de los STC8 */
#define XS(a) (*(volatile __xdata uint8_t *)(a))
#define I2CCFG   XS(0xFE80)
#define I2CMSCR  XS(0xFE81)
#define I2CMSST  XS(0xFE82)
#define I2CTXD   XS(0xFE86)
#define I2CRXD   XS(0xFE87)

/* patas de cada opcion: puerto (0-7) y mascara de SCL|SDA */
static __code const uint8_t pin_puerto[4] = { 1, 2, 7, 3 };
static __code const uint8_t pin_mascara[4] = { 0x30, 0x30, 0xC0, 0x0C };

/* espera el fin de la orden en curso; 0 si el bus no responde */
static uint8_t esperar(void)
{
	__data unsigned int n = 0;
	while (!(I2CMSST & 0x40)) {
		if (++n == 0) return 0;
	}
	I2CMSST &= ~0x40;
	return 1;
}

static uint8_t orden(uint8_t cmd)
{
	I2CMSCR = cmd;
	return esperar();
}

static void hw_iniciar(void)
{
	__data uint8_t m = pin_mascara[wire_pins], p = pin_puerto[wire_pins];
	__data unsigned long f = (unsigned long)T0_TICKS * 1000UL;   /* frecuencia real */
	__data unsigned int sp;

	if (wire_speed == 0xFF) {
		sp = (unsigned int)((f / 100000UL / 2 - 4) / 2);
		wire_speed = sp > 63 ? 63 : sp;
	}
	P_SW2 = (P_SW2 & ~0x30) | (wire_pins << 4) | 0x80;
	/* drenador abierto (PxM1 = PxM0 = 1) y pull-up interno */
	switch (p) {
	case 1: P1M1 |= m; P1M0 |= m; break;
	case 2: P2M1 |= m; P2M0 |= m; break;
	case 3: P3M1 |= m; P3M0 |= m; break;
	case 7: P7M1 |= m; P7M0 |= m; break;
	}
	XS(0xFE10 + p) |= m;                 /* PxPU */
	I2CCFG = 0xC0 | wire_speed;          /* ENI2C, maestro, velocidad */
	I2CMSST = 0x00;
}

static uint8_t i2c_start(void) { return orden(0x01); }
static uint8_t i2c_stop(void)  { return orden(0x06); }

/* envia un byte; 0 = ACK, 1 = NACK, 2 = el bus no responde */
static uint8_t i2c_send(uint8_t b)
{
	I2CTXD = b;
	if (!orden(0x02)) return 2;
	if (!orden(0x03)) return 2;
	return (I2CMSST & 0x02) ? 1 : 0;
}

/* recibe un byte y responde ACK (ack = 1) o NACK */
static int i2c_recv(uint8_t ack)
{
	__data uint8_t b;
	if (!orden(0x04)) return -1;
	b = I2CRXD;
	I2CMSST = ack ? 0x00 : 0x01;         /* MSACKO */
	if (!orden(0x05)) return -1;
	return b;
}

#else
/* ------------------------------------------------ STC15: por programa */
static __sbit __at(0x95) SCL_P;          /* P1.5 */
static __sbit __at(0x94) SDA_P;          /* P1.4 */
static __data uint8_t medio = 3;         /* espera de medio periodo, en us */

static void espera(void) { delayMicroseconds(medio); }

static uint8_t scl_alto(void)
{
	__data unsigned int n = 0;
	SCL_P = 1;
	while (!SCL_P) {                     /* el esclavo puede retener el reloj */
		if (++n == 0) return 0;
	}
	return 1;
}

static void hw_iniciar(void)
{
	/* cuasi-bidireccional: escribir 1 suelta la linea */
	P1M1 &= ~0x30;
	P1M0 &= ~0x30;
	SDA_P = 1;
	SCL_P = 1;
	if (wire_speed != 0xFF) medio = wire_speed;
}

static uint8_t i2c_start(void)
{
	SDA_P = 1;
	if (!scl_alto()) return 0;
	espera();
	SDA_P = 0;
	espera();
	SCL_P = 0;
	return 1;
}

static uint8_t i2c_stop(void)
{
	SDA_P = 0;
	espera();
	if (!scl_alto()) return 0;
	espera();
	SDA_P = 1;
	espera();
	return 1;
}

static uint8_t i2c_send(uint8_t b)
{
	__data uint8_t i, ack;
	for (i = 0; i < 8; i++) {
		SDA_P = (b & 0x80) ? 1 : 0;
		b <<= 1;
		espera();
		if (!scl_alto()) return 2;
		espera();
		SCL_P = 0;
	}
	SDA_P = 1;
	espera();
	if (!scl_alto()) return 2;
	ack = SDA_P;
	espera();
	SCL_P = 0;
	return ack ? 1 : 0;
}

static int i2c_recv(uint8_t ack)
{
	__data uint8_t i, b = 0;
	SDA_P = 1;
	for (i = 0; i < 8; i++) {
		espera();
		if (!scl_alto()) return -1;
		b = (b << 1) | SDA_P;
		espera();
		SCL_P = 0;
	}
	SDA_P = ack ? 0 : 1;
	espera();
	if (!scl_alto()) return -1;
	espera();
	SCL_P = 0;
	SDA_P = 1;
	return b;
}
#endif

/* ------------------------------------------------ interfaz Wire */
void Wire_begin(void)
{
	hw_iniciar();
	tx_len = rx_len = rx_pos = 0;
}

void Wire_end(void)
{
#ifdef STC8
	I2CCFG = 0;
#endif
}

void Wire_setPins(uint8_t opcion)
{
	wire_pins = opcion & 3;
}

void Wire_setClock(unsigned long hz)
{
#ifdef STC8
	__data unsigned long f = (unsigned long)T0_TICKS * 1000UL;
	__data unsigned long d = f / hz / 2;
	__data unsigned int sp = d > 4 ? (unsigned int)((d - 4) / 2) : 0;
	wire_speed = sp > 63 ? 63 : sp;
	I2CCFG = 0xC0 | wire_speed;
#else
	/* por programa: medio periodo en us (aproximado; tope real unos 100 kHz) */
	__data unsigned long m = 500000UL / hz;
	wire_speed = m < 1 ? 1 : m > 250 ? 250 : (uint8_t)m;
	medio = wire_speed;
#endif
}

void Wire_beginTransmission(uint8_t address)
{
	tx_addr = address;
	tx_len = 0;
	tx_err = 0;
}

size_t Wire_write(uint8_t data)
{
	if (tx_len >= WIRE_BUFFER_SIZE) {
		tx_err = 1;
		return 0;
	}
	tx_buf[tx_len++] = data;
	return 1;
}

size_t Wire_write_sn(const uint8_t *data, size_t quantity)
{
	size_t n = 0;
	while (quantity--) n += Wire_write(*data++);
	return n;
}

size_t Wire_write_s(const char *str)
{
	size_t n = 0;
	while (*str) n += Wire_write((uint8_t)*str++);
	return n;
}

uint8_t Wire_endTransmission1(uint8_t sendStop)
{
	__data uint8_t i, r, err = ERR_OK;

	if (!i2c_start()) return ERR_TIMEOUT;
	r = i2c_send(tx_addr << 1);
	if (r == 2) return ERR_TIMEOUT;
	if (r) {
		err = ERR_NACK_A;
	} else {
		for (i = 0; i < tx_len; i++) {
			r = i2c_send(tx_buf[i]);
			if (r == 2) return ERR_TIMEOUT;
			if (r) {
				err = ERR_NACK_D;
				break;
			}
		}
	}
	if (sendStop || err) i2c_stop();
	tx_len = 0;
	if (!err && tx_err) err = 1;          /* datos demasiado largos */
	return err;
}

uint8_t Wire_requestFrom3(uint8_t address, uint8_t quantity, uint8_t sendStop)
{
	__data uint8_t i;
	__data int b;

	rx_len = rx_pos = 0;
	if (quantity > WIRE_BUFFER_SIZE) quantity = WIRE_BUFFER_SIZE;
	if (quantity == 0) return 0;
	if (!i2c_start()) return 0;
	if (i2c_send((address << 1) | 1)) {
		i2c_stop();
		return 0;
	}
	for (i = 0; i < quantity; i++) {
		b = i2c_recv(i + 1 < quantity);  /* ACK salvo en el ultimo byte */
		if (b < 0) return rx_len;
		rx_buf[rx_len++] = (uint8_t)b;
	}
	if (sendStop) i2c_stop();
	return rx_len;
}

int Wire_available(void)
{
	return rx_len - rx_pos;
}

int Wire_read(void)
{
	if (rx_pos >= rx_len) return -1;
	return rx_buf[rx_pos++];
}

int Wire_peek(void)
{
	if (rx_pos >= rx_len) return -1;
	return rx_buf[rx_pos];
}
