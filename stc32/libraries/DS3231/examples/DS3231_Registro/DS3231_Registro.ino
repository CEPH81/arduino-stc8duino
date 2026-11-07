/*
 * DS3231_Registro - cada minuto guarda en la EEPROM AT24C32 del modulo la
 * hora y la temperatura del DS3231 (4 bytes por registro: 1024 registros,
 * algo mas de 17 horas; luego vuelve a empezar).
 *
 * En el monitor (115200):
 *   v   ver todos los registros guardados
 *   b   borrar (empezar de nuevo)
 *
 * Conexiones (modulo ZS-042): VCC, GND, SCL -> P1.5, SDA -> P1.4.
 * Registro: [hora][minuto][temperatura en cuartos de grado, 2 bytes].
 * La posicion del siguiente registro se guarda en los 2 primeros bytes.
 */
#include <Wire.h>
#include <DS3231.h>
#include <AT24C32.h>

#define INICIO  4           /* los registros empiezan aqui */
#define TOTAL   1023        /* (4096 - 4) / 4 */

RtcTiempo t;
unsigned int siguiente;     /* numero del proximo registro */
uint8_t minutoAnterior = 99;

void guardarPosicion(void) {
  uint8_t b[2];
  b[0] = (uint8_t)(siguiente >> 8);
  b[1] = (uint8_t)siguiente;
  at24c32_write(0, b, 2);
}

void dosCifras(uint8_t v) {
  if (v < 10) Serial.print("0");
  Serial.print(v);
}

void ver(void) {
  unsigned int i;
  uint8_t r[4];
  int q;
  Serial.print(siguiente);
  Serial.println(" registros:");
  for (i = 0; i < siguiente; i++) {
    at24c32_read(INICIO + i * 4, r, 4);
    q = (int)((r[2] << 8) | r[3]);
    dosCifras(r[0]);
    Serial.print(":");
    dosCifras(r[1]);
    Serial.print("  ");
    if (q < 0) { Serial.print("-"); q = -q; }
    Serial.print(q / 4);
    Serial.print(".");
    Serial.print((q & 3) * 25);
    Serial.println(" C");
  }
}

void setup() {
  uint8_t b[2];
  Serial.begin(115200);
  Serial.println("DS3231_Registro (v = ver, b = borrar)");
  Wire.begin();
  if (ds3231_begin() != 0) Serial.println("el DS3231 no responde");
  if (at24c32_begin(0x57) != 0) Serial.println("la AT24C32 no responde (pruebe 0x50)");
  if (ds3231_lostPower()) {
    ds3231_compileTime(&t, __DATE__, __TIME__);
    ds3231_set(&t);
  }
  at24c32_read(0, b, 2);
  siguiente = ((unsigned int)b[0] << 8) | b[1];
  if (siguiente > TOTAL) siguiente = 0;        /* memoria nueva (todo en 0xFF) */
  Serial.print("registros guardados: ");
  Serial.println(siguiente);
}

void loop() {
  uint8_t r[4];
  int c;
  if (Serial.available()) {
    c = Serial.read();
    if (c == 'v') ver();
    if (c == 'b') { siguiente = 0; guardarPosicion(); Serial.println("borrado"); }
  }
  if (ds3231_get(&t) == 0 && t.minuto != minutoAnterior) {
    minutoAnterior = t.minuto;
    c = ds3231_temperature() / 25;              /* cuartos de grado */
    r[0] = t.hora;
    r[1] = t.minuto;
    r[2] = (uint8_t)(c >> 8);
    r[3] = (uint8_t)c;
    at24c32_write(INICIO + siguiente * 4, r, 4);
    siguiente++;
    if (siguiente >= TOTAL) siguiente = 0;
    guardarPosicion();
    Serial.print("guardado ");
    dosCifras(t.hora);
    Serial.print(":");
    dosCifras(t.minuto);
    Serial.println();
  }
  delay(200);
}
