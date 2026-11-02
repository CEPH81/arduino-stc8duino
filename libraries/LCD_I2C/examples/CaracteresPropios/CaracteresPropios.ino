/*
 * CaracteresPropios - dibuja caracteres propios en la LCD I2C (corazon, campana,
 * sonrisa, flecha) y una barra que se llena; el pulsador (P3.3) apaga y enciende
 * la luz de fondo.
 *
 * Conexiones: SCL P1.5, SDA P1.4 (ver Wire.setPins), VCC y GND al adaptador.
 * Direccion: 0x27 (PCF8574) o 0x3F (PCF8574A).
 */
#include <Wire.h>
#include <LCD_I2C.h>

#define LCD_DIR 0x27
#ifdef BOTON_BUILTIN
#define BOTON BOTON_BUILTIN
#else
#define BOTON P3_3
#endif

const uint8_t corazon[8] = { 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00, 0x00 };
const uint8_t campana[8] = { 0x04, 0x0E, 0x0E, 0x0E, 0x1F, 0x00, 0x04, 0x00 };
const uint8_t sonrisa[8] = { 0x00, 0x0A, 0x0A, 0x00, 0x11, 0x0E, 0x00, 0x00 };
const uint8_t flecha[8]  = { 0x00, 0x04, 0x02, 0x1F, 0x02, 0x04, 0x00, 0x00 };
const uint8_t lleno[8]   = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F };

uint8_t luz = 1, boton_antes = 1, barra = 0;

void setup() {
  pinMode(BOTON, INPUT_PULLUP);
  Wire.begin();
  lcd_begin(LCD_DIR, 16, 2);
  lcd_backlight();
  lcd_createChar(0, corazon);
  lcd_createChar(1, campana);
  lcd_createChar(2, sonrisa);
  lcd_createChar(3, flecha);
  lcd_createChar(4, lleno);
  lcd_setCursor(0, 0);
  lcd_print("STC8 ");
  lcd_write(0);
  lcd_write(1);
  lcd_write(2);
  lcd_write(3);
}

void loop() {
  uint8_t b = digitalRead(BOTON);
  if (b == LOW && boton_antes == HIGH) {     /* al apretar */
    luz = !luz;
    if (luz) lcd_backlight(); else lcd_noBacklight();
  }
  boton_antes = b;

  /* barra que se llena en la segunda linea y vuelve a empezar */
  lcd_setCursor(barra, 1);
  lcd_write(4);
  if (++barra >= 16) {
    barra = 0;
    lcd_setCursor(0, 1);
    lcd_print("                ");
  }
  delay(150);
}
