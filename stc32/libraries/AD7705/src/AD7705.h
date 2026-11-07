/*
 * AD7705.h - ADC sigma-delta AD7705 / TM7705 (16 bits, 2 canales
 * diferenciales) por SPI, para stc8duino (STC8H...) y stc32duino (STC32G).
 *
 * Modulo TM7705 (cristal de 4,9152 MHz y referencia de 2,5 V):
 *   VCC 5 V (o 3,3 V), GND, CS -> pata a eleccion, DIN -> MOSI P1.3,
 *   DOUT -> MISO P1.4, SCK -> SCLK P1.5, DRDY -> pata a eleccion (o sin
 *   conectar: AD7705_SIN_DRDY), RST a VCC.
 *   Canal 0: AIN1+ / AIN1-   Canal 1: AIN2+ / AIN2-
 *
 *   SPI.begin();
 *   ad7705_begin(CS, DRDY, AD7705_50HZ);        // 0 si responde
 *   ad7705_setup(0, AD7705_G1, AD7705_UNIPOLAR); // calibra el canal 0
 *   v = ad7705_read(0);                          // 0-65535
 *   uv = ad7705_toMicrovolts(v, 2500, AD7705_G1, AD7705_UNIPOLAR);
 *
 * Unipolar: 0 = 0 V, 65535 = Vref / ganancia. Bipolar: 32768 = 0 V.
 * Al alternar de canal se descartan 2 conversiones (filtro): a 50 Hz cada
 * cambio tarda unos 60 ms.
 * Las entradas no deben salir de GND..VCC.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef AD7705_h
#define AD7705_h

#include <Arduino.h>
#include <SPI.h>

#define AD7705_SIN_DRDY 0xFF

/* muestras por segundo (cristal de 4,9152 MHz) */
#define AD7705_50HZ   0
#define AD7705_60HZ   1
#define AD7705_250HZ  2
#define AD7705_500HZ  3

/* ganancia */
#define AD7705_G1    0
#define AD7705_G2    1
#define AD7705_G4    2
#define AD7705_G8    3
#define AD7705_G16   4
#define AD7705_G32   5
#define AD7705_G64   6
#define AD7705_G128  7

#define AD7705_UNIPOLAR 1
#define AD7705_BIPOLAR  0

uint8_t      ad7705_begin(uint8_t cs, uint8_t drdy, uint8_t velocidad);   /* 0 si responde */
uint8_t      ad7705_setup(uint8_t canal, uint8_t ganancia, uint8_t unipolar); /* 0 si calibro */
uint8_t      ad7705_ready(uint8_t canal);                 /* 1: hay un dato nuevo */
unsigned int ad7705_read(uint8_t canal);                  /* espera el dato (hasta 0,5 s) */
long         ad7705_toMicrovolts(unsigned int valor, unsigned int vref_mv,
                                 uint8_t ganancia, uint8_t unipolar);

#endif
