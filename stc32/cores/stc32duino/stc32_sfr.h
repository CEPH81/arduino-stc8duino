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
sbit TF1  = TCON^7;
sbit TR1  = TCON^6;
sbit TF0  = TCON^5;
sbit TR0  = TCON^4;
sbit IE1  = TCON^3;
sbit IT1  = TCON^2;
sbit IE0  = TCON^1;
sbit IT0  = TCON^0;
sfr TMOD  = 0x89;
sfr TL0   = 0x8A;
sfr TL1   = 0x8B;
sfr TH0   = 0x8C;
sfr TH1   = 0x8D;
sfr AUXR  = 0x8E;
sfr SCON  = 0x98;
sbit TI   = SCON^1;
sbit RI   = SCON^0;
sfr SBUF  = 0x99;
sfr P_SW1 = 0xA2;
sfr IE    = 0xA8;
sbit EA   = IE^7;
sbit ES   = IE^4;
sbit ET1  = IE^3;
sbit EX1  = IE^2;
sbit ET0  = IE^1;
sbit EX0  = IE^0;
sfr ADC_CONTR = 0xBC;
sfr ADC_RES   = 0xBD;
sfr ADC_RESL  = 0xBE;
sfr IAP_CONTR = 0xC7;
sfr ADCCFG    = 0xDE;
sfr T2H   = 0xD6;
sfr T2L   = 0xD7;
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

/* modulo PWMB (canales PWM5-PWM8) y ADC (XFR) */
#define XFR(a) (*(unsigned char volatile far *)(a))
#define ADCTIM      XFR(0x7efea8)
#define PWMB_ENO    XFR(0x7efeb5)
#define PWMB_PS     XFR(0x7efeb6)
#define PWMB_CR1    XFR(0x7efee0)
#define PWMB_EGR    XFR(0x7efee7)
#define PWMB_CCMR1_DIR 0x7efee8UL   /* CCMR1..4 (canales 5..8) seguidos */
#define PWMB_CCER1  XFR(0x7efeec)
#define PWMB_CCER2  XFR(0x7efeed)
#define PWMB_PSCRH  XFR(0x7efef0)
#define PWMB_PSCRL  XFR(0x7efef1)
#define PWMB_ARRH   XFR(0x7efef2)
#define PWMB_ARRL   XFR(0x7efef3)
#define PWMB_CCR5H_DIR 0x7efef5UL   /* CCR5H, CCR5L, CCR6H ... CCR8L */
#define PWMB_BKR    XFR(0x7efefd)

#endif
