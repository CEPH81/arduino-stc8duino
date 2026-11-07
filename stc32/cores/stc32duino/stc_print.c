/*
 * stc_print.c - numeros a texto para print/println (ver stc_print.h).
 * Devuelven un texto en un bufer comun: hay que usarlo enseguida.
 * Parte de stc8duino. LGPL 2.1.
 */
#include "Arduino.h"

static char stc_buf[36];

const char *stc_fmt_ulong(unsigned long v, unsigned char base)
{
	char *p = &stc_buf[sizeof(stc_buf) - 1];
	unsigned char d;
	if (base < 2 || base > 16) base = 10;
	*p = 0;
	do {
		d = (unsigned char)(v % base);
		v /= base;
		*--p = d < 10 ? '0' + d : 'A' + d - 10;
	} while (v);
	return p;
}

const char *stc_fmt_long(long v, unsigned char base)
{
	char *p;
	if (base != 10 || v >= 0)
		return stc_fmt_ulong((unsigned long)v, base);
	p = (char *)stc_fmt_ulong((unsigned long)(-v), 10);
	*--p = '-';
	return p;
}

const char *stc_fmt_float(float v, unsigned char decimales)
{
	char *p = stc_buf;
	const char *e;
	unsigned long entero;
	float redondeo = 0.5;
	unsigned char i, d;

	if (v != v) return "nan";
	if (v < 0) {
		*p++ = '-';
		v = -v;
	}
	if (v > 4294967040.0) return "ovf";
	if (decimales > 8) decimales = 8;
	for (i = 0; i < decimales; i++)
		redondeo /= 10;
	v += redondeo;
	entero = (unsigned long)v;
	v -= entero;
	e = stc_fmt_ulong(entero, 10);   /* escribe al final del bufer */
	while (*e)
		*p++ = *e++;
	if (decimales) {
		*p++ = '.';
		while (decimales--) {
			v *= 10;
			d = (unsigned char)v;
			*p++ = '0' + d;
			v -= d;
		}
	}
	*p = 0;
	return stc_buf;
}
