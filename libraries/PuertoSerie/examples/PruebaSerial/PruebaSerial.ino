/*
 * PruebaSerial - prueba completa de Serial (RX P3.0, TX P3.1) a 115200 baudios.
 *
 * Al arrancar imprime todos los tipos de dato y formatos de Serial.print() y mide
 * cuanto tarda en enviar texto (el envio usa un bufer y no espera a la UART).
 * Despues espera lineas del monitor serie (con "Nueva linea" o "Ambos NL y CR")
 * y responde con lo recibido, su largo y su suma, para comprobar que no se
 * pierden ni se cambian caracteres. Comandos:
 *   t   repite la prueba de tipos       v   prueba de velocidad (1000 bytes)
 */
#define LARGO_MAX 64

char linea[LARGO_MAX + 1];
uint8_t largo = 0;
unsigned int lineas = 0;

void pruebaTipos(void) {
  int i = -1234;
  unsigned int u = 54321;
  long l = -123456789L;
  unsigned long ul = 4000000000UL;
#ifndef STC8G
  float f = 3.14159;   /* los float ocupan unos 3 KB: no caben en el STC8G1K08 (8 KB) */
#endif
  char c = 'A';

  Serial.println("--- Tipos y formatos ---");
  Serial.print("int: ");            Serial.println(i);
  Serial.print("unsigned int: ");   Serial.println(u);
  Serial.print("long: ");           Serial.println(l);
  Serial.print("unsigned long: ");  Serial.println(ul);
#ifndef STC8G
  Serial.print("float: ");          Serial.println(f);
  Serial.print("float, 4 dec.: ");  Serial.println(f, 4);
#endif
  Serial.print("char: ");           Serial.println(c);
  Serial.print("200 en HEX: ");     Serial.println(200, HEX);
  Serial.print("200 en BIN: ");     Serial.println(200, BIN);
  Serial.print("200 en OCT: ");     Serial.println(200, OCT);
  Serial.print("write(65): ");      Serial.write(65);  Serial.println();
  Serial.print("caracteres especiales: ");  Serial.println("\"comillas\" y 'apostrofes' 100%");
}

void pruebaVelocidad(void) {
  unsigned long t0, t1;
  int i;

  Serial.println("--- Velocidad ---");
  Serial.flush();                      /* esperar a que el bufer quede vacio */
  t0 = micros();
  for (i = 0; i < 20; i++) {
    Serial.print("0123456789012345678901234567890123456789012345678");  /* 49 + 1 = 50 */
    Serial.write('\n');
  }
  t1 = micros();
  Serial.flush();
  Serial.print("1000 bytes puestos en el bufer en ");
  Serial.print(t1 - t0);
  Serial.println(" us");
  t1 = micros();
  Serial.print("tiempo total con el envio: ");
  Serial.print(t1 - t0);
  Serial.println(" us (a 115200 baudios, 1000 bytes son unos 86800 us)");
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("stc8duino: PruebaSerial");
  Serial.print("Compilado: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
  pruebaTipos();
  pruebaVelocidad();
  Serial.println("Escriba una linea (t = tipos, v = velocidad):");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c != '\n' && largo < LARGO_MAX) {
      linea[largo++] = c;
      continue;
    }
    linea[largo] = 0;
    if (largo == 1 && linea[0] == 't') {
      pruebaTipos();
    } else if (largo == 1 && linea[0] == 'v') {
      pruebaVelocidad();
    } else {
      unsigned int suma = 0;
      uint8_t i;
      for (i = 0; i < largo; i++) suma += (uint8_t)linea[i];
      lineas++;
      Serial.print("linea ");
      Serial.print(lineas);
      Serial.print(": \"");
      Serial.print(linea);
      Serial.print("\"  largo ");
      Serial.print(largo);
      Serial.print("  suma ");
      Serial.println(suma);
    }
    largo = 0;
  }
}
