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
volatile u8 count_minutes = 0;

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

void BSP_TIM3_Init_1Minute(void)
{
    TIM_TimeBaseInitTypeDef tbs  = {0};
    NVIC_InitTypeDef        nvic = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    tbs.TIM_Prescaler     = 48000 - 1;         /* Tần số đếm 1kHz (1ms) */
    tbs.TIM_Period        = 60000 - 1;         /* 60000ms = 1 phút */
    tbs.TIM_CounterMode   = TIM_CounterMode_Up;
    tbs.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &tbs);

    /* Xóa cờ ngắt phát sinh khi init để tránh ngắt ngay lập tức khi NVIC được cấu hình */
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    // TẮT / KHÔNG CHO CHẠY KHI MỚI KHỞI ĐỘNG
    TIM_Cmd(TIM3, DISABLE); 

    nvic.NVIC_IRQChannel                   = TIM3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
}

// Bắt đầu đếm ngược 1 phút
void Timer3_Start_1Minute(void)
{
    TIM_Cmd(TIM3, DISABLE);                  // Tạm tắt timer (nếu đang chạy dở)
    TIM_SetCounter(TIM3, 0);                 // Reset bộ đếm về 0 để bảo đảm đủ 60s
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // Xóa cờ ngắt cũ
    TIM_Cmd(TIM3, ENABLE);                   // Bắt đầu đếm
    PRINT("[TIMER] Started 1 minute countdown...\r\n");
}

// Hủy đếm/Dừng timer bất cứ lúc nào
void Timer3_Stop(void)
{
    TIM_Cmd(TIM3, DISABLE);
    PRINT("[TIMER] Stopped!\r\n");
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

u8 BSP_GetCountMinutes(void)
{
    return count_minutes;
}

void BSP_ResetCountMinutes(void)
{
    count_minutes = 0;
}