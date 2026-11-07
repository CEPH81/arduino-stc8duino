/*
 * SPI.h - SPI maestro por hardware (modulo SPI del STC32G).
 *
 *   SPI.begin();                                    // patas por defecto (abajo)
 *   SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
 *   digitalWrite(CS, LOW);
 *   r = SPI.transfer(0x9F);  SPI.transfer(buffer, n);
 *   digitalWrite(CS, HIGH);
 *   SPI.endTransaction();
 *
 * Patas (SPI.setPins, antes de begin):      SS    MOSI  MISO  SCLK
 *   0 (por defecto)                         P1.2  P1.3  P1.4  P1.5
 *   1                                       P2.2  P2.3  P2.4  P2.5
 *   2 (solo LQFP64)                         P7.4  P7.5  P7.6  P7.7
 *   3                                       P3.5  P3.4  P3.3  P3.2
 * Ojo: la opcion 0 usa P1.4/P1.5, las mismas del I2C (Wire) por defecto. Para
 * usar los dos a la vez, SPI.setPins(1) o Wire.setPins(1).
 * La pata SS del modulo no se usa: el chip select (CS) de cada dispositivo se
 * maneja con digitalWrite como en Arduino.
 *
 * Velocidad: F_CPU / 2, 4, 8 o 16 (a 24 MHz: 12, 6, 3 o 1,5 MHz); se elige la
 * mas rapida que no pase de la pedida.
 *
 * Parte de stc8duino (version stc32duino). LGPL 2.1.
 */
#ifndef SPI_h
#define SPI_h

#include <Arduino.h>

#define SPI_MODE0 0x00     /* CPOL 0, CPHA 0 */
#define SPI_MODE1 0x04     /* CPOL 0, CPHA 1 */
#define SPI_MODE2 0x08     /* CPOL 1, CPHA 0 */
#define SPI_MODE3 0x0C     /* CPOL 1, CPHA 1 */
#ifndef LSBFIRST
#define LSBFIRST 0
#define MSBFIRST 1
#endif

#define SPI_CLOCK_DIV2   2
#define SPI_CLOCK_DIV4   4
#define SPI_CLOCK_DIV8   8
#define SPI_CLOCK_DIV16  16

void     SPI_begin(void);
void     SPI_end(void);
void     SPI_setPins(uint8_t opcion);
void     SPI_beginTransaction(unsigned long reloj, uint8_t orden, uint8_t modo);
void     SPI_endTransaction(void);
void     SPI_setBitOrder(uint8_t orden);
void     SPI_setDataMode(uint8_t modo);
void     SPI_setClockDivider(uint8_t divisor);
uint8_t  SPI_transfer(uint8_t b);
uint16_t SPI_transfer16(uint16_t w);
void     SPI_transferBuf(void *buf, unsigned int n);   /* envia y recibe en el mismo bufer */

/* SPISettings(reloj, orden, modo): en C son los tres valores separados por comas */
#define SPISettings(reloj, orden, modo) (unsigned long)(reloj), (uint8_t)(orden), (uint8_t)(modo)

/* sintaxis estilo Arduino en el sketch (stcar -obj: SPI.metodo -> SPI__metodo_N) */
#define SPI__begin_0()                SPI_begin()
#define SPI__end_0()                  SPI_end()
#define SPI__setPins_1(o)             SPI_setPins(o)
#define SPI__beginTransaction_1(s)    SPI_beginTransaction(s)
#define SPI__endTransaction_0()       SPI_endTransaction()
#define SPI__setBitOrder_1(o)         SPI_setBitOrder(o)
#define SPI__setDataMode_1(m)         SPI_setDataMode(m)
#define SPI__setClockDivider_1(d)     SPI_setClockDivider(d)
#define SPI__transfer_1(b)            SPI_transfer((uint8_t)(b))
#define SPI__transfer_L(b)            SPI_transfer((uint8_t)(b))
#define SPI__transfer_2(buf, n)       SPI_transferBuf((buf), (n))
#define SPI__transfer16_1(w)          SPI_transfer16((uint16_t)(w))
#define SPI__transfer16_L(w)          SPI_transfer16((uint16_t)(w))
#define SPI__usingInterrupt_1(x)      ((void)0)

#endif
