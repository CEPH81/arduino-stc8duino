/*
 * W25Q.c - memorias flash SPI W25Qxx (ver W25Q.h).
 * C portable: compila igual con SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include <SPI.h>
#include "W25Q.h"

#define O_WREN     0x06
#define O_RDSR1    0x05
#define O_READ     0x03
#define O_PP       0x02
#define O_SE       0x20
#define O_BE64     0xD8
#define O_CE       0xC7
#define O_JEDEC    0x9F
#define O_PD       0xB9
#define O_RPD      0xAB

static uint8_t pin_cs;

static void sel(void)
{
	SPI_beginTransaction(8000000UL, MSBFIRST, SPI_MODE0);
	digitalWrite(pin_cs, LOW);
}

static void desel(void)
{
	digitalWrite(pin_cs, HIGH);
}

static void orden_dir(uint8_t o, unsigned long dir)
{
	SPI_transfer(o);
	SPI_transfer((uint8_t)(dir >> 16));
	SPI_transfer((uint8_t)(dir >> 8));
	SPI_transfer((uint8_t)dir);
}

uint8_t w25q_busy(void)
{
	uint8_t s;
	sel();
	SPI_transfer(O_RDSR1);
	s = SPI_transfer(0xFF);
	desel();
	return s & 1;
}

static void esperar(void)
{
	while (w25q_busy())
		;
}

static void habilitar(void)
{
	sel();
	SPI_transfer(O_WREN);
	desel();
}

void w25q_wakeUp(void)
{
	sel();
	SPI_transfer(O_RPD);
	desel();
	delayMicroseconds(5);
}

void w25q_powerDown(void)
{
	sel();
	SPI_transfer(O_PD);
	desel();
}

unsigned long w25q_jedecID(void)
{
	unsigned long id;
	sel();
	SPI_transfer(O_JEDEC);
	id = (unsigned long)SPI_transfer(0xFF) << 16;
	id |= (unsigned long)SPI_transfer(0xFF) << 8;
	id |= SPI_transfer(0xFF);
	desel();
	return id;
}

uint8_t w25q_begin(uint8_t cs)
{
	unsigned long id;
	pin_cs = cs;
	pinMode(cs, OUTPUT);
	digitalWrite(cs, HIGH);
	w25q_wakeUp();
	id = w25q_jedecID();
	return (id == 0 || id == 0xFFFFFFUL) ? 1 : 0;
}

unsigned long w25q_size(void)
{
	uint8_t c = (uint8_t)w25q_jedecID();        /* 0x14 = 1 MB ... 0x18 = 16 MB */
	if (c < 0x10 || c > 0x18) return 0;
	return 1UL << c;
}

void w25q_read(unsigned long dir, uint8_t *p, unsigned int n)
{
	sel();
	orden_dir(O_READ, dir);
	while (n--) *p++ = SPI_transfer(0xFF);
	desel();
}

void w25q_write(unsigned long dir, const uint8_t *p, unsigned int n)
{
	unsigned int k;
	while (n) {
		k = 256 - (unsigned int)(dir & 0xFF);   /* hasta el final de la pagina */
		if (k > n) k = n;
		habilitar();
		sel();
		orden_dir(O_PP, dir);
		dir += k;
		n -= k;
		while (k--) SPI_transfer(*p++);
		desel();
		esperar();                           /* hasta 3 ms por pagina */
	}
}

static void borrar(uint8_t o, unsigned long dir)
{
	habilitar();
	sel();
	orden_dir(o, dir);
	desel();
	esperar();
}

void w25q_eraseSector(unsigned long dir) { borrar(O_SE, dir); }
void w25q_eraseBlock(unsigned long dir)  { borrar(O_BE64, dir); }

void w25q_eraseChip(void)
{
	habilitar();
	sel();
	SPI_transfer(O_CE);
	desel();
	esperar();
}
