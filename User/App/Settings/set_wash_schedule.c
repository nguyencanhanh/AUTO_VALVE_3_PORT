/*****************************************************************************
 * File Name   : set_wash_schedule.c
 * Layer       : App - Settings
 * Description : Setting 2 - Cai dat thoi gian chu ky suc rua (HH:MM)
 *
 * Sub-states:
 *   SCHED_SHOW      - Hien thi thoi gian hien tai, khong nhap nhay
 *   SCHED_EDIT_HOUR - Nhap nhay gio, UP/DOWN chinh gio
 *   SCHED_EDIT_MIN  - Nhap nhay phut, UP/DOWN chinh phut
 *
 * Button mapping (received from app_settings):
 *   ENTER (MENU)  - Cycle: SHOW -> EDIT_HOUR -> EDIT_MIN -> SHOW (save)
 *   UP            - Tang gio/phut (short press hoac auto-repeat)
 *   DOWN          - Giam gio/phut (short press hoac auto-repeat)
 *   BACK (SELECT) - Xu ly boi app_settings (exit to NAV)
 *
 * LED6 tat de tranh nham lan voi che do cai dat thoi gian RTC.
 *****************************************************************************/
#include "set_wash_schedule.h"
#include "App/Data/data_store.h"
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_display.h"
#include "debug.h"

typedef enum {
  SCHED_SHOW = 0,
  SCHED_EDIT_HOUR,
  SCHED_EDIT_MIN
} SchedEditState_t;

static SchedEditState_t s_state;
static u8 s_hour;
static u8 s_minute;

/*----------------------------------------------------------------------------
 * Refresh display with current schedule hour:minute
 *----------------------------------------------------------------------------*/
static void prv_UpdateDisplay(void) {
  DRV_Display_Time(s_hour, s_minute);
  DRV_Display_SetDP(1, 1); /* Dau cham o giua khong nhap nhay */
}

/*============================================================================
 * Set_WashSchedule_Enter - Doc EEPROM, hien thi, khong blink
 *============================================================================*/
void Set_WashSchedule_Enter(void) {
  AppSettings_t *cfg = Data_GetSettings();
  s_hour = cfg->wash_sched_hour;
  s_minute = cfg->wash_sched_min;
  if (s_hour > 23 || s_minute > 59) {
    s_hour = 0;
    s_minute = 0;
  }
  s_state = SCHED_SHOW;

  /* LED6 OFF de tranh nham lan voi cai dat thoi gian RTC */
  GPIO_SetBits(GPIOA, LED6_PIN);

  DRV_Display_SetBlink(0x0F, 0); /* No blink */
  prv_UpdateDisplay();
  PRINT("[SET_SCHED] Enter: %02d:%02d\r\n", s_hour, s_minute);
}

/*============================================================================
 * Set_WashSchedule_Run - Xu ly nut bam
 *============================================================================*/
void Set_WashSchedule_Run(BtnAction_t *actions) {
  BtnAction_t enter_act = actions[BTN_IDX_MENU];
  BtnAction_t up_act = actions[BTN_IDX_UP];
  BtnAction_t down_act = actions[BTN_IDX_DOWN];

  /*--------------------------------------------------------------
   * ENTER cycles through states
   *--------------------------------------------------------------*/
  if (enter_act == BTN_ACT_SHORT) {
    switch (s_state) {
    case SCHED_SHOW:
      /* Bat dau chinh gio */
      s_state = SCHED_EDIT_HOUR;
      DRV_Display_SetBlink(0x0C, 0); /* Blink D3+D2 (hour digits) */
      DRV_Display_SetBlink(0x03, 1); /* Stop blink D1+D0 */
      PRINT("[SET_SCHED] Edit HOUR\r\n");
      break;

    case SCHED_EDIT_HOUR:
      /* Chuyen sang chinh phut */
      s_state = SCHED_EDIT_MIN;
      DRV_Display_SetBlink(0x0C, 1); /* Stop blink D3+D2 */
      DRV_Display_SetBlink(0x03, 0); /* Blink D1+D0 (minute digits) */
      PRINT("[SET_SCHED] Edit MINUTE\r\n");
      break;

    case SCHED_EDIT_MIN:
      /* Ket thuc chinh: luu vao EEPROM */
      s_state = SCHED_SHOW;
      DRV_Display_SetBlink(0x0F, 0); /* Stop all blink */
      Data_SetWashSchedule(s_hour, s_minute);
      PRINT("[SET_SCHED] Saved: %02d:%02d\r\n", s_hour, s_minute);
      APP_Settings_SetNavMode();
      break;
    }
    prv_UpdateDisplay();
    return;
  }

  /*--------------------------------------------------------------
   * UP/DOWN adjusts current value (short, long, or repeat)
   *--------------------------------------------------------------*/
  u8 do_up = (up_act == BTN_ACT_SHORT || up_act == BTN_ACT_LONG ||
              up_act == BTN_ACT_REPEAT);
  u8 do_down = (down_act == BTN_ACT_SHORT || down_act == BTN_ACT_LONG ||
                down_act == BTN_ACT_REPEAT);

  if (s_state == SCHED_EDIT_HOUR) {
    if (do_up) {
      s_hour++;
      if (s_hour > 23)
        s_hour = 0;
      prv_UpdateDisplay();
    }
    if (do_down) {
      if (s_hour > 0)
        s_hour--;
      else
        s_hour = 23;
      prv_UpdateDisplay();
    }
  }

  if (s_state == SCHED_EDIT_MIN) {
    if (do_up) {
      s_minute++;
      if (s_minute > 59)
        s_minute = 0;
      prv_UpdateDisplay();
    }
    if (do_down) {
      if (s_minute > 0)
        s_minute--;
      else
        s_minute = 59;
      prv_UpdateDisplay();
    }
  }
}

/*============================================================================
 * Set_WashSchedule_Exit - Tat blink, khong luu neu chua nhan Enter xong
 *============================================================================*/
void Set_WashSchedule_Exit(void) {
  BSP_LED_AllOff();
  DRV_Display_Blank();
  DRV_Display_SetBlink(0x0F, 0);
  s_state = SCHED_SHOW;
  PRINT("[SET_SCHED] Exit\r\n");
}
