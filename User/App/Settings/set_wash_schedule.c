/*****************************************************************************
 * File Name   : set_wash_schedule.c
 * Layer       : App - Settings
 * Description : Setting 2 - Set wash cycle schedule time
 *               (Fixed time of day when wash cycle starts)
 *****************************************************************************/
#include "set_wash_schedule.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_button.h"

void Set_WashSchedule_Enter(void) {
    /* TODO: Display current wash schedule time */
}

void Set_WashSchedule_Run(BtnAction_t *actions) {
    /* TODO: BTN_UP/BTN_DOWN to adjust schedule time */
}

void Set_WashSchedule_Exit(void) {
    /* TODO: Save schedule to EEPROM */
}
