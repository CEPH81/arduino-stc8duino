/*
 * NRF24_Emisor - envia cada segundo un paquete de 8 bytes con una cuenta y el
 * tiempo encendido, y muestra si el receptor lo confirmo. Usar con
 * NRF24_Receptor en otra placa (STC8H o STC32G, da igual).
 *
 * Conexiones: VCC 3,3 V (con un condensador de 10-100 uF), GND,
 *   CE -> P2.1   CSN -> P2.2   SCK -> P1.5   MOSI -> P1.3   MISO -> P1.4
 *
 * Los numeros se envian byte por byte (el menos significativo primero): el
 * STC32G y el STC8H guardan los long en distinto orden en memoria.
 */
#include <SPI.h>
#include <NRF24.h>

#define PIN_CE  P2_1
#define PIN_CSN P2_2

const uint8_t direccion[5] = { 'S', 'T', 'C', '0', '1' };
unsigned long cuenta = 0;

void poner32(uint8_t *p, unsigned long v) {
  p[0] = (uint8_t)v;
  p[1] = (uint8_t)(v >> 8);
  p[2] = (uint8_t)(v >> 16);
  p[3] = (uint8_t)(v >> 24);
}

void setup() {
  Serial.begin(115200);
  Serial.println("NRF24_Emisor");
  SPI.begin();
  if (nrf24_begin(PIN_CE, PIN_CSN) != 0) {
    Serial.println("el modulo no responde: revise las conexiones");
  }
  nrf24_setChannel(76);
  nrf24_setPayloadSize(8);
  nrf24_openWritingPipe(direccion);
  nrf24_stopListening();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  uint8_t paquete[8];
  cuenta++;
  poner32(paquete, cuenta);
  poner32(paquete + 4, millis());
  Serial.print("envio ");
  Serial.print(cuenta);
  if (nrf24_write(paquete, 8)) {
    Serial.println(": recibido");
    digitalToggle(LED_BUILTIN);
  } else {
    Serial.println(": sin respuesta del receptor");
  }
  delay(1000);
}
