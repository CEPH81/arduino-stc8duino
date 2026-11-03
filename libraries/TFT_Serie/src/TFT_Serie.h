/*
 * TFT_Serie.h - pantallas TFT que se manejan con comandos de texto por el
 * puerto serie (modulos JC018-QQVGA-ST-02-V02 de 1.8", JC22-V05 de 2.2" y
 * los "UART LCD" del mismo fabricante).
 *
 * Medidas: por omision 1.8" de 128x160 (JC018-QQVGA). Para la de 2.2"
 * escriba antes del #include:   #define TFT_JC22
 * y TFT_ANCHO / TFT_ALTO tendran 176x220 (en vertical, DIR(0)).
 *
 * Usa la UART2 del chip: TX2 = P1.1 (va al RX de la pantalla)
 *                        RX2 = P1.0 (va al TX de la pantalla)
 * asi el Serial (P3.0/P3.1) queda libre para cargar programas y para el monitor.
 * OJO: la UART2 y el Serial comparten el temporizador 2, deben usar la MISMA
 * velocidad (115200 en los ejemplos, la de fabrica de la pantalla).
 *
 *   tft_begin(115200);                 // 0 si la pantalla contesta "OK"
 *   tft_clear(TFT_NEGRO);
 *   tft_texto(16, 0, 0, "Hola", TFT_BLANCO);
 *   tft_textoInicio(32, 10, 40);       // texto armado con tft_print:
 *   tft_print(millis() / 1000);        //   numeros, variables...
 *   tft_textoFin(TFT_AMARILLO);
 *   tft_rectLleno(0, 100, 50, 120, TFT_ROJO);
 *
 * Coordenadas en puntos desde la esquina de arriba a la izquierda. Letras de
 * 16, 24 o 32 puntos de alto (8, 12 o 16 de ancho). Colores 0 a 15.
 * Cada comando espera el "OK" de la pantalla; si la pata RX2 no esta conectada,
 * espera una pausa fija (TFT_PAUSA_MS) y sigue.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef TFT_Serie_h
#define TFT_Serie_h

#include <Arduino.h>

#if defined(TFT_JC22)
#define TFT_ANCHO   176
#define TFT_ALTO    220
#define TFT_MODELO  "JC22 2.2in"
#else                            /* JC018-QQVGA-ST-02-V02, 1.8" */
#define TFT_ANCHO   128
#define TFT_ALTO    160
#define TFT_MODELO  "JC018 1.8in"
#endif

#define TFT_NEGRO        0
#define TFT_ROJO         1
#define TFT_VERDE        2
#define TFT_AZUL         3
#define TFT_AMARILLO     4
#define TFT_CIAN         5
#define TFT_MORADO       6
#define TFT_GRIS         7
#define TFT_GRIS_CLARO   8
#define TFT_MARRON       9
#define TFT_VERDE_OSCURO 10
#define TFT_AZUL_OSCURO  11
#define TFT_OCRE         12
#define TFT_NARANJA      13
#define TFT_ROSA         14
#define TFT_BLANCO       15

#ifndef TFT_PAUSA_MS
#define TFT_PAUSA_MS 30          /* sin RX2: pausa despues de cada comando */
#endif

uint8_t tft_begin(unsigned long baud);      /* 0 si la pantalla responde */
uint8_t tft_responde(void);                 /* 1 si contesta "OK" */
size_t  tft_write(uint8_t c);               /* un byte tal cual */
int     tft_read(void);                     /* byte recibido o -1 */
uint8_t tft_enviar(const char *cmd);        /* comando completo, p.ej. "BL(100);" */
uint8_t tft_fin(void);                      /* termina la linea y espera el OK (0 = OK) */

void tft_clear(uint8_t color);
void tft_direccion(uint8_t horizontal);     /* 0 vertical, 1 horizontal */
void tft_brillo(uint8_t b);                 /* BL(b): ver el manual de su modulo */
void tft_fondo(uint8_t color);              /* color de fondo de los textos */
void tft_punto(int x, int y, uint8_t color);
void tft_linea(int x1, int y1, int x2, int y2, uint8_t color);
void tft_rect(int x1, int y1, int x2, int y2, uint8_t color);
void tft_rectLleno(int x1, int y1, int x2, int y2, uint8_t color);
void tft_circulo(int x, int y, int r, uint8_t color);
void tft_circuloLleno(int x, int y, int r, uint8_t color);
void tft_texto(uint8_t tam, int x, int y, const char *s, uint8_t color);
void tft_textoInicio(uint8_t tam, int x, int y);   /* luego tft_print(...) */
void tft_textoFin(uint8_t color);

#ifdef STC_SKETCH
#define tft_print(...)  S8_PRINT(tft_write, __VA_ARGS__)
#endif

#endif
