/*****************************************************************************
 * File Name   : app_timer.c
 * Layer       : App
 * Description : Timer Mode - skeleton
 *
 * TODO: Implement schedule matching logic:
 *   static Schedule_t s_schedule = {0};
 *
 *   APP_Timer_Run():
 *     every 1 second:
 *       RTC_Read(&rtc);
 *       if (rtc.hour == s_schedule.open_hour &&
 *           rtc.minute == s_schedule.open_min && rtc.second == 0)
 *           → BSP_Motor_On()
 *       if (rtc.hour == s_schedule.close_hour &&
 *           rtc.minute == s_schedule.close_min && rtc.second == 0)
 *           → BSP_Motor_Off()
 *
 *   Display: show open_hour:open_min or active countdown
 *****************************************************************************/
#include "app_timer.h"
#include "BSP/bsp_tim.h"
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_buzzer.h"

static uint8_t s_open_hour  = 6;
static uint8_t s_open_min   = 0;
static uint8_t s_close_hour = 7;
static uint8_t s_close_min  = 0;

void APP_Timer_Enter(void)
{
    /* TODO: Display open schedule */
    DRV_Display_Time(s_open_hour, s_open_min);
}

void APP_Timer_Run(void)
{
    /* TODO: Check RTC and trigger valve accordingly */
}

void APP_Timer_Exit(void)
{
    /* Nothing to clean up */
}

void APP_Timer_SetOpen(uint8_t hour, uint8_t minute)
{
    s_open_hour = hour;
    s_open_min  = minute;
}

void APP_Timer_SetClose(uint8_t hour, uint8_t minute)
{
    s_close_hour = hour;
    s_close_min  = minute;
}
