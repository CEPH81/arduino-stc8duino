/*
 * Wire.h - I2C maestro para stc32duino (modulo I2C del STC32G).
 *
 *   Wire.begin();                       // 100 kHz, SCL P1.5 / SDA P1.4
 *   Wire.setPins(1);                    // otras patas (antes de begin), ver abajo
 *   Wire.setClock(400000);
 *   Wire.beginTransmission(0x27);  Wire.write(0x55);  Wire.endTransmission();
 *   n = Wire.requestFrom(0x27, 2);  while (Wire.available()) x = Wire.read();
 *
 * Patas (Wire.setPins):
 *   0: SCL P1.5, SDA P1.4 (por defecto)   1: SCL P2.5, SDA P2.4
 *   2: SCL P7.7, SDA P7.6 (solo LQFP64)   3: SCL P3.2, SDA P3.3
 * Las patas quedan en drenador abierto con la resistencia de pull-up interna;
 * para cables largos o 400 kHz ponga resistencias de 4,7 k a VCC.
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

/* sintaxis estilo Arduino en el sketch: stcar -obj cambia Wire.metodo(...)
 * por Wire__metodo_N(...) (N = numero de argumentos, o S/C/L/U/F) */
#define Wire__begin_0()               Wire_begin()
#define Wire__end_0()                 Wire_end()
#define Wire__setPins_1(o)            Wire_setPins(o)
#define Wire__setClock_1(hz)          Wire_setClock(hz)
#define Wire__beginTransmission_1(a)  Wire_beginTransmission(a)
#define Wire__endTransmission_0()     Wire_endTransmission1(1)
#define Wire__endTransmission_1(s)    Wire_endTransmission1(s)
#define Wire__requestFrom_2(a, n)     Wire_requestFrom3(a, n, 1)
#define Wire__requestFrom_3(a, n, s)  Wire_requestFrom3(a, n, s)
#define Wire__write_1(x)              Wire_write((uint8_t)(x))
#define Wire__write_C(x)              Wire_write((uint8_t)(x))
#define Wire__write_L(x)              Wire_write((uint8_t)(x))
#define Wire__write_U(x)              Wire_write((uint8_t)(x))
#define Wire__write_F(x)              Wire_write((uint8_t)(x))
#define Wire__write_S(s)              Wire_write_s(s)
#define Wire__write_2(b, n)           Wire_write_sn((const uint8_t *)(b), n)
#define Wire__available_0()           Wire_available()
#define Wire__read_0()                Wire_read()
#define Wire__peek_0()                Wire_peek()

#endif
