/*
 * MCP4725.h - DAC MCP4725 (12 bits, una salida) por I2C, para stc8duino
 * (STC8H...) y stc32duino (STC32G).
 *
 * Conexiones del modulo: VCC 3,3 o 5 V (la salida va de 0 a VCC), GND,
 *   SCL -> P1.5, SDA -> P1.4. Direccion 0x60 (A0 a GND) o 0x61 (A0 a VCC);
 *   algunos modulos vienen como 0x62/0x63 (revise con un escaner I2C).
 *
 *   Wire.begin();
 *   mcp4725_begin(0x60);            // 0 si responde
 *   mcp4725_setValue(2048);         // 0-4095: salida = valor * VCC / 4096
 *   mcp4725_setValueEEPROM(1000);   // ademas lo guarda para el encendido
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef MCP4725_h
#define MCP4725_h

#include <Arduino.h>
#include <Wire.h>

/* modos de bajo consumo: la salida queda a GND con esta resistencia */
#define MCP4725_ACTIVO  0
#define MCP4725_1K      1
#define MCP4725_100K    2
#define MCP4725_500K    3

uint8_t mcp4725_begin(uint8_t addr);                 /* 0 si responde */
uint8_t mcp4725_setValue(unsigned int valor);        /* 0 si bien */
uint8_t mcp4725_setValueEEPROM(unsigned int valor);  /* tarda hasta 50 ms */
int     mcp4725_getValue(void);                      /* valor actual, -1 si error */
uint8_t mcp4725_powerDown(uint8_t modo);             /* MCP4725_1K... */

#endif
