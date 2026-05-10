/*****************************************************************************
 * File Name   : set_wash_period.c
 * Layer       : App - Settings
 * Description : Setting 4 - Set wash cycle period
 *               (Interval/frequency between wash cycles)
 *****************************************************************************/
#include "set_wash_period.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_button.h"

void Set_WashPeriod_Enter(void) {
    /* TODO: Display current wash period */
}

void Set_WashPeriod_Run(BtnAction_t *actions) {
    /* TODO: BTN_UP/BTN_DOWN to adjust period */
}

void Set_WashPeriod_Exit(void) {
    /* TODO: Save wash period to EEPROM */
}
