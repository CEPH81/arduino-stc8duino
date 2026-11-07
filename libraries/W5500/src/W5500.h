/*
 * W5500.h - Ethernet con el modulo W5500 por SPI, para stc8duino (STC8H...) y
 * stc32duino (STC32G). TCP (servidor y cliente) y UDP con IP fija, 8 sockets.
 *
 * Conexiones (VCC 3,3 V; el modulo consume unos 130 mA; las patas aceptan 5 V):
 *   CS (SCS) -> pata a eleccion   MISO -> P1.4   MOSI -> P1.3   SCLK -> P1.5
 *   RST -> 3,3 V o a una pata (w5500 lo reinicia por programa)   INT (no se usa)
 *   SPI por defecto: ver SPI.setPins.
 *
 *   uint8_t mac[6] = {0x02,0x53,0x54,0x43,0x00,0x01};   // 02:... = MAC local
 *   uint8_t ip[4] = {192,168,1,177}, gw[4] = {192,168,1,1}, mascara[4] = {255,255,255,0};
 *   SPI.begin();
 *   w5500_begin(CS, mac, ip, gw, mascara);              // 0 si el modulo responde
 *   w5500_tcpListen(0, 80);                             // socket 0, puerto 80
 *   if (w5500_status(0) == W5500_ESTABLISHED && w5500_available(0)) ...
 *
 * Los textos y datos van tal cual: no importa el orden de bytes del micro.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef W5500_h
#define W5500_h

#include <Arduino.h>
#include <SPI.h>

/* estado de un socket (registro Sn_SR) */
#define W5500_CLOSED      0x00
#define W5500_INIT        0x13
#define W5500_LISTEN      0x14
#define W5500_ESTABLISHED 0x17
#define W5500_CLOSE_WAIT  0x1C
#define W5500_UDP         0x22

uint8_t  w5500_begin(uint8_t cs, const uint8_t *mac, const uint8_t *ip,
                     const uint8_t *puerta, const uint8_t *mascara);
uint8_t  w5500_linkUp(void);                       /* 1 si el cable esta conectado */
void     w5500_getIP(uint8_t *ip);
uint8_t  w5500_status(uint8_t s);
uint8_t  w5500_tcpListen(uint8_t s, unsigned int puerto);    /* 0 si quedo escuchando */
uint8_t  w5500_tcpConnect(uint8_t s, const uint8_t *ip, unsigned int puerto, unsigned int puerto_local);
uint8_t  w5500_udpBegin(uint8_t s, unsigned int puerto);
unsigned int w5500_available(uint8_t s);           /* bytes recibidos */
unsigned int w5500_read(uint8_t s, uint8_t *buf, unsigned int n);
int      w5500_readByte(uint8_t s);                /* -1 si no hay */
unsigned int w5500_write(uint8_t s, const uint8_t *buf, unsigned int n);
unsigned int w5500_writeStr(uint8_t s, const char *texto);
unsigned int w5500_udpSend(uint8_t s, const uint8_t *ip, unsigned int puerto,
                           const uint8_t *buf, unsigned int n);
unsigned int w5500_udpReceive(uint8_t s, uint8_t *buf, unsigned int max,
                              uint8_t *ip, unsigned int *puerto);
void     w5500_disconnect(uint8_t s);
void     w5500_close(uint8_t s);

#endif
