/*
 * stc_sfr.h - registros (SFR) de los STC8H, STC8G, STC8A y STC15 para SDCC.
 *
 * Familias (las define boards.txt con -D):
 *   STC8H  STC8H8K64U ...        STC8G  STC8G1K08 ...
 *   STC8A  STC8A8K64D4 ...       STC15  STC15W4K56S4, STC15F2K60S2 ...
 *
 * Los registros extendidos de los STC8 (XSFR, en la memoria xdata desde 0x7EFE00)
 * se usan con P_SW2 |= 0x80 (EAXFR) antes de tocarlos.
 *
 * Parte de stc8duino. LGPL 2.1.
 */
#ifndef STC_SFR_H
#define STC_SFR_H

#if defined(STC8H) || defined(STC8G) || defined(STC8A)
#define STC8 1
#endif

/* ---------- 8051 ---------- */
__sfr __at(0x80) P0;
__sfr __at(0x81) SP;
__sfr __at(0x82) DPL;
__sfr __at(0x83) DPH;
__sfr __at(0x87) PCON;
__sfr __at(0x88) TCON;
__sfr __at(0x89) TMOD;
__sfr __at(0x8A) TL0;
__sfr __at(0x8B) TL1;
__sfr __at(0x8C) TH0;
__sfr __at(0x8D) TH1;
__sfr __at(0x90) P1;
__sfr __at(0x98) SCON;
__sfr __at(0x99) SBUF;
__sfr __at(0xA0) P2;
__sfr __at(0xA8) IE;
__sfr __at(0xB0) P3;
__sfr __at(0xB8) IP;
__sfr __at(0xD0) PSW;
__sfr __at(0xE0) ACC;
__sfr __at(0xF0) B;

/* bits */
__sbit __at(0x88) IT0;
__sbit __at(0x89) IE0;
__sbit __at(0x8A) IT1;
__sbit __at(0x8B) IE1;
__sbit __at(0x8C) TR0;
__sbit __at(0x8D) TF0;
__sbit __at(0x8E) TR1;
__sbit __at(0x8F) TF1;
__sbit __at(0x98) RI;
__sbit __at(0x99) TI;
__sbit __at(0x9C) REN;
__sbit __at(0xA8) EX0;
__sbit __at(0xA9) ET0;
__sbit __at(0xAA) EX1;
__sbit __at(0xAB) ET1;
__sbit __at(0xAC) ES;
__sbit __at(0xAD) EADC;
__sbit __at(0xAF) EA;
__sbit __at(0xB9) PT0;
__sbit __at(0xBC) PS;

/* ---------- STC (comunes a STC8 y STC15) ---------- */
__sfr __at(0x8E) AUXR;
__sfr __at(0x8F) INTCLKO;     /* STC15: INT_CLKO */
__sfr __at(0x91) P1M1;
__sfr __at(0x92) P1M0;
__sfr __at(0x93) P0M1;
__sfr __at(0x94) P0M0;
__sfr __at(0x95) P2M1;
__sfr __at(0x96) P2M0;
__sfr __at(0x97) PCON2;       /* STC15: CLK_DIV */
__sfr __at(0x9A) S2CON;
__sfr __at(0x9B) S2BUF;
__sfr __at(0x9D) P1ASF;       /* STC15: patas analogicas */
__sfr __at(0xA2) P_SW1;
__sfr __at(0xAF) IE2;
__sfr __at(0xB1) P3M1;
__sfr __at(0xB2) P3M0;
__sfr __at(0xB3) P4M1;
__sfr __at(0xB4) P4M0;
__sfr __at(0xB5) IP2;
__sfr __at(0xBA) P_SW2;
__sfr __at(0xBC) ADC_CONTR;
__sfr __at(0xBD) ADC_RES;
__sfr __at(0xBE) ADC_RESL;
__sfr __at(0xC0) P4;
__sfr __at(0xC1) WDT_CONTR;
__sfr __at(0xC2) IAP_DATA;
__sfr __at(0xC3) IAP_ADDRH;
__sfr __at(0xC4) IAP_ADDRL;
__sfr __at(0xC5) IAP_CMD;
__sfr __at(0xC6) IAP_TRIG;
__sfr __at(0xC7) IAP_CONTR;
__sfr __at(0xC8) P5;
__sfr __at(0xC9) P5M1;
__sfr __at(0xCA) P5M0;
__sfr __at(0xCB) P6M1;
__sfr __at(0xCC) P6M0;
__sfr __at(0xCD) SPSTAT;
__sfr __at(0xCE) SPCTL;
__sfr __at(0xCF) SPDAT;
__sfr __at(0xD1) T4T3M;
__sfr __at(0xD2) T4H;
__sfr __at(0xD3) T4L;
__sfr __at(0xD4) T3H;
__sfr __at(0xD5) T3L;
__sfr __at(0xD6) T2H;
__sfr __at(0xD7) T2L;
__sfr __at(0xE1) P7M1;
__sfr __at(0xE2) P7M0;
__sfr __at(0xE8) P6;
__sfr __at(0xF8) P7;

#ifdef STC8
__sfr __at(0xDE) ADCCFG;
__sfr __at(0xF5) IAP_TPS;
__sfr __at(0xFF) RSTCFG;
__sfr __at(0x9E) LIRTRIM;
__sfr __at(0x9F) IRTRIM;
/* registros extendidos (XSFR), con P_SW2 |= 0x80 */
#define XSFR(a) (*(volatile __xdata unsigned char *)(a))
#define CKSEL     XSFR(0xFE00)
#define CLKDIV    XSFR(0xFE01)
#define P0PU      XSFR(0xFE10)
#define P1PU      XSFR(0xFE11)
#define P2PU      XSFR(0xFE12)
#define P3PU      XSFR(0xFE13)
#define P4PU      XSFR(0xFE14)
#define P5PU      XSFR(0xFE15)
#define P6PU      XSFR(0xFE16)
#define P7PU      XSFR(0xFE17)
#define ADCTIM    XSFR(0xFEA8)
#endif

/* bits de AUXR */
#define AUXR_T0x12  0x80
#define AUXR_T1x12  0x40
#define AUXR_T2R    0x10
#define AUXR_T2x12  0x04
#define AUXR_S1ST2  0x01

#endif
