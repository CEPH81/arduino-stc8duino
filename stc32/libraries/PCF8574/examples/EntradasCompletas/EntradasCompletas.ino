/*
 * EntradasCompletas - las 8 patas del PCF8574 como entradas; cada cambio se
 * muestra por el monitor serie (115200) y el LED de la placa indica si hay
 * alguna apretada.
 *
 * Conexiones: SCL P1.5, SDA P1.4 (ver Wire.setPins), VCC y GND al modulo.
 * Pulsadores entre cada pata P0-P7 y GND (sueltos se leen 1, apretados 0).
 * Direccion: 0x20 con A0-A2 a GND (PCF8574A: 0x38).
 */
#include <Wire.h>
#include <PCF8574.h>

#define EXPANSOR 0x20

int anterior = -2;

void setup() {
  Serial.begin(115200);
  Serial.println("stc32duino: PCF8574 EntradasCompletas");
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin();
  pcf8574_begin(EXPANSOR);      /* todas como entrada */
}

void loop() {
  int v = pcf8574_read(EXPANSOR);
  uint8_t i;

  if (v != anterior) {
    anterior = v;
    if (v < 0) {
      Serial.println("el PCF8574 no responde");
    } else {
      Serial.print("P7..P0 = ");
      for (i = 0; i < 8; i++) Serial.print((v >> (7 - i)) & 1 ? "1" : "0");
      Serial.print("   apretadas:");
      for (i = 0; i < 8; i++) {
        if (!((v >> i) & 1)) {
          Serial.print(" P");
          Serial.print(i);
        }
      }
      Serial.println();
      digitalWrite(LED_BUILTIN, v != 0xFF);
    }
  }
  delay(20);
}
