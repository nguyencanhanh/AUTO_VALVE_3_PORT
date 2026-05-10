/*****************************************************************************
 * File Name   : drv_button.h
 * Layer       : Drivers - Hardware Abstraction
 * Description : Keyboard lock/unlock management only.
 *
 * Combo Unlock: Hold BTN2 (UP) + BTN3 (DOWN) for 5 seconds
 * Auto-Lock:   Re-lock after 1 minute of no activity
 *
 * Button event handling (short/long press) is done in app_settings.c
 *****************************************************************************/
#ifndef __DRV_BUTTON_H
#define __DRV_BUTTON_H

#include "BSP/bsp_config.h"

/* Timing constants */
#define BTN_UNLOCK_HOLD_MS  5000u   /* 5 seconds combo hold to unlock */
#define BTN_AUTOLOCK_MS     60000u  /* 1 minute idle to auto-lock */

/* Initialize lock system (call once after boot) */
void DRV_Button_Init(void);

/* Tick function (call every 1ms from TIM2 ISR) */
void DRV_Button_Tick(void);

/* Check keyboard lock state: 1 = locked, 0 = unlocked */
u8 DRV_Button_IsLocked(void);

/* Reset idle timer (call from app layer when any button is pressed) */
void DRV_Button_ResetIdle(void);

#endif /* __DRV_BUTTON_H */
