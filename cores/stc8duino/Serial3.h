/*
 * Serial3.h - segundo puerto serie (UART3) de STC8H y STC8A, para modulos como
 * el JDY-40 o el DT-06 mientras Serial (UART1) queda para el monitor.
 *
 *   Serial3.begin(9600);      RX3 = P0.0, TX3 = P0.1
 *   Serial3.print("AT\r\n");  if (Serial3.available()) c = Serial3.read();
 *
 * Hay que activarlo en Herramientas > Serial3 (UART3) > Si: su interrupcion
 * (la numero 17) alarga la tabla de interrupciones unos 100 bytes en todos los
 * programas, asi que por omision esta apagado y no ocupa memoria.
 * Usa el temporizador 3, asi que su velocidad es independiente de la de Serial.
 * Recepcion por interrupcion con bufer de 16 bytes; el envio espera a que salga
 * cada byte (a 9600 baudios, 1 ms por byte).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef Serial3_h
#define Serial3_h

#if (defined(STC8H) || defined(STC8A)) && defined(STC_SERIAL3)

#define SERIAL3_RX_BUFFER_SIZE 16   /* fijo: la interrupcion (en ensamblador) usa 0x0F */

void   Serial3_begin(unsigned long baud);
void   Serial3_end(void);
int    Serial3_available(void);
int    Serial3_peek(void);
int    Serial3_read(void);
size_t Serial3_write(uint8_t c);
void   Serial3_flush(void);

#ifdef STC_SKETCH
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

#elif defined(STC_SKETCH)
/* sin activar: el error del enlazador dice que hacer */
void Active_Serial3_en_Herramientas_Serial3_UART3(void);
#define Serial3__begin(baud)  Active_Serial3_en_Herramientas_Serial3_UART3()
#endif
#endif
