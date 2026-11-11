/*****************************************************************************
 * File Name   : ch32x035_it.c
 * Project     : AUTO_VALVE_MAIN_V1_3
 * Description : Interrupt Service Routines
 *
 *   NMI_Handler          : NMI exception (loop)
 *   HardFault_Handler    : Hard fault → System reset
 *   TIM2_UP_IRQHandler   : 1ms tick → buzzer tick + display refresh
 *   EXTI15_8_IRQHandler  : Flow sensor pulse count (PA8, EXTI Line8)
 *****************************************************************************/
#include "ch32x035_it.h"
#include "BSP/bsp_config.h"
#include "BSP/bsp_tim.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_buzzer.h"
#include "Drivers/drv_button.h"

/*============================================================================
 * EXTERN VARIABLES (define trong main.c)
 *============================================================================*/
extern volatile u32 g_flow_pulses;

/*============================================================================
 * Prototype declarations
 *============================================================================*/
void NMI_Handler(void)          __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void)    __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_UP_IRQHandler(void)   __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI15_8_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*============================================================================
 * NMI_Handler
 *============================================================================*/
void NMI_Handler(void)
{
    while (1) {}
}

/*============================================================================
 * HardFault_Handler
 *============================================================================*/
void HardFault_Handler(void)
{
    NVIC_SystemReset();
    while (1) {}
}

/*============================================================================
 * TIM2_UP_IRQHandler - 1ms system tick
 *
 * Thuc hien theo thu tu uu tien:
 *   1. g_tick_ms++              : nhip tim toan he thong
 *   2. DRV_Buzzer_Tick()        : cap nhat pattern buzzer
 *   3. DRV_Display_Refresh()    : quet 1 digit every 3ms (≈83Hz/digit)
 *============================================================================*/
void TIM2_UP_IRQHandler(void)
{
    static u8 s_disp_cnt = 0;

    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == RESET) return;
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    /* 1ms tick */
    g_tick_ms++;

    /* Buzzer pattern (every 1ms) */
    DRV_Buzzer_Tick();

    /* Button scan (every 1ms) */
    DRV_Button_Tick();

    /* Display refresh: 1 digit/3ms → 4 digits = 12ms/cycle ≈ 83Hz */
    if (++s_disp_cnt >= 3) {
        s_disp_cnt = 0;
        DRV_Display_Refresh();
    }
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        
        count_minutes++;
        PRINT("[TIM3] Da het 60 giay ke tu khi Start!\r\n");
    }
}
/*============================================================================
 * EXTI15_8_IRQHandler - Flow sensor pulse count (PA8 = EXTI Line 8)
 *============================================================================*/
void EXTI15_8_IRQHandler(void)
{
    if (EXTI_GetITStatus(FLOW_EXTI_LINE) != RESET) {
        EXTI_ClearITPendingBit(FLOW_EXTI_LINE);
        g_flow_pulses++;
    }
}
