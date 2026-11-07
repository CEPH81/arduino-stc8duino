/*
 * MCP23017.h - expansor de 16 entradas/salidas MCP23017 por I2C, para
 * stc8duino (STC8H...) y stc32duino (STC32G).
 *
 * Conexiones: VDD 3,3 o 5 V, VSS a GND, SCL -> P1.5, SDA -> P1.4,
 *   RESET a VDD, A0-A2 a GND = direccion 0x20 (hasta 0x27).
 * Patas: 0-7 = GPA0-GPA7, 8-15 = GPB0-GPB7. Nota: en los MCP23017 nuevos
 *   GPA7 y GPB7 solo sirven como salida (hoja de datos, rev. 2022).
 *
 *   Wire.begin();
 *   mcp23017_begin(0x20);                         // todas entradas; 0 si responde
 *   mcp23017_pinMode(0x20, 0, OUTPUT);
 *   mcp23017_digitalWrite(0x20, 0, HIGH);
 *   mcp23017_pinMode(0x20, 8, INPUT_PULLUP);
 *   b = mcp23017_digitalRead(0x20, 8);            // 0/1, -1 si no responde
 *   mcp23017_writePort(0x20, MCP23017_A, 0x55);   // las 8 patas de un puerto
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef MCP23017_h
#define MCP23017_h

#include <Arduino.h>
#include <Wire.h>

#define MCP23017_A 0
#define MCP23017_B 1

uint8_t mcp23017_begin(uint8_t addr);
uint8_t mcp23017_pinMode(uint8_t addr, uint8_t pata, uint8_t modo);
uint8_t mcp23017_digitalWrite(uint8_t addr, uint8_t pata, uint8_t valor);
int     mcp23017_digitalRead(uint8_t addr, uint8_t pata);
uint8_t mcp23017_portMode(uint8_t addr, uint8_t puerto, uint8_t entradas, uint8_t pullups);
uint8_t mcp23017_writePort(uint8_t addr, uint8_t puerto, uint8_t valor);
int     mcp23017_readPort(uint8_t addr, uint8_t puerto);

#endif
