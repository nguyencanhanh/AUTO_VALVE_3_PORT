#include "debug.h"
#include "disp_rtc.h"
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_ds1307.h"
#include "Drivers/drv_button.h"

/*============================================================================
 * Chức năng: Hiển thị thời gian thực (RTC)
 *============================================================================*/

void Disp_RTC_Enter(u8 hour, u8 minute) {
    /* Khởi tạo màn hình, đọc RTC */
    if(DRV_Button_IsLocked()) {
        GPIO_ResetBits(GPIOA, LED7_PIN);
    }
    GPIO_ResetBits(GPIOA, LED6_PIN);
    DRV_Display_Time(hour, minute);
}

void Disp_RTC_Exit(void) {
    /* Dọn dẹp trước khi chuyển sang chức năng khác */
    DRV_Display_Blank();
    BSP_LED_AllOff();
}
