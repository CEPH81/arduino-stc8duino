/*
 * DS3231_Hora - muestra cada segundo la fecha, la hora y la temperatura del
 * DS3231. Si el reloj se detuvo (pila agotada o primera vez) lo pone con la
 * hora en que se compilo el programa.
 *
 * En el monitor (115200, con "Nueva linea"):
 *   h                         vuelve a tomar la hora de la compilacion
 *   2026-11-07 13:45:00       pone esa fecha y hora
 *
 * Conexiones (modulo ZS-042): VCC, GND, SCL -> P1.5, SDA -> P1.4.
 */
#include <Wire.h>
#include <DS3231.h>

const char *dias[] = { "", "lunes", "martes", "miercoles", "jueves", "viernes", "sabado", "domingo" };
char linea[24];
uint8_t largo = 0;
RtcTiempo t;

void dosCifras(uint8_t v) {
  if (v < 10) Serial.print("0");
  Serial.print(v);
}

void mostrar(void) {
  int c;
  const char *nombre;
  if (ds3231_get(&t) != 0) {
    Serial.println("el DS3231 no responde");
    return;
  }
  nombre = dias[t.diaSemana % 8];
  Serial.print(nombre);
  Serial.print(" ");
  Serial.print(t.anio);
  Serial.print("-");
  dosCifras(t.mes);
  Serial.print("-");
  dosCifras(t.dia);
  Serial.print(" ");
  dosCifras(t.hora);
  Serial.print(":");
  dosCifras(t.minuto);
  Serial.print(":");
  dosCifras(t.segundo);
  c = ds3231_temperature();
  Serial.print("   ");
  if (c < 0) { Serial.print("-"); c = -c; }
  Serial.print(c / 100);
  Serial.print(".");
  dosCifras((uint8_t)(c % 100));
  Serial.println(" C");
}

uint8_t num(const char *p) { return (uint8_t)((p[0] - '0') * 10 + (p[1] - '0')); }

void orden(void) {
  if (linea[0] == 'h') {
    ds3231_compileTime(&t, __DATE__, __TIME__);
  } else if (largo >= 19) {                     /* AAAA-MM-DD HH:MM:SS */
    t.anio = 2000 + num(linea + 2);
    t.mes = num(linea + 5);
    t.dia = num(linea + 8);
    t.hora = num(linea + 11);
    t.minuto = num(linea + 14);
    t.segundo = num(linea + 17);
  } else {
    Serial.println("escriba h o AAAA-MM-DD HH:MM:SS");
    return;
  }
  ds3231_set(&t);
  Serial.println("hora ajustada");
}

void setup() {
  Serial.begin(115200);
  Serial.println("DS3231_Hora");
  Wire.begin();
  if (ds3231_begin() != 0) Serial.println("el DS3231 no responde: revise SDA y SCL");
  if (ds3231_lostPower()) {
    Serial.println("el reloj estaba detenido: se pone la hora de la compilacion");
    ds3231_compileTime(&t, __DATE__, __TIME__);
    ds3231_set(&t);
  }
}

void loop() {
  static unsigned long antes = 0;
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
  if (millis() - antes >= 1000) {
    antes = millis();
    mostrar();
  }
}
