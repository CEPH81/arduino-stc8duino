/*
 * MicroSD.h - tarjetas microSD / SD por SPI con FAT16 y FAT32, para
 * stc8duino (STC8H...) y stc32duino (STC32G).
 *
 * Conexiones del modulo (con regulador y adaptador de niveles si la placa es
 * de 5 V; la tarjeta es de 3,3 V):
 *   VCC, GND, CS -> pata a eleccion, MOSI -> P1.3, MISO -> P1.4, SCK -> P1.5.
 * Formatee la tarjeta en FAT32 (o FAT16 si es de 2 GB o menos).
 *
 *   SPI.begin();
 *   sd_begin(CS);                            // 0 si la tarjeta y su FAT estan bien
 *   sd_open("DATOS.TXT", SD_AGREGAR);        // la crea si no existe
 *   sd_print("hola\r\n");  sd_write(buf, n);
 *   sd_close();                              // guarda el largo: no lo olvide
 *   sd_open("DATOS.TXT", SD_LEER);
 *   while ((c = sd_read()) >= 0) Serial.write(c);
 *
 * Limites (para que quepa en 8051): un archivo abierto a la vez, solo en la
 * carpeta raiz, nombres 8.3 (hasta 8 letras + 3 de extension). Usa un bufer
 * de 512 bytes de RAM. Los archivos creados llevan la fecha 2026-01-01.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef MicroSD_h
#define MicroSD_h

#include <Arduino.h>
#include <SPI.h>

/* SDCC: los calculos con long piden variables temporales en la RAM interna
 * directa (DSEG, unos 120 bytes para todo el programa). Con __reentrant van
 * a la pila (que tiene mas espacio) solo mientras la funcion corre. */
#if defined(SDCC) || defined(__SDCC)
#define SD_PILA __reentrant
#else
#define SD_PILA
#endif

/* modos de sd_open */
#define SD_LEER      0     /* leer desde el principio */
#define SD_ESCRIBIR  1     /* crear o vaciar, y escribir desde el principio */
#define SD_AGREGAR   2     /* crear si no existe, y escribir al final */

/* resultados (0 = bien) */
#define SD_OK            0
#define SD_SIN_TARJETA   1     /* no responde: cableado, CS, tension */
#define SD_TARJETA_RARA  2     /* respondio pero no termino de iniciar */
#define SD_SIN_FAT       3     /* no hay FAT16/FAT32 (formatee la tarjeta) */
#define SD_NO_EXISTE     4
#define SD_LLENA         5     /* sin espacio o carpeta raiz llena */
#define SD_ERROR_ES      6     /* error al leer o escribir un sector */
#define SD_NOMBRE_MALO   7

/* tipos de tarjeta y de FAT */
#define SD_TIPO_SD1  1
#define SD_TIPO_SD2  2
#define SD_TIPO_SDHC 3

uint8_t       sd_begin(uint8_t cs) SD_PILA;
uint8_t       sd_cardType(void);           /* SD_TIPO_... (0 si no hay) */
uint8_t       sd_fatType(void);            /* 16 o 32 */
unsigned long sd_cardSize(void);           /* en sectores de 512 bytes */
unsigned long sd_volumeSize(void);         /* MB del volumen FAT */

/* listado de la carpeta raiz: sd_dirStart(); while (sd_dirNext(nombre, &tam)) ... */
void          sd_dirStart(void);
uint8_t       sd_dirNext(char *nombre, unsigned long *tam) SD_PILA;   /* nombre: 13 bytes */

uint8_t       sd_exists(const char *nombre);
uint8_t       sd_open(const char *nombre, uint8_t modo) SD_PILA;
int           sd_read(void);                                  /* -1 al final */
unsigned int  sd_readBuf(void *p, unsigned int n) SD_PILA;
unsigned int  sd_write(const void *p, unsigned int n) SD_PILA;
unsigned int  sd_print(const char *s);
void          sd_printNumber(long v) SD_PILA;
unsigned long sd_size(void);
unsigned long sd_position(void);
uint8_t       sd_flush(void) SD_PILA;              /* guarda sin cerrar */
uint8_t       sd_close(void);
uint8_t       sd_remove(const char *nombre);

/* sectores sueltos (sin FAT) */
uint8_t       sd_readSector(unsigned long n, uint8_t *buf);
uint8_t       sd_writeSector(unsigned long n, const uint8_t *buf);

#endif
