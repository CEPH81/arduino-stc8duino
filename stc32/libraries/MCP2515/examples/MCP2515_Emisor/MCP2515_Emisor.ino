/*
 * MCP2515_Emisor - envia por el bus CAN, cada segundo, una trama 0x100 con
 * una cuenta y la lectura de A0. Usar con MCP2515_Receptor en otra placa
 * (o con cualquier otro nodo CAN a la misma velocidad).
 *
 * Conexiones: CS -> P2.2, SO -> P1.4, SI -> P1.3, SCK -> P1.5, VCC 5 V, GND.
 *   CAN_H con CAN_H y CAN_L con CAN_L entre los modulos, y el puente de 120 ohm
 *   (J1) puesto en los dos extremos del bus.
 */
#include <SPI.h>
#include <MCP2515.h>

#define PIN_CS     P2_2
#define CRISTAL    MCP_8MHZ
#define VELOCIDAD  CAN_500KBPS

unsigned int cuenta = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("MCP2515_Emisor");
  SPI.begin();
  if (mcp2515_begin(PIN_CS, VELOCIDAD, CRISTAL) != MCP2515_OK) {
    Serial.println("el MCP2515 no responde: revise las conexiones");
  }
}

void loop() {
  uint8_t datos[4];
  int a0 = analogRead(A0);
  uint8_t r;
  cuenta++;
  datos[0] = (uint8_t)(cuenta >> 8);    /* orden del bus CAN: byte alto primero */
  datos[1] = (uint8_t)cuenta;
  datos[2] = (uint8_t)(a0 >> 8);
  datos[3] = (uint8_t)a0;
  r = mcp2515_send(0x100, 0, 4, datos);
  Serial.print("trama ");
  Serial.print(cuenta);
  if (r == MCP2515_OK) Serial.println(" enviada");
  else Serial.println(" sin confirmar: ningun otro nodo en el bus (o velocidad distinta)");
  delay(1000);
}
