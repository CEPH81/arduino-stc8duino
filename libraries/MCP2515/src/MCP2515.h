/*
 * MCP2515.h - bus CAN con el modulo MCP2515 (+ transceptor TJA1050) por SPI,
 * para stc8duino (STC8H...) y stc32duino (STC32G).
 *
 * Conexiones del modulo (VCC 5 V para el TJA1050; las patas SPI aceptan 3,3 V):
 *   CS -> pata a eleccion   SO -> MISO   SI -> MOSI   SCK -> SCLK   INT (no se usa)
 *   SPI por defecto: MOSI P1.3, MISO P1.4, SCLK P1.5 (ver SPI.setPins).
 *   CAN_H y CAN_L al bus, con una resistencia de 120 ohm en cada extremo
 *   (el modulo trae una con un puente J1).
 *   Mire el cristal del modulo: casi siempre 8 MHz (8.000) y a veces 16 MHz.
 *
 *   SPI.begin();
 *   mcp2515_begin(CS, CAN_500KBPS, MCP_8MHZ);    // 0 si el modulo responde
 *   mcp2515_send(0x123, 0, 3, datos);            // id, extendido (0/1), largo, datos
 *   if (mcp2515_available()) mcp2515_read(&id, &ext, &largo, datos);
 *
 * Recibe todas las tramas (sin filtros). Las de peticion remota (RTR) se leen
 * como tramas de largo 0.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef MCP2515_h
#define MCP2515_h

#include <Arduino.h>
#include <SPI.h>

#define CAN_50KBPS   0
#define CAN_100KBPS  1
#define CAN_125KBPS  2
#define CAN_250KBPS  3
#define CAN_500KBPS  4
#define CAN_1000KBPS 5

#define MCP_8MHZ  8
#define MCP_16MHZ 16

#define MCP2515_OK        0
#define MCP2515_ERROR     1     /* el modulo no responde */
#define MCP2515_SIN_ACK   2     /* nadie confirmo la trama (bus sin otro nodo) */

uint8_t mcp2515_begin(uint8_t cs, uint8_t velocidad, uint8_t cristal);
uint8_t mcp2515_setNormalMode(void);
uint8_t mcp2515_setLoopbackMode(void);     /* recibe sus propias tramas, sin bus */
uint8_t mcp2515_setListenOnlyMode(void);   /* solo escucha, no confirma */
uint8_t mcp2515_send(unsigned long id, uint8_t extendido, uint8_t largo, const uint8_t *datos);
uint8_t mcp2515_available(void);
uint8_t mcp2515_read(unsigned long *id, uint8_t *extendido, uint8_t *largo, uint8_t *datos);
uint8_t mcp2515_errors(void);              /* registro EFLG (0 = sin errores) */
uint8_t mcp2515_readRegister(uint8_t reg);

#endif
