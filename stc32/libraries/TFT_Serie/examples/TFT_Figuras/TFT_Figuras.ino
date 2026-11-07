/*
 * TFT_Figuras - recorre los dibujos de la pantalla serie: los 16 colores,
 * lineas, rectangulos, circulos y puntos. Cada pantalla dura 3 segundos.
 *
 * Pantallas: JC018-QQVGA-ST-02-V02 (1.8", 128x160) por omision; para la
 * JC22-V05 (2.2", 176x220) quite el // de TFT_JC22. Todo se ajusta solo.
 * Conexiones: RX de la pantalla -> P1.1, TX de la pantalla -> P1.0, VCC y GND.
 */
//#define TFT_JC22
#include <TFT_Serie.h>

#define ARRIBA 20                      /* debajo del titulo */

uint8_t paso = 0;

void titulo(const char *t) {
  tft_clear(TFT_NEGRO);
  tft_fondo(TFT_NEGRO);
  tft_texto(16, 2, 2, t, TFT_BLANCO);
}

void colores(void) {
  uint8_t c;
  int w = TFT_ANCHO / 4, h = (TFT_ALTO - ARRIBA) / 4;
  int x, y;
  titulo("16 colores");
  for (c = 0; c < 16; c++) {
    x = (c % 4) * w;
    y = ARRIBA + (c / 4) * h;
    tft_rectLleno(x, y, x + w - 2, y + h - 2, c);
    tft_fondo(c);
    tft_textoInicio(16, x + 2, y + 2);
    tft_print(c);
    tft_textoFin(c == TFT_BLANCO || c == TFT_AMARILLO || c == TFT_CIAN ? TFT_NEGRO : TFT_BLANCO);
  }
  tft_fondo(TFT_NEGRO);
}

void lineas(void) {
  int x;
  titulo("Lineas");
  for (x = 0; x < TFT_ANCHO; x += 10) {
    tft_linea(0, ARRIBA, x, TFT_ALTO - 1, TFT_CIAN);
    tft_linea(TFT_ANCHO - 1, ARRIBA, TFT_ANCHO - 1 - x, TFT_ALTO - 1, TFT_AMARILLO);
  }
}

void rectangulos(void) {
  uint8_t i;
  int d = TFT_ANCHO / 18;
  titulo("Rectangulos");
  for (i = 0; i < 8; i++) {
    tft_rect(i * d, ARRIBA + i * d, TFT_ANCHO - 1 - i * d, TFT_ALTO - 1 - i * d, 1 + i);
  }
  tft_rectLleno(TFT_ANCHO / 2 - 10, TFT_ALTO / 2, TFT_ANCHO / 2 + 10, TFT_ALTO / 2 + 20, TFT_ROJO);
}

void circulos(void) {
  uint8_t i;
  int d = TFT_ANCHO / 16;
  int cy = ARRIBA + (TFT_ALTO - ARRIBA) / 2;
  titulo("Circulos");
  for (i = 1; i < 8; i++) {
    tft_circulo(TFT_ANCHO / 2, cy, i * d, 8 + i);
  }
  tft_circuloLleno(TFT_ANCHO / 2, cy, d - 2, TFT_VERDE);
}

void puntos(void) {
  uint8_t i;
  titulo("Puntos al azar");
  for (i = 0; i < 150; i++) {
    tft_punto(random(TFT_ANCHO), ARRIBA + random(TFT_ALTO - ARRIBA), 1 + random(15));
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("stc32duino: TFT_Figuras");
  if (tft_begin(115200) != 0) Serial.println("la pantalla no contesta el OK");
}

void loop() {
  switch (paso) {
  case 0: colores(); break;
  case 1: lineas(); break;
  case 2: rectangulos(); break;
  case 3: circulos(); break;
  default: puntos(); break;
  }
  paso = (paso + 1) % 5;
  delay(3000);
}
