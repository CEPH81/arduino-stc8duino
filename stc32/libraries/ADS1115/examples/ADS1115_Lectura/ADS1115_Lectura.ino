/*
 * ADS1115_Lectura - lee las 4 entradas del ADS1115 contra GND cada segundo y
 * las muestra en milivoltios (monitor a 115200).
 *
 * Conexiones: VDD 5 V (o 3,3 V), GND, SCL -> P1.5, SDA -> P1.4, ADDR a GND.
 * Pruebe con un potenciometro: extremos a VDD y GND, el cursor a A0.
 */
#include <Wire.h>
#include <ADS1115.h>

/* imprime microvoltios como milivoltios con 3 decimales */
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
  Serial.println("ADS1115_Lectura");
  Wire.begin();
  if (ads1115_begin(0x48) != 0) {
    Serial.println("el ADS1115 no responde: revise SDA, SCL y ADDR");
  }
  ads1115_setGain(ADS1115_6V144);        /* +-6,144 V: sirve para 0-5 V */
}

void loop() {
  uint8_t c;
  int v;
  for (c = 0; c < 4; c++) {
    v = ads1115_readChannel(c);
    Serial.print("A");
    Serial.print(c);
    Serial.print(": ");
    Serial.print(v);
    Serial.print(" = ");
    imprimirMV(ads1115_toMicrovolts(v));
    Serial.print("   ");
  }
  Serial.println();
  delay(1000);
}
