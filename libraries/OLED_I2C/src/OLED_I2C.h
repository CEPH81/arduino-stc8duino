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
 * Parte de stc8duino. LGPL 2.1.
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

#ifdef STC_SKETCH
#define oled_print(...)  S8_PRINT(oled_write, __VA_ARGS__)
#endif

#endif
