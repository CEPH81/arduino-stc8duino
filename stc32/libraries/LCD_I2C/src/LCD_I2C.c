/* LCD_I2C.c - LCD HD44780 por I2C con PCF8574 (ver LCD_I2C.h). Parte de stc8duino. LGPL 2.1. */
#include <Arduino.h>
#include <Wire.h>
#include "LCD_I2C.h"

#define RS 0x01
#define EN 0x04
#define BL 0x08

static uint8_t lcd_addr, lcd_cols, lcd_rows, lcd_luz = BL, lcd_ctrl;

/* un nibble (en los 4 bits altos) con su pulso de E; mode = 0 orden, RS dato */
static void nibble_en(uint8_t v)
{
	Wire_write(v);
	Wire_write(v | EN);
	Wire_write(v);
}

/* un byte completo en una sola transmision I2C */
static void enviar(uint8_t valor, uint8_t mode)
{
	uint8_t m = mode | lcd_luz;
	Wire_beginTransmission(lcd_addr);
	nibble_en((valor & 0xF0) | m);
	nibble_en((valor << 4) | m);
	Wire_endTransmission();
}

/* solo el nibble alto (arranque en 4 bits) */
static void enviar4(uint8_t nib)
{
	Wire_beginTransmission(lcd_addr);
	nibble_en((nib << 4) | lcd_luz);
	Wire_endTransmission();
}

void lcd_command(uint8_t value)
{
	enviar(value, 0);
	if (value <= 0x03) delay(2);          /* clear y home tardan 1,52 ms */
}

size_t lcd_write(uint8_t value)
{
	enviar(value, RS);
	return 1;
}

size_t lcd_writeStr(const char *s)
{
	size_t n = 0;
	while (*s) {
		enviar((uint8_t)*s++, RS);
		n++;
	}
	return n;
}

uint8_t lcd_begin(uint8_t addr, uint8_t cols, uint8_t rows)
{
	uint8_t r;
	lcd_addr = addr;
	lcd_cols = cols;
	lcd_rows = rows;
	delay(50);                            /* la pantalla necesita >40 ms al encender */
	Wire_beginTransmission(lcd_addr);
	Wire_write(lcd_luz);
	r = Wire_endTransmission();
	/* secuencia de inicio en 4 bits (hoja de datos del HD44780, figura 24) */
	enviar4(0x03); delay(5);
	enviar4(0x03); delay(5);
	enviar4(0x03); delayMicroseconds(150);
	enviar4(0x02);
	lcd_command(0x20 | (rows > 1 ? 0x08 : 0x00));   /* 4 bits, 1 o 2 lineas, 5x8 */
	lcd_ctrl = 0x04;                                /* pantalla encendida, sin cursor */
	lcd_command(0x08 | lcd_ctrl);
	lcd_clear();
	lcd_command(0x06);                              /* escribir de izquierda a derecha */
	return r;
}

void lcd_clear(void) { lcd_command(0x01); }
void lcd_home(void)  { lcd_command(0x02); }

void lcd_setCursor(uint8_t col, uint8_t row)
{
	uint8_t off;
	if (row >= lcd_rows) row = lcd_rows - 1;
	switch (row) {
	case 0:  off = 0x00; break;
	case 1:  off = 0x40; break;
	case 2:  off = lcd_cols; break;          /* 20x4: 0x14, 16x4: 0x10 */
	default: off = 0x40 + lcd_cols; break;
	}
	lcd_command(0x80 | (off + col));
}

static void control(void) { lcd_command(0x08 | lcd_ctrl); }
void lcd_display(void)   { lcd_ctrl |= 0x04;  control(); }
void lcd_noDisplay(void) { lcd_ctrl &= ~0x04; control(); }
void lcd_cursor(void)    { lcd_ctrl |= 0x02;  control(); }
void lcd_noCursor(void)  { lcd_ctrl &= ~0x02; control(); }
void lcd_blink(void)     { lcd_ctrl |= 0x01;  control(); }
void lcd_noBlink(void)   { lcd_ctrl &= ~0x01; control(); }
void lcd_scrollDisplayLeft(void)  { lcd_command(0x18); }
void lcd_scrollDisplayRight(void) { lcd_command(0x1C); }

static void luz(void)
{
	Wire_beginTransmission(lcd_addr);
	Wire_write(lcd_luz);
	Wire_endTransmission();
}
void lcd_backlight(void)   { lcd_luz = BL; luz(); }
void lcd_noBacklight(void) { lcd_luz = 0;  luz(); }

void lcd_createChar(uint8_t location, const uint8_t *charmap)
{
	uint8_t i;
	lcd_command(0x40 | ((location & 7) << 3));
	for (i = 0; i < 8; i++) lcd_write(charmap[i]);
	lcd_command(0x80);                    /* volver a la memoria de texto */
}
