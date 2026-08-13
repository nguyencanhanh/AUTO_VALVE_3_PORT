/*****************************************************************************
 * File Name   : app_error.h
 * Layer       : App
 * Description : Error Handler - Display and manage system error states.
 *
 * Error codes (bitmask, hien thi "E-X" tren 7-segment):
 *   ERR_SENSOR      0x01  (E1) - Cam bien vi tri loi
 *   ERR_MOTOR       0x02  (E2) - Motor loi (ket hoac mat tin hieu)
 *   ERR_VALVE_SLOW  0x04  (E3) - Van bi ket / motor chay qua cham (timeout)
 *   ERR_EEPROM      0x08  (E4) - EEPROM AT24C64 loi
 *
 * Hien thi: "E-X" voi X = ma loi (1-4), nhap nhay lien tuc
 * Buzzer:   BUZ_ALARM keu lien tuc
 * Button:   BTN_SELECT → thu xoa loi va retry
 *           BTN_MENU   → tat buzzer, giu hien thi loi
 *****************************************************************************/
#ifndef __APP_ERROR_H
#define __APP_ERROR_H

#include "BSP/bsp_config.h"

/* ---- Error code bitmask ---- */
#define ERR_NONE         0x00u
#define ERR_SENSOR       0x01u   /* E1: Cam bien vi tri loi */
#define ERR_MOTOR        0x02u   /* E2: Motor loi */
#define ERR_VALVE_SLOW   0x04u   /* E3: Van bi ket / motor qua cham */
#define ERR_EEPROM       0x08u   /* E4: EEPROM AT24C64 loi */

/* ---- Error number for display (1-4) ---- */
#define ERR_NUM_SENSOR     1
#define ERR_NUM_MOTOR      2
#define ERR_NUM_VALVE_SLOW 3
#define ERR_NUM_EEPROM     4

/**
 * @brief Set a new error flag (co the goi tu bat ky module nao).
 *        Tu dong chuyen sang error screen neu dang o home.
 * @param flag  Bitmask loi (ERR_SENSOR, ERR_MOTOR, ...)
 */
void APP_Error_Set(uint8_t flag);

/**
 * @brief Clear an error flag.
 * @param flag  Bitmask loi can xoa
 */
void APP_Error_Clear(uint8_t flag);

/**
 * @brief Check if any error is active.
 * @return 1 neu co loi, 0 neu khong
 */
uint8_t APP_Error_HasError(void);

/**
 * @brief Get current error flags bitmask.
 * @return Bitmask cua tat ca loi dang active
 */
uint8_t APP_Error_GetFlags(void);

/**
 * @brief Enter error display mode.
 *        Hien thi "E-X" tren 7-seg, bat buzzer alarm.
 */
void APP_Error_Enter(void);

/**
 * @brief Run error mode (goi trong main loop).
 *        - Hien thi ma loi nhap nhay
 *        - Neu nhieu loi, luan phien hien thi moi 2 giay
 *        - BTN_SELECT → xoa loi hien tai, thu retry
 *        - BTN_MENU   → tat buzzer (loi van con)
 */
void APP_Error_Run(void);

/**
 * @brief Exit error mode.
 *        Tat buzzer, xoa blink.
 */
void APP_Error_Exit(void);

#endif /* __APP_ERROR_H */
