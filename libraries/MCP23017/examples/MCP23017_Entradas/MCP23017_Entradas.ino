/*
 * MCP23017_Entradas - 7 pulsadores en el puerto B encienden 7 LEDs del
 * puerto A, y cada cambio se muestra en el monitor. (GPB7 no se usa: en los
 * MCP23017 actuales GPA7 y GPB7 solo funcionan como salida.)
 *
 * Conexiones: VDD, VSS a GND, SCL -> P1.5, SDA -> P1.4, RESET a VDD,
 *   A0-A2 a GND (0x20).
 *   GPB0-GPB6: pulsador a GND (usa las resistencias de pull-up internas).
 *   GPA0-GPA6: LED + 330 ohm a GND.
 */
#include <Wire.h>
#include <MCP23017.h>

#define EXP 0x20

int antes = -2;

void setup() {
  uint8_t i;
  Serial.begin(115200);
  Serial.println("MCP23017_Entradas");
  Wire.begin();
  if (mcp23017_begin(EXP) != 0) Serial.println("el MCP23017 no responde");
  for (i = 0; i < 7; i++) {
    mcp23017_pinMode(EXP, i, OUTPUT);                       /* GPA: LEDs */
    mcp23017_pinMode(EXP, (uint8_t)(8 + i), INPUT_PULLUP);  /* GPB: pulsadores */
  }
}

void loop() {
  int b = mcp23017_readPort(EXP, MCP23017_B);
  if (b < 0) return;
  b = ~b & 0x7F;                       /* pulsado = 0 en la pata: se invierte */
  mcp23017_writePort(EXP, MCP23017_A, (uint8_t)b);
  if (b != antes) {
    antes = b;
    Serial.print("pulsadores: ");
    Serial.println(b, BIN);
  }
  delay(20);
}
