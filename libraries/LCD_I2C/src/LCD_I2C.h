/*
 * LCD_I2C.h - pantalla LCD de caracteres (HD44780) con adaptador I2C PCF8574.
 *
 *   Wire.begin();
 *   lcd_begin(0x27, 16, 2);       // direccion (0x27 PCF8574, 0x3F PCF8574A), columnas, filas
 *   lcd_print("Hola");            // textos y numeros, como Serial.print
 *   lcd_print(x, HEX);  lcd_print(3.14, 2);
 *   lcd_setCursor(0, 1);          // columna, fila (desde 0)
 *   lcd_clear();  lcd_home();
 *   lcd_backlight();  lcd_noBacklight();
 *   lcd_cursor();  lcd_noCursor();  lcd_blink();  lcd_noBlink();
 *   lcd_display();  lcd_noDisplay();
 *   lcd_createChar(0, figura);  lcd_write(0);   // 8 caracteres propios (0-7)
 *
 * Adaptador habitual: P0 RS, P1 RW, P2 E, P3 luz de fondo, P4-P7 D4-D7.
 * Cada caracter va en una sola transmision I2C (6 bytes): unos 0,6 ms a 100 kHz.
 * Si no se ve nada, ajuste el contraste con el potenciometro del adaptador.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef LCD_I2C_h
#define LCD_I2C_h

#include <Arduino.h>
#include <Wire.h>

uint8_t lcd_begin(uint8_t addr, uint8_t cols, uint8_t rows);   /* 0 si responde */
void    lcd_clear(void);
void    lcd_home(void);
void    lcd_setCursor(uint8_t col, uint8_t row);
void    lcd_backlight(void);
void    lcd_noBacklight(void);
void    lcd_display(void);
void    lcd_noDisplay(void);
void    lcd_cursor(void);
void    lcd_noCursor(void);
void    lcd_blink(void);
void    lcd_noBlink(void);
void    lcd_scrollDisplayLeft(void);
void    lcd_scrollDisplayRight(void);
void    lcd_createChar(uint8_t location, const uint8_t *charmap);
void    lcd_command(uint8_t value);
size_t  lcd_write(uint8_t value);

/* lcd_print(x) / lcd_print(x, HEX) / lcd_print(x, 2): igual que Serial.print */
#ifdef STC_SKETCH
#define lcd_print(...)   S8_PRINT(lcd_write, __VA_ARGS__)
#endif

#endif
