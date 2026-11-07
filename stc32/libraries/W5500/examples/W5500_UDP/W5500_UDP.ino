/*
 * W5500_UDP - recibe mensajes UDP en el puerto 8888, los muestra por el
 * monitor serie (115200) y responde "eco: " + el mensaje al que lo envio.
 * Pruebe desde el PC, por ejemplo con:  echo hola | nc -u -w1 192.168.1.177 8888
 *
 * Conexiones: VCC 3,3 V, GND, CS -> P2.2, MISO -> P1.4, MOSI -> P1.3,
 *   SCLK -> P1.5, RST -> 3,3 V. Cambie IP, PUERTA y MASCARA segun su red.
 */
#include <SPI.h>
#include <W5500.h>

#define PIN_CS P2_2
#define SOCKET 1
#define PUERTO 8888

uint8_t mac[6]     = { 0x02, 0x53, 0x54, 0x43, 0x00, 0x01 };
uint8_t ip[4]      = { 192, 168, 1, 177 };
uint8_t puerta[4]  = { 192, 168, 1, 1 };
uint8_t mascara[4] = { 255, 255, 255, 0 };

uint8_t datos[64];
uint8_t respuesta[70];

void setup() {
  Serial.begin(115200);
  Serial.println("W5500_UDP: puerto 8888");
  SPI.begin();
  if (w5500_begin(PIN_CS, mac, ip, puerta, mascara) != 0) {
    Serial.println("el W5500 no responde: revise las conexiones");
  }
  w5500_udpBegin(SOCKET, PUERTO);
}

void loop() {
  uint8_t origen[4];
  unsigned int puerto, n, i;
  n = w5500_udpReceive(SOCKET, datos, sizeof(datos), origen, &puerto);
  if (n > 0) {
    Serial.print("de ");
    Serial.print(origen[0]); Serial.print(".");
    Serial.print(origen[1]); Serial.print(".");
    Serial.print(origen[2]); Serial.print(".");
    Serial.print(origen[3]); Serial.print(":");
    Serial.print(puerto);
    Serial.print("  ");
    Serial.print(n);
    Serial.println(" bytes");
    respuesta[0] = 'e'; respuesta[1] = 'c'; respuesta[2] = 'o';
    respuesta[3] = ':'; respuesta[4] = ' ';
    for (i = 0; i < n; i++) respuesta[5 + i] = datos[i];
    w5500_udpSend(SOCKET, origen, puerto, respuesta, n + 5);
  }
}
