/*
 * SerialComandos - control por el monitor serie (115200 baudios, sin ajuste de linea
 * o con "Nueva linea"):
 *   1  enciende el LED        0  lo apaga        t  lo cambia
 *   a  lee A0 (P1.0)          m  muestra millis() y micros()
 *   ?  ayuda
 */
void ayuda(void) {
  Serial.println("Comandos: 1 0 t (LED), a (A0), m (tiempo), ? (ayuda)");
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.print("stc8duino: SerialComandos, compilado ");
  Serial.println(__TIME__);
  ayuda();
}

void loop() {
  int c = Serial.read();
  switch (c) {
  case -1:              /* nada recibido */
  case '\r':
  case '\n':
    break;
  case '1':
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("LED encendido");
    break;
  case '0':
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("LED apagado");
    break;
  case 't':
    digitalToggle(LED_BUILTIN);
    Serial.println("LED cambiado");
    break;
  case 'a':
    Serial.print("A0 = ");
    Serial.println(analogRead(A0));
    break;
  case 'm':
    Serial.print("millis = ");
    Serial.print(millis());
    Serial.print("  micros = ");
    Serial.println(micros());
    break;
  default:
    ayuda();
  }
}
