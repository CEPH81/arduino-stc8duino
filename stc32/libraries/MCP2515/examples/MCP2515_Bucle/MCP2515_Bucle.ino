/*
 * MCP2515_Bucle - prueba el modulo MCP2515 sin bus CAN: en modo bucle
 * (loopback) el MCP2515 recibe sus propias tramas. Envia una trama estandar y
 * una extendida cada 2 s y comprueba que vuelvan iguales (monitor a 115200).
 *
 * Conexiones: CS -> P2.2, SO -> P1.4 (MISO), SI -> P1.3 (MOSI), SCK -> P1.5,
 *   VCC 5 V y GND. CRISTAL: mire el del modulo (8.000 = MCP_8MHZ).
 */
#include <SPI.h>
#include <MCP2515.h>

#define PIN_CS  P2_2
#define CRISTAL MCP_8MHZ

uint8_t cuenta = 0;

void probar(unsigned long id, uint8_t ext) {
  uint8_t datos[8], recibido[8];
  unsigned long id_r;
  uint8_t ext_r, largo_r, i, iguales;
  unsigned long t0;

  for (i = 0; i < 8; i++) datos[i] = cuenta + i;
  if (mcp2515_send(id, ext, 8, datos) != MCP2515_OK) {
    Serial.println("  no se pudo enviar");
    return;
  }
  t0 = millis();
  while (!mcp2515_available()) {
    if (millis() - t0 > 100) {
      Serial.println("  no volvio la trama");
      return;
    }
  }
  mcp2515_read(&id_r, &ext_r, &largo_r, recibido);
  iguales = (id_r == id && ext_r == ext && largo_r == 8);
  for (i = 0; i < 8; i++) if (recibido[i] != datos[i]) iguales = 0;
  Serial.print(ext ? "  extendida 0x" : "  estandar  0x");
  Serial.print(id_r, HEX);
  Serial.print(" largo ");
  Serial.print(largo_r);
  Serial.println(iguales ? "  correcta" : "  DISTINTA");
}

void setup() {
  Serial.begin(115200);
  Serial.println("MCP2515_Bucle");
  SPI.begin();
  if (mcp2515_begin(PIN_CS, CAN_500KBPS, CRISTAL) != MCP2515_OK) {
    Serial.println("el MCP2515 no responde: revise CS, SO, SI, SCK y VCC");
  }
  mcp2515_setLoopbackMode();
}

void loop() {
  cuenta++;
  Serial.print("vuelta ");
  Serial.println(cuenta);
  probar(0x123, 0);
  probar(0x18DAF110UL, 1);
  delay(2000);
}
