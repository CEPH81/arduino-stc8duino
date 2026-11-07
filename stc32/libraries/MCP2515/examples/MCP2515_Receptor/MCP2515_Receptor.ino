/*
 * MCP2515_Receptor - muestra todas las tramas del bus CAN por el monitor serie
 * (115200 baudios): identificador, largo y datos en hexadecimal. Si la trama
 * es la 0x100 de MCP2515_Emisor, muestra tambien la cuenta y A0.
 *
 * Conexiones: CS -> P2.2, SO -> P1.4, SI -> P1.3, SCK -> P1.5, VCC 5 V, GND.
 */
#include <SPI.h>
#include <MCP2515.h>

#define PIN_CS     P2_2
#define CRISTAL    MCP_8MHZ
#define VELOCIDAD  CAN_500KBPS

void setup() {
  Serial.begin(115200);
  Serial.println("MCP2515_Receptor");
  SPI.begin();
  if (mcp2515_begin(PIN_CS, VELOCIDAD, CRISTAL) != MCP2515_OK) {
    Serial.println("el MCP2515 no responde: revise las conexiones");
  }
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  unsigned long id;
  uint8_t ext, largo, i;
  uint8_t datos[8];

  if (mcp2515_available()) {
    mcp2515_read(&id, &ext, &largo, datos);
    Serial.print("id 0x");
    Serial.print(id, HEX);
    Serial.print(ext ? " (ext) [" : " [");
    Serial.print(largo);
    Serial.print("]");
    for (i = 0; i < largo; i++) {
      Serial.print(" ");
      if (datos[i] < 16) Serial.print("0");
      Serial.print(datos[i], HEX);
    }
    if (id == 0x100 && !ext && largo == 4) {
      Serial.print("   cuenta ");
      Serial.print(((unsigned int)datos[0] << 8) | datos[1]);
      Serial.print("  A0 ");
      Serial.print(((unsigned int)datos[2] << 8) | datos[3]);
    }
    Serial.println();
    digitalToggle(LED_BUILTIN);
  }
}
