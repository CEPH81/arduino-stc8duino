/*
 * SD_Registro - registrador de datos: cada segundo agrega una linea al
 * archivo DATOS.CSV con el tiempo (s) y la lectura de A0 (P1.0). Se abre
 * con Excel o LibreOffice (separado por ';').
 *
 * Escriba 'v' en el monitor para ver el archivo y 'b' para borrarlo.
 * El LED de la placa parpadea en cada escritura.
 *
 * Conexiones: VCC, GND, CS -> P2.2, MOSI -> P1.3, MISO -> P1.4, SCK -> P1.5.
 *   Potenciometro: extremos a VCC y GND, cursor a P1.0.
 * Para sacar la tarjeta sin perder datos, espere a que el LED este apagado:
 * cada linea se guarda con sd_close().
 */
#include <SPI.h>
#include <MicroSD.h>

#define PIN_CS  P2_2
#define ARCHIVO "DATOS.CSV"

uint8_t lista = 0;

void ver(void) {
  int c;
  if (sd_open(ARCHIVO, SD_LEER) != SD_OK) { Serial.println("(no hay datos)"); return; }
  while ((c = sd_read()) >= 0) Serial.write(c);
  sd_close();
}

void crear(void) {
  if (!sd_exists(ARCHIVO)) {
    sd_open(ARCHIVO, SD_ESCRIBIR);
    sd_print("segundos;A0\r\n");
    sd_close();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("SD_Registro (v = ver, b = borrar)");
  pinMode(LED_BUILTIN, OUTPUT);
  SPI.begin();
  if (sd_begin(PIN_CS) != SD_OK) {
    Serial.println("no se pudo usar la tarjeta (pruebe SD_Info)");
    return;
  }
  lista = 1;
  crear();
}

void loop() {
  static unsigned long antes = 0;
  int c;
  if (!lista) return;
  if (Serial.available()) {
    c = Serial.read();
    if (c == 'v') ver();
    if (c == 'b') { sd_remove(ARCHIVO); crear(); Serial.println("borrado"); }
  }
  if (millis() - antes >= 1000) {
    antes = millis();
    digitalWrite(LED_BUILTIN, HIGH);
    if (sd_open(ARCHIVO, SD_AGREGAR) == SD_OK) {
      sd_printNumber((long)(antes / 1000));
      sd_print(";");
      sd_printNumber(analogRead(A0));
      sd_print("\r\n");
      sd_close();
    } else {
      Serial.println("error al escribir");
    }
    digitalWrite(LED_BUILTIN, LOW);
  }
}
