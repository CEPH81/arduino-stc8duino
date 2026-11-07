/*
 * PruebaTiempos - comprueba millis(), micros(), delay() y delayMicroseconds()
 * por el monitor serie (115200 baudios).
 *
 * Cada medicion incluye lo que tarda una llamada a micros(): la primera linea
 * ("micros() solo") muestra ese tiempo, que hay que restar a las demas.
 *
 * Despues de cada medicion saca por P1_1 una onda de unos 1 kHz durante 1 s
 * (500 us en alto y 500 us en bajo) para medirla con el osciloscopio.
 */
#define PATA_ONDA P1_1

void medir(const char *nombre, unsigned long t0, unsigned long t1) {
  Serial.print(nombre);
  Serial.print(": ");
  Serial.print(t1 - t0);
  Serial.println(" us");
}

void setup() {
  pinMode(PATA_ONDA, OUTPUT);
  Serial.begin(115200);
  Serial.print("stc32duino: PruebaTiempos, compilado ");
  Serial.println(__TIME__);
}

void loop() {
  unsigned long t0, t1;
  unsigned int i;

  t0 = micros(); t1 = micros();
  medir("micros() solo (se resta a las demas)", t0, t1);

  t0 = micros(); delay(100); t1 = micros();
  medir("delay(100), esperado 100000", t0, t1);

  t0 = micros(); delayMicroseconds(1000); t1 = micros();
  medir("delayMicroseconds(1000), esperado 1000", t0, t1);

  t0 = micros(); delayMicroseconds(50); t1 = micros();
  medir("delayMicroseconds(50), esperado 50", t0, t1);

  t0 = millis(); delay(1000); t1 = millis();
  Serial.print("delay(1000) medido con millis: ");
  Serial.print(t1 - t0);
  Serial.println(" ms");

  for (i = 0; i < 1000; i++) {         /* 1 s de onda de ~1 kHz en P1_1 */
    digitalWrite(PATA_ONDA, HIGH);
    delayMicroseconds(500);
    digitalWrite(PATA_ONDA, LOW);
    delayMicroseconds(500);
  }
  Serial.println();
}
