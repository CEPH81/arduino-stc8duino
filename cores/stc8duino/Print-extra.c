/*
 * Print-extra.c - variantes de Print_* que no usa la sintaxis Serial.print():
 * van aparte para que el enlazador solo las incluya si se llaman.
 * Tomado de Print.c (sduino, LGPL 2.1).
 */
#include "Arduino.h"
#include "Print.h"

#define printNumber	Print_print_ub
#define printInt	Print_print_ib

size_t Print_print_sn(writefunc_p writefunc, const uint8_t *buffer, size_t size)
{
  size_t n = 0;
  while (size--) {
    if (writefunc(*buffer++)) n++;
    else break;
  }
  return n;
}

size_t Print_print_u(writefunc_p writefunc, unsigned long n)
{
	return printNumber(writefunc, n,10);
}

size_t Print_print_i(writefunc_p writefunc, long n)
{
	return printInt(writefunc, n,10);
}

size_t Print_println_sn(writefunc_p writefunc, const uint8_t *buffer, size_t size)
{
	size_t r;

	r = Print_print_sn(writefunc, buffer, size);
	return r + Print_println(writefunc);
}

size_t Print_println_s(writefunc_p writefunc, const char *str)
{
	size_t r;

	r = Print_print_s(writefunc, str);
	return r + Print_println(writefunc);
}

size_t Print_println_u(writefunc_p writefunc, unsigned long n)
{
	size_t r;

	r = printNumber(writefunc, n,10);
	return r + Print_println(writefunc);
}

size_t Print_println_i(writefunc_p writefunc, long n)
{
	size_t r;

	r = printInt(writefunc, n,10);
	return r + Print_println(writefunc);
}

size_t Print_println_ub(writefunc_p writefunc, unsigned long n, uint8_t base)
{
	size_t r;

	r = printNumber(writefunc, n,base);
	return r + Print_println(writefunc);
}

size_t Print_println_ib(writefunc_p writefunc, long n, uint8_t base)
{
	size_t r;

	r = printInt(writefunc, n,base);
	return r + Print_println(writefunc);
}

