/*****************************************************************************
 * File Name   : set_backwash_time.c
 * Layer       : App - Settings
 * Description : Setting 5 - Cai dat thoi gian suc rua nguoc (phut)
 *
 * Hien thi: " -XX" (dash + so phut, VD: " -30" = 30 phut)
 *   00 = bo qua qua trinh suc rua nguoc
 *   CHU Y: Neu ca suc xuoi (fast_rinse) va suc nguoc (backwash) deu = 00
 *          thi tu dong dat backwash = 10 (khong the bo qua ca 2)
 *
 * LED12 ON = Don vi tinh la PHUT
 * LED2  ON = Che do suc nguoc
 *
 * Sub-states:
 *   BW_SHOW - Hien thi " -XX", khong nhap nhay
 *   BW_EDIT - Nhap nhay so XX, UP/DOWN chinh (00-99)
 *
 * Button mapping:
 *   ENTER (MENU)  - Cycle: SHOW -> EDIT -> SHOW (save)
 *   UP            - Tang (short/long/repeat)
 *   DOWN          - Giam (short/long/repeat)
 *   BACK (SELECT) - Xu ly boi app_settings (exit to NAV)
 *****************************************************************************/
#include "set_backwash_time.h"
#include "App/Data/data_store.h"
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_display.h"
#include "debug.h"

#define BACKWASH_MAX 99
#define BACKWASH_SAFETY 10 /* Gia tri tu dong neu ca 2 deu = 0 */

typedef enum { BW_SHOW = 0, BW_EDIT } BWEditState_t;

static BWEditState_t s_state;
static u8 s_minutes;

/*----------------------------------------------------------------------------
 * Hien thi " -XX" len 7-segment
 *----------------------------------------------------------------------------*/
static void prv_UpdateDisplay(void) {
  u8 tens = s_minutes / 10;
  u8 ones = s_minutes % 10;
  DRV_Display_Text(0x02, SEG_CHAR_DASH, tens, ones);
}

/*----------------------------------------------------------------------------
 * Kiem tra an toan: khong cho phep ca backwash va fast_rinse deu = 0
 *----------------------------------------------------------------------------*/
static void prv_SafetyCheck(void) {
  AppSettings_t *cfg = Data_GetSettings();
  if (s_minutes == 0 && cfg->fast_rinse_sec == 0) {
    s_minutes = BACKWASH_SAFETY;
    PRINT("[SET_BW] Safety: both=0, set backwash=%d\r\n", BACKWASH_SAFETY);
  }
}

/*============================================================================
 * Set_BackwashTime_Enter - Doc EEPROM, bat LED12+LED2, hien thi
 *============================================================================*/
void Set_BackwashTime_Enter(void) {
  AppSettings_t *cfg = Data_GetSettings();
  s_minutes = cfg->backwash_sec;
  if (s_minutes > BACKWASH_MAX) {
    s_minutes = 0;
  }
  s_state = BW_SHOW;

  /* LED12 ON = don vi phut, LED2 ON = che do suc nguoc */
  GPIO_ResetBits(GPIOA, LED12_PIN); /* Active LOW */
  GPIO_ResetBits(GPIOC, LED2_PIN);  /* Active LOW */

  DRV_Display_SetBlink(0x0F, 0); /* No blink */
  prv_UpdateDisplay();
  PRINT("[SET_BW] Enter: -%02d min\r\n", s_minutes);
}

/*============================================================================
 * Set_BackwashTime_Run - Xu ly nut bam
 *============================================================================*/
void Set_BackwashTime_Run(BtnAction_t *actions) {
  BtnAction_t enter_act = actions[BTN_IDX_MENU];
  BtnAction_t up_act = actions[BTN_IDX_UP];
  BtnAction_t down_act = actions[BTN_IDX_DOWN];

  /*--------------------------------------------------------------
   * ENTER toggles between SHOW and EDIT
   *--------------------------------------------------------------*/
  if (enter_act == BTN_ACT_SHORT) {
    switch (s_state) {
    case BW_SHOW:
      /* Bat dau chinh so */
      s_state = BW_EDIT;
      DRV_Display_SetBlink(0x0C, 1); /* Stop blink D3+D2 */
      DRV_Display_SetBlink(0x03, 0); /* Blink D1+D0 */
      PRINT("[SET_BW] Edit\r\n");
      break;

    case BW_EDIT:
      /* Kiem tra an toan truoc khi luu */
      prv_SafetyCheck();
      /* Ket thuc chinh: luu vao EEPROM */
      s_state = BW_SHOW;
      DRV_Display_SetBlink(0x0F, 0); /* Stop all blink */
      Data_SetBackwashTime(s_minutes);
      PRINT("[SET_BW] Saved: -%02d min\r\n", s_minutes);
      APP_Settings_SetNavMode();
      break;
    }
    prv_UpdateDisplay();
    return;
  }

  /*--------------------------------------------------------------
   * UP/DOWN adjusts count (only in EDIT mode)
   *--------------------------------------------------------------*/
  if (s_state != BW_EDIT)
    return;

  u8 do_up = (up_act == BTN_ACT_SHORT || up_act == BTN_ACT_LONG ||
              up_act == BTN_ACT_REPEAT);
  u8 do_down = (down_act == BTN_ACT_SHORT || down_act == BTN_ACT_LONG ||
                down_act == BTN_ACT_REPEAT);

  if (do_up) {
    if (s_minutes < BACKWASH_MAX)
      s_minutes++;
    else
      s_minutes = 0;
    prv_UpdateDisplay();
  }
  if (do_down) {
    if (s_minutes > 0)
      s_minutes--;
    else
      s_minutes = BACKWASH_MAX;
    prv_UpdateDisplay();
  }
}

/*============================================================================
 * Set_BackwashTime_Exit - Tat LED, tat blink
 *============================================================================*/
void Set_BackwashTime_Exit(void) {
  /* Tat LED12 va LED2 */
  GPIO_SetBits(GPIOA, LED12_PIN);
  GPIO_SetBits(GPIOC, LED2_PIN);
  BSP_LED_AllOff();
  DRV_Display_Blank();
  DRV_Display_SetBlink(0x0F, 0);
  s_state = BW_SHOW;
  PRINT("[SET_BW] Exit\r\n");
}
