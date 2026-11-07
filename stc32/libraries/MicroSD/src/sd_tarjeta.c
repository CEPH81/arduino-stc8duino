/*
 * sd_tarjeta.c - tarjeta SD en modo SPI: inicio y sectores de 512 bytes
 * (ver MicroSD.h). C portable: SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <SPI.h>
#include "MicroSD.h"
#include "sd_interno.h"

static uint8_t pin_cs, tipo, rapido;

static void sel(void)
{
	/* al iniciar, lo mas lento que permita el SPI (la norma pide <= 400 kHz) */
	SPI_beginTransaction(rapido ? 8000000UL : 100000UL, MSBFIRST, SPI_MODE0);
	digitalWrite(pin_cs, LOW);
}

static void desel(void)
{
	digitalWrite(pin_cs, HIGH);
	SPI_transfer(0xFF);                  /* la tarjeta suelta MISO con un byte mas */
}

static uint8_t esperar_libre(unsigned int ms)
{
	unsigned long t0 = millis();
	do {
		if (SPI_transfer(0xFF) == 0xFF) return 1;
	} while (millis() - t0 < ms);
	return 0;
}

/* envia un comando y devuelve la respuesta R1 (deja CS en bajo) */
static uint8_t comando(uint8_t cmd, unsigned long arg)
{
	uint8_t r, i;
	desel();
	sel();
	if (cmd != 0 && !esperar_libre(500)) return 0xFF;
	SPI_transfer((uint8_t)(0x40 | cmd));
	SPI_transfer((uint8_t)(arg >> 24));
	SPI_transfer((uint8_t)(arg >> 16));
	SPI_transfer((uint8_t)(arg >> 8));
	SPI_transfer((uint8_t)arg);
	SPI_transfer((uint8_t)(cmd == 0 ? 0x95 : cmd == 8 ? 0x87 : 0x01));
	r = 0xFF;
	for (i = 0; i < 10; i++) {
		r = SPI_transfer(0xFF);
		if (!(r & 0x80)) break;
	}
	return r;
}

static uint8_t acomando(uint8_t cmd, unsigned long arg)
{
	uint8_t r = comando(55, 0);
	if (r > 1) return r;
	return comando(cmd, arg);
}

static uint8_t esperar_token(void)
{
	unsigned long t0 = millis();
	uint8_t r;
	do {
		r = SPI_transfer(0xFF);
		if (r != 0xFF) return r;
	} while (millis() - t0 < 300);
	return 0xFF;
}

uint8_t sd_cardInit(uint8_t cs)
{
	uint8_t i, r, ocr[4];
	unsigned long t0;
	pin_cs = cs;
	tipo = 0;
	rapido = 0;
	pinMode(cs, OUTPUT);
	digitalWrite(cs, HIGH);
	SPI_beginTransaction(100000UL, MSBFIRST, SPI_MODE0);
	for (i = 0; i < 10; i++) SPI_transfer(0xFF);    /* 80 pulsos con CS en alto */

	for (i = 0; i < 20; i++) {                      /* CMD0: modo SPI */
		r = comando(0, 0);
		if (r == 1) break;
	}
	if (r != 1) { desel(); return SD_SIN_TARJETA; }

	r = comando(8, 0x1AAUL);                        /* CMD8: tarjeta v2? */
	if (r == 1) {
		for (i = 0; i < 4; i++) ocr[i] = SPI_transfer(0xFF);
		if (ocr[2] != 0x01 || ocr[3] != 0xAA) { desel(); return SD_TARJETA_RARA; }
		tipo = SD_TIPO_SD2;
	} else {
		tipo = SD_TIPO_SD1;
	}

	t0 = millis();                                  /* ACMD41 hasta que salga de "idle" */
	do {
		r = acomando(41, tipo == SD_TIPO_SD2 ? 0x40000000UL : 0);
		if (millis() - t0 > 2000) { desel(); tipo = 0; return SD_TARJETA_RARA; }
	} while (r != 0);

	if (tipo == SD_TIPO_SD2) {
		if (comando(58, 0) == 0) {                  /* OCR: CCS = tarjeta SDHC */
			for (i = 0; i < 4; i++) ocr[i] = SPI_transfer(0xFF);
			if (ocr[0] & 0x40) tipo = SD_TIPO_SDHC;
		}
	}
	if (tipo != SD_TIPO_SDHC) comando(16, 512);     /* bloques de 512 */
	desel();
	rapido = 1;
	return SD_OK;
}

uint8_t sd_cardType(void) { return tipo; }

uint8_t sd_readSector(unsigned long n, uint8_t *buf)
{
	unsigned int i;
	if (tipo != SD_TIPO_SDHC) n <<= 9;
	if (comando(17, n) != 0 || esperar_token() != 0xFE) {
		desel();
		return SD_ERROR_ES;
	}
	for (i = 0; i < 512; i++) buf[i] = SPI_transfer(0xFF);
	SPI_transfer(0xFF);                              /* CRC */
	SPI_transfer(0xFF);
	desel();
	return SD_OK;
}

uint8_t sd_writeSector(unsigned long n, const uint8_t *buf)
{
	unsigned int i;
	uint8_t r;
	if (tipo != SD_TIPO_SDHC) n <<= 9;
	if (comando(24, n) != 0) { desel(); return SD_ERROR_ES; }
	SPI_transfer(0xFF);
	SPI_transfer(0xFE);                              /* inicio de bloque */
	for (i = 0; i < 512; i++) SPI_transfer(buf[i]);
	SPI_transfer(0xFF);                              /* CRC (no se usa en SPI) */
	SPI_transfer(0xFF);
	r = (uint8_t)(SPI_transfer(0xFF) & 0x1F);
	if (r != 0x05 || !esperar_libre(600)) { desel(); return SD_ERROR_ES; }
	desel();
	return SD_OK;
}

unsigned long sd_cardSize(void)
{
	uint8_t csd[16], i, n;
	unsigned long c;
	if (comando(9, 0) != 0 || esperar_token() != 0xFE) { desel(); return 0; }
	for (i = 0; i < 16; i++) csd[i] = SPI_transfer(0xFF);
	SPI_transfer(0xFF);
	SPI_transfer(0xFF);
	desel();
	if ((csd[0] >> 6) == 1) {                        /* CSD v2 (SDHC) */
		c = ((unsigned long)(csd[7] & 0x3F) << 16) | ((unsigned int)csd[8] << 8) | csd[9];
		return (c + 1) << 10;
	}
	c = ((unsigned long)(csd[6] & 3) << 10) | ((unsigned int)csd[7] << 2) | (csd[8] >> 6);
	n = (uint8_t)((((csd[9] & 3) << 1) | (csd[10] >> 7)) + 2 + (csd[5] & 15) - 9);
	return (c + 1) << n;
}
