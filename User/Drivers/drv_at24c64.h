#ifndef __DRV_AT24C64_H
#define __DRV_AT24C64_H

#include "BSP/bsp_config.h"

void DRV_AT24C64_Init(void);
u8 DRV_AT24C64_WriteByte(u16 addr, u8 data);
u8 DRV_AT24C64_ReadByte(u16 addr, u8 *data);
u8 DRV_AT24C64_WriteBuffer(u16 addr, u8 *buf, u16 len);
u8 DRV_AT24C64_ReadBuffer(u16 addr, u8 *buf, u16 len);

#endif /* __DRV_AT24C64_H */
