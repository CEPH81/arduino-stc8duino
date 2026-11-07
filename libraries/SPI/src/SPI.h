/*
 * SPI.h - SPI maestro por hardware para stc8duino (STC8H, STC8G, STC8A, STC15).
 *
 *   SPI.begin();                                    // patas por defecto (abajo)
 *   SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
 *   digitalWrite(CS, LOW);
 *   r = SPI.transfer(0x9F);  SPI.transfer(buffer, n);
 *   digitalWrite(CS, HIGH);
 *   SPI.endTransaction();
 *
 * Patas (SPI.setPins, antes de begin):      SS    MOSI  MISO  SCLK
 *   0 (por defecto, todos los modelos)      P1.2  P1.3  P1.4  P1.5
 *   STC8:  1                                P2.2  P2.3  P2.4  P2.5
 *          2 (encapsulados con P7)          P7.4  P7.5  P7.6  P7.7
 *          3                                P3.5  P3.4  P3.3  P3.2
 *   STC15: 1                                P2.4  P2.3  P2.2  P2.1
 *          2                                P5.4  P4.0  P4.1  P4.3
 * Ojo: la opcion 0 usa P1.4/P1.5, las mismas del I2C (Wire) por defecto. Para
 * usar los dos a la vez, SPI.setPins(1) o Wire.setPins(1).
 * El chip select (CS) de cada dispositivo se maneja con digitalWrite.
 *
 * Velocidad: STC8 F_CPU / 2, 4, 8 o 16; STC15 F_CPU / 4, 16, 64 o 128. Se
 * elige la mas rapida que no pase de la pedida.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef SPI_h
#define SPI_h

#include <Arduino.h>

#define SPI_MODE0 0x00     /* CPOL 0, CPHA 0 */
#define SPI_MODE1 0x04     /* CPOL 0, CPHA 1 */
#define SPI_MODE2 0x08     /* CPOL 1, CPHA 0 */
#define SPI_MODE3 0x0C     /* CPOL 1, CPHA 1 */

#define SPI_CLOCK_DIV2   2
#define SPI_CLOCK_DIV4   4
#define SPI_CLOCK_DIV8   8
#define SPI_CLOCK_DIV16  16
#define SPI_CLOCK_DIV64  64
#define SPI_CLOCK_DIV128 128

void     SPI_begin(void);
void     SPI_end(void);
void     SPI_setPins(uint8_t opcion);
void     SPI_beginTransaction(unsigned long reloj, uint8_t orden, uint8_t modo);
void     SPI_setBitOrder(uint8_t orden);
void     SPI_setDataMode(uint8_t modo);
void     SPI_setClockDivider(uint8_t divisor);
uint8_t  SPI_transfer(uint8_t b);
uint16_t SPI_transfer16(uint16_t w);
void     SPI_transferBuf(void *buf, unsigned int n);   /* envia y recibe en el mismo bufer */
#define  SPI_endTransaction()

/* SPISettings(reloj, orden, modo): en C son los tres valores separados por comas */
#define SPISettings(reloj, orden, modo) (unsigned long)(reloj), (uint8_t)(orden), (uint8_t)(modo)

/* --- sintaxis estilo Arduino en el sketch: SPI.begin()... (ver stc_obj.h) --- */
#ifdef STC_SKETCH
#define SPI__begin()                  SPI_begin()
#define SPI__end()                    SPI_end()
#define SPI__setPins(o)               SPI_setPins(o)
#define SPI__beginTransaction(...)    SPI_beginTransaction(__VA_ARGS__)
#define SPI__endTransaction()
#define SPI__setBitOrder(o)           SPI_setBitOrder(o)
#define SPI__setDataMode(m)           SPI_setDataMode(m)
#define SPI__setClockDivider(d)       SPI_setClockDivider(d)
#define SPI__transfer(...)            S8_OVERLOAD(SPI__transfer, __VA_ARGS__)
#define SPI__transfer1(b)             SPI_transfer((uint8_t)(b))
#define SPI__transfer2(buf, n)        SPI_transferBuf((buf), (n))
#define SPI__transfer16(w)            SPI_transfer16((uint16_t)(w))
#define SPI__usingInterrupt(x)
#endif

#endif
