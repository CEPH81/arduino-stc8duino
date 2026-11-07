/*
 * JDY40.c - JDY-40 por Serial3 (ver JDY40.h).
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include "JDY40.h"


static uint8_t pin_set_, pin_cs_ = JDY40_SIN_PATA;

static void enviar_texto(const char *s)
{
	while (*s) Serial3_write((uint8_t)*s++);
}

uint8_t jdy40_begin(uint8_t pin_set, uint8_t pin_cs, unsigned long baud)
{
	pin_set_ = pin_set;
	pin_cs_ = pin_cs;
	pinMode(pin_set_, OUTPUT);
	digitalWrite(pin_set_, HIGH);              /* transmision transparente */
	if (pin_cs_ != JDY40_SIN_PATA) {
		pinMode(pin_cs_, OUTPUT);
		digitalWrite(pin_cs_, LOW);            /* despierto */
	}
	Serial3_begin(baud);
	delay(100);
	return 0;
}

void jdy40_sleep(void)
{
	if (pin_cs_ != JDY40_SIN_PATA) digitalWrite(pin_cs_, HIGH);
}

void jdy40_wake(void)
{
	if (pin_cs_ != JDY40_SIN_PATA) digitalWrite(pin_cs_, LOW);
	delay(5);
}

uint8_t jdy40_command(const char *orden, char *respuesta, uint8_t max)
{
	uint8_t n = 0;
	int c;
	unsigned long t0;

	digitalWrite(pin_set_, LOW);              /* modo de comandos */
	delay(60);
	while (Serial3_available()) Serial3_read();
	enviar_texto(orden);
	enviar_texto("\r\n");
	t0 = millis();
	while (millis() - t0 < 300) {
		c = Serial3_read();
		if (c < 0) continue;
		if (c == '\r' || c == '\n') {
			if (n > 0) break;                  /* fin de la primera linea */
			continue;
		}
		if (respuesta && n + 1 < max) respuesta[n] = (char)c;
		n++;
	}
	if (respuesta && max) respuesta[n < max ? n : max - 1] = 0;
	digitalWrite(pin_set_, HIGH);
	delay(60);
	return n;
}

/* "AT+XXXX" + numero en decimal (cifras) o hexadecimal (4 cifras) */
static uint8_t poner(const char *orden, unsigned int v, uint8_t cifras, uint8_t hex)
{
	char buf[16], r[12];
	uint8_t i = 0, k;
	while (orden[i]) {
		buf[i] = orden[i];
		i++;
	}
	for (k = cifras; k > 0; k--) {
		uint8_t d = hex ? (uint8_t)(v & 15) : (uint8_t)(v % 10);
		buf[i + k - 1] = d < 10 ? '0' + d : 'A' + d - 10;
		v = hex ? (v >> 4) : (v / 10);
	}
	buf[i + cifras] = 0;
	jdy40_command(buf, r, sizeof(r));
	return (r[0] == 'O' && r[1] == 'K') || (r[0] == '+' && r[1] == 'O') ? 0 : 1;
}

uint8_t jdy40_setChannel(uint8_t canal) { return poner("AT+RFC", canal, 3, 0); }
uint8_t jdy40_setRFID(unsigned int id)  { return poner("AT+RFID", id, 4, 1); }
uint8_t jdy40_setDVID(unsigned int id)  { return poner("AT+DVID", id, 4, 1); }
uint8_t jdy40_setPower(uint8_t nivel)   { return poner("AT+POWE", nivel > 9 ? 9 : nivel, 1, 0); }
