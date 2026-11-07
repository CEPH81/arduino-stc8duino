/*
 * Wire.c - I2C maestro con el modulo I2C del STC32G (ver Wire.h).
 * Registros I2CCFG..I2CRXD en 7EFE80H-7EFE87H (XFR, necesitan EAXFR = 1).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include "Wire.h"

#define I2CCFG   XFR(0x7efe80)
#define I2CMSCR  XFR(0x7efe81)
#define I2CMSST  XFR(0x7efe82)
#define I2CTXD   XFR(0x7efe86)
#define I2CRXD   XFR(0x7efe87)

#define ERR_OK      0
#define ERR_NACK_A  2
#define ERR_NACK_D  3
#define ERR_TIMEOUT 5

static uint8_t tx_buf[WIRE_BUFFER_SIZE], rx_buf[WIRE_BUFFER_SIZE];
static uint8_t tx_addr, tx_len, rx_len, rx_pos, wire_pins, tx_err;
static uint8_t wire_speed = 0xFF;      /* 0xFF = calcular para 100 kHz */

/* patas de cada opcion: puerto (0-7) y mascara de SCL|SDA */
static const uint8_t pin_puerto[4] = { 1, 2, 7, 3 };
static const uint8_t pin_mascara[4] = { 0x30, 0x30, 0xC0, 0x0C };

/* espera el fin de la orden en curso; 0 si el bus no responde */
static uint8_t esperar(void)
{
	unsigned int n = 0;
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

static uint8_t calc_speed(unsigned long hz)
{
	/* SCL = F_CPU / 2 / (SPEED * 2 + 4) */
	unsigned long d = F_CPU / hz / 2;
	unsigned int sp = d > 4 ? (unsigned int)((d - 4) / 2) : 0;
	return sp > 63 ? 63 : (uint8_t)sp;
}

static void hw_iniciar(void)
{
	uint8_t m = pin_mascara[wire_pins], p = pin_puerto[wire_pins];

	if (wire_speed == 0xFF) wire_speed = calc_speed(100000UL);
	EAXFR = 1;
	P_SW2 = (P_SW2 & ~0x30) | (wire_pins << 4) | 0x80;
	/* drenador abierto (PxM1 = PxM0 = 1) y pull-up interno */
	switch (p) {
	case 1: P1M1 |= m; P1M0 |= m; P1PU |= m; break;
	case 2: P2M1 |= m; P2M0 |= m; P2PU |= m; break;
	case 3: P3M1 |= m; P3M0 |= m; P3PU |= m; break;
	case 7: P7M1 |= m; P7M0 |= m; P7PU |= m; break;
	}
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
	uint8_t b;
	if (!orden(0x04)) return -1;
	b = I2CRXD;
	I2CMSST = ack ? 0x00 : 0x01;         /* MSACKO */
	if (!orden(0x05)) return -1;
	return b;
}

void Wire_begin(void)
{
	hw_iniciar();
	tx_len = rx_len = rx_pos = 0;
}

void Wire_end(void)
{
	I2CCFG = 0;
}

void Wire_setPins(uint8_t opcion)
{
	wire_pins = opcion & 3;
}

void Wire_setClock(unsigned long hz)
{
	wire_speed = calc_speed(hz);
	I2CCFG = 0xC0 | wire_speed;
}

void Wire_beginTransmission(uint8_t address)
{
	tx_addr = address;
	tx_len = 0;
	tx_err = 0;
}

size_t Wire_write(uint8_t dato)
{
	if (tx_len >= WIRE_BUFFER_SIZE) {
		tx_err = 1;
		return 0;
	}
	tx_buf[tx_len++] = dato;
	return 1;
}

size_t Wire_write_sn(const uint8_t *dato, size_t quantity)
{
	size_t n = 0;
	while (quantity--) n += Wire_write(*dato++);
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
	uint8_t i, r, err = ERR_OK;

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
	uint8_t i;
	int b;

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
