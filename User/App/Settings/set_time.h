#ifndef __SET_TIME_H
#define __SET_TIME_H

#include "BSP/bsp_config.h"
#include "App/app_settings.h"

/* Setting 1: Set RTC time (HH:MM) */
void Set_Time_Enter(void);
void Set_Time_Run(BtnAction_t *actions);
void Set_Time_Exit(void);

#endif /* __SET_TIME_H */
