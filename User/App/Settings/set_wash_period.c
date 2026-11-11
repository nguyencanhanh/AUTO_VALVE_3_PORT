/*****************************************************************************
 * File Name   : set_wash_period.c
 * Layer       : App - Settings
 * Description : Setting 4 - Cai dat chu ky suc rua (so ngay)
 *
 * Hien thi: " -XX" (dash + so ngay, VD: " -03" = 3 ngay)
 *   00 = 1 ngay suc rua 1 lan
 *   01 = 2 ngay/lan, ...
 *   Toi da 99 ngay
 *
 * LED9 ON  = Don vi tinh la NGAY
 * LED1 ON  = Che do loc nuoc hoat dong theo so ngay
 *
 * Sub-states:
 *   PERIOD_SHOW - Hien thi " -XX", khong nhap nhay
 *   PERIOD_EDIT - Nhap nhay so XX, UP/DOWN chinh (00-99)
 *
 * Button mapping:
 *   ENTER (MENU)  - Cycle: SHOW -> EDIT -> SHOW (save)
 *   UP            - Tang (short/long/repeat)
 *   DOWN          - Giam (short/long/repeat)
 *   BACK (SELECT) - Xu ly boi app_settings (exit to NAV)
 *****************************************************************************/
#include "set_wash_period.h"
#include "App/Data/data_store.h"
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_display.h"
#include "debug.h"

#define WASH_PERIOD_MAX 99

typedef enum { PERIOD_SHOW = 0, PERIOD_EDIT } PeriodEditState_t;

static PeriodEditState_t s_state;
static u8 s_days;
/*----------------------------------------------------------------------------
 * Hien thi " -XX" len 7-segment
 *   D3 = blank
 *   D2 = '-'
 *   D1 = hang chuc
 *   D0 = hang don vi
 *----------------------------------------------------------------------------*/
static void prv_UpdateDisplay(void) {
  u8 tens = s_days / 10;
  u8 ones = s_days % 10;
  DRV_Display_Text(0x01, SEG_CHAR_DASH, tens, ones);
}

/*============================================================================
 * Set_WashPeriod_Enter - Doc EEPROM, bat LED9+LED1, hien thi
 *============================================================================*/
void Set_WashPeriod_Enter(void) {
  AppSettings_t *cfg = Data_GetSettings();
  s_days = cfg->wash_period_hours; /* Dung field nay luu so ngay */
  if (s_days > WASH_PERIOD_MAX) {
    s_days = 0;
  }
  s_state = PERIOD_SHOW;

  /* LED9 ON = don vi la ngay, LED1 ON = che do loc theo ngay */
  GPIO_ResetBits(GPIOA, LED9_PIN); /* Active LOW */
  GPIO_ResetBits(GPIOC, LED1_PIN); /* Active LOW */

  DRV_Display_SetBlink(0x0F, 0); /* No blink */
  prv_UpdateDisplay();
  PRINT("[SET_PERIOD] Enter: -%02d days\r\n", s_days);
}

/*============================================================================
 * Set_WashPeriod_Run - Xu ly nut bam
 *============================================================================*/
void Set_WashPeriod_Run(BtnAction_t *actions) {
  BtnAction_t enter_act = actions[BTN_IDX_MENU];
  BtnAction_t up_act = actions[BTN_IDX_UP];
  BtnAction_t down_act = actions[BTN_IDX_DOWN];

  /*--------------------------------------------------------------
   * ENTER toggles between SHOW and EDIT
   *--------------------------------------------------------------*/
  if (enter_act == BTN_ACT_SHORT) {
    switch (s_state) {
    case PERIOD_SHOW:
      /* Bat dau chinh so */
      s_state = PERIOD_EDIT;
      DRV_Display_SetBlink(0x0C, 1); /* Stop blink D3+D2 */
      DRV_Display_SetBlink(0x03, 0); /* Blink D1+D0 (so ngay) */
      PRINT("[SET_PERIOD] Edit\r\n");
      break;

    case PERIOD_EDIT:
      /* Ket thuc chinh: luu vao EEPROM */
      s_state = PERIOD_SHOW;
      DRV_Display_SetBlink(0x0F, 0); /* Stop all blink */
      Data_SetWashPeriod(s_days);
      PRINT("[SET_PERIOD] Saved: -%02d days\r\n", s_days);
      APP_Settings_SetNavMode();
      break;
    }
    prv_UpdateDisplay();
    return;
  }

  /*--------------------------------------------------------------
   * UP/DOWN adjusts count (only in EDIT mode)
   *--------------------------------------------------------------*/
  if (s_state != PERIOD_EDIT)
    return;

  u8 do_up = (up_act == BTN_ACT_SHORT || up_act == BTN_ACT_LONG ||
              up_act == BTN_ACT_REPEAT);
  u8 do_down = (down_act == BTN_ACT_SHORT || down_act == BTN_ACT_LONG ||
                down_act == BTN_ACT_REPEAT);

  if (do_up) {
    if (s_days < WASH_PERIOD_MAX)
      s_days++;
    else
      s_days = 0;
    prv_UpdateDisplay();
  }
  if (do_down) {
    if (s_days > 0)
      s_days--;
    else
      s_days = WASH_PERIOD_MAX;
    prv_UpdateDisplay();
  }
}

/*============================================================================
 * Set_WashPeriod_Exit - Tat LED, tat blink
 *============================================================================*/
void Set_WashPeriod_Exit(void) {
  /* Tat LED9 va LED1 */
  GPIO_SetBits(GPIOA, LED9_PIN);
  GPIO_SetBits(GPIOC, LED1_PIN);
  BSP_LED_AllOff();
  DRV_Display_Blank();
  DRV_Display_SetBlink(0x0F, 0);
  s_state = PERIOD_SHOW;
  PRINT("[SET_PERIOD] Exit\r\n");
}
