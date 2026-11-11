/*****************************************************************************
 * File Name   : set_fast_rinse_time.c
 * Layer       : App - Settings
 * Description : Setting 6 - Cai dat thoi gian suc rua xuoi (phut)
 *
 * Hien thi: " -XX" (dash + so phut, VD: " -15" = 15 phut)
 *   00 = bo qua qua trinh suc rua xuoi
 *   CHU Y: Neu ca suc xuoi (fast_rinse) va suc nguoc (backwash) deu = 00
 *          thi tu dong dat fast_rinse = 10 (khong the bo qua ca 2)
 *
 * LED12 ON = Don vi tinh la PHUT
 * LED5  ON = Che do suc xuoi
 *
 * Sub-states:
 *   FR_SHOW - Hien thi " -XX", khong nhap nhay
 *   FR_EDIT - Nhap nhay so XX, UP/DOWN chinh (00-99)
 *
 * Button mapping:
 *   ENTER (MENU)  - Cycle: SHOW -> EDIT -> SHOW (save)
 *   UP            - Tang (short/long/repeat)
 *   DOWN          - Giam (short/long/repeat)
 *   BACK (SELECT) - Xu ly boi app_settings (exit to NAV)
 *****************************************************************************/
#include "set_fast_rinse_time.h"
#include "App/Data/data_store.h"
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_display.h"
#include "debug.h"

#define FAST_RINSE_MAX 99
#define FAST_RINSE_SAFETY 10 /* Gia tri tu dong neu ca 2 deu = 0 */

typedef enum { FR_SHOW = 0, FR_EDIT } FREditState_t;

static FREditState_t s_state;
static u8 s_minutes;

/*----------------------------------------------------------------------------
 * Hien thi " -XX" len 7-segment
 *----------------------------------------------------------------------------*/
static void prv_UpdateDisplay(void) {
  u8 tens = s_minutes / 10;
  u8 ones = s_minutes % 10;
  DRV_Display_Text(0x03, SEG_CHAR_DASH, tens, ones);
}

/*----------------------------------------------------------------------------
 * Kiem tra an toan: khong cho phep ca backwash va fast_rinse deu = 0
 *----------------------------------------------------------------------------*/
static void prv_SafetyCheck(void) {
  AppSettings_t *cfg = Data_GetSettings();
  if (s_minutes == 0 && cfg->backwash_sec == 0) {
    s_minutes = FAST_RINSE_SAFETY;
    PRINT("[SET_FR] Safety: both=0, set fast_rinse=%d\r\n", FAST_RINSE_SAFETY);
  }
}

/*============================================================================
 * Set_FastRinseTime_Enter - Doc EEPROM, bat LED12+LED5, hien thi
 *============================================================================*/
void Set_FastRinseTime_Enter(void) {
  AppSettings_t *cfg = Data_GetSettings();
  s_minutes = cfg->fast_rinse_sec;
  if (s_minutes > FAST_RINSE_MAX) {
    s_minutes = 0;
  }
  s_state = FR_SHOW;

  /* LED12 ON = don vi phut, LED5 ON = che do suc xuoi */
  GPIO_ResetBits(GPIOA, LED12_PIN); /* Active LOW */
  GPIO_ResetBits(GPIOA, LED5_PIN);  /* Active LOW */

  DRV_Display_SetBlink(0x0F, 0); /* No blink */
  prv_UpdateDisplay();
  PRINT("[SET_FR] Enter: -%02d min\r\n", s_minutes);
}

/*============================================================================
 * Set_FastRinseTime_Run - Xu ly nut bam
 *============================================================================*/
void Set_FastRinseTime_Run(BtnAction_t *actions) {
  BtnAction_t enter_act = actions[BTN_IDX_MENU];
  BtnAction_t up_act = actions[BTN_IDX_UP];
  BtnAction_t down_act = actions[BTN_IDX_DOWN];

  /*--------------------------------------------------------------
   * ENTER toggles between SHOW and EDIT
   *--------------------------------------------------------------*/
  if (enter_act == BTN_ACT_SHORT) {
    switch (s_state) {
    case FR_SHOW:
      /* Bat dau chinh so */
      s_state = FR_EDIT;
      DRV_Display_SetBlink(0x0C, 1); /* Stop blink D3+D2 */
      DRV_Display_SetBlink(0x03, 0); /* Blink D1+D0 */
      PRINT("[SET_FR] Edit\r\n");
      break;

    case FR_EDIT:
      /* Kiem tra an toan truoc khi luu */
      prv_SafetyCheck();
      /* Ket thuc chinh: luu vao EEPROM */
      s_state = FR_SHOW;
      DRV_Display_SetBlink(0x0F, 0); /* Stop all blink */
      Data_SetFastRinseTime(s_minutes);
      PRINT("[SET_FR] Saved: -%02d min\r\n", s_minutes);
      APP_Settings_SetNavMode();
      break;
    }
    prv_UpdateDisplay();
    return;
  }

  /*--------------------------------------------------------------
   * UP/DOWN adjusts count (only in EDIT mode)
   *--------------------------------------------------------------*/
  if (s_state != FR_EDIT)
    return;

  u8 do_up = (up_act == BTN_ACT_SHORT || up_act == BTN_ACT_LONG ||
              up_act == BTN_ACT_REPEAT);
  u8 do_down = (down_act == BTN_ACT_SHORT || down_act == BTN_ACT_LONG ||
                down_act == BTN_ACT_REPEAT);

  if (do_up) {
    if (s_minutes < FAST_RINSE_MAX)
      s_minutes++;
    else
      s_minutes = 0;
    prv_UpdateDisplay();
  }
  if (do_down) {
    if (s_minutes > 0)
      s_minutes--;
    else
      s_minutes = FAST_RINSE_MAX;
    prv_UpdateDisplay();
  }
}

/*============================================================================
 * Set_FastRinseTime_Exit - Tat LED, tat blink
 *============================================================================*/
void Set_FastRinseTime_Exit(void) {
  /* Tat LED12 va LED5 */
  GPIO_SetBits(GPIOA, LED12_PIN);
  GPIO_SetBits(GPIOA, LED5_PIN);

  DRV_Display_Blank();
  DRV_Display_SetBlink(0x0F, 0);
  BSP_LED_AllOff();
  s_state = FR_SHOW;
  PRINT("[SET_FR] Exit\r\n");
}
