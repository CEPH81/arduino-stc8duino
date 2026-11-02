/*
 * LogADC - usa STC_Log para mostrar una lectura analogica con distintos niveles:
 * verde (I) normal, amarillo (W) cerca de los extremos, rojo (E) en los extremos.
 *
 * Potenciometro: extremos a GND y VCC, cursor a ENTRADA (A0 = P1.0).
 * Terminal con colores ANSI a 115200 baudios (en el monitor del Arduino IDE
 * cambie COLORES a 0).
 */
#include <STC_Log.h>

#define COLORES 1
#define ENTRADA A0
#define VCC_MV  5000

static const char *TAG = "ADC";

void setup() {
  Log_Init(115200);
  Log_SetColors(COLORES);
  Log_SetLevel(LOG_DEBUG);
  analogReadResolution(12);
  LOGI(TAG, "stc8duino: LogADC (0 a 4095)");
}

void loop() {
  int v = analogRead(ENTRADA);
  long mv = (long)v * VCC_MV / 4095;

  if (v < 50 || v > 4045) {
    LOGE(TAG, "valor %4d (%ld mV): en el extremo", v, mv);
  } else if (v < 400 || v > 3695) {
    LOGW(TAG, "valor %4d (%ld mV): cerca del extremo", v, mv);
  } else {
    LOGI(TAG, "valor %4d (%ld mV)", v, mv);
  }
  LOGD(TAG, "en hexadecimal: 0x%03X", v);
  delay(500);
}
