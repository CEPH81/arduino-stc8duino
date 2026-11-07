/*
 * PruebaSPI - prueba el SPI sin ningun modulo: una MOSI con MISO (un cable de
 * P1.3 a P1.4) y lo que se envia debe volver igual. Monitor serie a 115200.
 *
 * Sin el cable, MISO queda en 1 (pull-up) y se recibe 0xFF.
 * Para probar otras patas: SPI.setPins(1) y un cable de P2.3 a P2.4.
 */
#include <SPI.h>

void setup() {
  Serial.begin(115200);
  Serial.println("stc32duino: PruebaSPI (cable de MOSI P1.3 a MISO P1.4)");
  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
}

void loop() {
  uint8_t envio[4] = { 0x55, 0xAA, 0x0F, 0xF0 };
  uint8_t i, recibido, errores = 0;
  unsigned long t0, t1;

  for (i = 0; i < 4; i++) {
    recibido = SPI.transfer(envio[i]);
    Serial.print("enviado 0x");
    Serial.print(envio[i], HEX);
    Serial.print("  recibido 0x");
    Serial.println(recibido, HEX);
    if (recibido != envio[i]) errores++;
  }
  t0 = micros();
  for (i = 0; i < 100; i++) SPI.transfer(i);
  t1 = micros();
  Serial.print("100 bytes en ");
  Serial.print(t1 - t0);
  Serial.println(" us");
  if (errores == 0) Serial.println("SPI correcto");
  else Serial.println("hay errores: revise el cable entre MOSI y MISO");
  Serial.println();
  delay(3000);
}
