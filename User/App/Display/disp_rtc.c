#include "debug.h"
#include "disp_rtc.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_ds1307.h"
#include "Drivers/drv_button.h"

/*============================================================================
 * Chức năng: Hiển thị thời gian thực (RTC)
 *============================================================================*/

void Disp_RTC_Enter(void) {
    /* TODO: Khởi tạo màn hình, chuẩn bị đọc RTC */
    if(DRV_Button_IsLocked()) {
        GPIO_ResetBits(GPIOA, LED7_PIN);
    }
    u8 hour = DS1307_GetHour();
    u8 minute = DS1307_GetMinute();
    DRV_Display_Time(hour, minute);
    PRINT("[RTC] Display RTC time: %02d:%02d\r\n", hour, minute);
}

void Disp_RTC_Run(void) {
    /* TODO: Đọc RTC định kỳ và hiển thị lên màn hình (HH:MM) */

}

void Disp_RTC_Exit(void) {
    /* TODO: Dọn dẹp trước khi chuyển sang chức năng khác */
    DRV_Display_Blank();
}
