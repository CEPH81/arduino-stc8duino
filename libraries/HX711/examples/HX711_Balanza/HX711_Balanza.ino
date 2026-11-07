/*
 * HX711_Balanza - balanza con celda de carga y HX711.
 *
 * Al arrancar hace la tara (sin peso). Para calibrar:
 *   1. Ponga un peso conocido, por ejemplo 500 g.
 *   2. Escriba en el monitor  c500  (c + gramos) y Enter.
 *   3. Anote la escala que muestra y pongala en ESCALA para no repetirlo.
 * Otras ordenes:  t = tara (poner a cero)
 *
 * Conexiones: VCC 5 V, GND, DT -> P3.2, SCK -> P3.3.
 */
#include <HX711.h>

#define PIN_DT  P3_2
#define PIN_SCK P3_3
#define ESCALA  420.0        /* cuentas por gramo: cambiela por la suya */

char linea[12];
uint8_t largo = 0;

long numero(const char *p) {
  long n = 0;
  while (*p >= '0' && *p <= '9') n = n * 10 + (*p++ - '0');
  return n;
}

void orden(void) {
  float escala;
  long gramos, valor;
  if (linea[0] == 't') {
    hx711_tare(10);
    Serial.println("tara hecha");
  } else if (linea[0] == 'c') {
    gramos = numero(linea + 1);
    if (gramos <= 0) { Serial.println("escriba c y los gramos, p. ej. c500"); return; }
    valor = hx711_getValue(20);
    escala = (float)valor / gramos;
    hx711_setScale(escala);
    Serial.print("escala = ");
    Serial.println(escala);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("HX711_Balanza (t = tara, c500 = calibrar con 500 g)");
  hx711_begin(PIN_DT, PIN_SCK, HX711_128);
  hx711_setScale(ESCALA);
  Serial.println("tara... (sin peso)");
  hx711_tare(20);
}

void loop() {
  float g;
  char c;
  while (Serial.available()) {
    c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      linea[largo] = 0;
      if (largo) orden();
      largo = 0;
    } else if (largo < sizeof(linea) - 1) {
      linea[largo++] = c;
    }
  }
  g = hx711_getUnits(5);
  Serial.print("peso: ");
  Serial.print(g, 1);
  Serial.println(" g");
}
