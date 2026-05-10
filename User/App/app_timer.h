/*****************************************************************************
 * File Name   : app_timer.h
 * Layer       : App
 * Description : Timer Mode - Scheduled valve open/close via RTC.
 *               User sets open time (HH:MM) and close time (HH:MM).
 *               Valve opens/closes automatically when RTC matches schedule.
 *****************************************************************************/
#ifndef __APP_TIMER_H
#define __APP_TIMER_H

#include "BSP/bsp_config.h"

/**
 * @brief Enter timer mode.
 *        Display current schedule or "--:--" if not set.
 */
void APP_Timer_Enter(void);

/**
 * @brief Run timer mode logic.
 *        - Check RTC every second against open/close schedule
 *        - Auto open valve at open_hour:open_min
 *        - Auto close valve at close_hour:close_min
 *        - BTN_SELECT → enter schedule edit sub-menu
 *        - BTN_MENU   → return to home
 */
void APP_Timer_Run(void);

/**
 * @brief Exit timer mode.
 */
void APP_Timer_Exit(void);

/**
 * @brief Set open schedule (hour, minute).
 */
void APP_Timer_SetOpen(uint8_t hour, uint8_t minute);

/**
 * @brief Set close schedule (hour, minute).
 */
void APP_Timer_SetClose(uint8_t hour, uint8_t minute);

#endif /* __APP_TIMER_H */
