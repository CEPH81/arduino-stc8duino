/*
 * HX711_Crudo - muestra el valor crudo del HX711 (24 bits con signo) unas
 * 10 veces por segundo. Sirve para comprobar el cableado: al apretar la
 * celda el numero debe cambiar.
 *
 * Conexiones: VCC 5 V, GND, DT -> P3.2, SCK -> P3.3.
 */
#include <HX711.h>

#define PIN_DT  P3_2
#define PIN_SCK P3_3

void setup() {
  Serial.begin(115200);
  Serial.println("HX711_Crudo");
  hx711_begin(PIN_DT, PIN_SCK, HX711_128);
}

void loop() {
  long v = hx711_read();
  Serial.println(v);
}
