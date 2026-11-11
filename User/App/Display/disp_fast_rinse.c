#include "disp_fast_rinse.h"
#include "App/Data/data_store.h"
#include "App/app_error.h"
#include "BSP/bsp_gpio.h"
#include "BSP/bsp_tim.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_pos_sensor.h"
#include "debug.h"

/*============================================================================
 * Chức năng: Sục Rửa Xuôi (Fast Rinse)
 *============================================================================*/
AppSettings_t *cfg;
static u8 fr_motor_running = 0;
extern volatile u8 remain_count;

#define FR_SENSOR_NO_CHANGE_TIMEOUT_MS  30000u  /* Cam bien khong doi 30s = van bi ket (E3) */

void Disp_FastRinse_Enter(void) {
  cfg = Data_GetSettings();
  fr_motor_running = 0;  /* Reset trang thai motor tu chu ky truoc */
  GPIO_ResetBits(GPIOA, LED5_PIN);  /* Active LOW: LED suc xuoi */
  GPIO_ResetBits(GPIOA, LED12_PIN); /* Active LOW: don vi phut */

  /* Neu thoi gian suc xuoi = 0 -> bo qua buoc nay */
  if (cfg->fast_rinse_sec == 0u) {
    DRV_Display_SetBlink(0x0F, 0);
    Data_SaveWashState(WASH_PHASE_BW, (u8)(remain_count + 1u));
    DRV_PosSensor_SetPosition(VALVE_POS_NEXT_STEP);
    PRINT("[DISP_FR] fast_rinse_sec=0, skip\r\n");
    return;
  }

  /* Phuc hoi mat dien: van da dung san tai POS_3 -> bat dau dem nguoc ngay */
  if (DRV_PosSensor_GetCurrentPosition() == VALVE_POS_3) {
    DRV_Display_SetBlink(0x0F, 0);
    DRV_Display_Text(0x03, SEG_CHAR_DASH,
                     cfg->fast_rinse_sec / 10, cfg->fast_rinse_sec % 10);
    Timer3_Start_1Minute();
    DRV_PosSensor_SetPosition(VALVE_POS_STOP);
    PRINT("[DISP_FR] Enter at POS3, start timer\r\n");
    return;
  }

  DRV_Display_Text(0x11, 0x00, 0x00, 0x11);

  PRINT("[DISP_FR] Enter\r\n");
  DRV_Display_ResetBlinkCount();
  DRV_Display_SetBlink(0x09, 0); /* Stop blink outer dash digits */
  DRV_Display_SetBlink(0x06, 1); /* Blink middle 00 digits */
}

void Disp_FastRinse_Run(void){
    if (!fr_motor_running && DRV_PosSensor_GetCurrentPosition() != VALVE_POS_STOP &&
        DRV_Display_GetBlinkCount() >= 4) {
        DRV_Display_SetBlink(0x0F, 0); /* Stop all blink */
        BSP_Motor_On();
        DRV_PosSensor_ResetChangeTimer();  /* Reset timer cam bien */
        fr_motor_running = 1;
    }
    DRV_PosSensor_Process();
    if(DRV_PosSensor_GetCurrentPosition() == VALVE_POS_3){
        BSP_Motor_Off();
        fr_motor_running = 0;  /* Motor tat, ngung theo doi */
        Timer3_Start_1Minute();
        DRV_PosSensor_SetPosition(VALVE_POS_STOP);
        PRINT("[DISP_FR] Set position to STOP\r\n");
    }
    if(DRV_PosSensor_GetCurrentPosition() == VALVE_POS_STOP){
        u16 elapsed_min = BSP_GetCountMinutes();
        u8 t = (cfg->fast_rinse_sec > elapsed_min) ? (u8)(cfg->fast_rinse_sec - elapsed_min) : 0u;
        DRV_Display_Text(0x03, SEG_CHAR_DASH, t/10, t%10);
        if(elapsed_min >= cfg->fast_rinse_sec){
            Timer3_Stop();
            Data_SaveWashState(WASH_PHASE_BW, (u8)(remain_count + 1u));
            DRV_PosSensor_SetPosition(VALVE_POS_NEXT_STEP);
        }
    }

    /* ---- Timeout: Cam bien khong doi trang thai qua lau khi motor dang chay ---- */
    if (fr_motor_running &&
        DRV_PosSensor_GetCurrentPosition() != VALVE_POS_STOP &&
        DRV_PosSensor_GetCurrentPosition() != VALVE_POS_NEXT_STEP) {
      u32 no_change_ms = DRV_PosSensor_GetTimeSinceLastChange();
      if (no_change_ms >= FR_SENSOR_NO_CHANGE_TIMEOUT_MS) {
        BSP_Motor_Off();
        fr_motor_running = 0;
        APP_Error_Set(ERR_VALVE_SLOW);  /* E3: Van bi ket / motor qua cham */
        PRINT("[DISP_FR] ERROR: Sensor no change %dms! E3\r\n", (int)no_change_ms);
      }
    }
        
}

void Disp_FastRinse_Exit(void) {
    /* TODO: Dọn dẹp trước khi thoát chế độ */
    BSP_LED_AllOff();
    DRV_Display_Blank();
    BSP_ResetCountMinutes();
    PRINT("[DISP_FR] Exit\r\n");
}
