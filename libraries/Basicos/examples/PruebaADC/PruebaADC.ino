/*
 * PruebaADC - lee una entrada analogica con 12 bits y muestra el valor, la
 * tension y cuanto tarda cada lectura (monitor serie a 115200 baudios).
 *
 * Conecte un potenciometro: extremos a GND y VCC, cursor a la pata ENTRADA.
 * La referencia del ADC es la alimentacion: ajuste VCC_MV (5000 o 3300).
 * Entradas del STC8H: A0-A7 = P1.0-P1.7, A8-A14 = P0.0-P0.6.
 */
#define ENTRADA A0
#define VCC_MV  5000

void setup() {
  Serial.begin(115200);
  Serial.println("stc8duino: PruebaADC");
  analogReadResolution(12);          /* 0 a 4095 en STC8H y STC8A */
}

void loop() {
  unsigned long t0, t1;
  int v;

  t0 = micros();
  v = analogRead(ENTRADA);
  t1 = micros();

  Serial.print("valor ");
  Serial.print(v);
  Serial.print("   ");
  Serial.print((long)v * VCC_MV / 4095);
  Serial.print(" mV   lectura en ");
  Serial.print(t1 - t0);
  Serial.println(" us");
  delay(500);
}
