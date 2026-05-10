#include "drv_at24c64.h"

#define EEPROM_ADDR ((AT24C64_ADDR << 1))
#define I2C_TIMEOUT 0x1000

/* Helper function cho co voi Timeout */
static u8 AT24C64_WaitEvent(uint32_t event) {
  u32 timeout = I2C_TIMEOUT;
  while (!I2C_CheckEvent(I2C1, event)) {
    if (--timeout == 0)
      return 1;
  }
  return 0;
}

void DRV_AT24C64_Init(void) {
  /* I2C1 already is initialize at BSP_I2C_Init() */
}

u8 DRV_AT24C64_WriteByte(u16 addr, u8 data) {
  while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    ;

  I2C_GenerateSTART(I2C1, ENABLE);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    goto error;

  I2C_Send7bitAddress(I2C1, EEPROM_ADDR, I2C_Direction_Transmitter);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    goto error;

  I2C_SendData(I2C1, (u8)(addr >> 8));
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    goto error;

  I2C_SendData(I2C1, (u8)(addr & 0xFF));
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    goto error;

  I2C_SendData(I2C1, data);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    goto error;

  I2C_GenerateSTOP(I2C1, ENABLE);
  Delay_Ms(5); /* Time luu of AT24C64 mat ~5ms */
  return 0;

error:
  I2C_GenerateSTOP(I2C1, ENABLE);
  return 1;
}

u8 DRV_AT24C64_ReadByte(u16 addr, u8 *data) {
  while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    ;

  I2C_GenerateSTART(I2C1, ENABLE);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    goto error;

  I2C_Send7bitAddress(I2C1, EEPROM_ADDR, I2C_Direction_Transmitter);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    goto error;

  I2C_SendData(I2C1, (u8)(addr >> 8));
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    goto error;

  I2C_SendData(I2C1, (u8)(addr & 0xFF));
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    goto error;

  I2C_GenerateSTART(I2C1, ENABLE);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    goto error;

  I2C_Send7bitAddress(I2C1, EEPROM_ADDR, I2C_Direction_Receiver);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    goto error;

  I2C_AcknowledgeConfig(I2C1, DISABLE);
  I2C_GenerateSTOP(I2C1, ENABLE);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
    goto error;

  *data = I2C_ReceiveData(I2C1);
  I2C_AcknowledgeConfig(I2C1, ENABLE);

  return 0;

error:
  I2C_GenerateSTOP(I2C1, ENABLE);
  return 1;
}

u8 DRV_AT24C64_WriteBuffer(u16 addr, u8 *buf, u16 len) {
  u16 i;
  for (i = 0; i < len; i++) {
    if (DRV_AT24C64_WriteByte(addr + i, buf[i]) != 0)
      return 1;
  }
  return 0;
}

u8 DRV_AT24C64_ReadBuffer(u16 addr, u8 *buf, u16 len) {
  u16 i;
  while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    ;

  I2C_GenerateSTART(I2C1, ENABLE);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    goto error;

  I2C_Send7bitAddress(I2C1, EEPROM_ADDR, I2C_Direction_Transmitter);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    goto error;

  I2C_SendData(I2C1, (u8)(addr >> 8));
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    goto error;

  I2C_SendData(I2C1, (u8)(addr & 0xFF));
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    goto error;

  I2C_GenerateSTART(I2C1, ENABLE);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    goto error;

  I2C_Send7bitAddress(I2C1, EEPROM_ADDR, I2C_Direction_Receiver);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    goto error;

  for (i = 0; i < len; i++) {
    if (i == len - 1) {
      I2C_AcknowledgeConfig(I2C1, DISABLE);
      I2C_GenerateSTOP(I2C1, ENABLE);
    }
    if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
      goto error;
    buf[i] = I2C_ReceiveData(I2C1);
  }
  I2C_AcknowledgeConfig(I2C1, ENABLE);
  return 0;

error:
  I2C_GenerateSTOP(I2C1, ENABLE);
  return 1;
}
