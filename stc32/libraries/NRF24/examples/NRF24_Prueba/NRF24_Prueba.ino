/*
 * NRF24_Prueba - comprueba la conexion con el modulo nRF24L01 y muestra sus
 * registros por el monitor serie (115200 baudios), cada 3 s.
 *
 * Conexiones: VCC 3,3 V (con un condensador de 10-100 uF), GND,
 *   CE -> P2.1   CSN -> P2.2   SCK -> P1.5   MOSI -> P1.3   MISO -> P1.4
 */
#include <SPI.h>
#include <NRF24.h>

#define PIN_CE  P2_1
#define PIN_CSN P2_2

void mostrar(const char *nombre, uint8_t reg) {
  uint8_t v = nrf24_readRegister(reg);
  Serial.print(nombre);
  Serial.print(" = 0x");
  if (v < 16) Serial.print("0");
  Serial.println(v, HEX);
}

void setup() {
  Serial.begin(115200);
  Serial.println("NRF24_Prueba");
  SPI.begin();
}

void loop() {
  if (nrf24_begin(PIN_CE, PIN_CSN) == 0) {
    Serial.println("modulo nRF24L01 conectado");
  } else {
    Serial.println("el modulo no responde: revise VCC (3,3 V), CSN, SCK, MOSI y MISO");
  }
  mostrar("CONFIG     (0x0E)", 0x00);
  mostrar("EN_AA      (0x3F)", 0x01);
  mostrar("SETUP_AW   (0x03)", 0x03);
  mostrar("SETUP_RETR (0x5F)", 0x04);
  mostrar("RF_CH      (0x4C)", 0x05);
  mostrar("RF_SETUP   (0x07)", 0x06);
  mostrar("STATUS     (0x0E)", 0x07);
  Serial.println();
  delay(3000);
}
