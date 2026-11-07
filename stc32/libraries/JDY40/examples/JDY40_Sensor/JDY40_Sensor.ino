/*
 * JDY40_Sensor - envia por la radio JDY-40, cada segundo, una linea de texto
 * con una cuenta y la lectura de A0. En la otra placa, JDY40_Chat la muestra.
 *
 * Serial3 se activa solo (no hace falta tocar el menu Serial3).
 * Conexiones: VCC 3,3 V, GND, TXD -> P0.0, RXD -> P0.1, SET -> P2.3, CS -> P2.4
 */
#include <JDY40.h>

#define PIN_SET P2_3
#define PIN_CS  P2_4

unsigned int cuenta = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("JDY40_Sensor");
  jdy40_begin(PIN_SET, PIN_CS, 9600);
  if (jdy40_setChannel(1) != 0) Serial.println("el JDY-40 no responde a los comandos AT");
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  int a0 = analogRead(A0);
  cuenta++;
  Serial3.print("sensor ");
  Serial3.print(cuenta);
  Serial3.print(" A0=");
  Serial3.println(a0);
  Serial.print("enviado ");
  Serial.println(cuenta);
  digitalToggle(LED_BUILTIN);
  delay(1000);
}
