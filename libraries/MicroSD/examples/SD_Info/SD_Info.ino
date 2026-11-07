/*
 * SD_Info - datos de la tarjeta microSD (tipo, capacidad, FAT) y lista de
 * los archivos de la carpeta raiz.
 *
 * Conexiones del modulo: VCC (5 V si trae regulador, si no 3,3 V), GND,
 *   CS -> P2.2, MOSI -> P1.3, MISO -> P1.4, SCK -> P1.5.
 * Tarjeta formateada en FAT32 (o FAT16 si es de 2 GB o menos).
 */
#include <SPI.h>
#include <MicroSD.h>

#define PIN_CS P2_2

void setup() {
  char nombre[13];
  unsigned long tam;
  uint8_t r, n = 0;
  Serial.begin(115200);
  Serial.println("SD_Info");
  SPI.begin();
  r = sd_begin(PIN_CS);
  if (r == SD_SIN_TARJETA) { Serial.println("no hay tarjeta o no responde: revise el cableado y CS"); return; }
  if (r == SD_TARJETA_RARA) { Serial.println("la tarjeta no termino de iniciar"); return; }
  Serial.print("tarjeta: ");
  r = sd_cardType();
  Serial.println(r == SD_TIPO_SDHC ? "SDHC / SDXC" : r == SD_TIPO_SD2 ? "SD v2" : "SD v1");
  Serial.print("capacidad: ");
  Serial.print(sd_cardSize() / 2048);
  Serial.println(" MB");
  if (sd_fatType() == 0) { Serial.println("sin FAT16/FAT32: formatee la tarjeta"); return; }
  Serial.print("formato: FAT");
  Serial.print(sd_fatType());
  Serial.print(", volumen de ");
  Serial.print(sd_volumeSize());
  Serial.println(" MB");
  Serial.println("archivos:");
  sd_dirStart();
  while (sd_dirNext(nombre, &tam)) {
    Serial.print("  ");
    Serial.print(nombre);
    Serial.print("  ");
    Serial.print(tam);
    Serial.println(" bytes");
    n++;
  }
  if (n == 0) Serial.println("  (vacia)");
}

void loop() {
}
