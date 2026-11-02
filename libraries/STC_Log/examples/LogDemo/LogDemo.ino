/*
 * LogDemo - recorre los niveles del log (STC_Log): cada color se muestra 5 s,
 * con un mensaje por segundo que cuenta de 1 a 5, y pasa al siguiente:
 *   E rojo -> W amarillo -> I verde -> D azul -> V gris -> vuelve a empezar
 *
 * Abra un terminal con colores ANSI a 115200 baudios (PuTTY, Tera Term, el
 * monitor serie de VS Code...). En el monitor serie del Arduino IDE los colores
 * salen como codigos raros: cambie COLORES a 0.
 * El LED cambia en cada mensaje.
 */
#include <STC_Log.h>

#define COLORES   1
#define SEGUNDOS  5        /* tiempo de cada color */

static const char *TAG = "DEMO";
static const char *nombres[] = { "", "rojo (error)", "amarillo (aviso)", "verde (info)",
                                 "azul (depuracion)", "gris (detalle)" };
uint8_t datos[20];

void setup() {
  uint8_t i;
  Log_Init(115200);
  Log_SetColors(COLORES);
  Log_SetLevel(LOG_VERBOSE);          /* mostrar todos los niveles */
  pinMode(LED_BUILTIN, OUTPUT);

  LOGI(TAG, "stc8duino: LogDemo, compilado %s %s", __DATE__, __TIME__);
  for (i = 0; i < sizeof(datos); i++) datos[i] = 'A' + i;
  Log_Hex(LOG_INFO, "HEX", datos, sizeof(datos));
}

void loop() {
  LOG_LevelTypeDef nivel;
  uint8_t s;

  for (nivel = LOG_ERROR; nivel <= LOG_VERBOSE; nivel = (LOG_LevelTypeDef)(nivel + 1)) {
    for (s = 1; s <= SEGUNDOS; s++) {
      Log_Write(nivel, TAG, "%s: %u de %u s", nombres[nivel], s, SEGUNDOS);
      digitalToggle(LED_BUILTIN);
      delay(1000);
    }
  }
  LOGI(TAG, "---- vuelta completa, millis = %lu ----", millis());
}
