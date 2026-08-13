#ifndef __DRV_POS_SENSOR_H
#define __DRV_POS_SENSOR_H

#include "BSP/bsp_config.h"

typedef enum {
    VALVE_POS_UNKNOWN = 0,
    VALVE_POS_1 = 1,  /* Position 1 (khe dai nhat: - -------) */
    VALVE_POS_2 = 2,  /* Position 2 (khe trung binh: - -----) */
    VALVE_POS_3 = 3,  /* Position 3 (khe ngan nhat: - ---) */
    VALVE_POS_STOP = 4,   /* Position 4 (valve da di chuyen) */
    VALVE_POS_NEXT_STEP = 5   /* Position 5 (valve da di chuyen den vi tri tiep theo) */
} ValvePosition_t;

/* Quet tin hieu cam bien va xac dinh vi tri (goi lien tuc trong main loop) */

void DRV_PosSensor_SetPosition(ValvePosition_t pos);

ValvePosition_t DRV_PosSensor_Process(void);

/* Tra ve vi tri da xac dinh hien tai */
ValvePosition_t DRV_PosSensor_GetCurrentPosition(void);

/* Doc trang thai pin sensor truc tiep (0 hoac 1) */
u8 DRV_PosSensor_ReadRawPin(void);

/* Tra ve so ms ke tu lan cuoi cam bien thay doi trang thai.
 * Dung de phat hien loi: motor chay ma cam bien khong doi -> ket/loi. */
u32 DRV_PosSensor_GetTimeSinceLastChange(void);

/* Reset timer. Goi khi bat motor de bat dau theo doi thay doi cam bien. */
void DRV_PosSensor_ResetChangeTimer(void);

#endif /* __DRV_POS_SENSOR_H */
