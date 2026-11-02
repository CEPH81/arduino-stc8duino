/*
 * HardwareSerial.h - Serial (UART1, TX P3.1 / RX P3.0) para STC8 y STC15.
 *
 * En el sketch: Serial.begin(9600); Serial.println(x); Serial.read()...
 * (ver stc_obj.h). La velocidad la da el temporizador 2 (modo 1T).
 *
 * Carga sin pulsar nada: si el sketch usa Serial, el cargador envia por el
 * puerto la secuencia "@STCISP#" y el chip se reinicia en el modo de carga.
 * Se desactiva con -DSTC_NO_AUTO_ISP (menu "Reinicio automatico").
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <stdint.h>
#include <stddef.h>
#include "Print.h"

#define SERIAL_8N1 0x00

#ifndef SERIAL_RX_BUFFER_SIZE
#define SERIAL_RX_BUFFER_SIZE 32   /* potencia de 2 */
#endif

void HardwareSerial_begin(unsigned long baud);
void HardwareSerial_begin_config(unsigned long baud, uint8_t config);
void HardwareSerial_end(void);
int HardwareSerial_available(void);
int HardwareSerial_peek(void);
int HardwareSerial_read(void);
size_t HardwareSerial_write(uint8_t c);
void HardwareSerial_flush(void);

#include "stc_obj.h"

#ifdef STC_SKETCH
#define Serial__begin(...)          S8_OVERLOAD(Serial__begin, __VA_ARGS__)
#define Serial__begin1(baud)        HardwareSerial_begin(baud)
#define Serial__begin2(baud, cfg)   HardwareSerial_begin_config(baud, cfg)
#define Serial__end()               HardwareSerial_end()
#define Serial__available()         HardwareSerial_available()
#define Serial__peek()              HardwareSerial_peek()
#define Serial__read()              HardwareSerial_read()
#define Serial__flush()             HardwareSerial_flush()
#define Serial__print(...)          S8_PRINT(HardwareSerial_write, __VA_ARGS__)
#define Serial__println(...)        S8_PRINTLN(HardwareSerial_write, __VA_ARGS__)
#define Serial__write(...)          S8_WRITE(HardwareSerial_write, __VA_ARGS__)
/* "if (Serial)" / "while (!Serial)": el puerto siempre esta listo */
#define Serial                      (1)
#endif

#endif
