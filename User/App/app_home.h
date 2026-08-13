/*****************************************************************************
 * File Name   : app_home.h
 * Layer       : App
 * Description : Home Screen - Standby display (clock HH:MM)
 *               This is the default screen shown after boot completes.
 *               From here the user can navigate to other modes.
 *****************************************************************************/
#ifndef __APP_HOME_H
#define __APP_HOME_H

#include "BSP/bsp_config.h"

/**
 * @brief Initialize the home screen module.
 *        Called once when entering home screen.
 */
void APP_Home_Enter(void);

/**
 * @brief Run home screen logic.
 *        Call continuously in main loop when home screen is active.
 *        - Displays current RTC time (HH:MM)
 *        - Listens for button events to enter other modes
 */
void APP_Home_Run(void);

/**
 * @brief Exit home screen.
 *        Called before switching to another screen.
 */
void APP_Home_Exit(void);

/**
 * @brief Start manual wash cycle.
 *        Triggered by BTN_SELECT when not in settings mode.
 *        Enters backwash cycle same as scheduled wash.
 */
void APP_Home_StartManualWash(void);

/**
 * @brief Check if currently in wash mode (BACKWASHING or RINSING).
 * @return 1 if washing, 0 otherwise.
 */
u8 APP_Home_IsWashing(void);

/**
 * @brief Skip current wash step and advance to next.
 *        Stops motor/timer, forces transition to next step.
 *        Used by BTN_SELECT during manual wash.
 */
void APP_Home_ManualNextStep(void);

#endif /* __APP_HOME_H */
