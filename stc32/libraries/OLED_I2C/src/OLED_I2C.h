/*
 * OLED_I2C.h - pantalla OLED 128x64 por I2C (SSD1306 0,96" o SH1106 1,3") en modo texto.
 *
 *   Wire.begin();
 *   oled_begin(0x3C, OLED_SSD1306);   // o OLED_SH1106 si la imagen sale corrida 2 puntos
 *   oled_clear();
 *   oled_setCursor(0, 0);             // x en puntos (0-127), fila de texto (0-7)
 *   oled_print("Hola");               // textos y numeros, como Serial.print
 *   oled_setTextSize(2);              // letras dobles (12x16, ocupan 2 filas)
 *   oled_setInvert(1);                // texto en negativo (para resaltar en menus)
 *   oled_clearLine(3);                // borra una fila de texto
 *
 * Caracteres de 5x8 en celdas de 6x8: 21 columnas x 8 filas.
 * No usa memoria de pantalla: escribe directamente en el OLED.
 *
 * Parte de stc8duino (version stc32duino). LGPL 2.1.
 */
#ifndef OLED_I2C_h
#define OLED_I2C_h

#include <Arduino.h>
#include <Wire.h>

#define OLED_SSD1306 0
#define OLED_SH1106  1

uint8_t oled_begin(uint8_t addr, uint8_t tipo);   /* 0 si responde */
void    oled_clear(void);
void    oled_clearLine(uint8_t fila);
void    oled_setCursor(uint8_t x, uint8_t fila);
void    oled_setTextSize(uint8_t s);              /* 1 o 2 */
void    oled_setInvert(uint8_t inv);              /* 1 = texto en negativo */
void    oled_setContrast(uint8_t c);
void    oled_display(uint8_t on);                 /* 0 apaga la pantalla */
size_t  oled_write(uint8_t c);

size_t oled_writeStr(const char *s);

/* oled_print(x) / oled_print(x, HEX) / oled_print(x, 2): igual que Serial.print.
 * stcar -obj cambia oled_print(...) por oled_print_N(...) segun el dato. */
#define oled_print_S(s)      oled_writeStr(s)
#define oled_print_C(c)      oled_write((uint8_t)(c))
#define oled_print_1(x)      oled_writeStr(STC_FMT(x, 10, 2))
#define oled_print_2(x, b)   oled_writeStr(STC_FMT(x, b, b))
#define oled_print_L(x)      oled_writeStr(stc_fmt_long((long)(x), 10))
#define oled_print_L2(x, b)  oled_writeStr(stc_fmt_long((long)(x), (b)))
#define oled_print_U(x)      oled_writeStr(stc_fmt_ulong((unsigned long)(x), 10))
#define oled_print_U2(x, b)  oled_writeStr(stc_fmt_ulong((unsigned long)(x), (b)))
#define oled_print_F(x)      oled_writeStr(stc_fmt_float((float)(x), 2))
#define oled_print_F2(x, d)  oled_writeStr(stc_fmt_float((float)(x), (d)))

#endif
