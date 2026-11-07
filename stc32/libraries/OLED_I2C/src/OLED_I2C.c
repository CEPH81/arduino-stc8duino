/* OLED_I2C.c - OLED SSD1306 / SH1106 por I2C en modo texto (ver OLED_I2C.h). Parte de stc8duino (version stc32duino). LGPL 2.1. */
#include <Arduino.h>
#include <Wire.h>
#include "OLED_I2C.h"

extern const uint8_t oled_fuente[95 * 5];

static uint8_t o_addr, o_tipo, o_x, o_fila, o_tam = 1, o_inv;

/* varias ordenes en una transmision */
static void ordenes(const uint8_t *c, uint8_t n)
{
	Wire_beginTransmission(o_addr);
	Wire_write(0x00);
	while (n--) Wire_write(*c++);
	Wire_endTransmission();
}

static void orden1(uint8_t a)
{
	Wire_beginTransmission(o_addr);
	Wire_write(0x00);
	Wire_write(a);
	Wire_endTransmission();
}

/* posicion en el OLED (direccionamiento por paginas, valido en los dos chips) */
static void posicion(uint8_t x, uint8_t fila)
{
	if (o_tipo == OLED_SH1106) x += 2;   /* el SH1106 tiene 132 columnas */
	Wire_beginTransmission(o_addr);
	Wire_write(0x00);
	Wire_write(0xB0 | (fila & 7));
	Wire_write(x & 0x0F);
	Wire_write(0x10 | (x >> 4));
	Wire_endTransmission();
}

static const uint8_t ini_ssd1306[] = {
	0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x02,
	0xA1, 0xC8, 0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF
};
static const uint8_t ini_sh1106[] = {
	0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0xAD, 0x8B,
	0xA1, 0xC8, 0xDA, 0x12, 0x81, 0x80, 0xD9, 0x22, 0xDB, 0x35, 0xA4, 0xA6, 0xAF
};

uint8_t oled_begin(uint8_t addr, uint8_t tipo)
{
	uint8_t r;
	o_addr = addr;
	o_tipo = tipo;
	delay(50);
	Wire_beginTransmission(o_addr);
	r = Wire_endTransmission();
	if (tipo == OLED_SH1106) ordenes(ini_sh1106, sizeof(ini_sh1106));
	else ordenes(ini_ssd1306, sizeof(ini_ssd1306));
	oled_clear();
	return r;
}

void oled_clearLine(uint8_t fila)
{
	uint8_t i, j;
	posicion(0, fila);
	for (i = 0; i < 128; i += 16) {      /* 8 transmisiones de 16 columnas */
		Wire_beginTransmission(o_addr);
		Wire_write(0x40);
		for (j = 0; j < 16; j++) Wire_write(0);
		Wire_endTransmission();
	}
	posicion(o_x, o_fila);
}

void oled_clear(void)
{
	uint8_t f;
	o_x = o_fila = 0;
	for (f = 0; f < 8; f++) oled_clearLine(f);
	posicion(0, 0);
}

void oled_setCursor(uint8_t x, uint8_t fila)
{
	o_x = x > 127 ? 127 : x;
	o_fila = fila & 7;
	posicion(o_x, o_fila);
}

void oled_setTextSize(uint8_t s) { o_tam = (s >= 2) ? 2 : 1; }
void oled_setInvert(uint8_t inv) { o_inv = inv ? 0xFF : 0; }
void oled_setContrast(uint8_t c) { orden1(0x81); orden1(c); }
void oled_display(uint8_t on)    { orden1(on ? 0xAF : 0xAE); }

/* duplica los 4 bits altos (alto = 1) o bajos de una columna para el texto doble */
static uint8_t doble(uint8_t b, uint8_t alto)
{
	uint8_t r = 0, i;
	if (alto) b >>= 4;
	for (i = 0; i < 4; i++)
		if (b & (1 << i)) r |= 3 << (2 * i);
	return r;
}

size_t oled_write(uint8_t c)
{
	const uint8_t *g;
	uint8_t i, mitad, ancho = 6 * o_tam;

	if (c == '\n') {
		o_x = 0;
		o_fila = (o_fila + o_tam) & 7;
		posicion(o_x, o_fila);
		return 1;
	}
	if (c == '\r') return 1;
	if (c < 0x20 || c > 0x7E) c = '?';
	if (o_x + ancho > 128) {             /* no cabe: sigue en la fila siguiente */
		o_x = 0;
		o_fila = (o_fila + o_tam) & 7;
		posicion(o_x, o_fila);
	}
	g = &oled_fuente[(c - 0x20) * 5];

	if (o_tam == 1) {
		Wire_beginTransmission(o_addr);
		Wire_write(0x40);
		for (i = 0; i < 5; i++) Wire_write(g[i] ^ o_inv);
		Wire_write(o_inv);
		Wire_endTransmission();
	} else {
		for (mitad = 0; mitad < 2; mitad++) {
			posicion(o_x, o_fila + mitad);
			Wire_beginTransmission(o_addr);
			Wire_write(0x40);
			for (i = 0; i < 5; i++) {
				uint8_t d = doble(g[i], mitad) ^ o_inv;
				Wire_write(d);
				Wire_write(d);
			}
			Wire_write(o_inv);
			Wire_write(o_inv);
			Wire_endTransmission();
		}
		posicion(o_x + ancho, o_fila);
	}
	o_x += ancho;
	return 1;
}

size_t oled_writeStr(const char *s)
{
	size_t n = 0;
	while (*s) n += oled_write((uint8_t)*s++);
	return n;
}
