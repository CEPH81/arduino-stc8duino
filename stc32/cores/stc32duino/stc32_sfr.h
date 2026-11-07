/*
 * stc32_sfr.h - registros del STC32G12K128 que usa el nucleo (Keil C251).
 * Direcciones tomadas de STC32G.H (STC). Los registros extendidos (XFR,
 * 0x7EFExx) necesitan EAXFR = 1, que pone init().
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef stc32_sfr_h
#define stc32_sfr_h

sfr P0    = 0x80;
sfr P1    = 0x90;
sfr P2    = 0xA0;
sfr P3    = 0xB0;
sfr P4    = 0xC0;
sfr P5    = 0xC8;
sfr P6    = 0xE8;
sfr P7    = 0xF8;

sfr P0M1  = 0x93;
sfr P0M0  = 0x94;
sfr P1M1  = 0x91;
sfr P1M0  = 0x92;
sfr P2M1  = 0x95;
sfr P2M0  = 0x96;
sfr P3M1  = 0xB1;
sfr P3M0  = 0xB2;
sfr P4M1  = 0xB3;
sfr P4M0  = 0xB4;
sfr P5M1  = 0xC9;
sfr P5M0  = 0xCA;
sfr P6M1  = 0xCB;
sfr P6M0  = 0xCC;
sfr P7M1  = 0xE1;
sfr P7M0  = 0xE2;

sfr TCON  = 0x88;
sbit TF0  = TCON^5;
sbit TR0  = TCON^4;
sfr TMOD  = 0x89;
sfr TL0   = 0x8A;
sfr TH0   = 0x8C;
sfr AUXR  = 0x8E;
sfr IE    = 0xA8;
sbit EA   = IE^7;
sbit ET0  = IE^1;
sfr P_SW2 = 0xBA;
sbit EAXFR = P_SW2^7;
sfr WTST  = 0xE9;
sfr CKCON = 0xEA;

/* resistencias de pull-up de cada puerto (XFR) */
#define P0PU (*(unsigned char volatile far *)0x7efe10)
#define P1PU (*(unsigned char volatile far *)0x7efe11)
#define P2PU (*(unsigned char volatile far *)0x7efe12)
#define P3PU (*(unsigned char volatile far *)0x7efe13)
#define P4PU (*(unsigned char volatile far *)0x7efe14)
#define P5PU (*(unsigned char volatile far *)0x7efe15)
#define P6PU (*(unsigned char volatile far *)0x7efe16)
#define P7PU (*(unsigned char volatile far *)0x7efe17)

#endif
