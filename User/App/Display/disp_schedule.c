/*****************************************************************************
 * File Name   : disp_schedule.c
 * Layer       : App - Display
 * Description : Hien thi thoi gian chu ky suc rua tren man hinh Home
 *               Chi hien thi, khong chinh sua (viec chinh sua nam trong
 *               Settings/set_wash_schedule.c)
 *****************************************************************************/
#include "disp_schedule.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_button.h"
#include "BSP/bsp_gpio.h"
#include "App/Data/data_store.h"
#include "debug.h"

/*============================================================================
 * Disp_Schedule_Enter - Hien thi thoi gian suc rua hien tai
 *============================================================================*/
void Disp_Schedule_Enter(void)
{
    AppSettings_t *cfg = Data_GetSettings();
    DRV_Display_Time(cfg->wash_sched_hour, cfg->wash_sched_min);
    DRV_Display_SetDP(1, 1); /* Dau cham o giua sang co dinh, khong nhap nhay */
    if(DRV_Button_IsLocked()) {
        GPIO_ResetBits(GPIOA, LED7_PIN);
    }
    GPIO_SetBits(GPIOA, LED6_PIN);
    PRINT("[SCHEDULE] Display: %02d:%02d\r\n", cfg->wash_sched_hour, cfg->wash_sched_min);
}

/*============================================================================
 * Disp_Schedule_Exit - Don dep
 *============================================================================*/
void Disp_Schedule_Exit(void)
{
    DRV_Display_Blank();
    BSP_LED_AllOff();
}
