/*
 * Blink - enciende y apaga un LED.
 *
 * LED_BUILTIN: P3.4 en la STC8H8K64U, P5.5 en las demas; cambielo por la pata de su LED,
 * por ejemplo P1_0 (tambien 0x10: las patas se escriben 0x<puerto><bit>).
 * Para comprobar que se grabo el programa nuevo, cambie TIEMPO_MS y vuelva a cargar.
 */
#define TIEMPO_MS 500

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(TIEMPO_MS);
  digitalWrite(LED_BUILTIN, LOW);
  delay(TIEMPO_MS);
}
