/*
 * ADS1115.h - convertidor ADC ADS1115 (16 bits, 4 canales) por I2C, para
 * stc8duino (STC8H...) y stc32duino (STC32G).
 *
 * Conexiones del modulo: VDD 3,3 o 5 V, GND, SCL -> P1.5, SDA -> P1.4.
 *   ADDR a GND = 0x48 (lo normal), a VDD = 0x49, a SDA = 0x4A, a SCL = 0x4B.
 *   Las entradas A0-A3 no deben pasar de VDD + 0,3 V aunque la escala sea mayor.
 *
 *   Wire.begin();
 *   ads1115_begin(0x48);                 // 0 si responde
 *   ads1115_setGain(ADS1115_4V096);      // escala +-4,096 V
 *   v = ads1115_readChannel(0);          // A0 contra GND: -32768..32767
 *   uv = ads1115_toMicrovolts(v);        // en microvoltios (long)
 *
 * Cada lectura arranca una conversion y espera su fin (a 128 muestras/s,
 * unos 8 ms).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef ADS1115_h
#define ADS1115_h

#include <Arduino.h>
#include <Wire.h>

/* ganancia = escala completa */
#define ADS1115_6V144  0
#define ADS1115_4V096  1
#define ADS1115_2V048  2     /* la del encendido */
#define ADS1115_1V024  3
#define ADS1115_0V512  4
#define ADS1115_0V256  5

/* muestras por segundo */
#define ADS1115_8SPS    0
#define ADS1115_16SPS   1
#define ADS1115_32SPS   2
#define ADS1115_64SPS   3
#define ADS1115_128SPS  4    /* la del encendido */
#define ADS1115_250SPS  5
#define ADS1115_475SPS  6
#define ADS1115_860SPS  7

/* pares diferenciales */
#define ADS1115_A0_A1  0
#define ADS1115_A0_A3  1
#define ADS1115_A1_A3  2
#define ADS1115_A2_A3  3

uint8_t ads1115_begin(uint8_t addr);               /* 0 si responde */
void    ads1115_setGain(uint8_t ganancia);
void    ads1115_setRate(uint8_t velocidad);
int     ads1115_readChannel(uint8_t canal);        /* 0-3 contra GND */
int     ads1115_readDiff(uint8_t par);             /* ADS1115_A0_A1... */
long    ads1115_toMicrovolts(int valor);           /* con la ganancia actual */

#endif
