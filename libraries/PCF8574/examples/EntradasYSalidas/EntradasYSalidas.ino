/*
 * EntradasYSalidas - PCF8574 mixto: P0-P3 entradas (pulsadores) y P4-P7 salidas
 * (LEDs). Cada pulsador enciende su LED: P0 -> P4, P1 -> P5, P2 -> P6, P3 -> P7,
 * y cada cambio se muestra por el monitor serie (115200 baudios).
 *
 * Conexiones: SCL P1.5, SDA P1.4 (ver Wire.setPins), VCC y GND al modulo.
 * Pulsadores de P0-P3 a GND; LEDs de VCC (con resistencia de 330 a 1 k) a P4-P7.
 * Direccion: 0x20 con A0-A2 a GND (PCF8574A: 0x38).
 */
#include <Wire.h>
#include <PCF8574.h>

#define EXPANSOR 0x20

int anterior = -2;

void setup() {
  uint8_t i;
  Serial.begin(115200);
  Serial.println("stc8duino: PCF8574 EntradasYSalidas");
  Wire.begin();
  if (pcf8574_begin(EXPANSOR) != 0) {
    Serial.println("el PCF8574 no responde: revise direccion y conexiones");
  }
  for (i = 0; i < 4; i++) pcf8574_pinMode(EXPANSOR, i, INPUT);    /* P0-P3 */
  for (i = 4; i < 8; i++) pcf8574_pinMode(EXPANSOR, i, OUTPUT);   /* P4-P7 */
}

void loop() {
  int v = pcf8574_read(EXPANSOR);
  uint8_t entradas;

  if (v < 0) {
    if (anterior != -1) Serial.println("el PCF8574 no responde");
    anterior = -1;
  } else {
    entradas = v & 0x0F;
    if (entradas != anterior) {
      anterior = entradas;
      /* pulsador apretado (0) -> su LED encendido (0): las 4 salidas de una vez;
       * las patas de entrada quedan en 1 aunque aqui se escriba 0 */
      pcf8574_write(EXPANSOR, entradas << 4);
      Serial.print("entradas P3..P0 = ");
      Serial.print(entradas & 8 ? "1" : "0");
      Serial.print(entradas & 4 ? "1" : "0");
      Serial.print(entradas & 2 ? "1" : "0");
      Serial.println(entradas & 1 ? "1" : "0");
    }
  }
  delay(20);
}
