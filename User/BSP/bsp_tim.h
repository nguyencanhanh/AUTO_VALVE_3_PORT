/*****************************************************************************
 * File Name   : bsp_tim.h
 * Layer       : BSP - Board Support Package
 * Description : TIM2 tick 1ms - nhip tim he thong
 *****************************************************************************/
#ifndef __BSP_TIM_H
#define __BSP_TIM_H

#include "bsp_config.h"

/* Bien dem tick toan cuc (tang every 1ms trong TIM2_UP ISR) */
extern volatile u32 g_tick_ms;

/* Initialize TIM2 ngat every 1ms */
void BSP_TIM2_Init(void);

/* Utility time */
u32  BSP_GetTick(void);              /* Tra ve g_tick_ms hien at */
u32  BSP_GetElapsed(u32 start_ms);   /* Khoang time elapsed */

#endif /* __BSP_TIM_H */
