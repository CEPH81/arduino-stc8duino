/*
 * AT24C32_Prueba - escribe un texto en la EEPROM AT24C32 (cruzando el borde
 * de una pagina a proposito), lo lee y lo compara; despues cuenta cuantas
 * veces se encendio la placa (byte 0).
 *
 * Conexiones (modulo ZS-042 del DS3231): VCC, GND, SCL -> P1.5, SDA -> P1.4.
 * Direccion 0x57 en el ZS-042; un modulo AT24C32 suelto suele estar en 0x50.
 */
#include <Wire.h>
#include <AT24C32.h>

#define DIR 0x57

const char texto[] = "Hola desde stc8duino: prueba de la EEPROM AT24C32";

void setup() {
  char leido[64];
  uint8_t n = sizeof(texto), i, iguales = 1;
  int arranques;
  Serial.begin(115200);
  Serial.println("AT24C32_Prueba");
  Wire.begin();
  if (at24c32_begin(DIR) != 0) {
    Serial.println("la AT24C32 no responde: revise la direccion (0x50-0x57)");
    return;
  }
  /* desde la posicion 20: cruza el borde de la pagina en la 32 */
  if (at24c32_write(20, (const uint8_t *)texto, n) != 0) Serial.println("error al escribir");
  at24c32_read(20, (uint8_t *)leido, n);
  Serial.print("leido: ");
  Serial.println(leido);
  for (i = 0; i < n; i++) if (leido[i] != texto[i]) iguales = 0;
  Serial.println(iguales ? "comparacion: OK" : "comparacion: DISTINTO");

  arranques = at24c32_readByte(0);
  if (arranques == 255) arranques = 0;          /* memoria nueva */
  arranques++;
  at24c32_writeByte(0, (uint8_t)arranques);
  Serial.print("arranques: ");
  Serial.println(arranques);
}

void loop() {
}
