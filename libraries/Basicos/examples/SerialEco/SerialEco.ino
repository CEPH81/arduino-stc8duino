/*
 * SerialEco - Serial (RX P3.0, TX P3.1) a 115200 baudios: devuelve lo que recibe
 * y cada segundo imprime el tiempo encendido.
 *
 * Con el menu "Reinicio automatico" en "Si, con Serial a 115200", la siguiente
 * carga no necesita apagar y encender la placa.
 */
unsigned long ultimo = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("stc8duino: SerialEco");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    Serial.write(c);
  }
  if (millis() - ultimo >= 1000) {
    ultimo = millis();
    Serial.print("encendido hace ");
    Serial.print(ultimo / 1000);
    Serial.println(" s");
  }
}
