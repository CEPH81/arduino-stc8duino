/*
 * HC4067_Pulsadores - 16 pulsadores con 5 patas: el 74HC4067 recorre los
 * canales y SIG lee cada pulsador (con la resistencia de pull-up interna).
 * Muestra en el monitor cual se pulso o se solto.
 *
 * Conexiones: VCC, GND, EN a GND, SIG -> P1.0,
 *   S0 -> P2.0, S1 -> P2.1, S2 -> P2.2, S3 -> P2.3.
 *   C0-C15: un pulsador entre cada canal y GND.
 */
#include <HC4067.h>

unsigned int antes = 0xFFFF;

void setup() {
  Serial.begin(115200);
  Serial.println("HC4067_Pulsadores");
  hc4067_begin(P2_0, P2_1, P2_2, P2_3, P1_0, HC4067_SIN_EN);
  hc4067_sigMode(INPUT_PULLUP);
}

void loop() {
  uint8_t c;
  unsigned int ahora = 0, cambio;
  for (c = 0; c < 16; c++)
    if (hc4067_digitalRead(c)) ahora |= 1U << c;
  cambio = ahora ^ antes;
  for (c = 0; c < 16; c++) {
    if (cambio & (1U << c)) {
      Serial.print("C");
      Serial.print(c);
      Serial.println((ahora & (1U << c)) ? " suelto" : " pulsado");
    }
  }
  antes = ahora;
  delay(20);
}
