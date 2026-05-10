#include "drv_ds1307.h"
#include "debug.h"
#include "BSP/bsp_i2c.h"

/* Helper: wait for I2C event with timeout */
static uint8_t DS1307_WaitEvent(uint32_t event) {
    uint32_t timeout = DS1307_TIMEOUT;
    while (!I2C_CheckEvent(I2C1, event)) {
        if (--timeout == 0) {
            PRINT("[DS1307] I2C Timeout at event: 0x%08X\r\n", event);
            return 1;
        }
    }
    return 0;
}

/* Helper: wait for I2C bus not busy with timeout */
static uint8_t DS1307_WaitNotBusy(void) {
    uint32_t timeout = DS1307_TIMEOUT;
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) {
        if (--timeout == 0) {
            PRINT("[DS1307] Bus BUSY timeout, resetting I2C\r\n");
            I2C_SoftwareResetCmd(I2C1, ENABLE);
            I2C_SoftwareResetCmd(I2C1, DISABLE);
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Sets the byte in the designated DS1307 register to value.
 * Interrupts are disabled during the entire I2C transaction to prevent
 * TIM2 ISR from corrupting the I2C state machine.
 */
void DS1307_SetRegByte(uint8_t regAddr, uint8_t val) {
    __disable_irq();

    if (DS1307_WaitNotBusy()) {
        __enable_irq();
        return;
    }

    I2C_GenerateSTART(I2C1, ENABLE);
    if (DS1307_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) goto error;

    I2C_Send7bitAddress(I2C1, (DS1307_I2C_ADDR << 1), I2C_Direction_Transmitter);
    if (DS1307_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) goto error;

    I2C_SendData(I2C1, regAddr);
    if (DS1307_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING)) goto error;

    I2C_SendData(I2C1, val);
    if (DS1307_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) goto error;

    I2C_GenerateSTOP(I2C1, ENABLE);
    __enable_irq();
    return;
error:
    if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF)) {
        I2C_ClearFlag(I2C1, I2C_FLAG_AF);
    }
    I2C_GenerateSTOP(I2C1, ENABLE);
    __enable_irq();
    /* Hard reset I2C peripheral to recover from any locked state */
    I2C_SoftwareResetCmd(I2C1, ENABLE);
    I2C_SoftwareResetCmd(I2C1, DISABLE);
    BSP_I2C_Init();
}

/**
 * @brief Gets the byte in the designated DS1307 register.
 * Interrupts are disabled during the entire I2C transaction.
 */
uint8_t DS1307_GetRegByte(uint8_t regAddr) {
    uint8_t val = 0;

    __disable_irq();

    if (DS1307_WaitNotBusy()) {
        __enable_irq();
        return 0;
    }

    I2C_GenerateSTART(I2C1, ENABLE);
    if (DS1307_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) goto error;

    I2C_Send7bitAddress(I2C1, (DS1307_I2C_ADDR << 1), I2C_Direction_Transmitter);
    if (DS1307_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) goto error;

    I2C_SendData(I2C1, regAddr);
    if (DS1307_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) goto error;

    I2C_GenerateSTART(I2C1, ENABLE); // RESTART
    if (DS1307_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) goto error;

    I2C_Send7bitAddress(I2C1, (DS1307_I2C_ADDR << 1), I2C_Direction_Receiver);
    if (DS1307_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) goto error;

    I2C_AcknowledgeConfig(I2C1, DISABLE);
    I2C_GenerateSTOP(I2C1, ENABLE);
    if (DS1307_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED)) goto error;

    val = I2C_ReceiveData(I2C1);
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    __enable_irq();
    return val;
error:
    if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF)) {
        I2C_ClearFlag(I2C1, I2C_FLAG_AF);
    }
    I2C_GenerateSTOP(I2C1, ENABLE);
    __enable_irq();
    /* Hard reset I2C peripheral to recover from any locked state */
    I2C_SoftwareResetCmd(I2C1, ENABLE);
    I2C_SoftwareResetCmd(I2C1, DISABLE);
    BSP_I2C_Init();
    return 0;
}

/**
 * @brief Initializes the DS1307 module.
 * I2C peripheral hardware init is handled by BSP_I2C_Init().
 */
void DS1307_Init(void) {
	DS1307_SetClockHalt(0);
}

void DS1307_SetClockHalt(uint8_t halt) {
	uint8_t ch = (halt ? 1 << 7 : 0);
	DS1307_SetRegByte(DS1307_REG_SECOND, ch | (DS1307_GetRegByte(DS1307_REG_SECOND) & 0x7f));
}

uint8_t DS1307_GetClockHalt(void) {
	return (DS1307_GetRegByte(DS1307_REG_SECOND) & 0x80) >> 7;
}

void DS1307_SetEnableSquareWave(DS1307_SquareWaveEnable mode){
	uint8_t controlReg = DS1307_GetRegByte(DS1307_REG_CONTROL);
	uint8_t newControlReg = (controlReg & ~(1 << 4)) | ((mode & 1) << 4);
	DS1307_SetRegByte(DS1307_REG_CONTROL, newControlReg);
}

void DS1307_SetInterruptRate(DS1307_Rate rate){
	uint8_t controlReg = DS1307_GetRegByte(DS1307_REG_CONTROL);
	uint8_t newControlReg = (controlReg & ~0x03) | rate;
	DS1307_SetRegByte(DS1307_REG_CONTROL, newControlReg);
}

uint8_t DS1307_GetDayOfWeek(void) {
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_DOW));
}

uint8_t DS1307_GetDate(void) {
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_DATE));
}

uint8_t DS1307_GetMonth(void) {
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_MONTH));
}

uint16_t DS1307_GetYear(void) {
	uint16_t cen = DS1307_GetRegByte(DS1307_REG_CENT) * 100;
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_YEAR)) + cen;
}

uint8_t DS1307_GetHour(void) {
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_HOUR) & 0x3f);
}

uint8_t DS1307_GetMinute(void) {
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_MINUTE));
}

uint8_t DS1307_GetSecond(void) {
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_SECOND) & 0x7f);
}

int8_t DS1307_GetTimeZoneHour(void) {
	return DS1307_GetRegByte(DS1307_REG_UTC_HR);
}

uint8_t DS1307_GetTimeZoneMin(void) {
	return DS1307_GetRegByte(DS1307_REG_UTC_MIN);
}

void DS1307_SetDayOfWeek(uint8_t dayOfWeek) {
	DS1307_SetRegByte(DS1307_REG_DOW, DS1307_EncodeBCD(dayOfWeek));
}

void DS1307_SetDate(uint8_t date) {
	DS1307_SetRegByte(DS1307_REG_DATE, DS1307_EncodeBCD(date));
}

void DS1307_SetMonth(uint8_t month) {
	DS1307_SetRegByte(DS1307_REG_MONTH, DS1307_EncodeBCD(month));
}

void DS1307_SetYear(uint16_t year) {
	DS1307_SetRegByte(DS1307_REG_CENT, year / 100);
	DS1307_SetRegByte(DS1307_REG_YEAR, DS1307_EncodeBCD(year % 100));
}

void DS1307_SetHour(uint8_t hour_24mode) {
	DS1307_SetRegByte(DS1307_REG_HOUR, DS1307_EncodeBCD(hour_24mode & 0x3f));
}

void DS1307_SetMinute(uint8_t minute) {
	DS1307_SetRegByte(DS1307_REG_MINUTE, DS1307_EncodeBCD(minute));
}

void DS1307_SetSecond(uint8_t second) {
	uint8_t ch = DS1307_GetClockHalt();
	DS1307_SetRegByte(DS1307_REG_SECOND, DS1307_EncodeBCD(second | ch));
}

void DS1307_SetTimeZone(int8_t hr, uint8_t min) {
	DS1307_SetRegByte(DS1307_REG_UTC_HR, hr);
	DS1307_SetRegByte(DS1307_REG_UTC_MIN, min);
}

uint8_t DS1307_DecodeBCD(uint8_t bin) {
	return (((bin & 0xf0) >> 4) * 10) + (bin & 0x0f);
}

uint8_t DS1307_EncodeBCD(uint8_t dec) {
	return (dec % 10 + ((dec / 10) << 4));
}
