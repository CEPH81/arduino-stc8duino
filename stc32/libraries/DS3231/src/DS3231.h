/*
 * DS3231.h - reloj de tiempo real DS3231 por I2C, para stc8duino (STC8H...)
 * y stc32duino (STC32G). La EEPROM AT24C32 del mismo modulo: AT24C32.h.
 *
 * Modulo ZS-042: VCC 3,3 o 5 V, GND, SCL -> P1.5, SDA -> P1.4.
 *   DS3231 en 0x68, AT24C32 en 0x57 (A0-A2 en alto).
 *   Ojo: con pila CR2032 (no recargable) quite el diodo o la resistencia de
 *   200 ohm del circuito de carga del modulo.
 *
 *   RtcTiempo t;
 *   Wire.begin();
 *   ds3231_begin();                        // 0 si responde
 *   ds3231_get(&t);                        // t.anio, t.mes, t.dia, t.hora...
 *   ds3231_set(&t);                        // el dia de la semana lo calcula
 *   c = ds3231_temperature();              // centesimas de grado: 2525 = 25,25 C
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef DS3231_h
#define DS3231_h

#include <Arduino.h>
#include <Wire.h>

typedef struct {
	unsigned int anio;       /* 2000-2099 */
	uint8_t mes;             /* 1-12 */
	uint8_t dia;             /* 1-31 */
	uint8_t hora;            /* 0-23 */
	uint8_t minuto;
	uint8_t segundo;
	uint8_t diaSemana;       /* 1 = lunes ... 7 = domingo */
} RtcTiempo;

/* onda cuadrada en la pata SQW */
#define DS3231_SQW_APAGADA 0
#define DS3231_SQW_1HZ     1
#define DS3231_SQW_1024HZ  2
#define DS3231_SQW_4096HZ  3
#define DS3231_SQW_8192HZ  4

uint8_t ds3231_begin(void);                     /* 0 si responde */
uint8_t ds3231_get(RtcTiempo *t);               /* 0 si bien */
uint8_t ds3231_set(RtcTiempo *t);               /* ajusta t->diaSemana */
uint8_t ds3231_lostPower(void);                 /* 1: se detuvo (sin pila): hay que ajustarlo */
int     ds3231_temperature(void);               /* centesimas de grado */
uint8_t ds3231_setSQW(uint8_t modo);
uint8_t ds3231_dayOfWeek(unsigned int anio, uint8_t mes, uint8_t dia);   /* 1 = lunes */
/* fecha y hora de la compilacion (__DATE__ y __TIME__) */
void    ds3231_compileTime(RtcTiempo *t, const char *fecha, const char *hora);

#endif
