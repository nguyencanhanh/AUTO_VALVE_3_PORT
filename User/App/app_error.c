/*****************************************************************************
 * File Name   : app_error.c
 * Layer       : App
 * Description : Error Handler - skeleton
 *
 * TODO: Implement error display and recovery logic:
 *
 *   APP_Error_Enter(flags):
 *     DRV_Display_Text(SEG_CHAR_E, SEG_CHAR_DASH, tens, units)
 *     DRV_Buzzer_Play(BUZ_ALARM)
 *
 *   APP_Error_Run():
 *     BTN_SELECT → try to reset fault condition → APP_Error_Clear(flag)
 *     BTN_MENU   → DRV_Buzzer_Stop() → return to home (error remains in log)
 *
 *   APP_Error_Set(flag):
 *     s_error_flags |= flag
 *     Automatically switch app state to error screen
 *****************************************************************************/
#include "app_error.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_buzzer.h"

static uint8_t s_error_flags = ERR_NONE;

void APP_Error_Enter(uint8_t error_flags)
{
    s_error_flags = error_flags;
    /* TODO: Display "E-XX" and play alarm */
    DRV_Buzzer_Play(BUZ_ALARM);
}

void APP_Error_Run(void)
{
    /* TODO: Handle BTN_SELECT (retry) and BTN_MENU (suppress) */
}

void APP_Error_Exit(void)
{
    DRV_Buzzer_Stop();
}

void APP_Error_Set(uint8_t flag)
{
    s_error_flags |= flag;
}

void APP_Error_Clear(uint8_t flag)
{
    s_error_flags &= ~flag;
}

uint8_t APP_Error_HasError(void)
{
    return (s_error_flags != ERR_NONE) ? 1u : 0u;
}
