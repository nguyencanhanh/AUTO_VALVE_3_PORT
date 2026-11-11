/*****************************************************************************
 * File Name   : disp_backwash.c
 * Layer       : App - Display
 * Description : Hien thi che do suc rua nguoc (Backwash) dang hoat dong.
 *
 * Hien thi: "2-XX"
 *   D3 = 2         (che do suc nguoc)
 *   D2 = '-'
 *   D1 = hang chuc  (so phut con lai)
 *   D0 = hang don vi
 *
 * LED2  ON = Dang o che do suc rua nguoc
 * LED12 ON = Don vi tinh la phut
 *****************************************************************************/
#include "disp_backwash.h"
#include "App/Data/data_store.h"
#include "App/app_error.h"
#include "BSP/bsp_gpio.h"
#include "BSP/bsp_tim.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_ds1307.h"
#include "Drivers/drv_pos_sensor.h"
#include "debug.h"

/*============================================================================
 * Disp_Backwash_Enter - Bat LED, hien thi lan dau
 * @param total_min: Tong thoi gian suc nguoc (phut)
 *============================================================================*/
static AppSettings_t *cfg;
static u32 time_start_backwash = 0;
static u8  s_motor_running = 0;      /* 1 = motor dang chay, can theo doi cam bien */
extern volatile u8 remain_count;

#define SENSOR_NO_CHANGE_TIMEOUT_MS  30000u  /* Cam bien khong doi 30s = van bi ket (E3) */

void Disp_Backwash_Enter(void) {
  /* LED2 ON = suc nguoc, LED12 ON = don vi phut */
  time_start_backwash = 0;
  s_motor_running = 0;  /* Reset trang thai motor tu chu ky truoc */
  cfg = Data_GetSettings();
  u8 total_cycles = (u8)(cfg->wash_count + 1u);

  DRV_Display_ResetBlinkCount();
  GPIO_ResetBits(GPIOC, LED2_PIN);  /* Active LOW (PC6) */
  GPIO_ResetBits(GPIOA, LED12_PIN); /* Active LOW */

  /* Hoan thanh tat ca chu ky -> quay ve vi tri loc POS_1 */
  if (remain_count >= total_cycles) {
    DRV_Display_SetBlink(0x0F, 0); /* Stop all blink */
    if (DRV_PosSensor_GetCurrentPosition() == VALVE_POS_1) {
      DRV_PosSensor_SetPosition(VALVE_POS_NEXT_STEP);
      PRINT("[DISP_BW] Enter at POS1, finish return home\r\n");
      return;
    }
    DRV_Display_Text(SEG_CHAR_DASH, 0x00, 0x00, SEG_CHAR_DASH);
    BSP_Motor_On();
    DRV_PosSensor_ResetChangeTimer();  /* Reset timer cam bien */
    s_motor_running = 1;
    return;
  }

  /* Neu thoi gian suc nguoc = 0 -> bo qua buoc nay, chuyen sang Fast Rinse */
  if (cfg->backwash_sec == 0u) {
    DRV_Display_SetBlink(0x0F, 0);
    Data_SaveWashState(WASH_PHASE_FR, remain_count);
    DRV_PosSensor_SetPosition(VALVE_POS_NEXT_STEP);
    PRINT("[DISP_BW] backwash_sec=0, skip to FR\r\n");
    return;
  }

  /* Phuc hoi mat dien: van da dung san tai POS_2 -> bat dau dem nguoc ngay */
  if (DRV_PosSensor_GetCurrentPosition() == VALVE_POS_2) {
    DRV_Display_SetBlink(0x0F, 0);
    DRV_Display_Text(0x02, SEG_CHAR_DASH, cfg->backwash_sec / 10, cfg->backwash_sec % 10);
    Timer3_Start_1Minute();
    DRV_PosSensor_SetPosition(VALVE_POS_STOP);
    PRINT("[DISP_BW] Enter at POS2, start timer\r\n");
    return;
  }

  /* Hien thi F-XX nhap nhay truoc khi quay den POS_2 */
  u8 count = (remain_count > 0 && remain_count < total_cycles)
                   ? (u8)(cfg->wash_count - remain_count + 1u)
                   : (u8)(cfg->wash_count - remain_count);
  DRV_Display_Text(0x0F, SEG_CHAR_DASH, count / 10, count % 10);
  DRV_Display_SetBlink(0x0C, 1); /* Blink D1+D0 (count digits) */
  DRV_Display_SetBlink(0x03, 0); /* Stop blink D3+D2 (F-) */
  PRINT("[DISP_BW] Enter (F-%02d)\r\n", count);
}

void Disp_Backwash_Run(void) {
  u8 total_cycles = (u8)(cfg->wash_count + 1u);
  DRV_PosSensor_Process();
  if (remain_count >= total_cycles) {
    if (DRV_PosSensor_GetCurrentPosition() == VALVE_POS_1) {
      BSP_Motor_Off();
      s_motor_running = 0;
      DRV_PosSensor_SetPosition(VALVE_POS_NEXT_STEP);
      PRINT("[DISP_BW] Set position to next step\r\n");
    }
    /* Timeout check: van bi ket khi quay ve home */
    if (s_motor_running) {
      u32 no_change_ms = DRV_PosSensor_GetTimeSinceLastChange();
      if (no_change_ms >= SENSOR_NO_CHANGE_TIMEOUT_MS) {
        BSP_Motor_Off();
        s_motor_running = 0;
        APP_Error_Set(ERR_VALVE_SLOW);
        PRINT("[DISP_BW] ERROR: Sensor no change %dms (return home)! E3\r\n", (int)no_change_ms);
      }
    }
    return;
  }
  if (!s_motor_running && DRV_PosSensor_GetCurrentPosition() != VALVE_POS_STOP &&
      DRV_Display_GetBlinkCount() >= 4) {
    DRV_Display_SetBlink(0x0F, 0); /* Stop all blink */
    BSP_Motor_On();
    DRV_PosSensor_ResetChangeTimer();  /* Reset timer cam bien */
    s_motor_running = 1;
    if (remain_count > 0) {
      time_start_backwash = BSP_GetTick();
    }
  }
  if (time_start_backwash && BSP_GetElapsed(time_start_backwash) > 3000 &&
      DRV_PosSensor_GetCurrentPosition() != VALVE_POS_STOP) {
    u8 count = cfg->wash_count - remain_count;
    DRV_Display_Text(0x0F, SEG_CHAR_DASH, count / 10, count % 10);
  }
  if (DRV_PosSensor_GetCurrentPosition() == VALVE_POS_2) {
    BSP_Motor_Off();
    s_motor_running = 0;  /* Motor tat, ngung theo doi */
    Timer3_Start_1Minute();
    DRV_PosSensor_SetPosition(VALVE_POS_STOP);
    PRINT("[DISP_BW] Set position to STOP\r\n");
  }
  if (DRV_PosSensor_GetCurrentPosition() == VALVE_POS_STOP) {
    u16 elapsed_min = BSP_GetCountMinutes();
    u8 t = (cfg->backwash_sec > elapsed_min) ? (u8)(cfg->backwash_sec - elapsed_min) : 0u;
    DRV_Display_Text(0x02, SEG_CHAR_DASH, t / 10, t % 10);
    if (elapsed_min >= cfg->backwash_sec) {
      Timer3_Stop();
      Data_SaveWashState(WASH_PHASE_FR, remain_count);
      DRV_PosSensor_SetPosition(VALVE_POS_NEXT_STEP);
      PRINT("[DISP_BW] Set position to next step\r\n");
    }
  }

  /* ---- Timeout: Cam bien khong doi trang thai qua lau khi motor dang chay ---- */
  if (s_motor_running &&
      DRV_PosSensor_GetCurrentPosition() != VALVE_POS_STOP &&
      DRV_PosSensor_GetCurrentPosition() != VALVE_POS_NEXT_STEP) {
    u32 no_change_ms = DRV_PosSensor_GetTimeSinceLastChange();
    if (no_change_ms >= SENSOR_NO_CHANGE_TIMEOUT_MS) {
      BSP_Motor_Off();
      s_motor_running = 0;
      APP_Error_Set(ERR_VALVE_SLOW);  /* E3: Van bi ket / motor qua cham */
      PRINT("[DISP_BW] ERROR: Sensor no change %dms! E3\r\n", (int)no_change_ms);
    }
  }
}

/*============================================================================
 * Disp_Backwash_Exit - Tat LED, xoa man hinh
 *============================================================================*/
void Disp_Backwash_Exit(void) {
  GPIO_SetBits(GPIOC, LED2_PIN);
  GPIO_SetBits(GPIOA, LED12_PIN);
  DRV_Display_Blank();
  BSP_LED_AllOff();
  BSP_ResetCountMinutes();
  PRINT("[DISP_BW] Exit\r\n");
}
