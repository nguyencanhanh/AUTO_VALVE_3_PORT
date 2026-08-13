#ifndef __DRV_AT24C64_H
#define __DRV_AT24C64_H

#include "BSP/bsp_config.h"

/* AT24C64 Specs */
#define AT24C64_MAX_ADDR 0x1FFF
#define AT24C64_PG_SIZE 32

/* I2C Addresses */
#define AT24C64_BASE_ADDR_W 0xA0
#define AT24C64_BASE_ADDR_R 0xA1
#define AT24C64_SET_ADDR 0x00 /* A0=A1=A2=GND */
#define AT24C64_HW_ADDR (AT24C64_BASE_ADDR_W | (AT24C64_SET_ADDR << 1))

/** Possible errors */
typedef enum {
  AT24_OK = 0,
  AT24_INIT_ERR = 1,
  AT24_W_BYTE_ERR = 2,
  AT24_W_BYTES_ERR = 3,
  AT24_R_BYTE_ERR = 4,
  AT24_R_BYTES_ERR = 5
} AT24Cxx_ERR_t;

/* Tinh gon API cho duy nhat 1 chip AT24C64 tren board */
void DRV_AT24C64_Init(void);
AT24Cxx_ERR_t DRV_AT24C64_WriteByte(u16 addr, u8 data);
AT24Cxx_ERR_t DRV_AT24C64_ReadByte(u16 addr, u8 *data);
AT24Cxx_ERR_t DRV_AT24C64_WriteBuffer(u16 addr, u8 *buf, u16 len);
AT24Cxx_ERR_t DRV_AT24C64_ReadBuffer(u16 addr, u8 *buf, u16 len);

#endif /* __DRV_AT24C64_H */
