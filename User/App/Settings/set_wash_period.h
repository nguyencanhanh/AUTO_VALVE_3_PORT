#ifndef __SET_WASH_PERIOD_H
#define __SET_WASH_PERIOD_H

#include "BSP/bsp_config.h"
#include "App/app_settings.h"

/* Setting 4: Set wash cycle period (interval between cycles) */
void Set_WashPeriod_Enter(void);
void Set_WashPeriod_Run(BtnAction_t *actions);
void Set_WashPeriod_Exit(void);

#endif /* __SET_WASH_PERIOD_H */
