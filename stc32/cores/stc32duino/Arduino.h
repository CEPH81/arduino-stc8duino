/*
 * Arduino.h - nucleo stc32duino para el STC32G12K128 (Keil C251, modelo XSMALL).
 *
 * pinMode, digitalWrite/Read/Toggle, millis, micros, delay, delayMicroseconds,
 * attachInterrupt (INT0/INT1), Serial, analogRead (ADC de 12 bits) y
 * analogWrite (PWM5-PWM8) y timer1Begin (temporizador 1). Las patas se nombran P3_4, P1_0...
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef Arduino_h
#define Arduino_h

typedef unsigned char  uint8_t;
typedef signed char    int8_t;
typedef unsigned int   uint16_t;   /* en C251 int es de 16 bits */
typedef signed int     int16_t;
typedef unsigned long  uint32_t;
typedef signed long    int32_t;
typedef unsigned char  boolean;
typedef unsigned char  byte;
typedef unsigned int   word;
#ifndef bool
#define bool  unsigned char
#define true  1
#define false 0
#endif

typedef unsigned int   size_t;

#define HIGH 1
#define LOW  0
#define FALLING 2
#define CHANGE  1
#define RISING  3

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#define constrain(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))
#define F(s) (s)
#define INPUT        0
#define OUTPUT       1
#define INPUT_PULLUP 2
#define OUTPUT_OPEN_DRAIN 3

#ifdef STC_PREPROC
/* preprocesado con SDCC (prototipos del IDE): palabras propias de C251 */
#define sfr  unsigned char
#define sbit unsigned char
#define far
#define interrupt(x)
#endif

#include "stc32_sfr.h"

/* patas: 0xPB = puerto P, bit B */
#define P0_0 0x00
#define P0_1 0x01
#define P0_2 0x02
#define P0_3 0x03
#define P0_4 0x04
#define P0_5 0x05
#define P0_6 0x06
#define P0_7 0x07
#define P1_0 0x10
#define P1_1 0x11
#define P1_2 0x12
#define P1_3 0x13
#define P1_4 0x14
#define P1_5 0x15
#define P1_6 0x16
#define P1_7 0x17
#define P2_0 0x20
#define P2_1 0x21
#define P2_2 0x22
#define P2_3 0x23
#define P2_4 0x24
#define P2_5 0x25
#define P2_6 0x26
#define P2_7 0x27
#define P3_0 0x30
#define P3_1 0x31
#define P3_2 0x32
#define P3_3 0x33
#define P3_4 0x34
#define P3_5 0x35
#define P3_6 0x36
#define P3_7 0x37
#define P4_0 0x40
#define P4_1 0x41
#define P4_2 0x42
#define P4_3 0x43
#define P4_4 0x44
#define P4_5 0x45
#define P4_6 0x46
#define P4_7 0x47
#define P5_0 0x50
#define P5_1 0x51
#define P5_2 0x52
#define P5_3 0x53
#define P5_4 0x54
#define P5_5 0x55
#define P5_6 0x56
#define P5_7 0x57
#define P6_0 0x60
#define P6_1 0x61
#define P6_2 0x62
#define P6_3 0x63
#define P6_4 0x64
#define P6_5 0x65
#define P6_6 0x66
#define P6_7 0x67
#define P7_0 0x70
#define P7_1 0x71
#define P7_2 0x72
#define P7_3 0x73
#define P7_4 0x74
#define P7_5 0x75
#define P7_6 0x76
#define P7_7 0x77

/* entradas analogicas: A0-A7 = P1.0-P1.7, A8-A14 = P0.0-P0.6 */
#define A0  P1_0
#define A1  P1_1
#define A2  P1_2
#define A3  P1_3
#define A4  P1_4
#define A5  P1_5
#define A6  P1_6
#define A7  P1_7
#define A8  P0_0
#define A9  P0_1
#define A10 P0_2
#define A11 P0_3
#define A12 P0_4
#define A13 P0_5
#define A14 P0_6

#include "pins_arduino.h"

#define interrupts()   (EA = 1)
#define noInterrupts() (EA = 0)

/* interrupciones externas: INT0 = P3.2, INT1 = P3.3 */
#define digitalPinToInterrupt(p) ((p) == P3_2 ? 0 : ((p) == P3_3 ? 1 : -1))
void attachInterrupt(uint8_t num, void (*func)(void), uint8_t mode);
void detachInterrupt(uint8_t num);

void init(void);
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int  digitalRead(uint8_t pin);
void digitalToggle(uint8_t pin);
int  analogRead(uint8_t pin);
void analogReadResolution(uint8_t bits);
void analogWrite(uint8_t pin, int val);

/* temporizador 1: llama a func() cada periodo_us (dentro de la interrupcion) */
void timer1Begin(unsigned long periodo_us, void (*func)(void));
void timer1End(void);
unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);

void setup(void);
void loop(void);

#include "stc_print.h"
#include "HardwareSerial.h"

#endif
