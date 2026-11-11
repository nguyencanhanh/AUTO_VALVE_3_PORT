/*****************************************************************************
 * File Name   : main.c
 * Project     : AUTO_VALVE_MAIN_V1_3
 * MCU         : CH32X035C8T6 @ 48MHz
 * Description : Entry point - System Init + Boot sequence + Main loop
 *
 * ======================== PROJECT STRUCTURE ================================
 *  User/
 *  ├── main.c              ← Entry point (file nay)
 *  ├── ch32x035_it.c       ← ISR: TIM2_UP, EXTI7_0
 *  ├── App/
 *  │   └── app_boot.c/h    ← Sequence boot B1→B2→B3
 *  ├── BSP/
 *  │   ├── bsp_config.h    ← Pin map & system constants
 *  │   ├── bsp_gpio.c/h    ← GPIO init & control
 *  │   └── bsp_tim.c/h     ← TIM2 1ms tick
 *  └── Drivers/
 *      ├── drv_display.c/h ← 7-Segment driver (GPIOB direct)
 *      └── drv_buzzer.c/h  ← Buzzer pattern driver (PB4)
 *
 * ======================== INCLUDE PATHS (MounRiver Studio) =================
 *  Them vao C/C++ Build → GNU RISC-V Cross C Compiler → Includes:
 *    ../User
 *    ../User/BSP
 *    ../User/Drivers
 *    ../User/App
 *****************************************************************************/

#include "debug.h"
#include "BSP/bsp_config.h"
#include "BSP/bsp_gpio.h"
#include "BSP/bsp_tim.h"
#include "BSP/bsp_i2c.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_button.h"
#include "Drivers/drv_pos_sensor.h"
#include "App/app_boot.h"
#include "App/app_home.h"
#include "App/app_settings.h"
#include "App/app_error.h"
#include "App/app_interlock.h"

/*============================================================================
 * GLOBAL VARIABLES (dung common voi ch32x035_it.c)
 *============================================================================*/
volatile u32 g_flow_pulses = 0;   /* Pulse flow sensor - dem trong EXTI ISR */

/*============================================================================
 * Flow Sensor EXTI Init - PA8 → EXTI Line 8 → EXTI15_8_IRQn
 *============================================================================*/
static void FlowSensor_EXTI_Init(void)
{
    EXTI_InitTypeDef exti = {0};
    NVIC_InitTypeDef nvic = {0};

    /* PA8 already configure Input Pull-Up trong BSP_GPIO_Init() */
    GPIO_EXTILineConfig(FLOW_EXTI_PORTSRC, FLOW_EXTI_PINSRC);

    exti.EXTI_Line    = FLOW_EXTI_LINE;
    exti.EXTI_Mode    = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling;   /* Canh xuong = 1 pulse */
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);

    nvic.NVIC_IRQChannel                   = FLOW_IRQn;   /* EXTI15_8_IRQn */
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
}

static void System_Init(void)
{
    /* 1. Core clock + debug UART */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);

    PRINT("\r\n================================================\r\n");
    PRINT("  AUTO VALVE NUOC V1.3 | CH32X035C8T6\r\n");
    PRINT("  Clock: %lu Hz | ChipID: %08lx\r\n",
           (u32)SystemCoreClock, (u32)DBGMCU_GetCHIPID());
    PRINT("================================================\r\n");

    /* 2. All GPIO */
    BSP_GPIO_Init();
    PRINT("[INIT] BSP GPIO  : OK\r\n");

    /* 3. Display buffer reset (GPIO already ready) */
    DRV_Display_Init();
    PRINT("[INIT] DRV DSP   : OK\r\n");

    /* 4. TIM2 tick 1ms (ngat turn on) */
    BSP_TIM2_Init();
    PRINT("[INIT] BSP TIM2  : OK\r\n");

    /* 5. Flow sensor EXTI */
    FlowSensor_EXTI_Init();
    PRINT("[INIT] Flow EXTI : OK\r\n");

    /* 6. Hardware I2C1 */
    BSP_I2C_Init();
    PRINT("[INIT] BSP I2C1  : OK\r\n");

    /* 7. Initialize TIM3 for 1-minute timer */
    BSP_TIM3_Init_1Minute();
    PRINT("[INIT] BSP TIM3  : OK\r\n");

    /* 8. Daisy-chain Interlock Manager */
    APP_Interlock_Init();
    PRINT("[INIT] Interlock : OK (PC14 In, PA9 Out)\r\n");
}

/*============================================================================
 * main()
 *============================================================================*/
int main(void)
{
    /* Initialize hardware */
    System_Init();

    /* Bat dau sequence Boot B1→B2→B3 */
    APP_Boot_Start();
    /* Vong lap main */
    while (1)
    {
        /* Always process interlock daisy chain (pass-through / state) */
        APP_Interlock_Run();

        if (!APP_Boot_IsDone()) {
            /* Currently trong sequence boot */
            APP_Boot_Run();
            if(APP_Boot_IsDone()) {
                APP_Home_Enter();
                DRV_Button_Init();
            }
        }
        else {
            APP_Home_Run();
            APP_Settings_Run();
        }
    }
}
