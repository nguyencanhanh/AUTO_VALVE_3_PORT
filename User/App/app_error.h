/*****************************************************************************
 * File Name   : app_error.h
 * Layer       : App
 * Description : Error Handler - Display and manage system error states.
 *
 * Error codes (bitmask):
 *   ERR_VALVE_STUCK   0x01  - Valve did not reach position within timeout
 *   ERR_FLOW_OVERRUN  0x02  - Flow volume exceeded safe limit
 *   ERR_RTC_FAIL      0x04  - RTC DS1307 not responding on I2C
 *   ERR_EEPROM_FAIL   0x08  - EEPROM AT24C64 not responding on I2C
 *   ERR_NO_POSITION   0x10  - Position sensor not detected
 *****************************************************************************/
#ifndef __APP_ERROR_H
#define __APP_ERROR_H

#include "BSP/bsp_config.h"

/* Error code bitmask */
#define ERR_NONE          0x00u
#define ERR_VALVE_STUCK   0x01u
#define ERR_FLOW_OVERRUN  0x02u
#define ERR_RTC_FAIL      0x04u
#define ERR_EEPROM_FAIL   0x08u
#define ERR_NO_POSITION   0x10u

/**
 * @brief Enter error display mode.
 *        Display "E-XX" on 7-seg. Play alarm buzzer.
 * @param error_flags Bitmask of active error codes
 */
void APP_Error_Enter(uint8_t error_flags);

/**
 * @brief Run error mode.
 *        - Keep displaying error code
 *        - BTN_SELECT → attempt to clear error and retry
 *        - BTN_MENU   → suppress alarm, return to home (error remains logged)
 */
void APP_Error_Run(void);

/**
 * @brief Exit error mode.
 *        Stop alarm buzzer.
 */
void APP_Error_Exit(void);

/**
 * @brief Set a new error flag (can be called from any module).
 */
void APP_Error_Set(uint8_t flag);

/**
 * @brief Clear an error flag.
 */
void APP_Error_Clear(uint8_t flag);

/**
 * @brief Check if any error is active.
 * @return 1 if error active, 0 if none
 */
uint8_t APP_Error_HasError(void);

#endif /* __APP_ERROR_H */
