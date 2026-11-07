/*
 * MCP4728_Cuatro - las 4 salidas del MCP4728 a la vez:
 *   A: rampa que sube      B: rampa que baja
 *   C: onda cuadrada       D: fija a la mitad
 * Con la referencia interna (2,048 V, ganancia x2) cada paso vale 1 mV:
 * mcp4728_setChannel(3, 1500) da 1,500 V.
 *
 * Conexiones: VCC 5 V, GND, SCL -> P1.5, SDA -> P1.4, LDAC a GND.
 */
#include <Wire.h>
#include <MCP4728.h>

unsigned int v = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("MCP4728_Cuatro");
  Wire.begin();
  Wire.setClock(400000);
  if (mcp4728_begin(0x60) != 0) Serial.println("el MCP4728 no responde");
  mcp4728_setVref(MCP4728_INTERNA);
  mcp4728_setGain(MCP4728_X2);          /* 0-4,095 V en pasos de 1 mV (con VCC 5 V) */
  mcp4728_setChannel(3, 2048);           /* D fija: 2,048 V */
}

void loop() {
  v += 32;
  if (v > 4095) v = 0;
  mcp4728_setAll(v, 4095 - v, (v & 1024) ? 4095 : 0, 2048);
}
