/*****************************************************************************
 * File Name   : app_boot.h
 * Layer       : App - Application Layer
 * Description : Sequence boot AUTO VALVE (B1 → B2 → B3)
 *
 *  B1a [0s    → 2s]    : Beep dai, 7-seg TAT, LED TAT
 *  B1b [2s    → 10s]   : LED BAT, 7-seg ALL ON ~8s
 *  B1c [10s   → 11s]   : Beep don boot ~1s
 *  B2  [11s   → 13s]   : Display product code "71P1"
 *  B3  [13s   → scan done] : Quay motor do ve vi tri loc, display "-00-"
 *  DONE                 : Switch sang screen cho (HH:MM)
 *****************************************************************************/
#ifndef __APP_BOOT_H
#define __APP_BOOT_H

#include "BSP/bsp_config.h"

typedef enum {
    APP_BOOT_B1_BIP_LONG = 0,
    APP_BOOT_B1_LED_ON,
    APP_BOOT_B1_BIP2,
    APP_BOOT_B2_PRODUCT,
    APP_BOOT_B3_SCAN,
    APP_BOOT_DONE
} AppBootState_t;

/* Boot sequence (goi 1 time after init hardware) */
void APP_Boot_Start(void);

/* State machine boot (called continuously in main loop) */
void APP_Boot_Run(void);

/* Check already complete chua */
u8   APP_Boot_IsDone(void);

#endif /* __APP_BOOT_H */
