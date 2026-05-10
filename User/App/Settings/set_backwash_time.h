#ifndef __SET_BACKWASH_TIME_H
#define __SET_BACKWASH_TIME_H

#include "BSP/bsp_config.h"
#include "App/app_settings.h"

/* Setting 5: Set backwash duration (seconds) */
void Set_BackwashTime_Enter(void);
void Set_BackwashTime_Run(BtnAction_t *actions);
void Set_BackwashTime_Exit(void);

#endif /* __SET_BACKWASH_TIME_H */
