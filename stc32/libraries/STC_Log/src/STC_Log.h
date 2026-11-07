/*
 * STC_Log.h - log por niveles con colores ANSI, estilo esp_log.
 * Misma interfaz que CH579_Log y W80x_Log: los programas pasan de una placa a
 * otra cambiando solo el #include.
 *
 *   Log_Init(115200);                 // Serial.begin(115200)
 *   Log_SetLevel(LOG_DEBUG);
 *   LOGI("MAIN", "valor=%d", x);      // [1234][I][MAIN] valor=5   (en verde)
 *
 * Formato: <color>[millis][nivel][tag] mensaje<reset>\r\n
 * Colores: E rojo, W amarillo, I verde, D azul, V gris.
 *
 * Se ven en terminales con ANSI (PuTTY, Tera Term, el monitor serie de VS Code /
 * PlatformIO). El monitor serie del Arduino IDE muestra los codigos de escape
 * tal cual: alli use Log_SetColors(0).
 *
 * Formatos de Log_Write (un printf pequeno, sin float):
 *   %d %i %u %x %X %o %b %c %s %%, con l para long (%ld %lu %lx),
 *   ancho y ceros a la izquierda (%5d %04X %08lx).
 *   En Keil C251 un char (o uint8_t) se pasa como 1 byte, no como int:
 *   conviertalo con (int) para %d, %u, %x o %c:  LOGI(TAG, "%c", (int)letra);
 * No llamar desde interrupciones.
 *
 * Parte de stc8duino (version stc32duino). LGPL 2.1.
 */
#ifndef STC_LOG_H
#define STC_LOG_H

#include <Arduino.h>

typedef enum {
	LOG_NONE = 0,
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG,
	LOG_VERBOSE
} LOG_LevelTypeDef;

void Log_Init(unsigned long baud);
void Log_SetLevel(LOG_LevelTypeDef level);
LOG_LevelTypeDef Log_GetLevel(void);
void Log_SetColors(uint8_t enable);
void Log_Write(LOG_LevelTypeDef level, const char *tag, const char *fmt, ...);
void Log_Hex(LOG_LevelTypeDef level, const char *tag, const void *datos, unsigned int len);

/* C251 no tiene macros con argumentos variables: LOGx son funciones */
void Log_E(const char *tag, const char *fmt, ...);
void Log_W(const char *tag, const char *fmt, ...);
void Log_I(const char *tag, const char *fmt, ...);
void Log_D(const char *tag, const char *fmt, ...);
void Log_V(const char *tag, const char *fmt, ...);
#define LOGE Log_E
#define LOGW Log_W
#define LOGI Log_I
#define LOGD Log_D
#define LOGV Log_V

#endif
