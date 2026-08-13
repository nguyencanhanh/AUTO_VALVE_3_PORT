/*****************************************************************************
 * File Name   : drv_pos_sensor.c
 * Layer       : Drivers
 * Description : Driver cam bien vi tri valve (Position Sensor)
 *               Chan: POS_SENSOR_PIN = GPIOA / GPIO_Pin_7
 *               HIGH = valve dang mo (open)
 *               LOW  = valve dang dong (closed)
 *****************************************************************************/
#include "drv_pos_sensor.h"
#include "BSP/bsp_tim.h"
#include "BSP/bsp_config.h"
#include "debug.h"

static u32 s_high_start_time = 0;
static u32 s_last_change_time = 0;   /* Thoi diem cam bien thay doi trang thai lan cuoi */
static u8 s_last_state = 0;
static ValvePosition_t s_current_position = VALVE_POS_UNKNOWN;

/*============================================================================
 * DRV_PosSensor_Process - Doc va tinh toan do rong xung de tim vi tri (khe)
 *============================================================================*/

void DRV_PosSensor_SetPosition(ValvePosition_t pos){
    s_current_position = pos;
}

ValvePosition_t DRV_PosSensor_Process(void)
{
    u8 current_state = GPIO_ReadInputDataBit(POS_SENSOR_PORT, POS_SENSOR_PIN);

    if (current_state != s_last_state) {
        s_last_change_time = BSP_GetTick();  /* Cap nhat thoi diem thay doi */
        PRINT("[POS] Sensor changed: %d -> %d\r\n", s_last_state, current_state);
        if (current_state == Bit_SET) {
            /* Canh len: Bat dau di vao khe (cam bien bat dau doc muc cao) */
            s_high_start_time = BSP_GetTick();
        } 
        else {
            /* Canh xuong: Ra khoi khe. Do thoi gian hoat dong (do rong xung) */
            u32 pulse_width = BSP_GetElapsed(s_high_start_time);

            /* Xac dinh vi tri dua tren thoi gian xung (chu y dieu chinh theo motor) */
            if (pulse_width >= 4000) {
                s_current_position = VALVE_POS_1; /* Khe dai: vi tri 1 */
            } 
            else if (pulse_width >= 2000) {
                s_current_position = VALVE_POS_2; /* Khe vua: vi tri 2 */
            } 
            else if (pulse_width >= 600) {
                s_current_position = VALVE_POS_3; /* Khe ngan: vi tri 3 */
            } 
            PRINT("[POS] Sensor slot width: %d ms -> POS: %d\r\n", (int)pulse_width, s_current_position);
        }
        s_last_state = current_state;
    }

    return s_current_position;
}

ValvePosition_t DRV_PosSensor_GetCurrentPosition(void)
{
    return s_current_position;
}

u8 DRV_PosSensor_ReadRawPin(void)
{
    return (u8)GPIO_ReadInputDataBit(POS_SENSOR_PORT, POS_SENSOR_PIN);
}

/*============================================================================
 * DRV_PosSensor_GetTimeSinceLastChange
 * Tra ve so ms ke tu lan cuoi cam bien thay doi trang thai.
 * Dung de phat hien loi: motor chay ma cam bien khong doi → ket/loi.
 *============================================================================*/
u32 DRV_PosSensor_GetTimeSinceLastChange(void)
{
    if (s_last_change_time == 0) return 0;
    return BSP_GetElapsed(s_last_change_time);
}

/*============================================================================
 * DRV_PosSensor_ResetChangeTimer
 * Reset timer thay doi trang thai. Goi khi bat dau motor de bat dau theo doi.
 *============================================================================*/
void DRV_PosSensor_ResetChangeTimer(void)
{
    s_last_change_time = BSP_GetTick();
}
