/*
 * STC_Log.c - log por niveles con colores ANSI (ver STC_Log.h).
 * Escribe directamente con HardwareSerial_write (Serial del nucleo).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <stdarg.h>
#include "STC_Log.h"

static __data uint8_t log_nivel = LOG_INFO;
static __data uint8_t log_colores = 1;

static __code const char letras[] = "-EWIDV";
static __code const char * __code const colores[] = {
	"", "\x1b[31m", "\x1b[33m", "\x1b[32m", "\x1b[34m", "\x1b[90m"
};

static void sale(char c)
{
	HardwareSerial_write((uint8_t)c);
}

static void sale_texto(const char *s)
{
	while (*s) sale(*s++);
}

/* numero en la base dada, con ancho minimo y relleno */
static void sale_num(unsigned long v, uint8_t base, uint8_t neg, uint8_t ancho, char relleno, uint8_t mayus)
{
	char buf[33];
	uint8_t n = 0, d;

	do {
		d = (uint8_t)(v % base);
		v /= base;
		buf[n++] = d < 10 ? '0' + d : (mayus ? 'A' : 'a') + d - 10;
	} while (v && n < 32);
	if (neg) {
		if (relleno == '0') {
			sale('-');
			if (ancho) ancho--;
		} else {
			buf[n++] = '-';
		}
	}
	while (ancho > n) {
		sale(relleno);
		ancho--;
	}
	while (n) sale(buf[--n]);
}

static void formatear(const char *fmt, va_list ap)
{
	char c;
	while ((c = *fmt++)) {
		uint8_t largo = 0, ancho = 0;
		char relleno = ' ';
		if (c != '%') {
			sale(c);
			continue;
		}
		c = *fmt++;
		if (c == '0') {
			relleno = '0';
			c = *fmt++;
		}
		while (c >= '0' && c <= '9') {
			ancho = ancho * 10 + (c - '0');
			c = *fmt++;
		}
		if (c == 'l') {
			largo = 1;
			c = *fmt++;
		}
		switch (c) {
		case 'd':
		case 'i': {
			long v = largo ? va_arg(ap, long) : (long)va_arg(ap, int);
			sale_num(v < 0 ? (unsigned long)(-v) : (unsigned long)v, 10, v < 0, ancho, relleno, 0);
			break;
		}
		case 'u':
		case 'x':
		case 'X':
		case 'o':
		case 'b': {
			unsigned long v = largo ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned int);
			uint8_t base = c == 'u' ? 10 : c == 'o' ? 8 : c == 'b' ? 2 : 16;
			sale_num(v, base, 0, ancho, relleno, c == 'X');
			break;
		}
		case 'c':
			sale((char)va_arg(ap, int));
			break;
		case 's': {
			const char *s = va_arg(ap, const char *);
			sale_texto(s ? s : "(null)");
			break;
		}
		case '%':
			sale('%');
			break;
		case 0:
			return;
		default:
			sale('%');
			sale(c);
		}
	}
}

/* <color>[millis][N][tag] */
static void cabecera(uint8_t nivel, const char *tag)
{
	if (log_colores) sale_texto(colores[nivel]);
	sale('[');
	sale_num(millis(), 10, 0, 0, ' ', 0);
	sale_texto("][");
	sale(letras[nivel]);
	sale_texto("][");
	sale_texto(tag);
	sale_texto("] ");
}

static void fin_linea(void)
{
	if (log_colores) sale_texto("\x1b[0m");
	sale_texto("\r\n");
}

void Log_Init(unsigned long baud)
{
	HardwareSerial_begin(baud);
}

void Log_SetLevel(LOG_LevelTypeDef level)
{
	log_nivel = level;
}

LOG_LevelTypeDef Log_GetLevel(void)
{
	return (LOG_LevelTypeDef)log_nivel;
}

void Log_SetColors(uint8_t enable)
{
	log_colores = enable ? 1 : 0;
}

void Log_Write(LOG_LevelTypeDef level, const char *tag, const char *fmt, ...)
{
	va_list ap;
	if (level == LOG_NONE || level > log_nivel) return;
	cabecera(level, tag);
	va_start(ap, fmt);
	formatear(fmt, ap);
	va_end(ap);
	fin_linea();
}

/* volcado hexadecimal: 16 bytes por linea con su posicion y el texto */
void Log_Hex(LOG_LevelTypeDef level, const char *tag, const void *data, unsigned int len)
{
	const uint8_t *p = (const uint8_t *)data;
	unsigned int i, j;
	if (level == LOG_NONE || level > log_nivel) return;
	for (i = 0; i < len; i += 16) {
		cabecera(level, tag);
		sale_num(i, 16, 0, 4, '0', 1);
		sale_texto(": ");
		for (j = i; j < i + 16; j++) {
			if (j < len) {
				sale_num(p[j], 16, 0, 2, '0', 1);
				sale(' ');
			} else {
				sale_texto("   ");
			}
		}
		sale(' ');
		for (j = i; j < i + 16 && j < len; j++)
			sale(p[j] >= 32 && p[j] < 127 ? p[j] : '.');
		fin_linea();
	}
}
