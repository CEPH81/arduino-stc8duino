/* sd_interno.h - entre la capa de la tarjeta (sd_tarjeta.c) y la FAT (sd_fat.c). */
#ifndef sd_interno_h
#define sd_interno_h

uint8_t sd_cardInit(uint8_t cs);

       /* 0, SD_SIN_TARJETA o SD_TARJETA_RARA */

#endif
