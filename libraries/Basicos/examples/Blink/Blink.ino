/*
 * Blink - enciende y apaga un LED cada medio segundo.
 *
 * LED_BUILTIN: P3.4 en la STC8H8K64U, P5.5 en las demas; cambielo por la pata de su LED,
 * por ejemplo P1_0 (tambien 0x10: las patas se escriben 0x<puerto><bit>).
 */
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
