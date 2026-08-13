/*****************************************************************************
 * File Name   : drv_at24c64.c
 * Layer       : Drivers
 * Description : Driver EEPROM AT24C64 via Hardware I2C1.
 *               Bao ve bus bang __disable_irq() de tranh xung dot voi DS1307.
 *****************************************************************************/
#include "drv_at24c64.h"
#include "BSP/bsp_i2c.h"
#include "debug.h"

#define I2C_TIMEOUT 0xFFFF

/* Helper: wait for I2C event with timeout */
static u8 AT24C64_WaitEvent(uint32_t event) {
  u32 timeout = I2C_TIMEOUT;
  while (!I2C_CheckEvent(I2C1, event)) {
    if (--timeout == 0)
      return 1;
  }
  return 0;
}

/* Helper: wait for I2C bus not busy with timeout + recovery */
static u8 AT24C64_WaitNotBusy(void) {
  u32 timeout = I2C_TIMEOUT;
  while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) {
    if (--timeout == 0) {
      I2C_SoftwareResetCmd(I2C1, ENABLE);
      I2C_SoftwareResetCmd(I2C1, DISABLE);
      BSP_I2C_Init();
      return 1;
    }
  }
  return 0;
}

/* Helper: error recovery — clear AF flag, STOP, reset I2C */
static void AT24C64_ErrorRecover(void) {
  if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF)) {
    I2C_ClearFlag(I2C1, I2C_FLAG_AF);
  }
  I2C_GenerateSTOP(I2C1, ENABLE);
  I2C_SoftwareResetCmd(I2C1, ENABLE);
  I2C_SoftwareResetCmd(I2C1, DISABLE);
  BSP_I2C_Init();
}

void DRV_AT24C64_Init(void) {
  /* I2C1 already initialized at BSP_I2C_Init() */
}

AT24Cxx_ERR_t DRV_AT24C64_WriteByte(u16 addr, u8 data) {
  __disable_irq();

  if (AT24C64_WaitNotBusy()) {
    PRINT("[AT24] WB FAIL: bus BUSY\r\n");
    __enable_irq();
    return AT24_W_BYTE_ERR;
  }

  I2C_GenerateSTART(I2C1, ENABLE);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) {
    PRINT("[AT24] WB FAIL: START\r\n");
    goto error;
  }

  I2C_Send7bitAddress(I2C1, AT24C64_HW_ADDR, I2C_Direction_Transmitter);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
    PRINT("[AT24] WB FAIL: ADDR NACK (0x%02X) AF=%d\r\n",
          AT24C64_HW_ADDR, I2C_GetFlagStatus(I2C1, I2C_FLAG_AF));
    goto error;
  }

  I2C_SendData(I2C1, (u8)(addr >> 8));
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING)) {
    PRINT("[AT24] WB FAIL: ADDR_H\r\n");
    goto error;
  }

  I2C_SendData(I2C1, (u8)(addr & 0xFF));
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING)) {
    PRINT("[AT24] WB FAIL: ADDR_L\r\n");
    goto error;
  }

  I2C_SendData(I2C1, data);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
    PRINT("[AT24] WB FAIL: DATA\r\n");
    goto error;
  }

  I2C_GenerateSTOP(I2C1, ENABLE);
  __enable_irq();
  Delay_Ms(5); /* EEPROM write cycle time (max 5ms) */
  return AT24_OK;

error:
  AT24C64_ErrorRecover();
  __enable_irq();
  return AT24_W_BYTE_ERR;
}

AT24Cxx_ERR_t DRV_AT24C64_ReadByte(u16 addr, u8 *data) {
  __disable_irq();

  if (AT24C64_WaitNotBusy()) {
    __enable_irq();
    return AT24_R_BYTE_ERR;
  }

  I2C_GenerateSTART(I2C1, ENABLE);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    goto error;

  I2C_Send7bitAddress(I2C1, AT24C64_HW_ADDR, I2C_Direction_Transmitter);
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

  I2C_Send7bitAddress(I2C1, AT24C64_HW_ADDR, I2C_Direction_Receiver);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    goto error;

  I2C_AcknowledgeConfig(I2C1, DISABLE);
  I2C_GenerateSTOP(I2C1, ENABLE);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
    goto error;

  *data = I2C_ReceiveData(I2C1);
  I2C_AcknowledgeConfig(I2C1, ENABLE);

  __enable_irq();
  return AT24_OK;

error:
  AT24C64_ErrorRecover();
  __enable_irq();
  return AT24_R_BYTE_ERR;
}

static AT24Cxx_ERR_t AT24C64_WritePage(u16 addr, u8 *buf, u16 len) {
  u16 i;
  __disable_irq();

  if (AT24C64_WaitNotBusy()) {
    __enable_irq();
    return AT24_W_BYTES_ERR;
  }

  I2C_GenerateSTART(I2C1, ENABLE);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    goto error;

  I2C_Send7bitAddress(I2C1, AT24C64_HW_ADDR, I2C_Direction_Transmitter);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    goto error;

  I2C_SendData(I2C1, (u8)(addr >> 8));
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    goto error;

  I2C_SendData(I2C1, (u8)(addr & 0xFF));
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING))
    goto error;

  for (i = 0; i < len; i++) {
    I2C_SendData(I2C1, buf[i]);
    if (AT24C64_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
      goto error;
  }

  I2C_GenerateSTOP(I2C1, ENABLE);
  __enable_irq();
  Delay_Ms(5); /* EEPROM write cycle time (max 5ms) */
  return AT24_OK;

error:
  AT24C64_ErrorRecover();
  __enable_irq();
  return AT24_W_BYTES_ERR;
}

AT24Cxx_ERR_t DRV_AT24C64_WriteBuffer(u16 addr, u8 *buf, u16 len) {
  u16 page_size = AT24C64_PG_SIZE; /* Using constant from header */
  u16 offset = 0;

  while (len > 0) {
    u16 page_remaining = page_size - (addr % page_size);
    u16 bytes_to_write = (len < page_remaining) ? len : page_remaining;

    if (AT24C64_WritePage(addr, buf + offset, bytes_to_write) != AT24_OK) {
      return AT24_W_BYTES_ERR;
    }

    addr += bytes_to_write;
    offset += bytes_to_write;
    len -= bytes_to_write;
  }
  return AT24_OK;
}

AT24Cxx_ERR_t DRV_AT24C64_ReadBuffer(u16 addr, u8 *buf, u16 len) {
  u16 i;
  __disable_irq();

  if (AT24C64_WaitNotBusy()) {
    __enable_irq();
    return AT24_R_BYTES_ERR;
  }

  I2C_GenerateSTART(I2C1, ENABLE);
  if (AT24C64_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    goto error;

  I2C_Send7bitAddress(I2C1, AT24C64_HW_ADDR, I2C_Direction_Transmitter);
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

  I2C_Send7bitAddress(I2C1, AT24C64_HW_ADDR, I2C_Direction_Receiver);
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

  __enable_irq();
  return AT24_OK;

error:
  AT24C64_ErrorRecover();
  __enable_irq();
  return AT24_R_BYTES_ERR;
}
