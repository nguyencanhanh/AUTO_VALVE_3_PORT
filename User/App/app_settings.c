/*****************************************************************************
 * File Name   : app_settings.c
 * Layer       : App
 * Description : Settings menu with integrated button handling.
 *
 * States:
 *   IDLE  - Unlocked, home screen showing, waiting for ENTER
 *   NAV   - Selecting setting 1-7 with UP/DOWN, LED8 ON
 *   EDIT  - Inside a sub-screen (e.g. set_time)
 *
 * Auto-repeat: holding UP/DOWN fires BTN_ACT_REPEAT every 150ms
 * Beep: every button press produces a beep
 *****************************************************************************/
#include "app_settings.h"
#include "BSP/bsp_gpio.h"
#include "BSP/bsp_tim.h"
#include "Drivers/drv_button.h"
#include "Drivers/drv_buzzer.h"
#include "Drivers/drv_display.h"
#include "app_home.h"
#include "debug.h"

/* Include all 7 setting sub-screens */
#include "Settings/set_backwash_time.h"
#include "Settings/set_fast_rinse_time.h"
#include "Settings/set_relay_mode.h"
#include "Settings/set_time.h"
#include "Settings/set_wash_count.h"
#include "Settings/set_wash_period.h"
#include "Settings/set_wash_schedule.h"

/*============================================================================
 * Button reading (timestamp-based, runs in main loop)
 *============================================================================*/
#define BTN_REPEAT_START_MS 500u    /* Start auto-repeat after 500ms hold */
#define BTN_REPEAT_INTERVAL_MS 150u /* Repeat every 150ms */

static const uint32_t s_btn_pins[BTN_IDX_COUNT] = {
    BTN_MENU_PIN, BTN_UP_PIN, BTN_DOWN_PIN, BTN_SELECT_PIN};

/* Per-button tracking */
static u32 s_btn_press_time[BTN_IDX_COUNT];
static u32 s_btn_last_repeat[BTN_IDX_COUNT];
static u8 s_btn_state[BTN_IDX_COUNT]; /* 0=released, 1=pressed */
static u8 s_btn_long_fired[BTN_IDX_COUNT];
static u8 s_btn_raw_prev[BTN_IDX_COUNT];

/*============================================================================
 * Settings state machine
 *============================================================================*/
typedef enum {
  SET_MODE_IDLE = 0, /* Unlocked, home showing, wait for ENTER */
  SET_MODE_NAV,      /* Navigating settings 1-7, LED8 ON */
  SET_MODE_EDIT      /* Inside a sub-screen */
} SetMode_t;

static SettingsItem_t s_current_item = SETTINGS_TIME;
static SetMode_t s_mode = SET_MODE_IDLE;

/*----------------------------------------------------------------------------
 * Sub-screen dispatch arrays
 *----------------------------------------------------------------------------*/
typedef void (*SubScreenEnterExit_t)(void);
typedef void (*SubScreenRun_t)(BtnAction_t*);

static const SubScreenEnterExit_t prv_EnterSubScreen[SETTINGS_ITEM_COUNT] = {
    Set_Time_Enter, Set_WashSchedule_Enter, Set_WashCount_Enter,
    Set_WashPeriod_Enter, Set_BackwashTime_Enter, Set_FastRinseTime_Enter, Set_RelayMode_Enter
};

static const SubScreenRun_t prv_RunSubScreen[SETTINGS_ITEM_COUNT] = {
    Set_Time_Run, Set_WashSchedule_Run, Set_WashCount_Run,
    Set_WashPeriod_Run, Set_BackwashTime_Run, Set_FastRinseTime_Run, Set_RelayMode_Run
};

static const SubScreenEnterExit_t prv_ExitSubScreen[SETTINGS_ITEM_COUNT] = {
    Set_Time_Exit, Set_WashSchedule_Exit, Set_WashCount_Exit,
    Set_WashPeriod_Exit, Set_BackwashTime_Exit, Set_FastRinseTime_Exit, Set_RelayMode_Exit
};

/*----------------------------------------------------------------------------
 * Read one button: returns NONE / SHORT / LONG / REPEAT
 *----------------------------------------------------------------------------*/
static BtnAction_t prv_ReadButton(BtnIdx_t idx) {
  uint8_t raw = BSP_Button_ReadRaw(s_btn_pins[idx]);
  BtnAction_t action = BTN_ACT_NONE;
  u32 now = BSP_GetTick();

  if (raw && !s_btn_raw_prev[idx]) {
    /* Rising edge: button just pressed */
    s_btn_press_time[idx] = now;
    s_btn_last_repeat[idx] = now;
    s_btn_state[idx] = 0;
    s_btn_long_fired[idx] = 0;
  }

  if (raw) {
    u32 held = now - s_btn_press_time[idx];

    /* Debounce */
    if (!s_btn_state[idx] && held >= DEBOUNCE_MS) {
      s_btn_state[idx] = 1;
    }

    if (s_btn_state[idx]) {
      if (!s_btn_long_fired[idx] && held >= BTN_REPEAT_START_MS) {
        /* First long press trigger */
        s_btn_long_fired[idx] = 1;
        s_btn_last_repeat[idx] = now;
        action = BTN_ACT_LONG;
      } else if (s_btn_long_fired[idx]) {
        /* Auto-repeat while held */
        if ((now - s_btn_last_repeat[idx]) >= BTN_REPEAT_INTERVAL_MS) {
          s_btn_last_repeat[idx] = now;
          action = BTN_ACT_REPEAT;
        }
      }
    }
  } else if (s_btn_raw_prev[idx]) {
    /* Falling edge: released */
    if (s_btn_state[idx] && !s_btn_long_fired[idx]) {
      action = BTN_ACT_SHORT;
    }
    s_btn_state[idx] = 0;
    s_btn_long_fired[idx] = 0;
  }

  s_btn_raw_prev[idx] = raw;
  return action;
}

/*============================================================================
 * APP_Settings_Enter - Called once when entering settings
 *============================================================================*/
void APP_Settings_Enter(void) {
  u8 i;
  s_current_item = SETTINGS_TIME;
  s_mode = SET_MODE_IDLE;

  for (i = 0; i < BTN_IDX_COUNT; i++) {
    s_btn_press_time[i] = 0;
    s_btn_last_repeat[i] = 0;
    s_btn_state[i] = 0;
    s_btn_long_fired[i] = 0;
    s_btn_raw_prev[i] = 0;
  }
}

/*============================================================================
 * APP_Settings_Run - Called from main loop
 *============================================================================*/
void APP_Settings_Run(void) {
  u8 i;
  BtnAction_t actions[BTN_IDX_COUNT];
  u8 any_activity = 0;
  BtnAction_t sel_action;

  sel_action = prv_ReadButton(BTN_IDX_SELECT);

  if (sel_action == BTN_ACT_SHORT) {
    DRV_Button_ResetIdle();
    DRV_Buzzer_Play(BUZ_BEEP1);
    if (APP_Home_IsWashing()) {
      APP_Home_ManualNextStep();
    } else if (DRV_Button_IsLocked()) {
      PRINT("[SETTINGS] Ignored SELECT because keyboard is locked\r\n");
    } else if (s_mode == SET_MODE_IDLE) {
      APP_Home_StartManualWash();
    } else if (s_mode == SET_MODE_EDIT) {
      if (prv_ExitSubScreen[s_current_item]) prv_ExitSubScreen[s_current_item]();
      s_mode = SET_MODE_NAV;
      if (prv_EnterSubScreen[s_current_item]) prv_EnterSubScreen[s_current_item]();
      GPIO_ResetBits(GPIOA, LED8_PIN); /* Ensure LED8 stays ON */
      PRINT("[SETTINGS] Returned to NAV from EDIT\r\n");
    } else if (s_mode == SET_MODE_NAV) {
      if (prv_ExitSubScreen[s_current_item]) prv_ExitSubScreen[s_current_item]();
      GPIO_SetBits(GPIOA, LED8_PIN); /* LED8 OFF */
      DRV_Display_SetBlink(0x0F, 0);
      s_mode = SET_MODE_IDLE;
      APP_Home_Enter();
      PRINT("[SETTINGS] Exited to Home\r\n");
    }
    return;
  }

  /* Skip all app-level key actions while keyboard is locked */
  if (DRV_Button_IsLocked()) {
    /* If we were active, force back to IDLE */
    if (s_mode != SET_MODE_IDLE) {
      if (s_mode == SET_MODE_EDIT)
        if (prv_ExitSubScreen[s_current_item]) prv_ExitSubScreen[s_current_item]();
      GPIO_SetBits(GPIOA, LED8_PIN); /* LED8 OFF */
      DRV_Display_SetBlink(0x0F, 0);
      s_mode = SET_MODE_IDLE;
      APP_Home_Enter(); /* Restore home display */
      PRINT("[SETTINGS] Auto-lock triggered, forced to IDLE\r\n");
    }
    return;
  }

  /* Read all 4 buttons */
  for (i = 0; i < BTN_IDX_COUNT; i++) {
    actions[i] = (i == BTN_IDX_SELECT) ? sel_action : prv_ReadButton(i);
    if (actions[i] != BTN_ACT_NONE) {
      any_activity = 1;
    }
  }

  /* Reset auto-lock idle timer on any activity */
  if (any_activity) {
    DRV_Button_ResetIdle();
    DRV_Buzzer_Play(BUZ_BEEP1); /* Beep on every button press */
  }

  /*==================================================================
   * IDLE mode: unlocked, home still showing, wait for ENTER
   *==================================================================*/
  if (s_mode == SET_MODE_IDLE) {
    if (actions[BTN_IDX_MENU] == BTN_ACT_SHORT) {
      if (APP_Home_IsWashing()) {
        DRV_Buzzer_Play(BUZ_BEEP2); /* Beep error, block settings during wash */
        return;
      }
      /* Enter settings mode */
      s_mode = SET_MODE_NAV;
      s_current_item = SETTINGS_TIME;
      GPIO_ResetBits(GPIOA, LED8_PIN); /* LED8 ON (Active LOW) */
      if (prv_EnterSubScreen[s_current_item]) prv_EnterSubScreen[s_current_item]();
      PRINT("[SETTINGS] Entered NAV mode, LED8 ON\r\n");
    }
    return;
  }

  /*==================================================================
   * NAV mode: select setting 1-7
   *==================================================================*/
  if (s_mode == SET_MODE_NAV) {

    if (actions[BTN_IDX_UP] == BTN_ACT_SHORT) {
      if (prv_ExitSubScreen[s_current_item]) prv_ExitSubScreen[s_current_item]();
      s_current_item++;
      PRINT("[SETTINGS] s_current_item = %d\r\n", s_current_item);
      if (s_current_item >= SETTINGS_ITEM_COUNT)
        s_current_item = SETTINGS_TIME;
      if (prv_EnterSubScreen[s_current_item]) prv_EnterSubScreen[s_current_item]();
      GPIO_ResetBits(GPIOA, LED8_PIN); /* Ensure LED8 stays ON */
    }

    if (actions[BTN_IDX_DOWN] == BTN_ACT_SHORT) {
      PRINT("[SETTINGS] s_current_item = %d\r\n", s_current_item);
      if (prv_ExitSubScreen[s_current_item]) prv_ExitSubScreen[s_current_item]();
      if (s_current_item > 0)
        s_current_item--;
      else
        s_current_item = SETTINGS_ITEM_COUNT - 1;
      if (prv_EnterSubScreen[s_current_item]) prv_EnterSubScreen[s_current_item]();
      GPIO_ResetBits(GPIOA, LED8_PIN); /* Ensure LED8 stays ON */
    }

    if (actions[BTN_IDX_MENU] == BTN_ACT_SHORT) {
      /* Enter selected sub-screen */
      s_mode = SET_MODE_EDIT;
      if (prv_RunSubScreen[s_current_item]) prv_RunSubScreen[s_current_item](actions);
    }
    return;
  }

  /*==================================================================
   * EDIT mode: inside a sub-screen
   *==================================================================*/
  if (s_mode == SET_MODE_EDIT) {
    /* Delegate to sub-screen */
    if (prv_RunSubScreen[s_current_item]) prv_RunSubScreen[s_current_item](actions);
  }
}

/*============================================================================
 * APP_Settings_Exit
 *============================================================================*/
void APP_Settings_Exit(void) {
  if (s_mode == SET_MODE_EDIT)
    if (prv_ExitSubScreen[s_current_item]) prv_ExitSubScreen[s_current_item]();
  GPIO_SetBits(GPIOA, LED8_PIN); /* LED8 OFF */
  DRV_Display_SetBlink(0x0F, 0);
  s_mode = SET_MODE_IDLE;
  s_current_item = SETTINGS_TIME;
  PRINT("[SETTINGS] Closed\r\n");
}

/*============================================================================
 * APP_Settings_IsActive - Returns 1 when settings controls the display
 *============================================================================*/
u8 APP_Settings_IsActive(void) { return (s_mode != SET_MODE_IDLE); }
