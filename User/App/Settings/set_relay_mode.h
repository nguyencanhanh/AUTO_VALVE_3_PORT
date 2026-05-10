#ifndef __SET_RELAY_MODE_H
#define __SET_RELAY_MODE_H

#include "BSP/bsp_config.h"
#include "App/app_settings.h"

/* Setting 7: Set standby relay signal on/off behavior */
void Set_RelayMode_Enter(void);
void Set_RelayMode_Run(BtnAction_t *actions);
void Set_RelayMode_Exit(void);

#endif /* __SET_RELAY_MODE_H */
