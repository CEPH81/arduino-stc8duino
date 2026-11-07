/*
 * HX711.h - amplificador HX711 (24 bits) para celdas de carga, para
 * stc8duino (STC8H...) y stc32duino (STC32G).
 *
 * Conexiones: VCC 5 V (o 3,3 V), GND, DT (DOUT) y SCK -> dos patas a eleccion.
 *   Celda de carga: E+ rojo, E- negro, A- blanco, A+ verde (si el peso sale
 *   negativo, intercambie A+ y A-).
 *
 *   hx711_begin(DT, SCK, HX711_128);
 *   hx711_tare(10);                  // sin peso: el cero
 *   hx711_setScale(420.5);           // cuentas por gramo (ver HX711_Balanza)
 *   g = hx711_getUnits(5);           // promedio de 5 lecturas, en gramos
 *
 * Cada lectura espera a que el HX711 tenga un dato (10 por segundo con RATE
 * a GND, 80 con RATE a VCC). Mientras se leen los 24 bits se apagan las
 * interrupciones (unos 100 us): si SCK queda en alto mas de 60 us el HX711
 * se duerme.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef HX711_h
#define HX711_h

#include <Arduino.h>

#define HX711_128 1      /* canal A, ganancia 128 */
#define HX711_32  2      /* canal B, ganancia 32 */
#define HX711_64  3      /* canal A, ganancia 64 */

void    hx711_begin(uint8_t dt, uint8_t sck, uint8_t ganancia);
uint8_t hx711_ready(void);                 /* 1: hay un dato */
long    hx711_read(void);                  /* crudo, con signo; espera hasta 1 s */
long    hx711_readAverage(uint8_t n);
void    hx711_tare(uint8_t n);             /* el promedio actual pasa a ser el cero */
void    hx711_setOffset(long cero);
long    hx711_getOffset(void);
long    hx711_getValue(uint8_t n);         /* promedio - cero */
void    hx711_setScale(float cuentas_por_unidad);
float   hx711_getScale(void);
float   hx711_getUnits(uint8_t n);         /* (promedio - cero) / escala */
void    hx711_powerDown(void);
void    hx711_powerUp(void);

#endif
