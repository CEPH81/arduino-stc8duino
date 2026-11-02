/*
 * BotonInterrupcion - cada pulsacion cambia el LED, detectada con una
 * interrupcion externa: loop() no tiene que estar leyendo el pulsador.
 *
 * En la STC8H8K64U el pulsador esta en P3.3, que es INT1:
 *   attachInterrupt(digitalPinToInterrupt(P3_3), funcion, FALLING)
 * (P3.2 es INT0). FALLING = al apretar (el pulsador lleva la pata a GND).
 * El antirrebote se hace en loop(): se ignoran las pulsaciones de menos de 50 ms
 * despues de la anterior. Muestra la cuenta por el monitor serie (115200).
 */
#ifdef BOTON_BUILTIN
#define BOTON BOTON_BUILTIN
#else
#define BOTON P3_3          /* P3_2 o P3_3 */
#endif

volatile uint8_t apretado = 0;
unsigned int pulsaciones = 0;
unsigned long ultima = 0;

void alApretar(void) {
  apretado = 1;          /* en la interrupcion solo se anota */
}

void setup() {
  Serial.begin(115200);
  Serial.println("stc8duino: BotonInterrupcion");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BOTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BOTON), alApretar, FALLING);
}

void loop() {
  if (apretado) {
    apretado = 0;
    if (millis() - ultima >= 50) {   /* antirrebote */
      ultima = millis();
      pulsaciones++;
      digitalToggle(LED_BUILTIN);
      Serial.print("pulsacion ");
      Serial.println(pulsaciones);
    }
  }
}
