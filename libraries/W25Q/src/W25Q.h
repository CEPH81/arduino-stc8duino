/*
 * W25Q.h - memorias flash SPI W25Qxx (W25Q80 a W25Q128, de 1 a 16 MB), para
 * stc8duino (STC8H...) y stc32duino (STC32G).
 *
 * Conexiones (la memoria es de 3,3 V: con placa a 5 V use un modulo con
 * regulador y adaptador de niveles):
 *   VCC 3,3 V, GND, CS -> pata a eleccion, DO (MISO) -> P1.4,
 *   DI (MOSI) -> P1.3, CLK -> P1.5, WP y HOLD a VCC.
 *
 *   SPI.begin();
 *   w25q_begin(CS);                       // 0 si responde
 *   w25q_size();                          // bytes (p. ej. 4194304 en un W25Q32)
 *   w25q_eraseSector(0);                  // pone 4 KB en 0xFF (unos 50 ms)
 *   w25q_write(0, datos, n);              // solo cambia bits de 1 a 0: borre antes
 *   w25q_read(0, datos, n);
 *
 * Las direcciones son de 24 bits (hasta 16 MB).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef W25Q_h
#define W25Q_h

#include <Arduino.h>
#include <SPI.h>

#define W25Q_SECTOR 4096UL
#define W25Q_BLOQUE 65536UL

uint8_t       w25q_begin(uint8_t cs);                       /* 0 si responde */
unsigned long w25q_jedecID(void);                           /* fabricante, tipo, capacidad */
unsigned long w25q_size(void);                              /* bytes, 0 si no responde */
void          w25q_read(unsigned long dir, uint8_t *p, unsigned int n);
void          w25q_write(unsigned long dir, const uint8_t *p, unsigned int n);
void          w25q_eraseSector(unsigned long dir);           /* 4 KB que contienen dir */
void          w25q_eraseBlock(unsigned long dir);            /* 64 KB que contienen dir */
void          w25q_eraseChip(void);                          /* todo: de 2 s a 40 s */
uint8_t       w25q_busy(void);
void          w25q_powerDown(void);
void          w25q_wakeUp(void);

#endif
