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

#endif /* __APP_HOME_H */
