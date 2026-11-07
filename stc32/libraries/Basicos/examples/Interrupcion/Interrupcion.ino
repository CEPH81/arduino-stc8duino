/*
 * Interrupcion - un pulsador a GND cambia el LED usando una interrupcion externa.
 *
 * Interrupciones externas: INT0 = P3.2, INT1 = P3.3. El numero se saca de la pata
 * con digitalPinToInterrupt(), asi basta con cambiar PULSADOR.
 * En la placa negra (LQFP64) y en la verde (S4) el pulsador esta en P3.3 (INT1).
 */
#ifdef BOTON_BUILTIN
#define PULSADOR BOTON_BUILTIN   /* P3.3 en las placas STC32G */
#else
#define PULSADOR P3_2            /* P3_2 (INT0) o P3_3 (INT1) */
#endif

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
    delay(30);           /* antirrebote sencillo */
    pulsado = 0;
    digitalToggle(LED_BUILTIN);
    Serial.println("pulsado");
  }
}
