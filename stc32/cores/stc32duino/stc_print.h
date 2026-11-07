/*
 * stc_print.h - imprimir numeros (print/println de Serial y demas objetos).
 *
 * C251 no tiene _Generic: el tipo del dato se averigua con expresiones que el
 * compilador resuelve al compilar:
 *   STC_ES_FLOAT(x): ((x)*0 + 1) / 2 vale 0 con enteros y 0,5 con float
 *   STC_ES_UNSIGNED(x): (x)*0 - 1 es positivo solo en unsigned int y long
 * (tools/wrapper/obj251.awk ya separo los textos, los char y las llamadas
 * a funcion, para que x no se evalue varias veces).
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef stc_print_h
#define stc_print_h

const char *stc_fmt_long(long v, unsigned char base);
const char *stc_fmt_ulong(unsigned long v, unsigned char base);
const char *stc_fmt_float(float v, unsigned char decimales);

#define STC_ES_FLOAT(x)    ((((x) * 0) + 1) / 2 != 0)
#define STC_ES_UNSIGNED(x) (((x) * 0 - 1) > 0)

/* x con base (DEC, HEX...) si es entero o con 'dec' decimales si es float */
#define STC_FMT(x, base, dec) \
	(STC_ES_FLOAT(x) ? stc_fmt_float((float)(x), (dec)) : \
	 STC_ES_UNSIGNED(x) ? stc_fmt_ulong((unsigned long)(x), (base)) : \
	 stc_fmt_long((long)(x), (base)))

#endif
