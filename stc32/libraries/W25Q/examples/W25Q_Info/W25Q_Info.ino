/*
 * W25Q_Info - identifica la memoria flash W25Qxx (fabricante, modelo y
 * tamano) y muestra sus primeros 64 bytes en hexadecimal.
 *
 * Conexiones: VCC 3,3 V, GND, CS -> P2.2, DO -> P1.4, DI -> P1.3,
 *   CLK -> P1.5, WP y HOLD a VCC.
 */
#include <SPI.h>
#include <W25Q.h>

#define PIN_CS P2_2

void setup() {
  unsigned long id;
  uint8_t b[16];
  uint8_t f, i;
  Serial.begin(115200);
  Serial.println("W25Q_Info");
  SPI.begin();
  if (w25q_begin(PIN_CS) != 0) {
    Serial.println("la memoria no responde: revise VCC (3,3 V), CS, DO, DI y CLK");
    return;
  }
  id = w25q_jedecID();
  Serial.print("JEDEC ID: 0x");
  Serial.println(id, HEX);
  Serial.print("fabricante: ");
  Serial.println((id >> 16) == 0xEF ? "Winbond" : "otro (compatible)");
  Serial.print("tamano: ");
  Serial.print(w25q_size() / 1024);
  Serial.println(" KB");
  for (f = 0; f < 4; f++) {
    w25q_read(f * 16, b, 16);
    for (i = 0; i < 16; i++) {
      if (b[i] < 16) Serial.print("0");
      Serial.print(b[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void loop() {
}
