/*****************************************************************************
 * File Name   : set_wash_count.c
 * Layer       : App - Settings
 * Description : Setting 3 - Cai dat so lan lap lai suc xuoi/nguoc
 *
 * Hien thi: F-XX (F = Forward/Backward cycle, XX = so lan lap 00-20)
 *   F-00 = 1 chu trinh (suc xuoi -> suc nguoc)
 *   F-01 = 2 chu trinh (suc xuoi -> suc nguoc -> suc xuoi -> suc nguoc)
 *   ...
 *   F-20 = toi da 20 lan lap lai
 *
 * Sub-states:
 *   COUNT_SHOW - Hien thi F-XX, khong nhap nhay
 *   COUNT_EDIT - Nhap nhay so XX, UP/DOWN chinh (00-20)
 *
 * Button mapping:
 *   ENTER (MENU)  - Cycle: SHOW -> EDIT -> SHOW (save)
 *   UP            - Tang so lan (short/long/repeat)
 *   DOWN          - Giam so lan (short/long/repeat)
 *   BACK (SELECT) - Xu ly boi app_settings (exit to NAV)
 *****************************************************************************/
#include "set_wash_count.h"
#include "App/Data/data_store.h"
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_display.h"
#include "debug.h"

#define WASH_COUNT_MAX 20

typedef enum { COUNT_SHOW = 0, COUNT_EDIT } CountEditState_t;

static CountEditState_t s_state;
static u8 s_count;

/*----------------------------------------------------------------------------
 * Hien thi F-XX len 7-segment
 *   D3 = 'F'  (dung SEG_CHAR_E voi segment khac, hoac ta dung index 0x0F)
 *   D2 = '-'
 *   D1 = hang chuc cua count
 *   D0 = hang don vi cua count
 *----------------------------------------------------------------------------*/
static void prv_UpdateDisplay(void) {
  u8 tens = s_count / 10;
  u8 ones = s_count % 10;
  /* F = SEG_CHAR_P thay the vi khong co font F rieng,
     hoac dung so 0x0F neu font co. Dung SEG_CHAR_E voi y nghia tuong tu */
  DRV_Display_Text(0x0F, SEG_CHAR_DASH, tens, ones);
}

/*============================================================================
 * Set_WashCount_Enter - Doc EEPROM, hien thi F-XX, khong blink
 *============================================================================*/
void Set_WashCount_Enter(void) {
  AppSettings_t *cfg = Data_GetSettings();
  s_count = cfg->wash_count;
  if (s_count > WASH_COUNT_MAX) {
    s_count = 0;
  }
  s_state = COUNT_SHOW;

  DRV_Display_SetBlink(0x0F, 0); /* No blink */
  prv_UpdateDisplay();
  PRINT("[SET_COUNT] Enter: F-%02d\r\n", s_count);
}

/*============================================================================
 * Set_WashCount_Run - Xu ly nut bam
 *============================================================================*/
void Set_WashCount_Run(BtnAction_t *actions) {
  BtnAction_t enter_act = actions[BTN_IDX_MENU];
  BtnAction_t up_act = actions[BTN_IDX_UP];
  BtnAction_t down_act = actions[BTN_IDX_DOWN];

  /*--------------------------------------------------------------
   * ENTER toggles between SHOW and EDIT
   *--------------------------------------------------------------*/
  if (enter_act == BTN_ACT_SHORT) {
    switch (s_state) {
    case COUNT_SHOW:
      /* Bat dau chinh so */
      s_state = COUNT_EDIT;
      DRV_Display_SetBlink(0x0C, 1); /* Stop blink D3+D2 */
      DRV_Display_SetBlink(0x03, 0); /* Blink D1+D0 (minute digits) */
      PRINT("[SET_COUNT] Edit\r\n");
      break;

    case COUNT_EDIT:
      /* Ket thuc chinh: luu vao EEPROM */
      s_state = COUNT_SHOW;
      Data_SetWashCount(s_count);
      DRV_Display_SetBlink(0x0F, 0); /* Stop all blink */
      PRINT("[SET_COUNT] Saved: F-%02d\r\n", s_count);
      APP_Settings_SetNavMode();
      break;
    }
    prv_UpdateDisplay();
    return;
  }

  /*--------------------------------------------------------------
   * UP/DOWN adjusts count (only in EDIT mode)
   *--------------------------------------------------------------*/
  if (s_state != COUNT_EDIT)
    return;

  u8 do_up = (up_act == BTN_ACT_SHORT || up_act == BTN_ACT_LONG ||
              up_act == BTN_ACT_REPEAT);
  u8 do_down = (down_act == BTN_ACT_SHORT || down_act == BTN_ACT_LONG ||
                down_act == BTN_ACT_REPEAT);

  if (do_up) {
    if (s_count < WASH_COUNT_MAX)
      s_count++;
    else
      s_count = 0;
    prv_UpdateDisplay();
  }
  if (do_down) {
    if (s_count > 0)
      s_count--;
    else
      s_count = WASH_COUNT_MAX;
    prv_UpdateDisplay();
  }
}

/*============================================================================
 * Set_WashCount_Exit - Tat blink, khong luu neu chua nhan Enter xong
 *============================================================================*/
void Set_WashCount_Exit(void) {
  BSP_LED_AllOff();
  DRV_Display_Blank();
  DRV_Display_SetBlink(0x0F, 0);
  s_state = COUNT_SHOW;
  PRINT("[SET_COUNT] Exit\r\n");
}
