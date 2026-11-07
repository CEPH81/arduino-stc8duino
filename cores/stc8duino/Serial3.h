/*
 * Serial3.h - segundo puerto serie (UART3) de STC8H y STC8A, para modulos como
 * el JDY-40 o el DT-06 mientras Serial (UART1) queda para el monitor.
 *
 *   Serial3.begin(9600);      RX3 = P0.0, TX3 = P0.1
 *   Serial3.print("AT\r\n");  if (Serial3.available()) c = Serial3.read();
 *
 * Para usarlo:  #include <UART3.h>  (JDY40 y DT06 ya lo incluyen), o
 * Herramientas > Serial3 (UART3) > Siempre. Los programas que no lo usan no
 * gastan memoria: su interrupcion (la 17) alarga la tabla de vectores ~100
 * bytes, y esa tabla solo la incluye si el sketch lo usa.
 * Usa el temporizador 3, asi que su velocidad es independiente de la de Serial.
 * Recepcion por interrupcion con bufer de 16 bytes; el envio espera a que salga
 * cada byte (a 9600 baudios, 1 ms por byte).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#if (defined(STC8H) || defined(STC8A)) && defined(STC_SERIAL3)
/* La guarda va dentro del #if: UART3.h define STC_SERIAL3 y vuelve a incluir
 * este archivo despues de Arduino.h. */
#ifndef Serial3_h
#define Serial3_h

#define SERIAL3_RX_BUFFER_SIZE 16   /* fijo: la interrupcion (en ensamblador) usa 0x0F */

void   Serial3_begin(unsigned long baud);
void   Serial3_end(void);
int    Serial3_available(void);
int    Serial3_peek(void);
int    Serial3_read(void);
size_t Serial3_write(uint8_t c);
void   Serial3_flush(void);

#ifdef STC_SKETCH
#undef Serial3__begin                       /* quita el aviso de abajo */
#define Serial3__begin(baud)        Serial3_begin(baud)
#define Serial3__end()              Serial3_end()
#define Serial3__available()        Serial3_available()
#define Serial3__peek()             Serial3_peek()
#define Serial3__read()             Serial3_read()
#define Serial3__flush()            Serial3_flush()
#define Serial3__print(...)         S8_PRINT(Serial3_write, __VA_ARGS__)
#define Serial3__println(...)       S8_PRINTLN(Serial3_write, __VA_ARGS__)
#define Serial3__write(...)         S8_WRITE(Serial3_write, __VA_ARGS__)
#endif
#endif /* Serial3_h */

#elif defined(STC_SKETCH) && !defined(Serial3__begin)
/* sin activar: el error del enlazador dice que hacer */
void Serial3_requiere_include_UART3_h(void);
#define Serial3__begin(baud)  Serial3_requiere_include_UART3_h()
#endif
