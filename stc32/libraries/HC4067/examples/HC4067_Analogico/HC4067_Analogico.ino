/*
 * HC4067_Analogico - lee las 16 entradas del multiplexor 74HC4067 con una
 * sola pata analogica y las muestra en una tabla cada medio segundo.
 *
 * Conexiones: VCC, GND, EN a GND, SIG -> A0 (P1.0),
 *   S0 -> P2.0, S1 -> P2.1, S2 -> P2.2, S3 -> P2.3.
 *   C0-C15: potenciometros, LDR con resistencia, sensores 0-VCC...
 */
#include <HC4067.h>

void setup() {
  Serial.begin(115200);
  Serial.println("HC4067_Analogico");
  hc4067_begin(P2_0, P2_1, P2_2, P2_3, A0, HC4067_SIN_EN);
}

void loop() {
  uint8_t c;
  for (c = 0; c < 16; c++) {
    Serial.print(hc4067_analogRead(c));
    Serial.print(c == 15 ? "\r\n" : "\t");
  }
  delay(500);
}
