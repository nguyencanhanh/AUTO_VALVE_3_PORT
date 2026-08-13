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
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_buzzer.h"
#include "Drivers/drv_pos_sensor.h"
#include "Drivers/drv_ds1307.h"
#include "Display/disp_filter.h"
#include "Display/disp_rtc.h"
#include "Display/disp_schedule.h"
#include "Display/disp_backwash.h"
#include "Display/disp_fast_rinse.h"
#include "App/app_settings.h"
#include "App/app_error.h"
#include "App/Data/data_store.h"

typedef enum {
    HOME_DISP_FILTER = 0,
    HOME_DISP_RTC,
    HOME_DISP_SCHEDULE,
    HOME_DISP_MAX,
    HOME_DISP_BACKWASHING,
    HOME_DISP_RINSING,
    HOME_DISP_ERROR,
} HomeDispState_t;

static HomeDispState_t s_home_state;
static u32 s_home_timer;
static u8 current_date;
static u8 current_hour;
static u8 current_minute;
volatile u8 remain_count;
bool is_next_day = true;

void APP_Home_Enter(void)
{
    s_home_timer = BSP_GetTick();
    current_date = DS1307_GetDate();
    current_hour = DS1307_GetHour();
    current_minute = DS1307_GetMinute();

    /* ---- Kiem tra EEPROM: co wash dang do tu lan mat dien truoc? ---- */
    u8 saved_phase = 0;
    u8 saved_remain = 0;
    Data_LoadWashState(&saved_phase, &saved_remain);

    if (saved_phase == WASH_PHASE_BW) {
        /* Phuc hoi: tiep tuc Backwash */
        remain_count = saved_remain;
        Disp_Backwash_Enter();
        s_home_state = HOME_DISP_BACKWASHING;
        PRINT("[HOME] RESUME: Backwash, remain=%d\r\n", remain_count);
        return;
    }
    else if (saved_phase == WASH_PHASE_FR) {
        /* Phuc hoi: tiep tuc Fast Rinse */
        remain_count = saved_remain;
        Disp_FastRinse_Enter();
        s_home_state = HOME_DISP_RINSING;
        PRINT("[HOME] RESUME: Fast Rinse, remain=%d\r\n", remain_count);
        return;
    }

    /* Khong co wash dang do → bat dau binh thuong */
    s_home_state = HOME_DISP_FILTER;
    Disp_Filter_Enter(current_date, current_hour, current_minute);
}

void APP_Home_Run(void)
{
    /* Don't update display while settings is active */
    if (APP_Settings_IsActive()) return;

    /* ---- Error check: chuyen sang error screen ngay khi co loi ---- */
    if (APP_Error_HasError()) {
        if (s_home_state != HOME_DISP_ERROR) {
            /* Thoat trang thai cu */
            if (s_home_state == HOME_DISP_FILTER)     Disp_Filter_Exit();
            else if (s_home_state == HOME_DISP_RTC)   Disp_RTC_Exit();
            else if (s_home_state == HOME_DISP_SCHEDULE) Disp_Schedule_Exit();
            else if (s_home_state == HOME_DISP_BACKWASHING) Disp_Backwash_Exit();
            else if (s_home_state == HOME_DISP_RINSING) Disp_FastRinse_Exit();

            APP_Error_Enter();
            s_home_state = HOME_DISP_ERROR;
            PRINT("[HOME] Switched to ERROR display\r\n");
        }
        APP_Error_Run();
        return;
    } else if (s_home_state == HOME_DISP_ERROR) {
        /* Loi da duoc xoa → quay lai man hinh Filter */
        APP_Error_Exit();
        s_home_state = HOME_DISP_FILTER;
        s_home_timer = BSP_GetTick();
        Disp_Filter_Enter(current_date, current_hour, current_minute);
        PRINT("[HOME] Error cleared, back to FILTER\r\n");
    }

    u32 elapsed = BSP_GetElapsed(s_home_timer);

    /* Chuyển màn hình mỗi 15 giây (15000 ms) */
    if (elapsed >= 15000) {
        current_date = DS1307_GetDate();
        current_hour = DS1307_GetHour();
        current_minute = DS1307_GetMinute();

        if(current_hour == 0 && current_minute == 0 && is_next_day){
            days_remaining_inc();
            is_next_day = false;
        }

        if(current_hour !=0 || current_minute != 0){
            is_next_day = true;
        }
        
        /* Thoát trạng thái cũ */
        
        PRINT("[HOME] Time to change screen, current state: %d\r\n", s_home_state);
        if (s_home_state == HOME_DISP_FILTER) Disp_Filter_Exit();
        else if (s_home_state == HOME_DISP_RTC) Disp_RTC_Exit();
        else if (s_home_state == HOME_DISP_SCHEDULE) Disp_Schedule_Exit();
        if (Check_Should_Wash(current_date, current_hour, current_minute) ){
            if(s_home_state != HOME_DISP_BACKWASHING && s_home_state != HOME_DISP_RINSING){
                PRINT("[HOME] Should wash\r\n");
                remain_count = 0;
                Data_SaveWashState(WASH_PHASE_BW, remain_count);
                Disp_Backwash_Enter();
                s_home_state = HOME_DISP_BACKWASHING;
            }
        }
        else if(s_home_state < HOME_DISP_MAX){           
            /* Chuyển sang trạng thái tiếp theo */
            s_home_state++;
            if (s_home_state >= HOME_DISP_MAX) {
                s_home_state = HOME_DISP_FILTER;
            }

            /* Vào trạng thái mới */
            if (s_home_state == HOME_DISP_FILTER) Disp_Filter_Enter(current_date, current_hour, current_minute);
            else if (s_home_state == HOME_DISP_RTC) Disp_RTC_Enter(current_hour, current_minute);
            else if (s_home_state == HOME_DISP_SCHEDULE) Disp_Schedule_Enter();

        }
        s_home_timer = BSP_GetTick();
    }
    if(s_home_state == HOME_DISP_BACKWASHING){
        Disp_Backwash_Run();
        if(DRV_PosSensor_GetCurrentPosition() == VALVE_POS_NEXT_STEP){
            Disp_Backwash_Exit();
            s_home_state = HOME_DISP_RINSING;
            if(remain_count >= Data_GetSettings()->wash_count){
                remain_count = 0;
                s_home_state = HOME_DISP_FILTER;
                Disp_Filter_Enter(current_date, current_hour, current_minute);
                DRV_PosSensor_SetPosition(VALVE_POS_UNKNOWN);
                Data_ClearWashState();  /* Hoan thanh → xoa trang thai */
                PRINT("[HOME] Finished washing, return to schedule screen\r\n");
                return;
            }
            DRV_PosSensor_SetPosition(VALVE_POS_UNKNOWN);
            Data_SaveWashState(WASH_PHASE_FR, remain_count);  /* Luu: chuyen sang FR */
            Disp_FastRinse_Enter();
        }
    }
    else if(s_home_state == HOME_DISP_RINSING){
        Disp_FastRinse_Run();
        if(DRV_PosSensor_GetCurrentPosition() == VALVE_POS_NEXT_STEP){
            Disp_FastRinse_Exit();
            remain_count++;
            Data_SaveWashState(WASH_PHASE_BW, remain_count);  /* Luu: chuyen sang BW */
            Disp_Backwash_Enter();
            DRV_PosSensor_SetPosition(VALVE_POS_UNKNOWN);
            s_home_state = HOME_DISP_BACKWASHING;
        }
    }
}

void APP_Home_Exit(void)
{
    /* Dọn dẹp trạng thái cuối cùng trước khi rời khỏi màn hình Home */
    if (s_home_state == HOME_DISP_FILTER) Disp_Filter_Exit();
    else if (s_home_state == HOME_DISP_RTC) Disp_RTC_Exit();
    else if (s_home_state == HOME_DISP_SCHEDULE) Disp_Schedule_Exit();
    else if (s_home_state == HOME_DISP_ERROR) APP_Error_Exit();
}

/*============================================================================
 * APP_Home_StartManualWash - Suc rua thu cong khi bam BTN_SELECT
 * Thuc hien chu trinh giong nhu den thoi gian suc rua tu dong.
 *============================================================================*/
void APP_Home_StartManualWash(void)
{
    /* Khong bat dau neu dang suc rua hoac dang loi */
    if (s_home_state == HOME_DISP_BACKWASHING ||
        s_home_state == HOME_DISP_RINSING ||
        s_home_state == HOME_DISP_ERROR) {
        PRINT("[HOME] Manual wash ignored: already washing or error\r\n");
        return;
    }

    /* Thoat trang thai hien thi hien tai */
    if (s_home_state == HOME_DISP_FILTER) Disp_Filter_Exit();
    else if (s_home_state == HOME_DISP_RTC) Disp_RTC_Exit();
    else if (s_home_state == HOME_DISP_SCHEDULE) Disp_Schedule_Exit();

    /* Bat dau chu trinh suc rua (giong nhu tu dong) */
    remain_count = 0;
    Data_SaveWashState(WASH_PHASE_BW, remain_count);  /* Luu vao EEPROM */
    Disp_Backwash_Enter();
    s_home_state = HOME_DISP_BACKWASHING;
    s_home_timer = BSP_GetTick();
    PRINT("[HOME] Manual wash started!\r\n");
}

/*============================================================================
 * APP_Home_IsWashing - Kiem tra co dang trong che do suc rua khong
 *============================================================================*/
u8 APP_Home_IsWashing(void)
{
    return (s_home_state == HOME_DISP_BACKWASHING ||
            s_home_state == HOME_DISP_RINSING) ? 1u : 0u;
}

/*============================================================================
 * APP_Home_ManualNextStep - Chuyen sang buoc tiep theo trong suc rua thu cong
 * Dung motor, timer, force VALVE_POS_NEXT_STEP de state machine tu transition.
 *============================================================================*/
void APP_Home_ManualNextStep(void)
{
    if (s_home_state != HOME_DISP_BACKWASHING &&
        s_home_state != HOME_DISP_RINSING) {
        return;  /* Khong dang wash → bo qua */
    }

    /* Dung moi thu dang chay */
    BSP_Motor_Off();
    Timer3_Stop();
    BSP_ResetCountMinutes();

    /* Force transition: dat position = NEXT_STEP
     * Vong lap APP_Home_Run() se tu dong chuyen sang buoc tiep theo */
    DRV_PosSensor_SetPosition(VALVE_POS_NEXT_STEP);
    PRINT("[HOME] Manual: Skip to next step (state=%d, remain=%d)\r\n",
          s_home_state, remain_count);
}
