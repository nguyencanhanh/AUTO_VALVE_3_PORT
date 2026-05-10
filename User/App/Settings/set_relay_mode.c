/*****************************************************************************
 * File Name   : set_relay_mode.c
 * Layer       : App - Settings
 * Description : Setting 7 - Set standby relay signal on/off behavior
 *               (Configure relay switching logic during standby mode)
 *****************************************************************************/
#include "set_relay_mode.h"
#include "Drivers/drv_display.h"
#include "Drivers/drv_button.h"

void Set_RelayMode_Enter(void) {
    /* TODO: Display current relay mode setting */
}

void Set_RelayMode_Run(BtnAction_t *actions) {
    /* TODO: BTN_UP/BTN_DOWN to toggle relay mode */
}

void Set_RelayMode_Exit(void) {
    /* TODO: Save relay mode to EEPROM */
}
