/*
 * PruebaTiempos - comprueba millis(), micros(), delay(), delayMicroseconds()
 * y analogRead(), y saca por P1_1 una onda de 1 kHz (temporizador 1) para medirla
 * con el osciloscopio: si no da 1 kHz, el menu "Reloj" no coincide con el chip.
 */
#define PATA_ONDA P1_1

void onda(void) {
  digitalToggle(PATA_ONDA);
}

void medir(const char *nombre, unsigned long t0, unsigned long t1) {
  Serial.print(nombre);
  Serial.print(": ");
  Serial.print(t1 - t0);
  Serial.println(" us");
}

void setup() {
  pinMode(PATA_ONDA, OUTPUT);
  Serial.begin(115200);
  Serial.print("stc8duino: PruebaTiempos, F_CPU = ");
  Serial.println(F_CPU);
  timer1Begin(500, onda);          /* cambia cada 500 us: 1 kHz */
}

void loop() {
  unsigned long t0, t1;

  t0 = micros(); delay(100); t1 = micros();
  medir("delay(100)            esperado 100000", t0, t1);

  t0 = micros(); delayMicroseconds(1000); t1 = micros();
  medir("delayMicroseconds(1000) esperado 1000", t0, t1);

  t0 = micros(); delayMicroseconds(50); t1 = micros();
  medir("delayMicroseconds(50) esperado 50", t0, t1);

  t0 = micros(); analogRead(A0); t1 = micros();
  medir("analogRead(A0)", t0, t1);

  t0 = micros(); Serial.print("0123456789"); t1 = micros();
  Serial.println();
  medir("Serial.print de 10 bytes", t0, t1);

  Serial.println();
  delay(2000);
}
