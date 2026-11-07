/*
 * DT06_Puente - une el monitor serie con el modulo WiFi DT-06: lo que escriba
 * en el monitor (115200) sale por la red, y lo que llegue por la red aparece en
 * el monitor. Sirve para probar la conexion configurada en la pagina web del
 * modulo (http://192.168.4.1) con un programa de TCP en el PC o el telefono.
 *
 * Serial3 se activa solo (no hace falta tocar el menu Serial3).
 * Conexiones: VCC 3,3 V, GND, TX del modulo -> P0.0, RX del modulo -> P0.1.
 * VELOCIDAD: la del puerto serie configurada en el modulo (115200 de fabrica).
 */
#include <DT06.h>

#define VELOCIDAD DT06_BAUD_FABRICA

void setup() {
  Serial.begin(115200);
  Serial.println("DT06_Puente: escriba y pulse Enter");
  dt06_begin(VELOCIDAD);
}

void loop() {
  int c;
  while (Serial.available()) {          /* monitor -> red */
    c = Serial.read();
    Serial3.write(c);
  }
  while (Serial3.available()) {         /* red -> monitor */
    c = Serial3.read();
    Serial.write(c);
  }
}
