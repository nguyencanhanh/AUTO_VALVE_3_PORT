/*****************************************************************************
 * File Name   : app_volume.c
 * Layer       : App
 * Description : Volume Mode - skeleton
 *
 * TODO: Implement flow counting logic:
 *   extern volatile u32 g_flow_pulses; (from main.c)
 *
 *   APP_Volume_Run():
 *     flow_ml = (g_flow_pulses * 1000) / FLOW_PULSES_PER_LITER
 *     DRV_Display_Number(flow_ml / 100, 0)  ← show x.xx L
 *
 *     if (flow_ml >= s_target_ml):
 *         BSP_Motor_Off()
 *         DRV_Buzzer_Play(BUZ_LONG)
 *         Reset g_flow_pulses = 0
 *
 *   BTN_UP / BTN_DOWN while idle: adjust s_target_ml by 1L steps
 *****************************************************************************/
#include "app_volume.h"
#include "BSP/bsp_tim.h"
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_buzzer.h"

extern volatile uint32_t g_flow_pulses;

static uint32_t s_target_ml = 10000u;   /* Default: 10 liters */
static uint32_t s_session_ml = 0u;

void APP_Volume_Enter(void)
{
    s_session_ml = 0;
    g_flow_pulses = 0;
    /* TODO: Display target volume */
    DRV_Display_Number((uint16_t)(s_target_ml / 1000u), 0);
}

void APP_Volume_Run(void)
{
    /* TODO: Calculate flow and auto-close when target reached */
}

void APP_Volume_Exit(void)
{
    /* Ensure valve is closed on exit */
    BSP_Motor_Off();
}

void APP_Volume_SetTarget(uint32_t target_ml)
{
    s_target_ml = target_ml;
}

uint32_t APP_Volume_GetFlow_ml(void)
{
    return (g_flow_pulses * 1000u) / FLOW_PULSES_PER_LITER;
}
