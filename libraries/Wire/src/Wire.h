/*
 * Wire.h - I2C maestro para stc8duino.
 *
 *   Wire.begin();                       // 100 kHz, SCL P1.5 / SDA P1.4
 *   Wire.setPins(1);                    // otras patas (antes de begin), ver abajo
 *   Wire.setClock(400000);
 *   Wire.beginTransmission(0x27);  Wire.write(0x55);  Wire.endTransmission();
 *   n = Wire.requestFrom(0x27, 2);  while (Wire.available()) x = Wire.read();
 *
 * STC8H, STC8G y STC8A: modulo I2C del chip. Patas (Wire.setPins):
 *   0: SCL P1.5, SDA P1.4 (por defecto)   1: SCL P2.5, SDA P2.4
 *   2: SCL P7.7, SDA P7.6                 3: SCL P3.2, SDA P3.3
 * STC15: I2C por programa en SCL P1.5, SDA P1.4 (setPins no tiene efecto).
 * Las patas quedan en drenador abierto con la resistencia de pull-up interna
 * (en los STC8); para cables largos o 400 kHz ponga resistencias de 4,7 k a VCC.
 *
 * endTransmission() devuelve 0 si todo fue bien, 2 si nadie respondio a la
 * direccion, 3 si no confirmaron un dato y 5 si el bus no responde (sin pull-up).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef Wire_h
#define Wire_h

#include <Arduino.h>

#ifndef WIRE_BUFFER_SIZE
#define WIRE_BUFFER_SIZE 32
#endif

void    Wire_begin(void);
void    Wire_end(void);
void    Wire_setPins(uint8_t opcion);
void    Wire_setClock(unsigned long hz);
void    Wire_beginTransmission(uint8_t address);
uint8_t Wire_endTransmission1(uint8_t sendStop);
#define Wire_endTransmission()  Wire_endTransmission1(1)
uint8_t Wire_requestFrom3(uint8_t address, uint8_t quantity, uint8_t sendStop);
#define Wire_requestFrom2(a, n) Wire_requestFrom3(a, n, 1)
size_t  Wire_write(uint8_t data);
size_t  Wire_write_s(const char *str);
size_t  Wire_write_sn(const uint8_t *data, size_t quantity);
int     Wire_available(void);
int     Wire_read(void);
int     Wire_peek(void);

/* --- sintaxis estilo Arduino en el sketch: Wire.begin()... (ver stc_obj.h) --- */
#ifdef STC_SKETCH
#define Wire__begin()                 Wire_begin()
#define Wire__end()                   Wire_end()
#define Wire__setPins(o)              Wire_setPins(o)
#define Wire__setClock(hz)            Wire_setClock(hz)
#define Wire__beginTransmission(a)    Wire_beginTransmission(a)
#define Wire__endTransmission(...)    S8_OVERLOAD(Wire__endTransmission, __VA_ARGS__)
#define Wire__endTransmission0()      Wire_endTransmission1(1)
#define Wire__endTransmission1(stop)  Wire_endTransmission1(stop)
#define Wire__requestFrom(...)        S8_OVERLOAD(Wire__requestFrom, __VA_ARGS__)
#define Wire__requestFrom2(a, n)      Wire_requestFrom3(a, n, 1)
#define Wire__requestFrom3(a, n, s)   Wire_requestFrom3(a, n, s)
#define Wire__write(...)              S8_OVERLOAD(Wire__write, __VA_ARGS__)
#define Wire__write1(x)               _Generic((x), char: Wire_write, signed char: Wire_write, \
                                               unsigned char: Wire_write, _Bool: Wire_write, \
                                               int: Wire_write, unsigned int: Wire_write, \
                                               long: Wire_write, unsigned long: Wire_write, \
                                               default: Wire_write_s)(x)
#define Wire__write2(buf, n)          Wire_write_sn((const uint8_t *)(buf), n)
#define Wire__available()             Wire_available()
#define Wire__read()                  Wire_read()
#define Wire__peek()                  Wire_peek()
#define Wire__print(...)              S8_PRINT(Wire_write, __VA_ARGS__)
#define Wire__println(...)            S8_PRINTLN(Wire_write, __VA_ARGS__)
#endif

#endif
