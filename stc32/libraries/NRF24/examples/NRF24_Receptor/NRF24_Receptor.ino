/*
 * NRF24_Receptor - recibe los paquetes de NRF24_Emisor y los muestra por el
 * monitor serie (115200 baudios). Cambia el LED con cada paquete.
 *
 * Conexiones: VCC 3,3 V (con un condensador de 10-100 uF), GND,
 *   CE -> P2.1   CSN -> P2.2   SCK -> P1.5   MOSI -> P1.3   MISO -> P1.4
 */
#include <SPI.h>
#include <NRF24.h>

#define PIN_CE  P2_1
#define PIN_CSN P2_2

const uint8_t direccion[5] = { 'S', 'T', 'C', '0', '1' };

unsigned long leer32(const uint8_t *p) {
  return (unsigned long)p[0] | ((unsigned long)p[1] << 8) |
         ((unsigned long)p[2] << 16) | ((unsigned long)p[3] << 24);
}

void setup() {
  Serial.begin(115200);
  Serial.println("NRF24_Receptor");
  SPI.begin();
  if (nrf24_begin(PIN_CE, PIN_CSN) != 0) {
    Serial.println("el modulo no responde: revise las conexiones");
  }
  nrf24_setChannel(76);
  nrf24_setPayloadSize(8);
  nrf24_openReadingPipe(1, direccion);
  nrf24_startListening();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  uint8_t paquete[8];
  if (nrf24_available()) {
    nrf24_read(paquete, 8);
    Serial.print("paquete ");
    Serial.print(leer32(paquete));
    Serial.print("  (el emisor lleva ");
    Serial.print(leer32(paquete + 4) / 1000);
    Serial.println(" s encendido)");
    digitalToggle(LED_BUILTIN);
  }
}
