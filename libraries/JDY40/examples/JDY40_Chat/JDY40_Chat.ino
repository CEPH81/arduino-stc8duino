/*
 * JDY40_Chat - lo que escriba en el monitor serie (115200) sale por la radio
 * JDY-40 y lo que llega por la radio aparece en el monitor. Con dos placas
 * (STC8H o STC32G, da igual) se puede chatear entre dos monitores.
 *
 * Antes: Herramientas > Serial3 (UART3) > Si.
 * Conexiones: VCC 3,3 V, GND, TXD -> P0.0, RXD -> P0.1, SET -> P2.3, CS -> P2.4
 * (o CS directo a GND y JDY40_SIN_PATA).
 */
#include <JDY40.h>

#define PIN_SET P2_3
#define PIN_CS  P2_4

void setup() {
  Serial.begin(115200);
  Serial.println("JDY40_Chat: escriba y pulse Enter");
  jdy40_begin(PIN_SET, PIN_CS, 9600);
}

void loop() {
  int c;
  while (Serial.available()) {          /* monitor -> radio */
    c = Serial.read();
    Serial3.write(c);
  }
  while (Serial3.available()) {         /* radio -> monitor */
    c = Serial3.read();
    Serial.write(c);
  }
}
