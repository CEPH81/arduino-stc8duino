/*
 * SalidasCompletas - las 8 patas del PCF8574 como salidas: luz que corre.
 *
 * Conexiones: SCL P1.5, SDA P1.4 (ver Wire.setPins), VCC y GND al modulo.
 * LEDs: de VCC, con resistencia de 330 a 1 k, al catodo en cada pata P0-P7
 * (el PCF8574 enciende un LED poniendo la pata a GND: 0 = encendido).
 * Direccion: 0x20 con A0-A2 a GND (PCF8574A: 0x38). Use EscanerI2C si no la sabe.
 */
#include <Wire.h>
#include <PCF8574.h>

#define EXPANSOR 0x20

void setup() {
  uint8_t i;
  Serial.begin(115200);
  Serial.println("stc8duino: PCF8574 SalidasCompletas");
  Wire.begin();
  if (pcf8574_begin(EXPANSOR) != 0) {
    Serial.println("el PCF8574 no responde: revise direccion y conexiones");
  }
  for (i = 0; i < 8; i++) pcf8574_pinMode(EXPANSOR, i, OUTPUT);
}

void loop() {
  uint8_t i;
  /* un LED encendido (bit en 0) que recorre P0 a P7 */
  for (i = 0; i < 8; i++) {
    pcf8574_write(EXPANSOR, ~(1 << i));
    delay(150);
  }
  /* las 8 encendidas, luego apagadas */
  pcf8574_write(EXPANSOR, 0x00);
  delay(500);
  pcf8574_write(EXPANSOR, 0xFF);
  delay(500);
}
