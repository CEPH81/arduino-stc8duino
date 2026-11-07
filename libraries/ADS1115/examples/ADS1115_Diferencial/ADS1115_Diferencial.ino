/*
 * ADS1115_Diferencial - mide la diferencia A0 - A1 con la escala mas fina
 * (+-256 mV, unos 8 microvoltios por paso): sirve para shunts de corriente,
 * termopares o puentes de galgas.
 *
 * Conexiones: VDD, GND, SCL -> P1.5, SDA -> P1.4, ADDR a GND.
 * Las dos entradas deben estar entre GND y VDD aunque su diferencia sea pequena.
 */
#include <Wire.h>
#include <ADS1115.h>

void setup() {
  Serial.begin(115200);
  Serial.println("ADS1115_Diferencial");
  Wire.begin();
  if (ads1115_begin(0x48) != 0) Serial.println("el ADS1115 no responde");
  ads1115_setGain(ADS1115_0V256);
  ads1115_setRate(ADS1115_8SPS);         /* lento = menos ruido */
}

void loop() {
  int v = ads1115_readDiff(ADS1115_A0_A1);
  long uv = ads1115_toMicrovolts(v);
  Serial.print("A0 - A1 = ");
  Serial.print(v);
  Serial.print("  (");
  Serial.print(uv);
  Serial.println(" uV)");
  delay(500);
}
