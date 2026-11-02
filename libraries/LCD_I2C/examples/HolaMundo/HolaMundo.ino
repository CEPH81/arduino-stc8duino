/*
 * HolaMundo - pantalla LCD 16x2 con adaptador I2C (PCF8574): texto en la primera
 * linea y en la segunda los segundos encendido y la lectura de A0.
 *
 * Conexiones: SCL P1.5, SDA P1.4 (ver Wire.setPins), VCC (5 V) y GND al adaptador.
 * Direccion: 0x27 (PCF8574) o 0x3F (PCF8574A): use EscanerI2C si no la sabe.
 * Si no se ve texto, gire el potenciometro de contraste del adaptador.
 */
#include <Wire.h>
#include <LCD_I2C.h>

#define LCD_DIR 0x27

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if (lcd_begin(LCD_DIR, 16, 2) != 0) {
    Serial.println("la pantalla no responde: revise la direccion (0x27 o 0x3F)");
  }
  lcd_backlight();
  lcd_print("Hola stc8duino!");
}

void loop() {
  lcd_setCursor(0, 1);
  lcd_print("t=");
  lcd_print(millis() / 1000);
  lcd_print("s A0=");
  lcd_print(analogRead(A0));
  lcd_print("   ");            /* borra restos de numeros mas largos */
  delay(250);
}
