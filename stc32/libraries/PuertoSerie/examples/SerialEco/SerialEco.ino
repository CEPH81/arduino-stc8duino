/*
 * SerialEco - Serial (RX P3.0, TX P3.1) a 115200 baudios: devuelve lo que recibe
 * y cada segundo imprime el tiempo encendido.
 *
 * Al arrancar muestra la fecha y hora de compilacion: si cambian, el programa
 * nuevo se grabo bien.
 * Monitor serie por el Link1D (TX/RX en P3.0/P3.1).
 */
unsigned long ultimo = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("stc32duino: SerialEco");
  Serial.print("Compilado: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    Serial.write(c);
  }
  if (millis() - ultimo >= 1000) {
    ultimo += 1000;               /* sin acumular retraso */
    Serial.print("encendido hace ");
    Serial.print(ultimo / 1000);
    Serial.println(" s");
  }
}
