/*
 * Serial3.h - segundo puerto serie (UART3) del STC32G, para modulos como el
 * JDY-40 o el DT-06 mientras Serial (UART1) queda para el monitor.
 *
 *   Serial3.begin(9600);      RX3 = P0.0, TX3 = P0.1
 *   Serial3.print("AT\r\n");  if (Serial3.available()) c = Serial3.read();
 *
 * Hay que activarlo en Herramientas > Serial3 (UART3) > Si (asi no ocupa
 * memoria en los programas que no lo usan).
 * Usa el temporizador 3: su velocidad es independiente de la de Serial.
 * Recepcion por interrupcion con bufer de 32 bytes; el envio espera a que
 * salga cada byte (a 9600 baudios, 1 ms por byte).
 *
 * Parte de stc8duino (version stc32duino). LGPL 2.1.
 */
#ifndef Serial3_h
#define Serial3_h

#ifdef STC_SERIAL3

#define SERIAL3_RX_BUFFER_SIZE 32

void   Serial3_begin(unsigned long baud);
void   Serial3_end(void);
int    Serial3_available(void);
int    Serial3_peek(void);
int    Serial3_read(void);
size_t Serial3_write(unsigned char c);
size_t Serial3_writeStr(const char *s);
size_t Serial3_writeBuf(const unsigned char *b, unsigned int n);
void   Serial3_flush(void);

#define Serial3__begin_1(b)        Serial3_begin(b)
#define Serial3__end_0()           Serial3_end()
#define Serial3__available_0()     Serial3_available()
#define Serial3__peek_0()          Serial3_peek()
#define Serial3__read_0()          Serial3_read()
#define Serial3__flush_0()         Serial3_flush()
#define Serial3__write_1(c)        Serial3_write((unsigned char)(c))
#define Serial3__write_C(c)        Serial3_write((unsigned char)(c))
#define Serial3__write_L(c)        Serial3_write((unsigned char)(c))
#define Serial3__write_U(c)        Serial3_write((unsigned char)(c))
#define Serial3__write_S(s)        Serial3_writeStr(s)
#define Serial3__write_2(b, n)     Serial3_writeBuf((const unsigned char *)(b), (n))
#define Serial3__print_S(s)        Serial3_writeStr(s)
#define Serial3__print_C(c)        Serial3_write((unsigned char)(c))
#define Serial3__print_1(x)        Serial3_writeStr(STC_FMT(x, 10, 2))
#define Serial3__print_2(x, b)     Serial3_writeStr(STC_FMT(x, b, b))
#define Serial3__print_L(x)        Serial3_writeStr(stc_fmt_long((long)(x), 10))
#define Serial3__print_L2(x, b)    Serial3_writeStr(stc_fmt_long((long)(x), (b)))
#define Serial3__print_U(x)        Serial3_writeStr(stc_fmt_ulong((unsigned long)(x), 10))
#define Serial3__print_U2(x, b)    Serial3_writeStr(stc_fmt_ulong((unsigned long)(x), (b)))
#define Serial3__print_F(x)        Serial3_writeStr(stc_fmt_float((float)(x), 2))
#define Serial3__print_F2(x, d)    Serial3_writeStr(stc_fmt_float((float)(x), (d)))
#define Serial3__println_0()       Serial3_writeStr("\r\n")
#define Serial3__println_S(s)      (Serial3_writeStr(s), Serial3_writeStr("\r\n"))
#define Serial3__println_C(c)      (Serial3_write((unsigned char)(c)), Serial3_writeStr("\r\n"))
#define Serial3__println_1(x)      (Serial3__print_1(x), Serial3_writeStr("\r\n"))
#define Serial3__println_2(x, b)   (Serial3__print_2(x, b), Serial3_writeStr("\r\n"))
#define Serial3__println_L(x)      (Serial3__print_L(x), Serial3_writeStr("\r\n"))
#define Serial3__println_L2(x, b)  (Serial3__print_L2(x, b), Serial3_writeStr("\r\n"))
#define Serial3__println_U(x)      (Serial3__print_U(x), Serial3_writeStr("\r\n"))
#define Serial3__println_U2(x, b)  (Serial3__print_U2(x, b), Serial3_writeStr("\r\n"))
#define Serial3__println_F(x)      (Serial3__print_F(x), Serial3_writeStr("\r\n"))
#define Serial3__println_F2(x, d)  (Serial3__print_F2(x, d), Serial3_writeStr("\r\n"))

#else
/* sin activar: el error del enlazador dice que hacer */
void Active_Serial3_en_Herramientas_Serial3_UART3(void);
#define Serial3__begin_1(b)  Active_Serial3_en_Herramientas_Serial3_UART3()
#endif

#endif
