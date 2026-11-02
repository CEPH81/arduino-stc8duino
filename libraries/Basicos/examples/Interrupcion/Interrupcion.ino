/*
 * Interrupcion - un pulsador entre P3.2 (INT0) y GND cambia el LED.
 * attachInterrupt(0, ...) es INT0 en P3.2; attachInterrupt(1, ...) es INT1 en P3.3.
 */
volatile uint8_t pulsado = 0;

void alPulsar(void) {
  pulsado = 1;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(P3_2, INPUT_PULLUP);
  attachInterrupt(0, alPulsar, FALLING);
}

void loop() {
  if (pulsado) {
    pulsado = 0;
    digitalToggle(LED_BUILTIN);
    Serial.println("pulsado");
    delay(50);   // antirrebote sencillo
  }
}
