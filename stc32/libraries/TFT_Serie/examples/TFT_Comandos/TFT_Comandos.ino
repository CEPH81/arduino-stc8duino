/*
 * TFT_Comandos - puente entre el monitor serie y la pantalla: lo que escriba
 * en el monitor (terminado en Enter) se envia tal cual a la pantalla y lo que
 * ella conteste aparece en el monitor. Sirve para probar todos sus comandos:
 *
 *   VER;                         version del programa de la pantalla
 *   CLR(3);                      borra en azul
 *   DIR(1);                      horizontal (DIR(0); vertical)
 *   BL(50);                      brillo (ver el manual de su modulo)
 *   DCV24(10,10,'Hola',4);       texto amarillo de 24 puntos
 *   BOXF(0,0,80,40,1);           rectangulo rojo relleno
 *   CIR(64,80,40,2);             circulo verde
 *   PL(0,0,127,159,15);          linea blanca en diagonal (1.8", 128x160)
 *   CLR(0);DCV16(0,0,'dos',15);  varios comandos en una linea (hasta 24)
 *
 * Monitor serie a 115200 con "Nueva linea" o "Ambos NL & CR".
 * Pantalla: RX -> P1.1, TX -> P1.0, VCC y GND.
 */
#include <TFT_Serie.h>

void setup() {
  Serial.begin(115200);
  Serial.println("stc32duino: TFT_Comandos");
  if (tft_begin(115200) == 0) Serial.println("pantalla lista; escriba un comando, p.ej. CLR(1);");
  else Serial.println("la pantalla no contesta: revise que su TX vaya a P1.0");
}

void loop() {
  int c;
  /* monitor -> pantalla */
  while (Serial.available()) {
    c = Serial.read();
    if (c == '\r') continue;                 /* el fin de linea lo pone tft_fin */
    if (c == '\n') {
      tft_write('\r');
      tft_write('\n');
    } else {
      tft_write(c);
    }
  }
  /* pantalla -> monitor */
  c = tft_read();
  if (c >= 0) Serial.write(c);
}
