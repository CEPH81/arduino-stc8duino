/*
 * Boton - el LED se enciende mientras el pulsador esta apretado.
 *
 * Pulsador entre la pata y GND, con la resistencia de pull-up interna
 * (INPUT_PULLUP): suelto se lee HIGH y apretado LOW.
 * En la STC8H8K64U el pulsador esta en P3.3 y el LED en P3.4.
 */
#ifdef BOTON_BUILTIN
#define BOTON BOTON_BUILTIN
#else
#define BOTON P3_3          /* cambielo por la pata de su pulsador */
#endif

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BOTON, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(BOTON) == LOW) {   /* apretado */
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
