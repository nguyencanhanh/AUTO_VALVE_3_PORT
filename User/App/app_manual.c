/*****************************************************************************
 * File Name   : app_manual.c
 * Layer       : App
 * Description : Manual Mode - skeleton
 *
 * TODO: Implement the following logic:
 *   - Display: "OPEN" or "CLS." based on valve state
 *   - BTN_VALVE → toggle valve:
 *       VALVE_CLOSED → Motor_On() → start timeout timer → wait position sensor
 *       VALVE_OPEN   → Motor_Off() → start timeout timer → wait position sensor
 *   - Monitor position sensor (PA7) to confirm open/close
 *   - If timeout exceeded → set error flag → show "Err 1" → play BUZ_ALARM
 *   - BTN_MENU → return to home screen
 *****************************************************************************/
#include "app_manual.h"
#include "BSP/bsp_gpio.h"
#include "BSP/bsp_tim.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_buzzer.h"

void APP_Manual_Enter(void)
{
    /* TODO: Show valve status */
}

void APP_Manual_Run(void)
{
    /* TODO: Handle BTN_VALVE, monitor sensors, update display */
}

void APP_Manual_Exit(void)
{
    /* TODO: Ensure relay is in safe state */
}
