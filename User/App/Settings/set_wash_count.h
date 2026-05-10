#ifndef __SET_WASH_COUNT_H
#define __SET_WASH_COUNT_H

#include "BSP/bsp_config.h"
#include "App/app_settings.h"

/* Setting 3: Set number of wash cycles per period */
void Set_WashCount_Enter(void);
void Set_WashCount_Run(BtnAction_t *actions);
void Set_WashCount_Exit(void);

#endif /* __SET_WASH_COUNT_H */
