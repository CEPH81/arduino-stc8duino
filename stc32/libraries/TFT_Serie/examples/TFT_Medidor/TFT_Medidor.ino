/*
 * TFT_Medidor - muestra la lectura del ADC en A3 (P1.3) en la pantalla serie:
 * valor, voltaje y una barra que crece. Solo se redibuja lo que cambia.
 * (A0 y A1 = P1.0 y P1.1 estan ocupadas por la pantalla.)
 *
 * Conecte un potenciometro: extremos a GND y VCC, el cursor a P1.3.
 * Pantallas: JC018-QQVGA-ST-02-V02 (1.8", 128x160) por omision; para la
 * JC22-V05 (2.2", 176x220) quite el // de TFT_JC22.
 * Conexiones: RX de la pantalla -> P1.1, TX de la pantalla -> P1.0, VCC y GND.
 */
//#define TFT_JC22
#include <TFT_Serie.h>

#define VCC_MV  5000                 /* tension de la placa en mV (3300 si es de 3.3 V) */
#define BARRA_H 24
#define BARRA_Y (TFT_ALTO - BARRA_H - 4)

int ultimo = -100;
int largo_antes = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("stc32duino: TFT_Medidor");
  analogReadResolution(10);
  if (tft_begin(115200) != 0) Serial.println("la pantalla no contesta el OK");

  tft_clear(TFT_NEGRO);
  tft_fondo(TFT_NEGRO);
  tft_texto(24, 2, 2, "Medidor A3", TFT_AMARILLO);
  tft_linea(0, 28, TFT_ANCHO - 1, 28, TFT_GRIS);
  tft_texto(16, 2, 32, "ADC (0-1023):", TFT_GRIS_CLARO);
  tft_texto(16, 2, 78, "Voltaje:", TFT_GRIS_CLARO);
  tft_rect(0, BARRA_Y - 1, TFT_ANCHO - 1, BARRA_Y + BARRA_H, TFT_BLANCO);
}

void loop() {
  int v = analogRead(A3);
  int mv, largo;

  if (v > ultimo + 2 || v < ultimo - 2) {       /* ignora el ruido pequeno */
    ultimo = v;
    mv = (long)v * VCC_MV / 1023;

    tft_textoInicio(24, 2, 50);
    tft_print(v);
    tft_print("    ");
    tft_textoFin(TFT_CIAN);

    tft_textoInicio(24, 2, 96);
    tft_print(mv / 1000);
    tft_print(".");
    if (mv % 1000 < 100) tft_print("0");
    if (mv % 1000 < 10) tft_print("0");
    tft_print(mv % 1000);
    tft_print(" V  ");
    tft_textoFin(TFT_VERDE);

    /* barra: solo se pinta o se borra la diferencia */
    largo = (long)v * (TFT_ANCHO - 4) / 1023;
    if (largo > largo_antes)
      tft_rectLleno(2 + largo_antes, BARRA_Y + 1, 2 + largo, BARRA_Y + BARRA_H - 2, v > 900 ? TFT_ROJO : TFT_VERDE);
    else if (largo < largo_antes)
      tft_rectLleno(2 + largo, BARRA_Y + 1, 2 + largo_antes, BARRA_Y + BARRA_H - 2, TFT_NEGRO);
    largo_antes = largo;

    Serial.print("ADC ");
    Serial.println(v);
  }
  delay(100);
}
