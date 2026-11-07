/*
 * MCP4725_Rampa - genera una senal triangular de 0 a VCC en la salida del
 * MCP4725 (observela con un osciloscopio; con un multimetro se ve subir y
 * bajar lentamente).
 *
 * Conexiones: VCC, GND, SCL -> P1.5, SDA -> P1.4. Salida: VOUT del modulo.
 */
#include <Wire.h>
#include <MCP4725.h>

#define PASO 64          /* mas pequeno = rampa mas suave y mas lenta */

void setup() {
  Serial.begin(115200);
  Serial.println("MCP4725_Rampa");
  Wire.begin();
  Wire.setClock(400000);                 /* el MCP4725 admite 400 kHz */
  if (mcp4725_begin(0x60) != 0) Serial.println("el MCP4725 no responde (pruebe 0x61, 0x62...)");
}

void loop() {
  unsigned int v;
  for (v = 0; v < 4096; v += PASO) mcp4725_setValue(v);
  for (v = 4095; v >= PASO; v -= PASO) mcp4725_setValue(v);
}
