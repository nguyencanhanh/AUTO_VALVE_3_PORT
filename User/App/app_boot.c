/*****************************************************************************
 * File Name   : app_boot.c
 * Layer       : App - Application Layer
 * Description : Boot sequence B1->B2->B3 based on technical specification
 *****************************************************************************/
#include "debug.h"
#include "app_boot.h"
#include "BSP/bsp_tim.h"
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_buzzer.h"
#include "Drivers/drv_ds1307.h"
#include "Drivers/drv_pos_sensor.h"
#include "App/Data/data_store.h"
#include "App/app_error.h"
#include <stdio.h> /* For printf */

/*============================================================================
 * PRIVATE VARIABLES
 *============================================================================*/
static AppBootState_t s_state = APP_BOOT_B1_BIP_LONG;
static u32            s_timer = 0;
static u8             s_done  = 0;
static u8             s_scan_motor_running = 0;
static ValvePosition_t s_boot_target_position = VALVE_POS_1;
u8                    previous_day;

#define BOOT_SENSOR_NO_CHANGE_TIMEOUT_MS 30000u
#define BOOT_SCAN_MAX_MS                 60000u

static ValvePosition_t prv_GetBootTargetPosition(void)
{
    u8 phase = WASH_PHASE_IDLE;
    u8 saved_remain = 0;
    AppSettings_t *cfg = Data_GetSettings();
    u8 total_cycles = (u8)(cfg->wash_count + 1u);

    Data_LoadWashState(&phase, &saved_remain);

    if (cfg->wash_count > 20u || phase > WASH_PHASE_FR ||
        saved_remain > total_cycles) {
        PRINT("[BOOT] Invalid wash state phase=%d remain=%d count=%d, clear\r\n",
              phase, saved_remain, cfg->wash_count);
        Data_ClearWashState();
        return VALVE_POS_1;
    }

    if (phase == WASH_PHASE_BW) {
        if (saved_remain >= total_cycles) {
            return VALVE_POS_1;
        }
        return VALVE_POS_2;
    }
    if (phase == WASH_PHASE_FR) {
        return VALVE_POS_3;
    }

    Data_ClearWashState();
    return VALVE_POS_1;
}

/*============================================================================
 * APP_Boot_Start - Start step B1a
 *============================================================================*/
void APP_Boot_Start(void)
{
    s_state = APP_BOOT_B1_BIP_LONG;
    s_timer = BSP_GetTick();
    s_done  = 0;
    s_scan_motor_running = 0;

    /* Initial safe state */
    DS1307_Init();
    Data_Init();        /* Load/init EEPROM settings */
    BSP_Motor_Off();
    BSP_Pump_Off();

    /* Step 1: Power ON -> Buzzer + LED + 7-segment ON simultaneously */
    BSP_Buzzer_On();
    BSP_LED_AllOn();
    DRV_Display_AllOn();

    PRINT("[BOOT] previous_day=%d\r\n", previous_day);

    PRINT("[BOOT] B1a: Buzzer ON (2s), LED/7SEG ON (8s)\r\n");
}

/*============================================================================
 * APP_Boot_Run - State machine process (called in main loop)
 *============================================================================*/
void APP_Boot_Run(void)
{
    u32 elapsed;

    if (s_done) return;
    elapsed = BSP_GetElapsed(s_timer);

    switch (s_state) {

    /*-----------------------------------------------------------------
     * STEP 1: POWER ON
     * - 0s -> 2s: Buzzer ON, LED + 7-segment ON
     *-----------------------------------------------------------------*/
    case APP_BOOT_B1_BIP_LONG:
        if (elapsed >= BOOT_BIP_LONG_MS) { // 2 seconds
            BSP_Buzzer_Off();              // Turn off buzzer
            PRINT("[BOOT] B1b: Buzzer OFF, LED remains ON until 8s\r\n");
            s_state = APP_BOOT_B1_LED_ON;
            /* Do not reset timer, continue counting to 8s */
        }
        break;

    /*-----------------------------------------------------------------
     * - 2s -> 8s: Buzzer OFF, LED + 7-segment remain ON
     *-----------------------------------------------------------------*/
    case APP_BOOT_B1_LED_ON:
        if (elapsed >= BOOT_LED_ON_MS) {    // 8 seconds (from 0 mark)
            BSP_Buzzer_On();                // Boot beep for 1 second
            BSP_LED_AllOff();               // Turn off all LEDs
            DRV_Display_Blank();            // Turn off 7-segment display
            PRINT("[BOOT] B1c: LED OFF, Buzzer beep 1s\r\n");
            s_state = APP_BOOT_B1_BIP2;
            s_timer = BSP_GetTick();        // Reset timer to count 1 second for buzzer
        }
        break;

    /*-----------------------------------------------------------------
     * - 8s -> 9s: Boot beep lasts for 1 second
     *-----------------------------------------------------------------*/
    case APP_BOOT_B1_BIP2:
        if (elapsed >= BOOT_BIP2_MS) {      // 1 second
            BSP_Buzzer_Off();               // Turn off buzzer after 1 second beep
            
            /* STEP 2: DISPLAY PRODUCT CODE */
            PRINT("[BOOT] B2: Display product code\r\n");
            DRV_Display_Text(PRODUCT_CODE_D3, PRODUCT_CODE_D2,
                             PRODUCT_CODE_D1, PRODUCT_CODE_D0);
            s_state = APP_BOOT_B2_PRODUCT;
            s_timer = BSP_GetTick();
        }
        break;

    /*-----------------------------------------------------------------
     * STEP 2: Display product code (e.g., 71P1)
     *-----------------------------------------------------------------*/
    case APP_BOOT_B2_PRODUCT:
        if (elapsed >= BOOT_PRODUCT_MS) {
            /* STEP 3: INITIAL FUNCTION SCAN */
            /* Display -00- during scan */
            s_boot_target_position = prv_GetBootTargetPosition();
            PRINT("[BOOT] B3: Initial scan function (-00-), target=%d\r\n",
                  s_boot_target_position);
            DRV_Display_Text(0x11, 0x00, 0x00, 0x11);
            DRV_PosSensor_SetPosition(VALVE_POS_UNKNOWN);
            DRV_PosSensor_ResetChangeTimer();
            BSP_Motor_On();
            s_scan_motor_running = 1;
            s_state = APP_BOOT_B3_SCAN;
            s_timer = BSP_GetTick();
        }
        break;

    /*-----------------------------------------------------------------
     * STEP 3: Run and scan position (-00-)
     *-----------------------------------------------------------------*/
    case APP_BOOT_B3_SCAN:
        DRV_PosSensor_Process();

        if (DRV_PosSensor_GetCurrentPosition() == s_boot_target_position) {
            BSP_Motor_Off();
            s_scan_motor_running = 0;
            PRINT("[BOOT] DONE: Initial scan found target position=%d\r\n",
                  s_boot_target_position);
            s_state = APP_BOOT_DONE;
            s_done  = 1;
            DRV_Display_Text(0x11, 0x00, 0x00, 0x11);
        } else if (elapsed >= BOOT_SCAN_MAX_MS) {
            BSP_Motor_Off();
            s_scan_motor_running = 0;
            APP_Error_Set(ERR_VALVE_SLOW);
            PRINT("[BOOT] ERROR: Initial scan max timeout target=%d! E3\r\n",
                  s_boot_target_position);
            s_state = APP_BOOT_DONE;
            s_done  = 1;
        } else if (s_scan_motor_running) {
            u32 no_change_ms = DRV_PosSensor_GetTimeSinceLastChange();
            if (no_change_ms >= BOOT_SENSOR_NO_CHANGE_TIMEOUT_MS) {
                BSP_Motor_Off();
                s_scan_motor_running = 0;
                APP_Error_Set(ERR_VALVE_SLOW);
                PRINT("[BOOT] ERROR: Sensor no change %dms during initial scan! E3\r\n",
                      (int)no_change_ms);
                s_state = APP_BOOT_DONE;
                s_done  = 1;
            }
        }
        break;

    default:
        if (s_scan_motor_running) {
            BSP_Motor_Off();
            s_scan_motor_running = 0;
        }
        s_done = 1;
        break;
    }
}

/*============================================================================
 * APP_Boot_IsDone
 *============================================================================*/
u8 APP_Boot_IsDone(void)
{
    return s_done;
}
