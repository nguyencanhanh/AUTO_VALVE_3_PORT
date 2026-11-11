/*****************************************************************************
 * File Name   : set_time.c
 * Layer       : App - Settings
 * Description : Setting 1 - Set RTC time (HH:MM)
 *
 * Sub-states:
 *   TIME_SHOW      - Display current time, no blinking
 *   TIME_EDIT_HOUR - Hour digits blink, UP/DOWN adjusts hour
 *   TIME_EDIT_MIN  - Minute digits blink, UP/DOWN adjusts minute
 *
 * Button mapping (received from app_settings):
 *   ENTER (MENU)  - Cycle: SHOW -> EDIT_HOUR -> EDIT_MIN -> SHOW (save)
 *   UP            - Increment hour/minute (short press or auto-repeat)
 *   DOWN          - Decrement hour/minute (short press or auto-repeat)
 *   BACK (SELECT) - Handled by app_settings (exit to NAV)
 *****************************************************************************/
#include "set_time.h"
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_ds1307.h"
#include "debug.h"

typedef enum { TIME_SHOW = 0, TIME_EDIT_HOUR, TIME_EDIT_MIN } TimeEditState_t;

static TimeEditState_t s_state;
static u8 s_hour;
static u8 s_minute;

/*----------------------------------------------------------------------------
 * Refresh display with current hour:minute
 *----------------------------------------------------------------------------*/
static void prv_UpdateDisplay(void) { DRV_Display_Time(s_hour, s_minute); }

/*============================================================================
 * Set_Time_Enter - Read RTC, show time, no blink
 *============================================================================*/
void Set_Time_Enter(void) {
  s_hour = DS1307_GetHour();
  s_minute = DS1307_GetMinute();
  s_state = TIME_SHOW;

  DRV_Display_SetBlink(0x0F, 0); /* No blink */
  GPIO_ResetBits(GPIOA, LED6_PIN);
  prv_UpdateDisplay();
  PRINT("[SET_TIME] Enter: %02d:%02d\r\n", s_hour, s_minute);
}

/*============================================================================
 * Set_Time_Run - Handle button actions
 *============================================================================*/
void Set_Time_Run(BtnAction_t *actions) {
  BtnAction_t enter_act = actions[BTN_IDX_MENU];
  BtnAction_t up_act = actions[BTN_IDX_UP];
  BtnAction_t down_act = actions[BTN_IDX_DOWN];

  /*--------------------------------------------------------------
   * ENTER cycles through states
   *--------------------------------------------------------------*/
  if (enter_act == BTN_ACT_SHORT) {
    switch (s_state) {
    case TIME_SHOW:
      /* Start editing hour */
      s_state = TIME_EDIT_HOUR;
      DRV_Display_SetBlink(0x0C, 0); /* Blink D3+D2 (hour digits) */
      DRV_Display_SetBlink(0x03, 1); /* Stop blink D1+D0 */
      PRINT("[SET_TIME] Edit HOUR\r\n");
      break;

    case TIME_EDIT_HOUR:
      /* Switch to editing minute */
      s_state = TIME_EDIT_MIN;
      DRV_Display_SetBlink(0x0C, 1); /* Stop blink D3+D2 */
      DRV_Display_SetBlink(0x03, 0); /* Blink D1+D0 (minute digits) */
      PRINT("[SET_TIME] Edit MINUTE\r\n");
      break;

    case TIME_EDIT_MIN:
      /* Finish editing: save to RTC */
      s_state = TIME_SHOW;
      DRV_Display_SetBlink(0x0F, 0); /* Stop all blink */
      DS1307_SetHour(s_hour);
      DS1307_SetMinute(s_minute);
      PRINT("[SET_TIME] Saved: %02d:%02d\r\n", s_hour, s_minute);
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

  if (s_state == TIME_EDIT_HOUR) {
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

  if (s_state == TIME_EDIT_MIN) {
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
 * Set_Time_Exit - Stop blinking, discard unsaved changes
 *============================================================================*/
void Set_Time_Exit(void) {
  GPIO_SetBits(GPIOA, LED6_PIN);
  BSP_LED_AllOff();
  DRV_Display_Blank();
  s_state = TIME_SHOW;
  PRINT("[SET_TIME] Exit\r\n");
}
