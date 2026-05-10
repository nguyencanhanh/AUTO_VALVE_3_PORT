#include "debug.h"
#include "disp_filter.h"
#include "Drivers/drv_display.h"
#include "BSP/bsp_gpio.h"
#include "Drivers/drv_button.h"

/*============================================================================
 * Chức năng: Lọc Nước (Water Filtration)
 *============================================================================*/

void Disp_Filter_Enter(void) {
    /* 1. Tắt tất cả LED trước khi vào chế độ mới */
    BSP_LED_AllOff();
    /* 2. Bật LED 1 (PC7), LED 7 (PA0), LED 9 (PA5) (Active LOW) */
    GPIO_ResetBits(GPIOC, LED1_PIN);
    GPIO_ResetBits(GPIOA, LED9_PIN);
    if(DRV_Button_IsLocked()) {
        GPIO_ResetBits(GPIOA, LED7_PIN);
    }

    /* 3. Hiển thị "01-1" lên màn hình 7 đoạn */
    DRV_Display_Text(1, 0x11, 0, 1);
    PRINT("[FILTER] Display filter time: 01-1\r\n");
}

void Disp_Filter_Run(void) {
    
}

void Disp_Filter_Exit(void) {
    /* Dọn dẹp tắt hết LED khi thoát chức năng */
    BSP_LED_AllOff();
}
