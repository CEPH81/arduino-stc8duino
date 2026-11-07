/*
 * Blink32 - primera prueba del STC32G12K128: parpadea LED_BUILTIN
 * (P3.4 en la placa LQFP64; P2.0 en la LQFP48, con un LED externo).
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
