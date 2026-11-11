/*****************************************************************************
 * File Name   : app_interlock.c
 * Layer       : App
 * Description : Daisy-chain interlock manager for multi-valve synchronization.
 *               Ensures only 1 valve washes at any given time.
 *               Incoming busy on PC14 delays/queues local wash request.
 *               Local wash or forwarded busy outputs on PA9.
 *****************************************************************************/
#include "app_interlock.h"
#include "BSP/bsp_gpio.h"
#include "BSP/bsp_tim.h"
#include "debug.h"

static u8  s_wash_pending    = 0u; /* 1 = Waiting in queue to wash */
static u8  s_interlock_owner = 0u; /* 1 = Currently holding the wash lock */
static u8  s_in_blanking     = 0u; /* 1 = In blanking interval to break ring echo */
static u32 s_blanking_timer  = 0u;

void APP_Interlock_Init(void)
{
    s_wash_pending = 0u;
    s_interlock_owner = 0u;
    s_in_blanking = 0u;
    s_blanking_timer = 0u;
    BSP_Interlock_SetOutput(0u);
}

void APP_Interlock_Run(void)
{
    u8 in_busy = BSP_Interlock_ReadInput();

    /* Manage blanking timer after finishing wash */
    if (s_in_blanking) {
        if (BSP_GetElapsed(s_blanking_timer) >= 200u || in_busy == 0u) {
            s_in_blanking = 0u;
        }
    }

    /* Output drive logic */
    if (s_interlock_owner) {
        /* Local valve is washing: assert BUSY */
        BSP_Interlock_SetOutput(1u);
    } else if (s_in_blanking) {
        /* Just released: force IDLE to let the 0 propagate through the loop */
        BSP_Interlock_SetOutput(0u);
    } else {
        /* Normal state (service/idle or waiting in queue): forward input busy signal */
        BSP_Interlock_SetOutput(in_busy);
    }
}

u8 APP_Interlock_IsBusy(void)
{
    if (s_interlock_owner) return 1u;
    return BSP_Interlock_ReadInput();
}

u8 APP_Interlock_CanStartWash(void)
{
    if (s_interlock_owner) return 0u;
    if (s_in_blanking) return 0u;
    return 1u;
}

void APP_Interlock_OnWashStart(void)
{
    s_interlock_owner = 1u;
    s_wash_pending = 0u;
    s_in_blanking = 0u;
    BSP_Interlock_SetOutput(1u);
    PRINT("[INTERLOCK] Acquired wash lock, driving BUSY (PA9=1)\r\n");
}

void APP_Interlock_OnWashFinished(void)
{
    s_interlock_owner = 0u;
    s_in_blanking = 1u;
    s_blanking_timer = BSP_GetTick();
    BSP_Interlock_SetOutput(0u);
    PRINT("[INTERLOCK] Released wash lock, driving IDLE (PA9=0)\r\n");
}

void APP_Interlock_RequestWash(void)
{
    s_wash_pending = 1u;
    PRINT("[INTERLOCK] Line busy: wash request queued\r\n");
}

u8 APP_Interlock_IsWashPending(void)
{
    return s_wash_pending;
}

void APP_Interlock_ClearPending(void)
{
    s_wash_pending = 0u;
}
