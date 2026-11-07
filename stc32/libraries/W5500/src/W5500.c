/*
 * W5500.c - W5500 por SPI (ver W5500.h). Trama: direccion (2 bytes), control
 * (bloque << 3 | escritura << 2 | modo variable) y datos.
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <SPI.h>
#include "W5500.h"

/* registros comunes (bloque 0) */
#define MR        0x0000
#define GAR       0x0001
#define SUBR      0x0005
#define SHAR      0x0009
#define SIPR      0x000F
#define PHYCFGR   0x002E
#define VERSIONR  0x0039
/* registros de cada socket */
#define Sn_MR     0x0000
#define Sn_CR     0x0001
#define Sn_IR     0x0002
#define Sn_SR     0x0003
#define Sn_PORT   0x0004
#define Sn_DIPR   0x000C
#define Sn_DPORT  0x0010
#define Sn_TX_FSR 0x0020
#define Sn_TX_WR  0x0024
#define Sn_RX_RSR 0x0026
#define Sn_RX_RD  0x0028
/* ordenes (Sn_CR) */
#define CR_OPEN    0x01
#define CR_LISTEN  0x02
#define CR_CONNECT 0x04
#define CR_DISCON  0x08
#define CR_CLOSE   0x10
#define CR_SEND    0x20
#define CR_RECV    0x40

#define BLOQUE_REG(s) ((uint8_t)(((s) << 2) + 1))
#define BLOQUE_TX(s)  ((uint8_t)(((s) << 2) + 2))
#define BLOQUE_RX(s)  ((uint8_t)(((s) << 2) + 3))

static uint8_t pin_cs;

static void inicio(unsigned int dir, uint8_t bloque, uint8_t escribir)
{
	SPI_beginTransaction(8000000UL, MSBFIRST, SPI_MODE0);
	digitalWrite(pin_cs, LOW);
	SPI_transfer((uint8_t)(dir >> 8));
	SPI_transfer((uint8_t)dir);
	SPI_transfer((uint8_t)((bloque << 3) | (escribir ? 0x04 : 0x00)));
}

static void fin(void)
{
	digitalWrite(pin_cs, HIGH);
}

static void escribir_n(unsigned int dir, uint8_t bloque, const uint8_t *p, unsigned int n)
{
	inicio(dir, bloque, 1);
	while (n--) SPI_transfer(*p++);
	fin();
}

static void leer_n(unsigned int dir, uint8_t bloque, uint8_t *p, unsigned int n)
{
	inicio(dir, bloque, 0);
	while (n--) *p++ = SPI_transfer(0);
	fin();
}

static void escribir8(unsigned int dir, uint8_t bloque, uint8_t v)
{
	escribir_n(dir, bloque, &v, 1);
}

static uint8_t leer8(unsigned int dir, uint8_t bloque)
{
	uint8_t v;
	leer_n(dir, bloque, &v, 1);
	return v;
}

static void escribir16(unsigned int dir, uint8_t bloque, unsigned int v)
{
	uint8_t b[2];
	b[0] = (uint8_t)(v >> 8);
	b[1] = (uint8_t)v;
	escribir_n(dir, bloque, b, 2);
}

static unsigned int leer16(unsigned int dir, uint8_t bloque)
{
	uint8_t b[2];
	leer_n(dir, bloque, b, 2);
	return ((unsigned int)b[0] << 8) | b[1];
}

/* los contadores de 16 bits pueden cambiar mientras se leen: leer hasta que coincidan */
static unsigned int leer16_estable(unsigned int dir, uint8_t bloque)
{
	unsigned int a, b;
	b = leer16(dir, bloque);
	do {
		a = b;
		b = leer16(dir, bloque);
	} while (a != b);
	return a;
}

static void orden(uint8_t s, uint8_t cr)
{
	unsigned long t0 = millis();
	escribir8(Sn_CR, BLOQUE_REG(s), cr);
	while (leer8(Sn_CR, BLOQUE_REG(s)) != 0) {        /* el W5500 la borra al aceptarla */
		if (millis() - t0 > 100) break;
	}
}

uint8_t w5500_begin(uint8_t cs, const uint8_t *mac, const uint8_t *ip,
                    const uint8_t *puerta, const uint8_t *mascara)
{
	unsigned long t0;
	pin_cs = cs;
	pinMode(pin_cs, OUTPUT);
	digitalWrite(pin_cs, HIGH);
	delay(10);
	escribir8(MR, 0, 0x80);                            /* reinicio por programa */
	t0 = millis();
	while (leer8(MR, 0) & 0x80) {
		if (millis() - t0 > 100) break;
	}
	if (leer8(VERSIONR, 0) != 0x04) return 1;
	escribir_n(SHAR, 0, mac, 6);
	escribir_n(GAR, 0, puerta, 4);
	escribir_n(SUBR, 0, mascara, 4);
	escribir_n(SIPR, 0, ip, 4);
	return 0;
}

uint8_t w5500_linkUp(void)
{
	return leer8(PHYCFGR, 0) & 0x01;
}

void w5500_getIP(uint8_t *ip)
{
	leer_n(SIPR, 0, ip, 4);
}

uint8_t w5500_status(uint8_t s)
{
	return leer8(Sn_SR, BLOQUE_REG(s));
}

static uint8_t abrir(uint8_t s, uint8_t modo, unsigned int puerto, uint8_t esperado)
{
	orden(s, CR_CLOSE);
	escribir8(Sn_MR, BLOQUE_REG(s), modo);
	escribir16(Sn_PORT, BLOQUE_REG(s), puerto);
	orden(s, CR_OPEN);
	return w5500_status(s) == esperado ? 0 : 1;
}

uint8_t w5500_tcpListen(uint8_t s, unsigned int puerto)
{
	if (abrir(s, 0x01, puerto, W5500_INIT)) return 1;
	orden(s, CR_LISTEN);
	return w5500_status(s) == W5500_LISTEN ? 0 : 1;
}

uint8_t w5500_tcpConnect(uint8_t s, const uint8_t *ip, unsigned int puerto, unsigned int puerto_local)
{
	unsigned long t0;
	uint8_t st;
	if (abrir(s, 0x01, puerto_local, W5500_INIT)) return 1;
	escribir_n(Sn_DIPR, BLOQUE_REG(s), ip, 4);
	escribir16(Sn_DPORT, BLOQUE_REG(s), puerto);
	orden(s, CR_CONNECT);
	t0 = millis();
	do {
		st = w5500_status(s);
		if (st == W5500_ESTABLISHED) return 0;
		if (st == W5500_CLOSED) return 1;
	} while (millis() - t0 < 3000);
	w5500_close(s);
	return 1;
}

uint8_t w5500_udpBegin(uint8_t s, unsigned int puerto)
{
	return abrir(s, 0x02, puerto, W5500_UDP);
}

unsigned int w5500_available(uint8_t s)
{
	return leer16_estable(Sn_RX_RSR, BLOQUE_REG(s));
}

unsigned int w5500_read(uint8_t s, uint8_t *buf, unsigned int n)
{
	unsigned int hay = w5500_available(s), rd;
	if (n > hay) n = hay;
	if (n == 0) return 0;
	rd = leer16(Sn_RX_RD, BLOQUE_REG(s));
	leer_n(rd, BLOQUE_RX(s), buf, n);                 /* el W5500 da la vuelta al bufer */
	escribir16(Sn_RX_RD, BLOQUE_REG(s), rd + n);
	orden(s, CR_RECV);
	return n;
}

int w5500_readByte(uint8_t s)
{
	uint8_t b;
	return w5500_read(s, &b, 1) ? b : -1;
}

static void cargar_tx(uint8_t s, const uint8_t *buf, unsigned int n)
{
	unsigned int wr = leer16(Sn_TX_WR, BLOQUE_REG(s));
	escribir_n(wr, BLOQUE_TX(s), buf, n);
	escribir16(Sn_TX_WR, BLOQUE_REG(s), wr + n);
}

unsigned int w5500_write(uint8_t s, const uint8_t *buf, unsigned int n)
{
	unsigned int libre, parte, hecho = 0;
	unsigned long t0;
	while (hecho < n) {
		t0 = millis();
		do {
			libre = leer16_estable(Sn_TX_FSR, BLOQUE_REG(s));
			if (w5500_status(s) != W5500_ESTABLISHED && w5500_status(s) != W5500_CLOSE_WAIT)
				return hecho;
			if (millis() - t0 > 2000) return hecho;
		} while (libre == 0);
		parte = n - hecho;
		if (parte > libre) parte = libre;
		cargar_tx(s, buf + hecho, parte);
		orden(s, CR_SEND);
		hecho += parte;
	}
	return hecho;
}

unsigned int w5500_writeStr(uint8_t s, const char *texto)
{
	unsigned int n = 0;
	while (texto[n]) n++;
	return w5500_write(s, (const uint8_t *)texto, n);
}

unsigned int w5500_udpSend(uint8_t s, const uint8_t *ip, unsigned int puerto,
                           const uint8_t *buf, unsigned int n)
{
	unsigned long t0;
	uint8_t ir;
	if (n > 1472) n = 1472;
	escribir_n(Sn_DIPR, BLOQUE_REG(s), ip, 4);
	escribir16(Sn_DPORT, BLOQUE_REG(s), puerto);
	cargar_tx(s, buf, n);
	orden(s, CR_SEND);
	t0 = millis();
	do {
		ir = leer8(Sn_IR, BLOQUE_REG(s));
		if (ir & 0x10) {                             /* SEND_OK */
			escribir8(Sn_IR, BLOQUE_REG(s), 0x10);
			return n;
		}
		if (ir & 0x08) {                             /* TIMEOUT (ARP sin respuesta) */
			escribir8(Sn_IR, BLOQUE_REG(s), 0x08);
			return 0;
		}
	} while (millis() - t0 < 1000);
	return 0;
}

unsigned int w5500_udpReceive(uint8_t s, uint8_t *buf, unsigned int max,
                              uint8_t *ip, unsigned int *puerto)
{
	uint8_t cab[8];
	unsigned int largo, rd, n;
	if (w5500_available(s) < 8) return 0;
	rd = leer16(Sn_RX_RD, BLOQUE_REG(s));
	leer_n(rd, BLOQUE_RX(s), cab, 8);                /* IP (4), puerto (2), largo (2) */
	largo = ((unsigned int)cab[6] << 8) | cab[7];
	n = largo > max ? max : largo;
	leer_n(rd + 8, BLOQUE_RX(s), buf, n);
	escribir16(Sn_RX_RD, BLOQUE_REG(s), rd + 8 + largo);   /* se descarta lo que no cupo */
	orden(s, CR_RECV);
	if (ip) {
		ip[0] = cab[0]; ip[1] = cab[1]; ip[2] = cab[2]; ip[3] = cab[3];
	}
	if (puerto) *puerto = ((unsigned int)cab[4] << 8) | cab[5];
	return n;
}

void w5500_disconnect(uint8_t s)
{
	orden(s, CR_DISCON);
}

void w5500_close(uint8_t s)
{
	orden(s, CR_CLOSE);
	escribir8(Sn_IR, BLOQUE_REG(s), 0xFF);
}
