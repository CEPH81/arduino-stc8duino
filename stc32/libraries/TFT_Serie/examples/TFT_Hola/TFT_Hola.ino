/*
 * TFT_Hola - primera prueba de la pantalla serie: textos en los tres
 * tamanos, colores y un contador de segundos que se actualiza solo.
 *
 * Pantallas: JC018-QQVGA-ST-02-V02 (1.8", 128x160) por omision.
 *            Para la JC22-V05 (2.2", 176x220) quite el // de la linea TFT_JC22.
 * Conexiones:
 *   VCC -> 5 V (o 3.3 V si su modulo lo admite)   GND -> GND
 *   RX de la pantalla -> P1.1 (TX2)    TX de la pantalla -> P1.0 (RX2)
 * El monitor serie (Serial, 115200) sigue en el USB / Link1D como siempre.
 * Serial y pantalla deben ir a la MISMA velocidad (comparten temporizador).
 */
//#define TFT_JC22
#include <TFT_Serie.h>

void setup() {
  Serial.begin(115200);
  Serial.println("stc32duino: TFT_Hola");

  if (tft_begin(115200) == 0) Serial.println("pantalla: responde OK");
  else Serial.println("pantalla: no contesta (revise RX/TX); sigo sin esperar el OK");

  tft_clear(TFT_NEGRO);
  tft_fondo(TFT_NEGRO);
  tft_texto(32, 4, 2, "Hola!", TFT_AMARILLO);
  tft_texto(24, 4, 38, "stc32duino", TFT_CIAN);
  tft_texto(16, 4, 66, "Pantalla serie", TFT_BLANCO);
  tft_texto(16, 4, 86, TFT_MODELO, TFT_VERDE);
  tft_linea(0, 106, TFT_ANCHO - 1, 106, TFT_GRIS);
  tft_texto(16, 4, 112, "Segundos:", TFT_GRIS_CLARO);
}

void loop() {
  /* DCV escribe con fondo: el numero nuevo tapa al anterior */
  tft_textoInicio(24, 4, 132);
  tft_print(millis() / 1000);
  tft_print("   ");
  tft_textoFin(TFT_NARANJA);
  delay(1000);
}
