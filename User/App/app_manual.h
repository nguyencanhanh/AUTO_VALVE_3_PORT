/*****************************************************************************
 * File Name   : app_manual.h
 * Layer       : App
 * Description : Manual Mode - Direct valve open/close via BTN_VALVE button.
 *               User can manually toggle valve regardless of schedule.
 *               Displays valve status: "OPEN" or "CLS."
 *****************************************************************************/
#ifndef __APP_MANUAL_H
#define __APP_MANUAL_H

#include "BSP/bsp_config.h"

/**
 * @brief Enter manual control mode.
 *        Display current valve status on 7-seg.
 */
void APP_Manual_Enter(void);

/**
 * @brief Run manual mode logic.
 *        - BTN_VALVE → toggle Open/Close
 *        - Monitor position sensor for confirmation
 *        - Timeout protection via VALVE_TIMEOUT_MS
 */
void APP_Manual_Run(void);

/**
 * @brief Exit manual mode.
 */
void APP_Manual_Exit(void);

#endif /* __APP_MANUAL_H */
