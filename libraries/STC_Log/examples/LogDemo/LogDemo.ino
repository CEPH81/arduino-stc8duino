/*
 * LogDemo - log por niveles con colores (STC_Log).
 *
 * Abra un terminal con colores ANSI a 115200 baudios (PuTTY, Tera Term, el
 * monitor serie de VS Code...). En el monitor serie del Arduino IDE los colores
 * salen como codigos raros: cambie COLORES a 0.
 *
 * Cada 2 s escribe un mensaje de cada nivel; el pulsador (P3.3 en la
 * STC8H8K64U) cambia el nivel minimo que se muestra.
 */
#include <STC_Log.h>

#define COLORES 1
#ifdef BOTON_BUILTIN
#define BOTON BOTON_BUILTIN
#else
#define BOTON P3_3
#endif

static const char *TAG = "DEMO";
LOG_LevelTypeDef nivel = LOG_VERBOSE;
unsigned int vuelta = 0;
uint8_t datos[20];

void setup() {
  uint8_t i;
  Log_Init(115200);
  Log_SetColors(COLORES);
  Log_SetLevel(nivel);
  pinMode(BOTON, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  LOGI(TAG, "stc8duino: LogDemo, compilado %s %s", __DATE__, __TIME__);
  LOGI(TAG, "F_CPU = %lu Hz", (unsigned long)F_CPU);
  for (i = 0; i < sizeof(datos); i++) datos[i] = 'A' + i;
  Log_Hex(LOG_INFO, "HEX", datos, sizeof(datos));
}

void loop() {
  vuelta++;
  LOGE(TAG, "error de ejemplo, vuelta %u", vuelta);
  LOGW(TAG, "aviso: temperatura alta (%d C)", 85);
  LOGI(TAG, "info: millis = %lu", millis());
  LOGD(TAG, "depuracion: 255 = 0x%02X = %b", 255, 255);
  LOGV(TAG, "detalle: vuelta %5u, -42 = %d", vuelta, -42);

  /* pulsador: nivel minimo E -> W -> I -> D -> V -> E... */
  if (digitalRead(BOTON) == LOW) {
    nivel = (nivel >= LOG_VERBOSE) ? LOG_ERROR : (LOG_LevelTypeDef)(nivel + 1);
    Log_SetLevel(nivel);
    LOGE(TAG, "nivel cambiado a %c", "-EWIDV"[nivel]);
    while (digitalRead(BOTON) == LOW) ;
  }
  digitalToggle(LED_BUILTIN);
  delay(2000);
}
