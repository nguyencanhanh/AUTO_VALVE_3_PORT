/*****************************************************************************
 * File Name   : app_error.c
 * Layer       : App
 * Description : Error Handler - Hien thi ma loi "E-X" va quan ly trang thai loi.
 *
 * Logic:
 *   - Khi co loi, hien thi "E-X" nhap nhay tren 7-segment
 *   - Neu co nhieu loi cung luc, luan phien hien thi moi 2 giay
 *   - Buzzer keu BUZ_ALARM lien tuc
 *   - BTN_SELECT (short press): xoa loi hien tai
 *   - BTN_MENU (short press): tat buzzer, loi van hien thi
 *****************************************************************************/
#include "app_error.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_buzzer.h"
#include "BSP/bsp_gpio.h"
#include "BSP/bsp_tim.h"
#include "debug.h"

/*============================================================================
 * Private variables
 *============================================================================*/
static uint8_t s_error_flags    = ERR_NONE;   /* Bitmask cac loi dang active */
static uint8_t s_display_index  = 0;          /* Index loi dang hien thi (0-3) */
static uint8_t s_buzzer_muted   = 0;          /* 1 = da tat buzzer */
static u32     s_rotate_timer   = 0;          /* Timer luan phien hien thi loi */

/* Bang map: bit position -> error number (1-4) */
static const uint8_t s_err_num[] = {
    ERR_NUM_SENSOR,      /* bit 0 → E1 */
    ERR_NUM_MOTOR,       /* bit 1 → E2 */
    ERR_NUM_VALVE_SLOW,  /* bit 2 → E3 */
    ERR_NUM_EEPROM       /* bit 3 → E4 */
};

#define ERR_BIT_COUNT  4   /* Tong so loai loi */

/*============================================================================
 * Private: Tim loi tiep theo dang active (vong tron)
 *============================================================================*/
static uint8_t prv_FindNextError(uint8_t start)
{
    uint8_t i;
    for (i = 0; i < ERR_BIT_COUNT; i++) {
        uint8_t idx = (start + i) % ERR_BIT_COUNT;
        if (s_error_flags & (1u << idx)) {
            return idx;
        }
    }
    return 0xFF;  /* Khong co loi nao */
}

/*============================================================================
 * Private: Hien thi "E-X" voi X = ma loi
 *============================================================================*/
static void prv_ShowError(uint8_t err_idx)
{
    uint8_t num = s_err_num[err_idx];
    /* D3='-', D2='E', D1=so loi, D0='-' */
    DRV_Display_Text(SEG_CHAR_DASH, SEG_CHAR_E, num, SEG_CHAR_DASH);
    /* Nhap nhay toan bo 4 digit */
    DRV_Display_SetBlink(0x0F, 1);
}

/*============================================================================
 * APP_Error_Set  - Goi tu bat ky module nao khi phat hien loi
 *============================================================================*/
void APP_Error_Set(uint8_t flag)
{
    s_error_flags |= flag;
    PRINT("[ERROR] Set flag: 0x%02X, total: 0x%02X\r\n", flag, s_error_flags);
}

/*============================================================================
 * APP_Error_Clear - Xoa mot loi cu the
 *============================================================================*/
void APP_Error_Clear(uint8_t flag)
{
    s_error_flags &= ~flag;
    PRINT("[ERROR] Clear flag: 0x%02X, remaining: 0x%02X\r\n", flag, s_error_flags);
}

/*============================================================================
 * APP_Error_HasError - Kiem tra co loi nao dang active khong
 *============================================================================*/
uint8_t APP_Error_HasError(void)
{
    return (s_error_flags != ERR_NONE) ? 1u : 0u;
}

/*============================================================================
 * APP_Error_GetFlags - Lay bitmask loi hien tai
 *============================================================================*/
uint8_t APP_Error_GetFlags(void)
{
    return s_error_flags;
}

/*============================================================================
 * APP_Error_Enter - Vao che do hien thi loi
 *============================================================================*/
void APP_Error_Enter(void)
{
    /* Tat het LED */
    BSP_LED_AllOff();

    /* Tim loi dau tien dang active */
    s_display_index = prv_FindNextError(0);
    s_buzzer_muted  = 0;
    s_rotate_timer  = BSP_GetTick();

    if (s_display_index != 0xFF) {
        prv_ShowError(s_display_index);
        PRINT("[ERROR] Enter: Showing E%d\r\n", s_err_num[s_display_index]);
    }

    /* Bat buzzer alarm */
    DRV_Buzzer_Play(BUZ_ALARM);
}

/*============================================================================
 * APP_Error_Run - Goi lien tuc trong main loop khi dang o trang thai loi
 *============================================================================*/
void APP_Error_Run(void)
{
    /* Neu khong con loi nao → thoat */
    if (s_error_flags == ERR_NONE) {
        return;
    }

    /* Luan phien hien thi loi moi 2 giay (neu co nhieu loi) */
    if (BSP_GetElapsed(s_rotate_timer) >= 2000) {
        s_rotate_timer = BSP_GetTick();

        /* Tim loi tiep theo */
        uint8_t next = prv_FindNextError(s_display_index + 1);
        if (next != 0xFF && next != s_display_index) {
            s_display_index = next;
            prv_ShowError(s_display_index);
            PRINT("[ERROR] Rotate: Showing E%d\r\n", s_err_num[s_display_index]);
        }
    }

    /* Dam bao hien thi loi hien tai van dung */
    if (!(s_error_flags & (1u << s_display_index))) {
        /* Loi hien tai da bi xoa → tim loi khac */
        uint8_t next = prv_FindNextError(0);
        if (next != 0xFF) {
            s_display_index = next;
            prv_ShowError(s_display_index);
        }
    }
}

/*============================================================================
 * APP_Error_Exit - Thoat che do loi
 *============================================================================*/
void APP_Error_Exit(void)
{
    DRV_Buzzer_Stop();
    DRV_Display_SetBlink(0x0F, 0);  /* Tat blink */
    DRV_Display_Blank();
    s_buzzer_muted = 0;
    PRINT("[ERROR] Exit error mode\r\n");
}
