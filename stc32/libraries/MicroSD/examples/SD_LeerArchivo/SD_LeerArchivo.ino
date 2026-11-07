/*
 * SD_LeerArchivo - muestra en el monitor el contenido de un archivo de la
 * tarjeta. Escriba el nombre (por ejemplo LEEME.TXT) y Enter.
 * Si el archivo no existe, lo crea con un texto de prueba.
 *
 * Conexiones: VCC, GND, CS -> P2.2, MOSI -> P1.3, MISO -> P1.4, SCK -> P1.5.
 */
#include <SPI.h>
#include <MicroSD.h>

#define PIN_CS P2_2

char linea[14];
uint8_t largo = 0;

void mostrar(const char *nombre) {
  int c;
  uint8_t r = sd_open(nombre, SD_LEER);
  if (r == SD_NO_EXISTE) {
    Serial.print(nombre);
    Serial.println(" no existe: se crea con un texto de prueba");
    sd_open(nombre, SD_ESCRIBIR);
    sd_print("Archivo creado por stc8duino.\r\nSegunda linea.\r\n");
    sd_close();
    r = sd_open(nombre, SD_LEER);
  }
  if (r != SD_OK) { Serial.println("no se pudo abrir (nombre 8.3: hasta 8 letras + 3)"); return; }
  Serial.print("--- ");
  Serial.print(nombre);
  Serial.print(" (");
  Serial.print(sd_size());
  Serial.println(" bytes) ---");
  while ((c = sd_read()) >= 0) Serial.write(c);
  sd_close();
  Serial.println("--- fin ---");
}

void setup() {
  Serial.begin(115200);
  Serial.println("SD_LeerArchivo: escriba un nombre de archivo y Enter");
  SPI.begin();
  if (sd_begin(PIN_CS) != SD_OK) Serial.println("no se pudo usar la tarjeta (pruebe SD_Info)");
}

void loop() {
  char c;
  while (Serial.available()) {
    c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      linea[largo] = 0;
      if (largo) mostrar(linea);
      largo = 0;
    } else if (largo < sizeof(linea) - 1) {
      linea[largo++] = c;
    }
  }
}
