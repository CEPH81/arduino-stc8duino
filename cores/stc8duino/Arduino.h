/*
 * Arduino.h - nucleo Arduino para STC8H, STC8G, STC8A y STC15 (SDCC, C).
 *
 * Numeracion de las patas: 0x<puerto><bit>, se lee como el nombre de la pata:
 *   P1.0 = P1_0 = 0x10, P3.2 = P3_2 = 0x32, P5.4 = P5_4 = 0x54.
 * (en hexadecimal para que el 8051 no tenga que dividir en cada digitalWrite)
 *
 * El sketch se compila como C (SDCC). La sintaxis Serial.print(x) funciona
 * igual que en Arduino (ver stc_obj.h).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef Arduino_h
#define Arduino_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "stc_sfr.h"
#include "binary.h"

typedef uint8_t byte;
typedef bool boolean;
typedef unsigned int word;

/* ---------- constantes ---------- */
#define HIGH 0x1
#define LOW  0x0

#define INPUT               0x00  /* alta impedancia */
#define OUTPUT              0x01  /* push-pull */
#define INPUT_PULLUP        0x02  /* con resistencia de pull-up */
#define OUTPUT_OPEN_DRAIN   0x03
#define QUASI_BIDIRECTIONAL 0x04  /* el modo clasico del 8051 */

#define CHANGE  1
#define FALLING 2
#define RISING  3

#define LSBFIRST 0
#define MSBFIRST 1

#undef PI
#undef HALF_PI
#undef TWO_PI
#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define round(x) ((x) >= 0 ? (long)((x) + 0.5) : (long)((x) - 0.5))
#define radians(deg) ((deg) * DEG_TO_RAD)
#define degrees(rad) ((rad) * RAD_TO_DEG)
#define sq(x) ((x) * (x))

#define lowByte(w)  ((uint8_t)((w) & 0xff))
#define highByte(w) ((uint8_t)((w) >> 8))
#define bitRead(value, bit)  (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)   ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))
#define bit(b) (1UL << (b))
#define makeWord(h, l) (((unsigned int)(h) << 8) | (l))

#define interrupts()   (EA = 1)
#define noInterrupts() (EA = 0)

#define clockCyclesPerMicrosecond() (F_CPU / 1000000L)

/* ---------- patas ---------- */
#define digitalPinToPort(p)    ((uint8_t)(p) >> 4)
#define digitalPinToBit(p)     ((p) & 0x07)
#define digitalPinToBitMask(p) (1 << ((p) & 0x07))
#define NOT_A_PIN 255

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

#include "pins_arduino.h"

/* ---------- funciones ---------- */
void init(void);
void initVariant(void);
void setup(void);
void loop(void);

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
uint8_t digitalRead(uint8_t pin);
void digitalToggle(uint8_t pin);

int analogRead(uint8_t pin);
void analogReadResolution(uint8_t bits);

unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);

void attachInterrupt(uint8_t num, void (*func)(void), uint8_t mode);
void detachInterrupt(uint8_t num);

/* temporizador 1: llama a func() cada periodo_us microsegundos (dentro de la interrupcion) */
void timer1Begin(unsigned long periodo_us, void (*func)(void));
void timer1End(void);

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);

void randomSeed(unsigned long seed);
long random(long howbig);
long random_minmax(long howsmall, long howbig);
long map(long x, long in_min, long in_max, long out_min, long out_max);

/* reinicia el chip; con isp = 1 arranca en el cargador de la ROM (modo de carga) */
void stcReset(uint8_t isp);

/* serialEvent(): si el sketch la define, se llama despues de cada loop() */
void serialEvent(void);
extern unsigned char runSerialEvent;

#include "stc_isr.h"
#include "HardwareSerial.h"

#endif
