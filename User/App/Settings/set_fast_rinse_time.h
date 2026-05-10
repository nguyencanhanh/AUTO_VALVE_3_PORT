#ifndef __SET_FAST_RINSE_TIME_H
#define __SET_FAST_RINSE_TIME_H

#include "BSP/bsp_config.h"
#include "App/app_settings.h"

/* Setting 6: Set fast rinse (forward wash) duration (seconds) */
void Set_FastRinseTime_Enter(void);
void Set_FastRinseTime_Run(BtnAction_t *actions);
void Set_FastRinseTime_Exit(void);

#endif /* __SET_FAST_RINSE_TIME_H */
