/*
 * MCP4725_Potenciometro - la salida del MCP4725 copia la tension de un
 * potenciometro en A0 (P1.0). Con 's' en el monitor guarda el valor en la
 * EEPROM del DAC: al volver a encenderlo arranca con esa tension.
 *
 * Conexiones: VCC, GND, SCL -> P1.5, SDA -> P1.4.
 *   Potenciometro: extremos a VCC y GND, el cursor a P1.0.
 */
#include <Wire.h>
#include <MCP4725.h>

unsigned int valor = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("MCP4725_Potenciometro (s = guardar en la EEPROM)");
  analogReadResolution(12);              /* 0-4095, igual que el DAC */
  Wire.begin();
  if (mcp4725_begin(0x60) != 0) Serial.println("el MCP4725 no responde");
}

void loop() {
  valor = analogRead(A0);
  mcp4725_setValue(valor);
  if (Serial.available() && Serial.read() == 's') {
    Serial.print(mcp4725_setValueEEPROM(valor) == 0 ? "guardado: " : "error al guardar: ");
    Serial.println(valor);
  }
  delay(20);
}
