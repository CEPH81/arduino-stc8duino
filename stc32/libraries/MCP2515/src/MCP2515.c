/*
 * MCP2515.c - controlador CAN MCP2515 por SPI (ver MCP2515.h).
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <SPI.h>
#include "MCP2515.h"

/* ordenes SPI */
#define I_RESET      0xC0
#define I_READ       0x03
#define I_WRITE      0x02
#define I_BITMOD     0x05
#define I_READ_RX    0x90     /* + 0x00 RXB0, + 0x04 RXB1 */
#define I_RTS_TX0    0x81
/* registros */
#define CANSTAT   0x0E
#define CANCTRL   0x0F
#define CNF3      0x28
#define CNF2      0x29
#define CNF1      0x2A
#define CANINTE   0x2B
#define CANINTF   0x2C
#define EFLG      0x2D
#define TXB0CTRL  0x30
#define TXB0SIDH  0x31
#define RXB0CTRL  0x60
#define RXB1CTRL  0x70
/* modos (CANCTRL bits 7:5) */
#define MODO_NORMAL   0x00
#define MODO_BUCLE    0x40
#define MODO_ESCUCHA  0x60
#define MODO_CONFIG   0x80

static uint8_t pin_cs;

/* CNF1, CNF2, CNF3 para 50k, 100k, 125k, 250k, 500k y 1M (punto de muestreo ~75 %) */
static const uint8_t tiempos8[6][3] = {
	{ 0x03, 0xB4, 0x86 }, { 0x01, 0xB4, 0x86 }, { 0x01, 0xB1, 0x85 },
	{ 0x00, 0xB1, 0x85 }, { 0x00, 0x90, 0x82 }, { 0x00, 0x80, 0x80 },
};
static const uint8_t tiempos16[6][3] = {
	{ 0x07, 0xFA, 0x87 }, { 0x03, 0xFA, 0x87 }, { 0x03, 0xF0, 0x86 },
	{ 0x41, 0xF1, 0x85 }, { 0x00, 0xF0, 0x86 }, { 0x00, 0xD0, 0x82 },
};

static void cs_bajo(void)
{
	SPI_beginTransaction(8000000UL, MSBFIRST, SPI_MODE0);
	digitalWrite(pin_cs, LOW);
}

static void cs_alto(void)
{
	digitalWrite(pin_cs, HIGH);
}

uint8_t mcp2515_readRegister(uint8_t reg)
{
	uint8_t v;
	cs_bajo();
	SPI_transfer(I_READ);
	SPI_transfer(reg);
	v = SPI_transfer(0xFF);
	cs_alto();
	return v;
}

static void escribir(uint8_t reg, uint8_t v)
{
	cs_bajo();
	SPI_transfer(I_WRITE);
	SPI_transfer(reg);
	SPI_transfer(v);
	cs_alto();
}

static void cambiar_bits(uint8_t reg, uint8_t mascara, uint8_t v)
{
	cs_bajo();
	SPI_transfer(I_BITMOD);
	SPI_transfer(reg);
	SPI_transfer(mascara);
	SPI_transfer(v);
	cs_alto();
}

static uint8_t poner_modo(uint8_t modo)
{
	unsigned long t0 = millis();
	cambiar_bits(CANCTRL, 0xE0, modo);
	while ((mcp2515_readRegister(CANSTAT) & 0xE0) != modo) {
		if (millis() - t0 > 50) return MCP2515_ERROR;
	}
	return MCP2515_OK;
}

uint8_t mcp2515_begin(uint8_t cs, uint8_t velocidad, uint8_t cristal)
{
	const uint8_t *t;
	pin_cs = cs;
	pinMode(pin_cs, OUTPUT);
	digitalWrite(pin_cs, HIGH);

	cs_bajo();
	SPI_transfer(I_RESET);
	cs_alto();
	delay(10);
	if (poner_modo(MODO_CONFIG) != MCP2515_OK) return MCP2515_ERROR;

	if (velocidad > CAN_1000KBPS) velocidad = CAN_500KBPS;
	t = (cristal == MCP_16MHZ) ? tiempos16[velocidad] : tiempos8[velocidad];
	escribir(CNF1, t[0]);
	escribir(CNF2, t[1]);
	escribir(CNF3, t[2]);
	escribir(CANINTE, 0x00);
	escribir(CANINTF, 0x00);
	escribir(RXB0CTRL, 0x64);    /* sin filtros y con paso a RXB1 si RXB0 esta lleno */
	escribir(RXB1CTRL, 0x60);
	return poner_modo(MODO_NORMAL);
}

uint8_t mcp2515_setNormalMode(void)     { return poner_modo(MODO_NORMAL); }
uint8_t mcp2515_setLoopbackMode(void)   { return poner_modo(MODO_BUCLE); }
uint8_t mcp2515_setListenOnlyMode(void) { return poner_modo(MODO_ESCUCHA); }

uint8_t mcp2515_errors(void)
{
	return mcp2515_readRegister(EFLG);
}

uint8_t mcp2515_send(unsigned long id, uint8_t extendido, uint8_t largo, const uint8_t *datos)
{
	uint8_t i, c;
	unsigned long t0;

	if (largo > 8) largo = 8;
	cs_bajo();
	SPI_transfer(I_WRITE);
	SPI_transfer(TXB0SIDH);
	if (extendido) {
		SPI_transfer((uint8_t)(id >> 21));
		SPI_transfer((uint8_t)((((id >> 18) & 0x07) << 5) | 0x08 | ((id >> 16) & 0x03)));
		SPI_transfer((uint8_t)(id >> 8));
		SPI_transfer((uint8_t)id);
	} else {
		SPI_transfer((uint8_t)(id >> 3));
		SPI_transfer((uint8_t)((id & 0x07) << 5));
		SPI_transfer(0);
		SPI_transfer(0);
	}
	SPI_transfer(largo);
	for (i = 0; i < largo; i++) SPI_transfer(datos[i]);
	cs_alto();

	cs_bajo();
	SPI_transfer(I_RTS_TX0);          /* pedir el envio */
	cs_alto();

	t0 = millis();
	do {
		c = mcp2515_readRegister(TXB0CTRL);
		if (!(c & 0x08)) return MCP2515_OK;            /* TXREQ a 0: enviada */
	} while (millis() - t0 < 20);
	cambiar_bits(TXB0CTRL, 0x08, 0x00);               /* abortar */
	return MCP2515_SIN_ACK;
}

uint8_t mcp2515_available(void)
{
	return (mcp2515_readRegister(CANINTF) & 0x03) ? 1 : 0;
}

uint8_t mcp2515_read(unsigned long *id, uint8_t *extendido, uint8_t *largo, uint8_t *datos)
{
	uint8_t f = mcp2515_readRegister(CANINTF);
	uint8_t sidh, sidl, eid8, eid0, dlc, i, n;

	if (!(f & 0x03)) return MCP2515_ERROR;
	cs_bajo();
	SPI_transfer((f & 0x01) ? I_READ_RX : (I_READ_RX | 0x04));
	sidh = SPI_transfer(0xFF);
	sidl = SPI_transfer(0xFF);
	eid8 = SPI_transfer(0xFF);
	eid0 = SPI_transfer(0xFF);
	dlc = SPI_transfer(0xFF);
	n = dlc & 0x0F;
	if (n > 8) n = 8;
	if (dlc & 0x40) n = 0;            /* trama remota (RTR): sin datos */
	for (i = 0; i < n; i++) datos[i] = SPI_transfer(0xFF);
	cs_alto();                        /* READ RX borra el aviso de ese bufer */

	if (sidl & 0x08) {
		*extendido = 1;
		*id = ((unsigned long)sidh << 21) | ((unsigned long)(sidl >> 5) << 18) |
		      ((unsigned long)(sidl & 0x03) << 16) | ((unsigned long)eid8 << 8) | eid0;
	} else {
		*extendido = 0;
		*id = ((unsigned long)sidh << 3) | (sidl >> 5);
	}
	*largo = n;
	return MCP2515_OK;
}
