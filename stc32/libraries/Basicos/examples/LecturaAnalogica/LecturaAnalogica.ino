/*
 * LecturaAnalogica - lee A0 (P1.0) y muestra el valor y la tension.
 * La referencia del ADC es la alimentacion (VCC_MV: 5000 o 3300 mV).
 * El STC32G tiene ADC de 12 bits: analogReadResolution(12) da 0 a 4095.
 */
#define VCC_MV 5000

void setup() {
  Serial.begin(115200);
  analogReadResolution(10);
}

void loop() {
  int v = analogRead(A0);
  Serial.print("A0 = ");
  Serial.print(v);
  Serial.print("  (");
  Serial.print((long)v * VCC_MV / 1023);
  Serial.println(" mV)");
  delay(500);
}
