/*
 * JDY40_Config - muestra la configuracion del JDY-40 y permite mandarle
 * comandos AT desde el monitor serie (115200, con "Nueva linea"):
 *   AT+RFC001     canal 001-128          AT+RFID8899  red (0000-FFFF)
 *   AT+DVID1122   equipo (0000-FFFF)     AT+POWE9     potencia 0-9
 *   AT+BAUD4      velocidad (4 = 9600)   AT+CLSSA0    modo transparente
 * Sin parametro (por ejemplo AT+RFC) devuelve el valor actual.
 *
 * Serial3 se activa solo (no hace falta tocar el menu Serial3).
 * Conexiones: VCC 3,3 V, GND, TXD -> P0.0, RXD -> P0.1, SET -> P2.3, CS -> P2.4
 */
#include <JDY40.h>

#define PIN_SET P2_3
#define PIN_CS  P2_4

char linea[24];
uint8_t largo = 0;

void consultar(const char *orden) {
  char r[24];
  Serial.print(orden);
  Serial.print(" -> ");
  if (jdy40_command(orden, r, sizeof(r)) > 0) Serial.println(r);
  else Serial.println("(sin respuesta: revise TXD/RXD y SET)");
}

void setup() {
  Serial.begin(115200);
  Serial.println("JDY40_Config");
  jdy40_begin(PIN_SET, PIN_CS, 9600);
  consultar("AT+BAUD");
  consultar("AT+RFID");
  consultar("AT+DVID");
  consultar("AT+RFC");
  consultar("AT+POWE");
  consultar("AT+CLSS");
  Serial.println("escriba un comando AT:");
}

void loop() {
  int c;
  while (Serial.available()) {
    c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      linea[largo] = 0;
      if (largo > 0) consultar(linea);
      largo = 0;
    } else if (largo < sizeof(linea) - 1) {
      linea[largo++] = (char)c;
    }
  }
}
