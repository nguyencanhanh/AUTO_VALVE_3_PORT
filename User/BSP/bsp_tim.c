/*****************************************************************************
 * File Name   : bsp_tim.c
 * Layer       : BSP - Board Support Package
 * Description : Configure TIM2 - Update interrupt every 1ms
 *               48MHz / Prescaler(48) / Period(1000) = 1kHz → 1ms
 *****************************************************************************/
#include "bsp_tim.h"

/*============================================================================
 * GLOBAL VARIABLES
 *============================================================================*/
volatile u32 g_tick_ms = 0;

/*============================================================================
 * BSP_TIM2_Init
 *============================================================================*/
void BSP_TIM2_Init(void)
{
    TIM_TimeBaseInitTypeDef tbs  = {0};
    NVIC_InitTypeDef        nvic = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    tbs.TIM_Prescaler     = 48 - 1;    /* 48MHz / 48 = 1MHz */
    tbs.TIM_Period        = 1000 - 1;  /* 1MHz / 1000 = 1kHz → 1ms */
    tbs.TIM_CounterMode   = TIM_CounterMode_Up;
    tbs.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &tbs);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);

    nvic.NVIC_IRQChannel                   = TIM2_UP_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority        = 1;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
}

/*============================================================================
 * Tick Utilities
 *============================================================================*/
u32 BSP_GetTick(void)
{
    return g_tick_ms;
}

u32 BSP_GetElapsed(u32 start_ms)
{
    return g_tick_ms - start_ms;
}
