/*
 * NRF24.c - nRF24L01(+) por SPI (ver NRF24.h).
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <SPI.h>
#include "NRF24.h"

/* ordenes */
#define R_REGISTER    0x00
#define W_REGISTER    0x20
#define R_RX_PAYLOAD  0x61
#define W_TX_PAYLOAD  0xA0
#define FLUSH_TX      0xE1
#define FLUSH_RX      0xE2
#define NOP           0xFF
/* registros */
#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define STATUS      0x07
#define RX_ADDR_P0  0x0A
#define TX_ADDR     0x10
#define RX_PW_P0    0x11
#define FIFO_STATUS 0x17
#define DYNPD       0x1C
#define FEATURE     0x1D
/* bits */
#define RX_DR   0x40
#define TX_DS   0x20
#define MAX_RT  0x10
#define PWR_UP  0x02
#define PRIM_RX 0x01

static uint8_t pin_ce, pin_csn, tam_paquete = 32;
static uint8_t dir_p0[5];          /* direccion de lectura del tubo 0 (se restaura) */
static uint8_t p0_usado;

static void cs_bajo(void)
{
	SPI_beginTransaction(8000000UL, MSBFIRST, SPI_MODE0);
	digitalWrite(pin_csn, LOW);
}

static void cs_alto(void)
{
	digitalWrite(pin_csn, HIGH);
}

static uint8_t orden(uint8_t o)
{
	uint8_t st;
	cs_bajo();
	st = SPI_transfer(o);
	cs_alto();
	return st;
}

uint8_t nrf24_readRegister(uint8_t reg)
{
	uint8_t v;
	cs_bajo();
	SPI_transfer(R_REGISTER | (reg & 0x1F));
	v = SPI_transfer(NOP);
	cs_alto();
	return v;
}

void nrf24_writeRegister(uint8_t reg, uint8_t valor)
{
	cs_bajo();
	SPI_transfer(W_REGISTER | (reg & 0x1F));
	SPI_transfer(valor);
	cs_alto();
}

static void escribir_bytes(uint8_t reg, const uint8_t *p, uint8_t n)
{
	cs_bajo();
	SPI_transfer(W_REGISTER | (reg & 0x1F));
	while (n--) SPI_transfer(*p++);
	cs_alto();
}

uint8_t nrf24_isConnected(void)
{
	uint8_t aw = nrf24_readRegister(SETUP_AW);
	return aw >= 1 && aw <= 3;
}

uint8_t nrf24_begin(uint8_t ce, uint8_t csn)
{
	pin_ce = ce;
	pin_csn = csn;
	pinMode(pin_ce, OUTPUT);
	pinMode(pin_csn, OUTPUT);
	digitalWrite(pin_ce, LOW);
	digitalWrite(pin_csn, HIGH);
	delay(5);                                   /* arranque del modulo */

	nrf24_writeRegister(CONFIG, 0x0C);          /* CRC 16 bits, apagado */
	nrf24_setRetries(5, 15);                    /* 1500 us, 15 reintentos */
	nrf24_setDataRate(NRF24_1MBPS);
	nrf24_setPALevel(NRF24_PA_MAX);
	nrf24_writeRegister(SETUP_AW, 0x03);        /* direcciones de 5 bytes */
	nrf24_writeRegister(EN_AA, 0x3F);           /* ACK automatico en todos los tubos */
	nrf24_writeRegister(EN_RXADDR, 0x03);       /* tubos 0 y 1 */
	nrf24_writeRegister(DYNPD, 0x00);
	nrf24_writeRegister(FEATURE, 0x00);
	nrf24_setChannel(76);
	nrf24_setPayloadSize(32);
	nrf24_writeRegister(STATUS, RX_DR | TX_DS | MAX_RT);
	orden(FLUSH_RX);
	orden(FLUSH_TX);
	nrf24_powerUp();
	return nrf24_isConnected() ? 0 : 1;
}

void nrf24_setChannel(uint8_t canal)
{
	nrf24_writeRegister(RF_CH, canal > 125 ? 125 : canal);
}

void nrf24_setPayloadSize(uint8_t n)
{
	uint8_t i;
	if (n < 1) n = 1;
	if (n > 32) n = 32;
	tam_paquete = n;
	for (i = 0; i < 6; i++) nrf24_writeRegister(RX_PW_P0 + i, n);
}

void nrf24_setDataRate(uint8_t velocidad)
{
	uint8_t r = nrf24_readRegister(RF_SETUP) & ~0x28;
	if (velocidad == NRF24_250KBPS) r |= 0x20;
	else if (velocidad == NRF24_2MBPS) r |= 0x08;
	nrf24_writeRegister(RF_SETUP, r);
}

void nrf24_setPALevel(uint8_t nivel)
{
	uint8_t r = nrf24_readRegister(RF_SETUP) & ~0x07;
	nrf24_writeRegister(RF_SETUP, r | ((nivel & 3) << 1) | 0x01);   /* bit 0: LNA */
}

void nrf24_setRetries(uint8_t espera, uint8_t cuantos)
{
	nrf24_writeRegister(SETUP_RETR, ((espera & 15) << 4) | (cuantos & 15));
}

void nrf24_openWritingPipe(const uint8_t *dir5)
{
	/* el tubo 0 recibe el ACK: lleva la misma direccion que el envio */
	escribir_bytes(RX_ADDR_P0, dir5, 5);
	escribir_bytes(TX_ADDR, dir5, 5);
}

void nrf24_openReadingPipe(uint8_t tubo, const uint8_t *dir5)
{
	uint8_t i;
	if (tubo > 5) return;
	if (tubo == 0) {
		for (i = 0; i < 5; i++) dir_p0[i] = dir5[i];
		p0_usado = 1;
	}
	if (tubo < 2) escribir_bytes(RX_ADDR_P0 + tubo, dir5, 5);
	else nrf24_writeRegister(RX_ADDR_P0 + tubo, dir5[0]);   /* 2-5: solo el primer byte */
	nrf24_writeRegister(EN_RXADDR, nrf24_readRegister(EN_RXADDR) | (1 << tubo));
}

void nrf24_powerUp(void)
{
	uint8_t c = nrf24_readRegister(CONFIG);
	if (!(c & PWR_UP)) {
		nrf24_writeRegister(CONFIG, c | PWR_UP);
		delay(5);                               /* 1,5 ms de arranque del oscilador */
	}
}

void nrf24_powerDown(void)
{
	digitalWrite(pin_ce, LOW);
	nrf24_writeRegister(CONFIG, nrf24_readRegister(CONFIG) & ~PWR_UP);
}

void nrf24_startListening(void)
{
	if (p0_usado) escribir_bytes(RX_ADDR_P0, dir_p0, 5);
	nrf24_writeRegister(CONFIG, nrf24_readRegister(CONFIG) | PWR_UP | PRIM_RX);
	nrf24_writeRegister(STATUS, RX_DR | TX_DS | MAX_RT);
	digitalWrite(pin_ce, HIGH);
	delayMicroseconds(130);
}

void nrf24_stopListening(void)
{
	digitalWrite(pin_ce, LOW);
	delayMicroseconds(130);
	nrf24_writeRegister(CONFIG, nrf24_readRegister(CONFIG) & ~PRIM_RX);
}

uint8_t nrf24_write(const void *buf, uint8_t n)
{
	const uint8_t *p = (const uint8_t *)buf;
	uint8_t i, st;
	unsigned long t0;

	orden(FLUSH_TX);
	nrf24_writeRegister(STATUS, RX_DR | TX_DS | MAX_RT);
	cs_bajo();
	SPI_transfer(W_TX_PAYLOAD);
	for (i = 0; i < tam_paquete; i++) SPI_transfer(i < n ? p[i] : 0);
	cs_alto();
	digitalWrite(pin_ce, HIGH);                 /* pulso de mas de 10 us */
	delayMicroseconds(15);
	digitalWrite(pin_ce, LOW);
	t0 = millis();
	do {
		st = orden(NOP);                        /* STATUS */
		if (st & (TX_DS | MAX_RT)) break;
	} while (millis() - t0 < 60);
	nrf24_writeRegister(STATUS, RX_DR | TX_DS | MAX_RT);
	if (!(st & TX_DS)) {
		orden(FLUSH_TX);
		return 0;
	}
	return 1;
}

uint8_t nrf24_availablePipe(void)
{
	uint8_t tubo;
	if (nrf24_readRegister(FIFO_STATUS) & 0x01) return 0xFF;   /* RX_EMPTY */
	tubo = (orden(NOP) >> 1) & 7;
	return tubo > 5 ? 0xFF : tubo;
}

uint8_t nrf24_available(void)
{
	return nrf24_availablePipe() != 0xFF;
}

void nrf24_read(void *buf, uint8_t n)
{
	uint8_t *p = (uint8_t *)buf;
	uint8_t i, b;
	cs_bajo();
	SPI_transfer(R_RX_PAYLOAD);
	for (i = 0; i < tam_paquete; i++) {
		b = SPI_transfer(NOP);
		if (i < n) p[i] = b;
	}
	cs_alto();
	nrf24_writeRegister(STATUS, RX_DR);
}
