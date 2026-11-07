/*
 * AD7705_Lectura - lee los dos canales del modulo TM7705 / AD7705 y los
 * muestra en milivoltios (monitor a 115200).
 *
 * Conexiones: VCC 5 V, GND, CS -> P2.2, DRDY -> P2.3, DIN -> P1.3,
 *   DOUT -> P1.4, SCK -> P1.5, RST a VCC (o al pin del modulo si lo trae).
 *   Entradas: AIN1+ y AIN2+ entre 0 y 2,5 V; AIN1- y AIN2- a GND.
 *   Pruebe con un potenciometro: extremos a 2,5 V (o REF del modulo) y GND.
 * Si no conecta DRDY, use AD7705_SIN_DRDY en lugar de PIN_DRDY.
 */
#include <SPI.h>
#include <AD7705.h>

#define PIN_CS   P2_2
#define PIN_DRDY P2_3
#define VREF_MV  2500        /* referencia del modulo TM7705 */

void imprimirMV(long uv) {
  long d;
  if (uv < 0) { Serial.print("-"); uv = -uv; }
  Serial.print(uv / 1000);
  Serial.print(".");
  d = uv % 1000;
  if (d < 100) Serial.print("0");
  if (d < 10) Serial.print("0");
  Serial.print(d);
  Serial.print(" mV");
}

void setup() {
  Serial.begin(115200);
  Serial.println("AD7705_Lectura");
  SPI.begin();
  if (ad7705_begin(PIN_CS, PIN_DRDY, AD7705_50HZ) != 0) {
    Serial.println("el AD7705 no responde: revise CS, DIN, DOUT, SCK y que RST este en alto");
  }
  if (ad7705_setup(0, AD7705_G1, AD7705_UNIPOLAR) != 0) Serial.println("canal 0: no termino la calibracion");
  if (ad7705_setup(1, AD7705_G1, AD7705_UNIPOLAR) != 0) Serial.println("canal 1: no termino la calibracion");
}

void loop() {
  unsigned int a = ad7705_read(0);
  unsigned int b = ad7705_read(1);
  Serial.print("AIN1: ");
  Serial.print(a);
  Serial.print(" = ");
  imprimirMV(ad7705_toMicrovolts(a, VREF_MV, AD7705_G1, AD7705_UNIPOLAR));
  Serial.print("    AIN2: ");
  Serial.print(b);
  Serial.print(" = ");
  imprimirMV(ad7705_toMicrovolts(b, VREF_MV, AD7705_G1, AD7705_UNIPOLAR));
  Serial.println();
  delay(500);
}
