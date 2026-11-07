/*
 * MCP4728.h - DAC MCP4728 (12 bits, 4 salidas A-D) por I2C, para stc8duino
 * (STC8H...) y stc32duino (STC32G).
 *
 * Conexiones del modulo: VCC 3,3 o 5 V, GND, SCL -> P1.5, SDA -> P1.4.
 *   LDAC a GND (o sin conectar si el modulo trae la resistencia a GND).
 *   Direccion 0x60 de fabrica (0x60-0x67).
 *
 *   Wire.begin();
 *   mcp4728_begin(0x60);                    // 0 si responde
 *   mcp4728_setChannel(0, 2048);            // canal A a la mitad
 *   mcp4728_setAll(0, 1024, 2048, 4095);    // los cuatro a la vez
 *
 * Referencia: por omision VDD (salida = valor * VDD / 4096). Con
 * mcp4728_setVref(MCP4728_INTERNA) usa 2,048 V internos: salida = valor * 0,5 mV
 * (x1) o valor * 1 mV con mcp4728_setGain(MCP4728_X2) (no pasa de VDD).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef MCP4728_h
#define MCP4728_h

#include <Arduino.h>
#include <Wire.h>

#define MCP4728_VDD     0
#define MCP4728_INTERNA 1
#define MCP4728_X1      0
#define MCP4728_X2      1

uint8_t mcp4728_begin(uint8_t addr);                          /* 0 si responde */
uint8_t mcp4728_setChannel(uint8_t canal, unsigned int valor); /* canal 0-3 = A-D */
uint8_t mcp4728_setAll(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
uint8_t mcp4728_setVref(uint8_t ref);       /* los 4 canales: MCP4728_VDD o _INTERNA */
uint8_t mcp4728_setGain(uint8_t ganancia);  /* los 4 canales: MCP4728_X1 o _X2 */
uint8_t mcp4728_saveEEPROM(void);           /* guarda los valores actuales (unos 50 ms) */

#endif
