/*****************************************************************************
 * File Name   : set_relay_mode.c
 * Layer       : App - Settings
 * Description : Setting 7 - Cai dat che do relay
 *
 * Hien thi: "b-01" hoac "b-02"
 *   b-01 (mode 0): Relay dong trong toan bo chu trinh suc rua
 *   b-02 (mode 1): Relay chi dong khi chuyen doi va do trang thai valve
 *                   (Khi dang trong chu trinh va may hien thi "-00-")
 *
 * Sub-states:
 *   RELAY_SHOW - Hien thi b-0X, khong nhap nhay
 *   RELAY_EDIT - Nhap nhay so, UP/DOWN chuyen mode
 *
 * Button mapping:
 *   ENTER (MENU)  - Cycle: SHOW -> EDIT -> SHOW (save)
 *   UP/DOWN       - Chuyen doi giua mode 01 va 02
 *   BACK (SELECT) - Xu ly boi app_settings (exit to NAV)
 *****************************************************************************/
#include "set_relay_mode.h"
#include "App/Data/data_store.h"
#include "Drivers/drv_display.h"
#include "debug.h"

typedef enum { RELAY_SHOW = 0, RELAY_EDIT } RelayEditState_t;

static RelayEditState_t s_state;
static u8 s_mode; /* 0 = mode1 (b-01), 1 = mode2 (b-02) */

/*----------------------------------------------------------------------------
 * Hien thi "b-0X" len 7-segment
 *   D3 = 'b'
 *   D2 = '-'
 *   D1 = 0
 *   D0 = 1 hoac 2
 *----------------------------------------------------------------------------*/
static void prv_UpdateDisplay(void) {
  DRV_Display_Text(0x0B, SEG_CHAR_DASH, 0, s_mode + 1);
}

/*============================================================================
 * Set_RelayMode_Enter - Doc EEPROM, hien thi b-0X
 *============================================================================*/
void Set_RelayMode_Enter(void) {
  AppSettings_t *cfg = Data_GetSettings();
  s_mode = cfg->relay_mode;
  if (s_mode > 1) {
    s_mode = 0;
  }
  s_state = RELAY_SHOW;

  DRV_Display_SetBlink(0x0F, 0); /* No blink */
  prv_UpdateDisplay();
  PRINT("[SET_RELAY] Enter: b-%02d\r\n", s_mode + 1);
}

/*============================================================================
 * Set_RelayMode_Run - Xu ly nut bam
 *============================================================================*/
void Set_RelayMode_Run(BtnAction_t *actions) {
  BtnAction_t enter_act = actions[BTN_IDX_MENU];
  BtnAction_t up_act = actions[BTN_IDX_UP];
  BtnAction_t down_act = actions[BTN_IDX_DOWN];

  /*--------------------------------------------------------------
   * ENTER toggles between SHOW and EDIT
   *--------------------------------------------------------------*/
  if (enter_act == BTN_ACT_SHORT) {
    switch (s_state) {
    case RELAY_SHOW:
      /* Bat dau chon mode */
      s_state = RELAY_EDIT;
      DRV_Display_SetBlink(0x0C, 1); /* Stop blink D3+D2 */
      DRV_Display_SetBlink(0x03, 0); /* Blink D1+D0 (so mode) */
      PRINT("[SET_RELAY] Edit\r\n");
      break;

    case RELAY_EDIT:
      /* Ket thuc: luu vao EEPROM */
      s_state = RELAY_SHOW;
      DRV_Display_SetBlink(0x0F, 0); /* Stop all blink */
      Data_SetRelayMode(s_mode);
      PRINT("[SET_RELAY] Saved: b-%02d\r\n", s_mode + 1);
      break;
    }
    prv_UpdateDisplay();
    return;
  }

  /*--------------------------------------------------------------
   * UP/DOWN toggles mode (only in EDIT mode)
   *--------------------------------------------------------------*/
  if (s_state != RELAY_EDIT)
    return;

  u8 do_up = (up_act == BTN_ACT_SHORT || up_act == BTN_ACT_LONG ||
              up_act == BTN_ACT_REPEAT);
  u8 do_down = (down_act == BTN_ACT_SHORT || down_act == BTN_ACT_LONG ||
                down_act == BTN_ACT_REPEAT);

  if (do_up || do_down) {
    /* Chi co 2 mode, toggle qua lai */
    s_mode = (s_mode == 0) ? 1 : 0;
    prv_UpdateDisplay();
  }
}

/*============================================================================
 * Set_RelayMode_Exit - Tat blink
 *============================================================================*/
void Set_RelayMode_Exit(void) {
  DRV_Display_Blank();
  DRV_Display_SetBlink(0x0F, 0);
  s_state = RELAY_SHOW;
  PRINT("[SET_RELAY] Exit\r\n");
}
