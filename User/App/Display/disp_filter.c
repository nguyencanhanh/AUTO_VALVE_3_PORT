#include "disp_filter.h"
#include "App/Data/data_store.h"
#include "BSP/bsp_gpio.h"
#include "BSP/bsp_tim.h"
#include "Drivers/drv_button.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_ds1307.h"
#include "debug.h"

static u8 days_remaining = 0;
static AppSettings_t *cfg;

/*----------------------------------------------------------------------------
 * Hien thi thoi gian con lai len 7-segment + LED chi thi
 *----------------------------------------------------------------------------*/
static void prv_ShowCountdown(u8 current_date, u8 current_hour,
                              u8 current_minute) {
  cfg = Data_GetSettings();
  if (cfg->wash_period_hours > 99) {
    cfg->wash_period_hours = 10;
  }

  GPIO_ResetBits(GPIOA, LED9_PIN); /* Bat LED9 (ngay) */

  u8 tens = (cfg->wash_period_hours - days_remaining) / 10;
  u8 ones = (cfg->wash_period_hours - days_remaining) % 10;
  DRV_Display_Text(0x01, SEG_CHAR_DASH, tens, ones);
  PRINT("[FILTER] Countdown: %d days\r\n", (int)(cfg->wash_period_hours - days_remaining));
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
}

bool Check_Should_Wash(u8 current_date, u8 current_hour, u8 current_minute) {
  if (days_remaining == cfg->wash_period_hours &&
      current_hour == cfg->wash_sched_hour &&
      (current_minute >= cfg->wash_sched_min &&
       current_minute <= cfg->wash_sched_min + 1)) {
    days_remaining = 0;
    return true;
  }
  return false;
}

void days_remaining_inc(void) { days_remaining++; }