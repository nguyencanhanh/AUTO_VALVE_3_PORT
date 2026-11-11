#ifndef __DISP_FILTER_H
#define __DISP_FILTER_H

#include "BSP/bsp_config.h"
#include <stdbool.h>

void Disp_Filter_Enter(u8 current_date, u8 current_hour, u8 current_minute);
void Disp_Filter_Run(u8 current_date, u8 current_hour, u8 current_minute);
void Disp_Filter_Exit(void);
bool Check_Should_Wash(u8 current_date, u8 current_hour, u8 current_minute);
void days_remaining_inc(void);
void Disp_Filter_ResetDays(void);

#endif /* __DISP_FILTER_H */
