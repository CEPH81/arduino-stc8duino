/*
 * BuscarLED - encuentra a que pin esta conectado un LED de la placa.
 *
 * Enciende los pines uno por uno, 1 segundo cada uno (P0.0, P0.1 ... P7.7).
 * Cuando se encienda el LED que busca, mantenga apretado el pulsador de P3.3
 * (S4 en la placa verde) hasta que el LED se quede fijo y suelte.
 * Luego el mismo LED dice el pin con parpadeos:
 *     puerto: N parpadeos  -  pausa  -  bit: M parpadeos   (0 = un parpadeo largo)
 *   ejemplo: P1.5 = 1 parpadeo, pausa, 5 parpadeos.
 * Se repite cada pocos segundos. Reinicie la placa para buscar de nuevo.
 *
 * No toca P3.0/P3.1 (USB y Serial) ni P3.2/P3.3 (pulsadores).
 */

#define BOTON P3_3

uint8_t pin_actual = 0;

uint8_t saltar(uint8_t p) {
  return p == P3_0 || p == P3_1 || p == P3_2 || p == P3_3;
}

void parpadeos(uint8_t pin, uint8_t n) {
  uint8_t i;
  if (n == 0) {                     /* cero: un parpadeo largo */
    digitalWrite(pin, HIGH);
    delay(1200);
    digitalWrite(pin, LOW);
    delay(400);
    return;
  }
  for (i = 0; i < n; i++) {
    digitalWrite(pin, HIGH);
    delay(250);
    digitalWrite(pin, LOW);
    delay(350);
  }
}

void mostrar(uint8_t pin) {
  for (;;) {
    parpadeos(pin, pin >> 4);       /* puerto */
    delay(1200);
    parpadeos(pin, pin & 7);        /* bit */
    delay(3000);
  }
}

void setup() {
  pinMode(BOTON, INPUT_PULLUP);
}

void loop() {
  unsigned long t0;
  uint8_t p = pin_actual;

  if (!saltar(p)) {
    pinMode(p, OUTPUT);
    digitalWrite(p, HIGH);
    t0 = millis();
    while (millis() - t0 < 1000) {
      if (digitalRead(BOTON) == LOW) {   /* el LED esta encendido ahora */
        while (digitalRead(BOTON) == LOW)
          ;
        delay(1000);
        digitalWrite(p, LOW);
        delay(1000);
        mostrar(p);
      }
    }
    digitalWrite(p, LOW);
    pinMode(p, INPUT);
  }
  /* siguiente pin: P0.0 ... P0.7, P1.0 ... P7.7 */
  if ((pin_actual & 7) == 7) pin_actual = (pin_actual & 0xF0) + 0x10;
  else pin_actual++;
  if (pin_actual > P7_7) pin_actual = 0;
}
