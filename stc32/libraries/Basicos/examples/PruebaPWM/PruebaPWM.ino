/*
 * PruebaPWM - analogWrite() en LED_BUILTIN con valores fijos
 * (P3.4 = PWM8 en la placa negra, P2.0 = PWM5 en la verde).
 *
 * Los valores 0, 4, 16, 64 y 255 se distinguen bien a ojo (el ojo nota mas los
 * cambios en valores bajos). Para medir con el osciloscopio: la frecuencia debe
 * ser de unos 1000 Hz y el pulso en alto, valor/256 del periodo (unos 3,9 us
 * por unidad; 255 = siempre en alto).
 * Cada valor dura 3 s y se anuncia por el monitor serie (115200 baudios).
 */
#define PATA LED_BUILTIN

const int valores[] = { 0, 4, 16, 64, 255 };

void setup() {
  Serial.begin(115200);
  Serial.println("stc32duino: PruebaPWM en LED_BUILTIN");
}

void loop() {
  int i;
  for (i = 0; i < 5; i++) {
    analogWrite(PATA, valores[i]);
    Serial.print("analogWrite(LED_BUILTIN, ");
    Serial.print(valores[i]);
    Serial.print(")  pulso en alto ");
    Serial.print((long)valores[i] * 3906 / 1000);
    Serial.println(" us de 1000");
    delay(3000);
  }
}
