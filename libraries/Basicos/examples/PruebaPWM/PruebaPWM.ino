/*
 * PruebaPWM - analogWrite() en P3.4 (PWM8) con valores fijos para medirlos con el
 * osciloscopio. La frecuencia debe ser de unos 1000 Hz y el ancho del pulso en
 * alto, valor/256 del periodo (unos 3,9 us por unidad):
 *   0 -> siempre en bajo   64 -> 25 %   128 -> 50 %   192 -> 75 %   255 -> siempre en alto
 * Cada valor dura 3 s y se anuncia por el monitor serie (115200 baudios).
 */
#define PATA P3_4

const int valores[] = { 0, 64, 128, 192, 255 };

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
    Serial.print(")  ciclo de trabajo ");
    Serial.print((long)valores[i] * 100 / 256);
    Serial.println(" %");
    delay(3000);
  }
}
