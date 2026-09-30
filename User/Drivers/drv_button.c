/*****************************************************************************
 * File Name   : drv_button.c
 * Layer       : Drivers - Hardware Abstraction
 * Description : Keyboard lock/unlock management.
 *
 * Combo Unlock: Hold BTN2 (UP) + BTN3 (DOWN) for 5 seconds
 *   -> Turn OFF LED 7, beep 1s, set unlocked
 *
 * Auto-Lock: If unlocked and idle for 1 minute
 *   -> Turn ON LED 7, beep 1s, set locked
 *****************************************************************************/
#include "drv_button.h"
#include "drv_buzzer.h"
#include "BSP/bsp_gpio.h"

/* Keyboard lock state: 1 = locked, 0 = unlocked */
static u8  s_locked = 1;

/* Combo hold counter (ms) */
static u16 s_combo_cnt   = 0;
static u8  s_combo_fired = 0;

/* Auto-lock idle counter (ms) */
static u32 s_idle_cnt = 0;

/*============================================================================
 * DRV_Button_Init
 *============================================================================*/
void DRV_Button_Init(void)
{
    GPIO_ResetBits(GPIOA, LED7_PIN);    /* LED7 ON = locked indicator */
    s_locked      = 1;
    s_combo_cnt   = 0;
    s_combo_fired = 0;
    s_idle_cnt    = 0;
}

/*============================================================================
 * DRV_Button_IsLocked
 *============================================================================*/
u8 DRV_Button_IsLocked(void)
{
    return s_locked;
}

/*============================================================================
 * DRV_Button_Lock - Lock keyboard immediately
 *============================================================================*/
void DRV_Button_Lock(void)
{
    GPIO_ResetBits(GPIOA, LED7_PIN);    /* LED7 ON (Active LOW) */
    DRV_Buzzer_Play(BUZ_LONG);
    s_locked      = 1;
    s_combo_cnt   = 0;
    s_combo_fired = 0;
    s_idle_cnt    = 0;
}

/*============================================================================
 * DRV_Button_ResetIdle - Call from app layer on any button activity
 *============================================================================*/
void DRV_Button_ResetIdle(void)
{
    s_idle_cnt = 0;
}

/*============================================================================
 * DRV_Button_Tick - Call every 1ms from TIM2 ISR
 *============================================================================*/
void DRV_Button_Tick(void)
{
    /* ---- Combo unlock: BTN2 (UP) + BTN3 (DOWN) held together 5s ---- */
    if (s_locked && !s_combo_fired) {
        uint8_t up_pressed   = BSP_Button_ReadRaw(BTN_UP_PIN);
        uint8_t down_pressed = BSP_Button_ReadRaw(BTN_DOWN_PIN);

        if (up_pressed && down_pressed) {
            s_combo_cnt++;
            if (s_combo_cnt >= BTN_UNLOCK_HOLD_MS) {
                GPIO_SetBits(GPIOA, LED7_PIN);      /* LED7 OFF (Active LOW) */
                DRV_Buzzer_Play(BUZ_LONG);          /* 1-second beep */
                s_locked = 0;
                s_combo_fired = 1;
                s_idle_cnt = 0;
            }
        } else {
            s_combo_cnt = 0;
            if (!up_pressed && !down_pressed) {
                s_combo_fired = 0;
            }
        }
    }

    /* ---- Auto-lock: re-lock after 1 minute of inactivity ---- */
    if (!s_locked) {
        s_idle_cnt++;
        if (s_idle_cnt >= BTN_AUTOLOCK_MS) {
            DRV_Button_Lock();
        }
    }
}
