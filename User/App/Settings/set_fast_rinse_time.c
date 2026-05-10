/*****************************************************************************
 * File Name   : set_fast_rinse_time.c
 * Layer       : App - Settings
 * Description : Setting 6 - Set fast rinse (forward wash) duration (seconds)
 *****************************************************************************/
#include "set_fast_rinse_time.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_button.h"

void Set_FastRinseTime_Enter(void) {
    /* TODO: Display current fast rinse duration */
}

void Set_FastRinseTime_Run(BtnAction_t *actions) {
    /* TODO: BTN_UP/BTN_DOWN to adjust fast rinse time */
}

void Set_FastRinseTime_Exit(void) {
    /* TODO: Save fast rinse duration to EEPROM */
}
