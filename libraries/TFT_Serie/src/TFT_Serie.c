/*
 * TFT_Serie.c - comandos de texto para pantallas TFT serie (ver TFT_Serie.h).
 * UART2 por consulta (sin interrupcion), velocidad con el temporizador 2.
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include "wiring_private.h"
#include "TFT_Serie.h"

#define S2TI 0x02
#define S2RI 0x01

static __bit con_ok;               /* la pantalla contesta "OK" */

size_t tft_write(uint8_t c)
{
	S2BUF = c;
	while (!(S2CON & S2TI)) ;
	S2CON &= ~S2TI;
	return 1;
}

int tft_read(void)
{
	uint8_t c;
	if (!(S2CON & S2RI)) return -1;
	c = S2BUF;
	S2CON &= ~S2RI;
	return c;
}

static void vaciar(void)
{
	while (tft_read() >= 0) ;
}

/* espera "OK" hasta 'ms' milisegundos; 0 = llego */
static uint8_t esperar_ok(unsigned int ms)
{
	unsigned long t0 = millis();
	uint8_t antes = 0;
	int c;
	while ((unsigned long)(millis() - t0) < ms) {
		c = tft_read();
		if (c < 0) continue;
		if (antes == 'O' && c == 'K') return 0;
		antes = c;
	}
	return 1;
}

static void cadena(const char *s)
{
	while (*s) tft_write(*s++);
}

static void numero(int v)
{
	char b[6];
	uint8_t n = 0;
	unsigned int u;
	if (v < 0) { tft_write('-'); u = -v; } else u = v;
	do { b[n++] = '0' + u % 10; u /= 10; } while (u);
	while (n) tft_write(b[--n]);
}

uint8_t tft_fin(void)
{
	tft_write('\r');
	tft_write('\n');
	if (con_ok) return esperar_ok(1000);
	delay(TFT_PAUSA_MS);
	return 0;
}

uint8_t tft_begin(unsigned long baud)
{
	unsigned int reload = (unsigned int)(65536UL - (STC_FREQ() / 4 + baud / 2) / baud);

	pinMode(P1_1, OUTPUT);                  /* TX2 */
	pinMode(P1_0, INPUT_PULLUP);            /* RX2 */
	P_SW2 &= ~0x01;                         /* UART2 en P1.0/P1.1 */
	IE2 &= ~0x01;                           /* sin interrupcion */
	S2CON = 0x10;                           /* 8 bits, recepcion activa */
	AUXR &= ~AUXR_T2R;                      /* temporizador 2 (el mismo del Serial) */
	T2L = reload & 0xFF;
	T2H = reload >> 8;
	AUXR |= AUXR_T2x12 | AUXR_T2R;

	/* la pantalla tarda en arrancar: probar hasta 2 s */
	con_ok = 1;
	{
		uint8_t i;
		for (i = 0; i < 8; i++) {
			vaciar();
			cadena("DIR(0);\r\n");
			if (esperar_ok(250) == 0) return 0;
		}
	}
	con_ok = 0;
	return 1;
}

uint8_t tft_responde(void)
{
	return con_ok;
}

uint8_t tft_enviar(const char *cmd)
{
	vaciar();
	cadena(cmd);
	return tft_fin();
}

/* NOMBRE(a,b,...,color); */
static void cmd(const char *nombre, int *v, uint8_t n)
{
	uint8_t i;
	vaciar();
	cadena(nombre);
	tft_write('(');
	for (i = 0; i < n; i++) {
		numero(v[i]);
		tft_write(i + 1 < n ? ',' : ')');
	}
	tft_write(';');
	tft_fin();
}

void tft_clear(uint8_t color)        { int v[1]; v[0] = color; cmd("CLR", v, 1); }
void tft_direccion(uint8_t h)        { int v[1]; v[0] = h; cmd("DIR", v, 1); }
void tft_brillo(uint8_t b)           { int v[1]; v[0] = b; cmd("BL", v, 1); }
void tft_fondo(uint8_t color)        { int v[1]; v[0] = color; cmd("SBC", v, 1); }

void tft_punto(int x, int y, uint8_t color)
{
	int v[3]; v[0] = x; v[1] = y; v[2] = color;
	cmd("PS", v, 3);
}

static void cuatro(const char *n, int a, int b, int c, int d, uint8_t color)
{
	int v[5]; v[0] = a; v[1] = b; v[2] = c; v[3] = d; v[4] = color;
	cmd(n, v, 5);
}

void tft_linea(int x1, int y1, int x2, int y2, uint8_t c)     { cuatro("PL", x1, y1, x2, y2, c); }
void tft_rect(int x1, int y1, int x2, int y2, uint8_t c)      { cuatro("BOX", x1, y1, x2, y2, c); }
void tft_rectLleno(int x1, int y1, int x2, int y2, uint8_t c) { cuatro("BOXF", x1, y1, x2, y2, c); }

void tft_circulo(int x, int y, int r, uint8_t color)
{
	int v[4]; v[0] = x; v[1] = y; v[2] = r; v[3] = color;
	cmd("CIR", v, 4);
}

void tft_circuloLleno(int x, int y, int r, uint8_t color)
{
	int v[4]; v[0] = x; v[1] = y; v[2] = r; v[3] = color;
	cmd("CIRF", v, 4);
}

void tft_textoInicio(uint8_t tam, int x, int y)
{
	vaciar();
	cadena("DCV");
	numero(tam);
	tft_write('(');
	numero(x);
	tft_write(',');
	numero(y);
	tft_write(',');
	tft_write('\'');
}

void tft_textoFin(uint8_t color)
{
	tft_write('\'');
	tft_write(',');
	numero(color);
	tft_write(')');
	tft_write(';');
	tft_fin();
}

void tft_texto(uint8_t tam, int x, int y, const char *s, uint8_t color)
{
	tft_textoInicio(tam, x, y);
	cadena(s);
	tft_textoFin(color);
}
