/*****************************************************************************
 * File Name   : app_home.c
 * Layer       : App
 * Description : Home Screen - Standby display
 *
 * TODO: Implement the following logic:
 *   - Read RTC time every 1 second → DRV_Display_Time(hour, minute)
 *   - BTN_UP    → enter Manual mode
 *   - BTN_DOWN  → cycle through modes (Timer, Volume)
 *   - BTN_SELECT (long) → enter Settings menu
 *****************************************************************************/
#include "debug.h"
#include "app_home.h"
#include "BSP/bsp_tim.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_buzzer.h"
#include "Display/disp_filter.h"
#include "Display/disp_rtc.h"
#include "Display/disp_schedule.h"

typedef enum {
    HOME_DISP_FILTER = 0,
    HOME_DISP_RTC,
    HOME_DISP_SCHEDULE,
    HOME_DISP_MAX
} HomeDispState_t;

static HomeDispState_t s_home_state;
static u32 s_home_timer;

void APP_Home_Enter(void)
{
    /* Bắt đầu với chức năng Lọc Nước */
    s_home_state = HOME_DISP_FILTER;
    s_home_timer = BSP_GetTick();
    Disp_Filter_Enter();
}

void APP_Home_Run(void)
{
    /* Don't update display while settings is active */
    if (APP_Settings_IsActive()) return;

    u32 elapsed = BSP_GetElapsed(s_home_timer);

    /* Chuyển màn hình mỗi 15 giây (15000 ms) */
    if (elapsed >= 15000) {
        PRINT("[HOME] Time to change screen, current state: %d\r\n", s_home_state);
        /* Thoát trạng thái cũ */
        if (s_home_state == HOME_DISP_FILTER) Disp_Filter_Exit();
        else if (s_home_state == HOME_DISP_RTC) Disp_RTC_Exit();
        else if (s_home_state == HOME_DISP_SCHEDULE) Disp_Schedule_Exit();

        /* Chuyển sang trạng thái tiếp theo */
        s_home_state++;
        if (s_home_state >= HOME_DISP_MAX) {
            s_home_state = HOME_DISP_FILTER;
        }

        /* Vào trạng thái mới */
        if (s_home_state == HOME_DISP_FILTER) Disp_Filter_Enter();
        else if (s_home_state == HOME_DISP_RTC) Disp_RTC_Enter();
        else if (s_home_state == HOME_DISP_SCHEDULE) Disp_Schedule_Enter();

        s_home_timer = BSP_GetTick();
    }

    // /* Gọi hàm Run của trạng thái hiện tại để duy trì logic */
    // if (s_home_state == HOME_DISP_FILTER) Disp_Filter_Run();
    // else if (s_home_state == HOME_DISP_RTC) Disp_RTC_Run();
    // else if (s_home_state == HOME_DISP_SCHEDULE) Disp_Schedule_Run();
}

void APP_Home_Exit(void)
{
    /* Dọn dẹp trạng thái cuối cùng trước khi rời khỏi màn hình Home */
    if (s_home_state == HOME_DISP_FILTER) Disp_Filter_Exit();
    else if (s_home_state == HOME_DISP_RTC) Disp_RTC_Exit();
    else if (s_home_state == HOME_DISP_SCHEDULE) Disp_Schedule_Exit();
}
