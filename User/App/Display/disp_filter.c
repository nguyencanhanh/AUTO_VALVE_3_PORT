#include "disp_filter.h"
#include "App/Data/data_store.h"
#include "BSP/bsp_gpio.h"
#include "BSP/bsp_tim.h"
#include "Drivers/drv_button.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_ds1307.h"
#include "debug.h"

static u8 s_days_elapsed = 0;
static u8 s_days_loaded = 0;
static AppSettings_t *cfg;

static void prv_EnsureDaysLoaded(void) {
  if (!s_days_loaded) {
    s_days_elapsed = Data_GetDaysElapsed();
    if (s_days_elapsed > 99u) {
      s_days_elapsed = 0u;
      Data_SetDaysElapsed(0u);
    }
    s_days_loaded = 1u;
  }
}

static u8 prv_GetWashPeriodDays(const AppSettings_t *settings) {
  u8 period = settings->wash_period_hours;

  if (period > 99u) {
    return 10u;
  }
  return period;
}

/*----------------------------------------------------------------------------
 * Hien thi thoi gian con lai len 7-segment + LED chi thi
 *----------------------------------------------------------------------------*/
static void prv_ShowCountdown(u8 current_date, u8 current_hour,
                              u8 current_minute) {
  prv_EnsureDaysLoaded();
  cfg = Data_GetSettings();
  u8 period_days = prv_GetWashPeriodDays(cfg);
  if (s_days_elapsed > period_days) {
    s_days_elapsed = period_days;
    Data_SetDaysElapsed(s_days_elapsed);
  }
  if (DRV_Button_IsLocked()) {
    GPIO_ResetBits(GPIOA, LED7_PIN);
  }
  GPIO_ResetBits(GPIOA, LED9_PIN); /* Bat LED9 (ngay) */
  GPIO_ResetBits(GPIOC, LED1_PIN); /* Bat LED1 (che do loc) */

  u8 remaining = period_days - s_days_elapsed;
  u8 tens = remaining / 10;
  u8 ones = remaining % 10;
  DRV_Display_Text(0x01, SEG_CHAR_DASH, tens, ones);
  PRINT("[FILTER] Countdown: %d days (elapsed=%d)\r\n", (int)remaining, (int)s_days_elapsed);
}

/*============================================================================
 * Disp_Filter_Enter
 *============================================================================*/
void Disp_Filter_Enter(u8 current_date, u8 current_hour, u8 current_minute) {
  /* 1. Tắt tất cả LED trước khi vào chế độ mới */
  BSP_LED_AllOff();
  prv_ShowCountdown(current_date, current_hour, current_minute);
}

/*============================================================================
 * Disp_Filter_Exit
 *============================================================================*/
void Disp_Filter_Exit(void) {
  /* Dọn dẹp tắt hết LED khi thoát chức năng */
  BSP_LED_AllOff();
  DRV_Display_Blank();
}

bool Check_Should_Wash(u8 current_date, u8 current_hour, u8 current_minute) {
  prv_EnsureDaysLoaded();
  AppSettings_t *settings = Data_GetSettings();
  u8 period_days = prv_GetWashPeriodDays(settings);
  u8 sched_min = settings->wash_sched_min;
  bool minute_match = (current_minute == sched_min) ||
                      (sched_min < 59u && current_minute == (u8)(sched_min + 1u));

  if (s_days_elapsed >= period_days &&
      current_hour == settings->wash_sched_hour &&
      minute_match) {
    s_days_elapsed = 0;
    Data_SetDaysElapsed(0);
    return true;
  }
  return false;
}

void days_remaining_inc(void) {
  prv_EnsureDaysLoaded();
  s_days_elapsed++;
  Data_SetDaysElapsed(s_days_elapsed);
}

void Disp_Filter_ResetDays(void) {
  prv_EnsureDaysLoaded();
  s_days_elapsed = 0;
  Data_SetDaysElapsed(0);
}