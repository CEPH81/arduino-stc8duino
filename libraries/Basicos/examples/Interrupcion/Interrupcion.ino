/*
 * Interrupcion - un pulsador entre P3.2 (INT0) y GND cambia el LED.
 *
 * Interrupciones externas: 0 = P3.2, 1 = P3.3 (FALLING o CHANGE),
 * 2 = P3.6, 3 = P3.7, 4 = P3.0 (solo FALLING).
 */
#define PULSADOR P3_2

volatile uint8_t pulsado = 0;

void alPulsar(void) {
  pulsado = 1;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PULSADOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PULSADOR), alPulsar, FALLING);
}

void loop() {
  if (pulsado) {
    delay(30);           /* antirrebote: se ignoran los rebotes de 30 ms */
    pulsado = 0;
    digitalToggle(LED_BUILTIN);
    Serial.println("pulsado");
  }
}
