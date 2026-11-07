/*
 * DT06_Datos - envia por WiFi, cada 2 s, una linea con el tiempo encendido y
 * la lectura de A0 (por ejemplo a un servidor TCP en el PC). Si llega la orden
 * "1" o "0" por la red, enciende o apaga el LED.
 *
 * Antes: Herramientas > Serial3 (UART3) > Si, y configure el modulo en su
 * pagina web (ver DT06.h).
 * Conexiones: VCC 3,3 V, GND, TX del modulo -> P0.0, RX del modulo -> P0.1.
 */
#include <DT06.h>

#define VELOCIDAD DT06_BAUD_FABRICA

unsigned long ultimo = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("DT06_Datos");
  dt06_begin(VELOCIDAD);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  int c;
  if (millis() - ultimo >= 2000) {
    ultimo = millis();
    Serial3.print("t=");
    Serial3.print(ultimo / 1000);
    Serial3.print("s A0=");
    Serial3.println(analogRead(A0));
    Serial.println("datos enviados");
  }
  while (Serial3.available()) {
    c = Serial3.read();
    if (c == '1') digitalWrite(LED_BUILTIN, HIGH);
    if (c == '0') digitalWrite(LED_BUILTIN, LOW);
    Serial.write(c);
  }
}
