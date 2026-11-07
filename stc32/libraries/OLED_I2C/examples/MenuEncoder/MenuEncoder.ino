/*
 * MenuEncoder - menu en una pantalla OLED 128x64 manejado con el modulo
 * "OLED + encoder" (CON SDA SCL PSH TRA TRB BAK GND VCC):
 *   girar el encoder          mueve la seleccion / cambia el valor
 *   CONFIRM o apretar la perilla   entra a editar / guarda el valor
 *   BACK                      sale sin guardar
 * "Brillo LED" cambia en vivo el brillo de un LED en P3.4 (PWM8; es el LED de la
 * placa negra; en la verde conecte un LED con resistencia a P3.4).
 * Cada accion se muestra tambien por el monitor serie (115200 baudios).
 *
 * Conexiones del modulo (cambie los #define si usa otras patas):
 *   VCC -> 3.3 V (el modulo indica 3V3)   GND -> GND
 *   SDA -> P1.4    SCL -> P1.5            (Wire.setPins para otras)
 *   TRA -> P2.0    TRB -> P2.1            (encoder A y B: las dos en el puerto P2)
 *   PSH -> P2.2    CON -> P2.3    BAK -> P2.4
 * Los pulsadores y el encoder llevan la pata a GND (se usa el pull-up interno).
 * Si cada clic del encoder avanza 2 (o medio paso), cambie PASOS_POR_CLIC.
 */
#include <Wire.h>
#include <OLED_I2C.h>

#define OLED_DIR  0x3C
#define TIPO      OLED_SSD1306     /* o OLED_SH1106 */

#define PIN_TRA   P2_0
#define PIN_TRB   P2_1
#define PIN_PSH   P2_2
#define PIN_CON   P2_3
#define PIN_BAK   P2_4
#define PASOS_POR_CLIC 4

#define LED_BRILLO P3_4           /* PWM8 (no usar P2.0: es TRA del encoder) */
#define N_OPCIONES 4

/* ---------- encoder: se lee cada 1 ms en la interrupcion del temporizador 1 ---------- */
volatile int8_t clics = 0;        /* clics acumulados (+ derecha, - izquierda) */
uint8_t enc_antes = 0;
int8_t enc_suma = 0;
const int8_t tabla_enc[16] = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0 };

void leerEncoder(void) {
  /* lectura directa del puerto P2 (no llamar a digitalRead dentro de la interrupcion) */
  uint8_t p = P2;
  uint8_t ab = ((p & digitalPinToBitMask(PIN_TRA)) ? 2 : 0) | ((p & digitalPinToBitMask(PIN_TRB)) ? 1 : 0);
  enc_suma += tabla_enc[(enc_antes << 2) | ab];
  enc_antes = ab;
  if (enc_suma >= PASOS_POR_CLIC) { clics++; enc_suma = 0; }
  else if (enc_suma <= -PASOS_POR_CLIC) { clics--; enc_suma = 0; }
}

int8_t tomarClics(void) {
  int8_t n;
  noInterrupts();
  n = clics;
  clics = 0;
  interrupts();
  return n;
}

/* ---------- pulsadores (en loop) ---------- */
uint8_t psh_antes = 1, con_antes = 1, bak_antes = 1;

uint8_t apretado(uint8_t pin, uint8_t *antes) {
  uint8_t v = digitalRead(pin);
  uint8_t flanco = (v == LOW && *antes == HIGH);
  *antes = v;
  return flanco;
}

/* ---------- menu ---------- */
int valores[N_OPCIONES] = { 50, 20, 128, 0 };
int minimo[N_OPCIONES]  = { 0, 0, 0, 0 };
int maximo[N_OPCIONES]  = { 100, 60, 255, 0 };
int paso[N_OPCIONES]    = { 1, 1, 5, 0 };
int8_t sel = 0;
uint8_t editando = 0;
int valor_antes;

void nombre(uint8_t i) {
  /* textos escritos aqui (no en un arreglo de punteros) */
  switch (i) {
  case 0: oled_print(" Velocidad "); break;
  case 1: oled_print(" Tiempo    "); break;
  case 2: oled_print(" Brillo LED"); break;
  default: oled_print(" Version   "); break;
  }
}

void dibujarOpcion(uint8_t i) {
  oled_setCursor(0, (uint8_t)(2 + i));
  oled_setInvert(i == sel && !editando);
  nombre(i);
  oled_print("  ");
  oled_setInvert(i == sel && editando);
  if (i == 3) {
    oled_print("stc32  ");
  } else {
    if (valores[i] < 100) oled_print(" ");
    if (valores[i] < 10) oled_print(" ");
    oled_print(valores[i]);
    oled_print(i == 1 ? " s  " : "    ");
  }
  oled_setInvert(0);
}

void dibujarAyuda(void) {
  oled_clearLine(7);
  oled_setCursor(0, 7);
  if (editando) oled_print("girar:valor  OK  BACK");
  else oled_print("girar:mover  OK:edit");
}

void dibujarTodo(void) {
  uint8_t i;
  oled_clear();
  oled_setInvert(1);
  oled_setCursor(0, 0);
  oled_print("   MENU stc32duino   ");
  oled_setInvert(0);
  for (i = 0; i < N_OPCIONES; i++) dibujarOpcion(i);
  dibujarAyuda();
}

void aplicar(uint8_t i) {
  if (i == 2) analogWrite(LED_BRILLO, valores[2]);
}

void setup() {
  Serial.begin(115200);
  Serial.println("stc32duino: MenuEncoder");
  pinMode(PIN_TRA, INPUT_PULLUP);
  pinMode(PIN_TRB, INPUT_PULLUP);
  pinMode(PIN_PSH, INPUT_PULLUP);
  pinMode(PIN_CON, INPUT_PULLUP);
  pinMode(PIN_BAK, INPUT_PULLUP);
  enc_antes = ((digitalRead(PIN_TRA) ? 2 : 0) | (digitalRead(PIN_TRB) ? 1 : 0));

  Wire.begin();
  if (oled_begin(OLED_DIR, TIPO) != 0) {
    Serial.println("la pantalla no responde: revise la direccion (0x3C o 0x3D)");
  }
  aplicar(2);
  dibujarTodo();
  timer1Begin(1000, leerEncoder);          /* encoder cada 1 ms */
}

void loop() {
  int8_t n = tomarClics();
  uint8_t ok = apretado(PIN_CON, &con_antes) | apretado(PIN_PSH, &psh_antes);
  uint8_t atras = apretado(PIN_BAK, &bak_antes);
  uint8_t antes;

  if (!editando) {
    if (n) {
      antes = sel;
      sel += n;
      while (sel < 0) sel += N_OPCIONES;
      while (sel >= N_OPCIONES) sel -= N_OPCIONES;
      dibujarOpcion(antes);
      dibujarOpcion(sel);
    }
    if (ok && sel != 3) {
      editando = 1;
      valor_antes = valores[sel];
      dibujarOpcion(sel);
      dibujarAyuda();
      Serial.print("editando opcion ");
      Serial.println(sel);
    }
  } else {
    if (n) {
      valores[sel] += n * paso[sel];
      if (valores[sel] < minimo[sel]) valores[sel] = minimo[sel];
      if (valores[sel] > maximo[sel]) valores[sel] = maximo[sel];
      aplicar(sel);
      dibujarOpcion(sel);
    }
    if (ok || atras) {
      if (atras) {                         /* BACK: deshacer */
        valores[sel] = valor_antes;
        aplicar(sel);
        Serial.println("cancelado");
      } else {
        Serial.print("guardado: ");
        Serial.println(valores[sel]);
      }
      editando = 0;
      dibujarOpcion(sel);
      dibujarAyuda();
    }
  }
  delay(10);
}
