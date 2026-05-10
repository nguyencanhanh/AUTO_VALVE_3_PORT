/*****************************************************************************
 * File Name   : drv_buzzer.c
 * Layer       : Drivers - Hardware Abstraction
 * Description : Dieu whenen buzzer bao PB4 voi cac pattern am thanh
 *****************************************************************************/
#include "drv_buzzer.h"
#include "BSP/bsp_gpio.h"

/* Buoc trong pattern: {on_ms, off_ms} - ket thuc bang {0, 0} */
typedef struct { u16 on_ms; u16 off_ms; } BuzStep_t;

/* Pattern definitions */
static const BuzStep_t s_pat_beep1[]  = { {100,0}, {0,0} };
static const BuzStep_t s_pat_beep2[]  = { {100,150}, {100,0}, {0,0} };
static const BuzStep_t s_pat_beep3[]  = { {80,100}, {80,100}, {80,0}, {0,0} };
static const BuzStep_t s_pat_long[]   = { {600,0}, {0,0} };
static const BuzStep_t s_pat_alarm[]  = {
    {50,50},{50,50},{50,50},{50,50},{50,50},{50,50},{50,50},{50,50},{0,0}
};
static const BuzStep_t s_pat_boot[]   = { {150,0}, {0,0} };

static const BuzStep_t *s_patterns[] = {
    NULL,           /* BUZ_OFF   */
    s_pat_beep1,    /* BUZ_BEEP1 */
    s_pat_beep2,    /* BUZ_BEEP2 */
    s_pat_beep3,    /* BUZ_BEEP3 */
    s_pat_long,     /* BUZ_LONG  */
    s_pat_alarm,    /* BUZ_ALARM */
    s_pat_boot,     /* BUZ_BOOT  */
};

/* Runtime state */
static const BuzStep_t *s_cur_pat  = NULL;
static u8   s_cur_step = 0;
static u16  s_step_cnt = 0;
static u8   s_is_on    = 0;
static u8   s_is_busy  = 0;

/*============================================================================
 * DRV_Buzzer_Play
 *============================================================================*/
void DRV_Buzzer_Play(BuzzerPattern_t pattern)
{
    if (pattern == BUZ_OFF || pattern >= 7) { DRV_Buzzer_Stop(); return; }
    s_cur_pat  = s_patterns[pattern];
    s_cur_step = 0;
    s_step_cnt = 0;
    s_is_on    = 1;
    s_is_busy  = 1;
    BSP_Buzzer_On();
}

/*============================================================================
 * DRV_Buzzer_Stop
 *============================================================================*/
void DRV_Buzzer_Stop(void)
{
    s_cur_pat = NULL;
    s_is_busy = 0;
    s_is_on   = 0;
    BSP_Buzzer_Off();
}

/*============================================================================
 * DRV_Buzzer_IsBusy
 *============================================================================*/
u8 DRV_Buzzer_IsBusy(void) { return s_is_busy; }

/*============================================================================
 * DRV_Buzzer_Tick - Goi every 1ms from TIM2 ISR
 *============================================================================*/
void DRV_Buzzer_Tick(void)
{
    const BuzStep_t *step;

    if (!s_cur_pat || !s_is_busy) return;
    step = &s_cur_pat[s_cur_step];

    if (step->on_ms == 0 && step->off_ms == 0) { DRV_Buzzer_Stop(); return; }

    s_step_cnt++;

    if (s_is_on) {
        if (s_step_cnt >= step->on_ms) {
            s_step_cnt = 0; s_is_on = 0; BSP_Buzzer_Off();
            if (step->off_ms == 0) {
                step = &s_cur_pat[++s_cur_step];
                if (step->on_ms == 0 && step->off_ms == 0) DRV_Buzzer_Stop();
                else { s_is_on = 1; BSP_Buzzer_On(); }
            }
        }
    } else {
        if (s_step_cnt >= step->off_ms) {
            s_step_cnt = 0;
            step = &s_cur_pat[++s_cur_step];
            if (step->on_ms == 0 && step->off_ms == 0) DRV_Buzzer_Stop();
            else { s_is_on = 1; BSP_Buzzer_On(); }
        }
    }
}
