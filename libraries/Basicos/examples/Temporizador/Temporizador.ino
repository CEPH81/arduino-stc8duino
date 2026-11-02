/*
 * Temporizador - el temporizador 1 llama a una funcion cada 100 ms (interrupcion),
 * sin depender de loop(). Aqui cambia el LED y cuenta los ticks.
 *
 *   timer1Begin(periodo_us, funcion);   timer1End();
 *
 * La funcion corre dentro de la interrupcion: corta, y las variables que use
 * loop() deben ser volatile.
 */
volatile unsigned int ticks = 0;

void cadaTick(void) {
  ticks++;
  digitalToggle(LED_BUILTIN);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("stc8duino: Temporizador (100 ms)");
  timer1Begin(100000UL, cadaTick);
}

void loop() {
  unsigned int t;
  noInterrupts();
  t = ticks;            /* lectura de 16 bits sin que la cambie la interrupcion */
  interrupts();
  Serial.print("ticks = ");
  Serial.println(t);
  delay(1000);
}
