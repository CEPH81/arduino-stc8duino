/*
 * NRF24.h - radio nRF24L01 / nRF24L01+ (2,4 GHz) por SPI, para stc8duino
 * (STC8H, STC8A, STC8G, STC15) y stc32duino (STC32G).
 *
 * Conexiones del modulo (alimentacion 3,3 V; las patas de datos aceptan 5 V):
 *   VCC 3.3V   GND      CE -> pata a eleccion     CSN -> pata a eleccion
 *   SCK -> SCLK         MOSI -> MOSI              MISO -> MISO      IRQ (no se usa)
 *   SPI por defecto: MOSI P1.3, MISO P1.4, SCLK P1.5 (ver SPI.setPins).
 *   Ponga un condensador de 10-100 uF entre VCC y GND del modulo: sin el,
 *   los modulos suelen fallar al transmitir.
 *
 *   SPI.begin();
 *   nrf24_begin(CE, CSN);                 // 0 si el modulo responde
 *   nrf24_setChannel(76);                 // 0-125 (2400 + canal MHz)
 *   nrf24_setPayloadSize(8);              // bytes por paquete, 1-32, igual en los dos
 *   nrf24_openWritingPipe(direccion);     // 5 bytes
 *   nrf24_openReadingPipe(1, direccion);
 *   nrf24_startListening();  if (nrf24_available()) nrf24_read(buf, 8);
 *   nrf24_stopListening();   ok = nrf24_write(buf, 8);   // 1 si llego (con ACK)
 *
 * Por omision: 1 Mbps, potencia maxima, CRC de 16 bits, ACK automatico con
 * hasta 15 reintentos cada 500 us, paquetes de 32 bytes.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef NRF24_h
#define NRF24_h

#include <Arduino.h>
#include <SPI.h>

#define NRF24_250KBPS 2
#define NRF24_1MBPS   0
#define NRF24_2MBPS   1

#define NRF24_PA_MIN  0     /* -18 dBm */
#define NRF24_PA_LOW  1     /* -12 dBm */
#define NRF24_PA_HIGH 2     /*  -6 dBm */
#define NRF24_PA_MAX  3     /*   0 dBm */

uint8_t nrf24_begin(uint8_t ce, uint8_t csn);
uint8_t nrf24_isConnected(void);
void    nrf24_setChannel(uint8_t canal);
void    nrf24_setPayloadSize(uint8_t n);
void    nrf24_setDataRate(uint8_t velocidad);
void    nrf24_setPALevel(uint8_t nivel);
void    nrf24_setRetries(uint8_t espera, uint8_t cuantos);   /* espera: 0-15 (x250 us + 250) */
void    nrf24_openWritingPipe(const uint8_t *dir5);
void    nrf24_openReadingPipe(uint8_t tubo, const uint8_t *dir5); /* tubo 1 (o 2-5: 1 byte distinto) */
void    nrf24_startListening(void);
void    nrf24_stopListening(void);
uint8_t nrf24_write(const void *buf, uint8_t n);     /* 1 si el receptor confirmo */
uint8_t nrf24_available(void);                       /* 1 si hay un paquete */
uint8_t nrf24_availablePipe(void);                   /* tubo del paquete (0-5) o 0xFF */
void    nrf24_read(void *buf, uint8_t n);
uint8_t nrf24_readRegister(uint8_t reg);
void    nrf24_writeRegister(uint8_t reg, uint8_t valor);
void    nrf24_powerDown(void);
void    nrf24_powerUp(void);

#endif
