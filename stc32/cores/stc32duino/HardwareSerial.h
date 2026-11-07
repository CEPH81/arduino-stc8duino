/*
 * HardwareSerial.h - Serial (UART1: RX P3.0, TX P3.1) del STC32G.
 *
 * En el sketch se escribe como en Arduino: Serial.begin(115200);
 * Serial.print("x = "); Serial.println(x, HEX); Serial.read()...
 * Al compilar, "stcar -obj" cambia Serial.metodo(...) por Serial__metodo_N(...)
 * (N = numero de argumentos, o S/C/L/F segun el tipo): estas macros.
 *
 * Envio y recepcion con bufer de 64 bytes e interrupcion. Usa el temporizador 2
 * para la velocidad. Con "@STCISP#" a la velocidad del programa, el chip se
 * reinicia en el cargador (carga sin apagar la placa).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef HardwareSerial_h
#define HardwareSerial_h

#define SERIAL_RX_BUFFER_SIZE 64
#define SERIAL_TX_BUFFER_SIZE 64

void Serial_begin(unsigned long baud);
void Serial_end(void);
int  Serial_available(void);
int  Serial_peek(void);
int  Serial_read(void);
void Serial_flush(void);
size_t Serial_write(unsigned char c);
size_t Serial_writeStr(const char *s);
size_t Serial_writeBuf(const unsigned char *b, unsigned int n);
size_t Serial_newline(void);

#define Serial__begin_1(b)        Serial_begin(b)
#define Serial__begin_2(b, cfg)   Serial_begin(b)
#define Serial__end_0()           Serial_end()
#define Serial__available_0()     Serial_available()
#define Serial__peek_0()          Serial_peek()
#define Serial__read_0()          Serial_read()
#define Serial__flush_0()         Serial_flush()
#define Serial__availableForWrite_0() (SERIAL_TX_BUFFER_SIZE - 1)

#define Serial__write_1(c)        Serial_write((unsigned char)(c))
#define Serial__write_C(c)        Serial_write((unsigned char)(c))
#define Serial__write_L(c)        Serial_write((unsigned char)(c))
#define Serial__write_S(s)        Serial_writeStr(s)
#define Serial__write_2(b, n)     Serial_writeBuf((const unsigned char *)(b), (n))

#define Serial__print_S(s)        Serial_writeStr(s)
#define Serial__print_C(c)        Serial_write((unsigned char)(c))
#define Serial__print_1(x)        Serial_writeStr(STC_FMT(x, 10, 2))
#define Serial__print_2(x, b)     Serial_writeStr(STC_FMT(x, b, b))
#define Serial__print_L(x)        Serial_writeStr(stc_fmt_long((long)(x), 10))
#define Serial__print_L2(x, b)    Serial_writeStr(stc_fmt_long((long)(x), (b)))
#define Serial__print_F(x)        Serial_writeStr(stc_fmt_float((float)(x), 2))
#define Serial__print_F2(x, d)    Serial_writeStr(stc_fmt_float((float)(x), (d)))

#define Serial__println_0()       Serial_newline()
#define Serial__println_S(s)      (Serial_writeStr(s), Serial_newline())
#define Serial__println_C(c)      (Serial_write((unsigned char)(c)), Serial_newline())
#define Serial__println_1(x)      (Serial__print_1(x), Serial_newline())
#define Serial__println_2(x, b)   (Serial__print_2(x, b), Serial_newline())
#define Serial__println_L(x)      (Serial__print_L(x), Serial_newline())
#define Serial__println_L2(x, b)  (Serial__print_L2(x, b), Serial_newline())
#define Serial__println_F(x)      (Serial__print_F(x), Serial_newline())
#define Serial__println_F2(x, d)  (Serial__print_F2(x, d), Serial_newline())

/* "if (Serial)" / "while (!Serial)": el puerto siempre esta listo */
#define Serial 1

#endif
