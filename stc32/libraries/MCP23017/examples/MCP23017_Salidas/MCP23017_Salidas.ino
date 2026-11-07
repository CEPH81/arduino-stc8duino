/*
 * MCP23017_Salidas - luces que corren por las 16 salidas del MCP23017.
 *
 * Conexiones: VDD, VSS a GND, SCL -> P1.5, SDA -> P1.4, RESET a VDD,
 *   A0, A1 y A2 a GND (direccion 0x20).
 *   En cada GPA0-GPA7 y GPB0-GPB7: LED + resistencia de 330 ohm a GND.
 */
#include <Wire.h>
#include <MCP23017.h>

#define EXP 0x20

void setup() {
  Serial.begin(115200);
  Serial.println("MCP23017_Salidas");
  Wire.begin();
  if (mcp23017_begin(EXP) != 0) Serial.println("el MCP23017 no responde: revise A0-A2, RESET y el I2C");
  mcp23017_portMode(EXP, MCP23017_A, 0x00, 0);   /* las 16 como salida */
  mcp23017_portMode(EXP, MCP23017_B, 0x00, 0);
}

void loop() {
  uint8_t i;
  for (i = 0; i < 16; i++) {
    /* una sola encendida: el bit i de los 16 */
    mcp23017_writePort(EXP, MCP23017_A, (uint8_t)(i < 8 ? 1 << i : 0));
    mcp23017_writePort(EXP, MCP23017_B, (uint8_t)(i >= 8 ? 1 << (i - 8) : 0));
    delay(80);
  }
}
