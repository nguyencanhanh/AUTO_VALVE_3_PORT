/*****************************************************************************
 * File Name   : drv_buzzer.h
 * Layer       : Drivers - Hardware Abstraction
 * Description : Dieu whenen buzzer bao PB4 voi pattern am thanh
 *               DRV_Buzzer_Tick() right is goi every 1ms from TIM2 ISR
 *****************************************************************************/
#ifndef __DRV_BUZZER_H
#define __DRV_BUZZER_H

#include "BSP/bsp_config.h"

/* Cac pattern am bao */
typedef enum {
    BUZ_OFF      = 0,
    BUZ_BEEP1    = 1,   /* 1 beep ngan  - phan hoi button bam */
    BUZ_BEEP2    = 2,   /* 2 beep       - xac nhan */
    BUZ_BEEP3    = 3,   /* 3 beep       - valve open thanh cong */
    BUZ_LONG     = 4,   /* 1 beep dai   - valve close / canh bao */
    BUZ_ALARM    = 5,   /* Beep lien tuc - loi nghiem trong */
    BUZ_BOOT     = 6,   /* Beep don ngan - tin hieu boot */
} BuzzerPattern_t;

void DRV_Buzzer_Play(BuzzerPattern_t pattern);
void DRV_Buzzer_Stop(void);
void DRV_Buzzer_Tick(void);         /* Goi every 1ms from TIM2 ISR */
u8   DRV_Buzzer_IsBusy(void);

#endif /* __DRV_BUZZER_H */
