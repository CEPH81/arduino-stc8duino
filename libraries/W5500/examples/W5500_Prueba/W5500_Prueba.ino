/*
 * W5500_Prueba - comprueba el modulo W5500: lo inicia con una IP fija y
 * muestra cada 2 s si el cable de red esta conectado (monitor a 115200).
 * Despues pruebe desde el PC:  ping 192.168.1.177
 *
 * Conexiones: VCC 3,3 V, GND, CS -> P2.2, MISO -> P1.4, MOSI -> P1.3,
 *   SCLK -> P1.5, RST -> 3,3 V.
 * Cambie IP, PUERTA y MASCARA segun su red (la IP no debe estar en uso).
 */
#include <SPI.h>
#include <W5500.h>

#define PIN_CS P2_2

uint8_t mac[6]     = { 0x02, 0x53, 0x54, 0x43, 0x00, 0x01 };
uint8_t ip[4]      = { 192, 168, 1, 177 };
uint8_t puerta[4]  = { 192, 168, 1, 1 };
uint8_t mascara[4] = { 255, 255, 255, 0 };

void mostrarIP(const uint8_t *d) {
  Serial.print(d[0]); Serial.print(".");
  Serial.print(d[1]); Serial.print(".");
  Serial.print(d[2]); Serial.print(".");
  Serial.println(d[3]);
}

void setup() {
  uint8_t leida[4];
  Serial.begin(115200);
  Serial.println("W5500_Prueba");
  SPI.begin();
  if (w5500_begin(PIN_CS, mac, ip, puerta, mascara) != 0) {
    Serial.println("el W5500 no responde: revise VCC (3,3 V), CS, MISO, MOSI y SCLK");
    return;
  }
  Serial.println("W5500 encontrado (version 4)");
  w5500_getIP(leida);
  Serial.print("IP: ");
  mostrarIP(leida);
}

void loop() {
  Serial.println(w5500_linkUp() ? "cable de red conectado" : "sin cable de red");
  delay(2000);
}
