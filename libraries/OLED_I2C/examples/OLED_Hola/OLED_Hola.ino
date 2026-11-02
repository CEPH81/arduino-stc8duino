/*
 * OLED_Hola - texto normal, doble e invertido en una pantalla OLED 128x64 por I2C,
 * con un contador de segundos.
 *
 * Conexiones: SDA P1.4, SCL P1.5 (ver Wire.setPins), VCC y GND.
 * Direccion habitual 0x3C (algunos modulos 0x3D). Si la imagen sale corrida o con
 * una columna de basura a la derecha, la pantalla es SH1106: cambie TIPO.
 */
#include <Wire.h>
#include <OLED_I2C.h>

#define OLED_DIR 0x3C
#define TIPO     OLED_SSD1306      /* o OLED_SH1106 */

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if (oled_begin(OLED_DIR, TIPO) != 0) {
    Serial.println("la pantalla no responde: revise la direccion (0x3C o 0x3D)");
  }
  oled_setInvert(1);
  oled_setCursor(0, 0);
  oled_print("   stc8duino OLED    ");
  oled_setInvert(0);
  oled_setCursor(0, 2);
  oled_print("Texto normal 5x8");
  oled_setCursor(0, 7);
  oled_print("SSD1306 / SH1106");
}

void loop() {
  oled_setTextSize(2);
  oled_setCursor(0, 4);
  oled_print(millis() / 1000);
  oled_print(" s  ");
  oled_setTextSize(1);
  delay(200);
}
