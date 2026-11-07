/*
 * W25Q_LeerEscribir - guarda un contador de arranques y un texto en la
 * memoria flash, y los lee de nuevo. Cada arranque usa el siguiente lugar
 * libre del sector 0; cuando se llena, lo borra y empieza otra vez (asi la
 * flash se gasta poco).
 *
 * Conexiones: VCC 3,3 V, GND, CS -> P2.2, DO -> P1.4, DI -> P1.3,
 *   CLK -> P1.5, WP y HOLD a VCC.
 */
#include <SPI.h>
#include <W25Q.h>

#define PIN_CS P2_2
#define TEXTO_DIR 0x1000UL            /* sector 1 */

const char texto[] = "stc8duino: hola desde la memoria flash W25Qxx";

void setup() {
  unsigned int pos;
  uint8_t b[2], cuenta = 0;
  char leido[64];
  Serial.begin(115200);
  Serial.println("W25Q_LeerEscribir");
  SPI.begin();
  if (w25q_begin(PIN_CS) != 0) {
    Serial.println("la memoria no responde");
    return;
  }

  /* contador: el primer byte libre (0xFF) del sector 0 */
  for (pos = 0; pos < 4096; pos++) {
    w25q_read(pos, b, 1);
    if (b[0] == 0xFF) break;
    cuenta = b[0];
  }
  if (pos == 4096) {                    /* sector lleno: borrar */
    w25q_eraseSector(0);
    pos = 0;
  }
  cuenta++;
  if (cuenta == 0xFF) cuenta = 1;
  b[0] = cuenta;
  w25q_write(pos, b, 1);
  Serial.print("arranque numero ");
  Serial.print(cuenta);
  Serial.print(" (guardado en la posicion ");
  Serial.print(pos);
  Serial.println(")");

  /* texto: borrar el sector 1, escribir y leer */
  w25q_eraseSector(TEXTO_DIR);
  w25q_write(TEXTO_DIR, (const uint8_t *)texto, sizeof(texto));
  w25q_read(TEXTO_DIR, (uint8_t *)leido, sizeof(texto));
  Serial.print("texto leido: ");
  Serial.println(leido);
}

void loop() {
}
