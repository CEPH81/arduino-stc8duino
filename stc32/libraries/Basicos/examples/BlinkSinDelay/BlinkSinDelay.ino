/*
 * BlinkSinDelay - parpadeo con millis(): loop() nunca se detiene, asi que se
 * pueden hacer otras cosas a la vez (aqui, leer un pulsador entre P3.2 y GND).
 */
#define PERIODO_MS 250
#define PULSADOR   P3_2

unsigned long anterior = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PULSADOR, INPUT_PULLUP);
}

void loop() {
  unsigned long ahora = millis();
  /* con el pulsador apretado parpadea cuatro veces mas rapido */
  unsigned long periodo = digitalRead(PULSADOR) ? PERIODO_MS : PERIODO_MS / 4;

  if (ahora - anterior >= periodo) {
    anterior = ahora;
    digitalToggle(LED_BUILTIN);
  }
}
