/*
 * DS3231.c - reloj DS3231 por I2C (ver DS3231.h).
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <Wire.h>
#include "DS3231.h"

#define DIR       0x68
#define REG_CTRL  0x0E
#define REG_ESTADO 0x0F
#define REG_TEMP  0x11

static uint8_t a_bcd(uint8_t v)  { return (uint8_t)(((v / 10) << 4) | (v % 10)); }
static uint8_t de_bcd(uint8_t v) { return (uint8_t)((v >> 4) * 10 + (v & 0x0F)); }

static uint8_t pedir(uint8_t reg, uint8_t n)
{
	Wire_beginTransmission(DIR);
	Wire_write(reg);
	if (Wire_endTransmission() != 0) return 1;
	return Wire_requestFrom2(DIR, n) == n ? 0 : 1;
}

static uint8_t escribir(uint8_t reg, uint8_t v)
{
	Wire_beginTransmission(DIR);
	Wire_write(reg);
	Wire_write(v);
	return Wire_endTransmission();
}

/* copia de trabajo: asi SDCC no guarda el puntero en la RAM interna */
static RtcTiempo x;

static int leer(uint8_t reg)
{
	if (pedir(reg, 1)) return -1;
	return (uint8_t)Wire_read();
}

uint8_t ds3231_begin(void)
{
	return leer(REG_CTRL) < 0 ? 1 : 0;
}

uint8_t ds3231_get(RtcTiempo *t)
{
	uint8_t h;
	if (pedir(0x00, 7)) return 1;
	x.segundo = de_bcd((uint8_t)(Wire_read() & 0x7F));
	x.minuto = de_bcd((uint8_t)(Wire_read() & 0x7F));
	h = (uint8_t)Wire_read();
	if (h & 0x40) {                               /* modo 12 h */
		x.hora = de_bcd((uint8_t)(h & 0x1F)) % 12;
		if (h & 0x20) x.hora += 12;
	} else {
		x.hora = de_bcd((uint8_t)(h & 0x3F));
	}
	x.diaSemana = (uint8_t)(Wire_read() & 0x07);
	x.dia = de_bcd((uint8_t)(Wire_read() & 0x3F));
	h = (uint8_t)Wire_read();                    /* mes y siglo */
	x.mes = de_bcd((uint8_t)(h & 0x1F));
	x.anio = 2000 + de_bcd((uint8_t)Wire_read()) + ((h & 0x80) ? 100 : 0);
	*t = x;
	return 0;
}

uint8_t ds3231_dayOfWeek(unsigned int anio, uint8_t mes, uint8_t dia)
{
	/* Sakamoto: 0 = domingo */
	static const uint8_t t[12] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
	unsigned int a = anio;
	uint8_t d;
	if (mes < 3) a--;
	d = (uint8_t)((a + a / 4 - a / 100 + a / 400 + t[(mes - 1) % 12] + dia) % 7);
	return d == 0 ? 7 : d;
}

uint8_t ds3231_set(RtcTiempo *t)
{
	uint8_t r, e;
	x = *t;
	x.diaSemana = ds3231_dayOfWeek(x.anio, x.mes, x.dia);
	t->diaSemana = x.diaSemana;
	Wire_beginTransmission(DIR);
	Wire_write(0x00);
	Wire_write(a_bcd(x.segundo));
	Wire_write(a_bcd(x.minuto));
	Wire_write(a_bcd(x.hora));                  /* modo 24 h */
	Wire_write(x.diaSemana);
	Wire_write(a_bcd(x.dia));
	Wire_write(a_bcd(x.mes));
	Wire_write(a_bcd((uint8_t)(x.anio % 100)));
	r = Wire_endTransmission();
	if (r) return r;
	e = (uint8_t)leer(REG_ESTADO);               /* borra OSF: ya tiene hora valida */
	return escribir(REG_ESTADO, (uint8_t)(e & 0x7F));
}

uint8_t ds3231_lostPower(void)
{
	int e = leer(REG_ESTADO);
	return (e < 0 || (e & 0x80)) ? 1 : 0;
}

int ds3231_temperature(void)
{
	int c;
	signed char ent;
	if (pedir(REG_TEMP, 2)) return -32768;
	ent = (signed char)Wire_read();
	c = ent * 100 + ((uint8_t)Wire_read() >> 6) * 25;
	return c;
}

uint8_t ds3231_setSQW(uint8_t modo)
{
	int c = leer(REG_CTRL);
	if (c < 0) return 1;
	c &= ~0x1C;                                  /* RS2, RS1, INTCN */
	if (modo == DS3231_SQW_APAGADA) c |= 0x04;   /* INTCN = 1: sin onda */
	else c |= (modo - 1) << 3;
	return escribir(REG_CTRL, (uint8_t)c);
}

static uint8_t dos(const char *p)
{
	return (uint8_t)((p[0] == ' ' ? 0 : (p[0] - '0') * 10) + (p[1] - '0'));
}

void ds3231_compileTime(RtcTiempo *t, const char *fecha, const char *hora)
{
	/* fecha "Nov  7 2026", hora "13:47:05" */
	static const char meses[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
	uint8_t m;
	for (m = 0; m < 12; m++)
		if (fecha[0] == meses[m * 3] && fecha[1] == meses[m * 3 + 1] && fecha[2] == meses[m * 3 + 2]) break;
	x.mes = (uint8_t)(m + 1);
	x.dia = dos(fecha + 4);
	x.anio = (unsigned int)(2000 + dos(fecha + 9));
	x.hora = dos(hora);
	x.minuto = dos(hora + 3);
	x.segundo = dos(hora + 6);
	x.diaSemana = ds3231_dayOfWeek(x.anio, x.mes, x.dia);
	*t = x;
}
