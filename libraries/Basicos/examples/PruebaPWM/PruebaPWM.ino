/*
 * PruebaPWM - analogWrite() en P3.4 (PWM8, el LED de la STC8H8K64U) con valores fijos.
 *
 * Los valores 0, 4, 16, 64 y 255 se distinguen bien a ojo (el ojo nota mas los
 * cambios en valores bajos). Para medir con el osciloscopio: la frecuencia debe
 * ser de unos 1000 Hz y el pulso en alto, valor/256 del periodo (unos 3,9 us
 * por unidad; 255 = siempre en alto).
 * Cada valor dura 3 s y se anuncia por el monitor serie (115200 baudios).
 */
#define PATA P3_4

const int valores[] = { 0, 4, 16, 64, 255 };

void setup() {
  Serial.begin(115200);
  Serial.println("stc8duino: PruebaPWM en P3.4");
}

void loop() {
  int i;
  for (i = 0; i < 5; i++) {
    analogWrite(PATA, valores[i]);
    Serial.print("analogWrite(P3_4, ");
    Serial.print(valores[i]);
    Serial.print(")  pulso en alto ");
    Serial.print((long)valores[i] * 3906 / 1000);
    Serial.println(" us de 1000");
    delay(3000);
  }
}
