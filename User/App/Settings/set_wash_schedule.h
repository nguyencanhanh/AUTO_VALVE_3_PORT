#ifndef __SET_WASH_SCHEDULE_H
#define __SET_WASH_SCHEDULE_H

#include "BSP/bsp_config.h"
#include "App/app_settings.h"

/* Setting 2: Set wash cycle schedule time (fixed time of day) */
void Set_WashSchedule_Enter(void);
void Set_WashSchedule_Run(BtnAction_t *actions);
void Set_WashSchedule_Exit(void);

#endif /* __SET_WASH_SCHEDULE_H */
