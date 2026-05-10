/*****************************************************************************
 * File Name   : drv_display.h
 * Layer       : Drivers - Hardware Abstraction
 * Description : Driver display 7-segment 4 digit (noi truc tiep GPIOB)
 *               Multiplexing via TIM2 ISR every ~3ms
 *****************************************************************************/
#ifndef __DRV_DISPLAY_H
#define __DRV_DISPLAY_H

#include "BSP/bsp_config.h"

/* ---- Ky tu dac biet (index vao bang font) ---- */
#define SEG_CHAR_BLANK  0x10   /* Turn off het */
#define SEG_CHAR_DASH   0x11   /* '-' */
#define SEG_CHAR_E      0x12   /* 'E' */
#define SEG_CHAR_H      0x13   /* 'H' */
#define SEG_CHAR_L      0x14   /* 'L' */
#define SEG_CHAR_O      0x15   /* 'O' */
#define SEG_CHAR_C      0x16   /* 'C' */
#define SEG_CHAR_P      0x17   /* 'P' */
#define SEG_CHAR_r      0x18   /* 'r' */
#define SEG_CHAR_n      0x19   /* 'n' */
#define SEG_CHAR_ALL    0x1A   /* All segment ON (test / boot) */

/* ---- API ---- */

/* Initialize buffer */
void DRV_Display_Init(void);

/* Display 4 ky tu (so 0-9 hoac SEG_CHAR_xxx) */
void DRV_Display_Text(u8 d3, u8 d2, u8 d1, u8 d0);

/* Display so 0-9999 */
void DRV_Display_Number(u16 number, u8 leading_zero);

/* Display HH:MM voi decimal point giua */
void DRV_Display_Time(u8 hour, u8 minute);

/* Turn on all segment (dung for boot test) */
void DRV_Display_AllOn(void);

/* Turn off all (blank) */
void DRV_Display_Blank(void);

/* Decimal point thap phan for digit 0-3 */
void DRV_Display_SetDP(u8 digit, u8 on);

/* Blink digit (bitmask: bit0=D0..bit3=D3) */
void DRV_Display_SetBlink(u8 digit_mask, u8 enable);

/* Refresh multiplexing - CALLED FROM TIM2 ISR every ~3ms */
void DRV_Display_Refresh(void);

#endif /* __DRV_DISPLAY_H */
