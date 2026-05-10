#ifndef __DRV_DS1307_H
#define __DRV_DS1307_H

#include "BSP/bsp_config.h"

/*----------------------------------------------------------------------------*/
#define DS1307_I2C_ADDR 	0x68
#define DS1307_REG_SECOND 	0x00
#define DS1307_REG_MINUTE 	0x01
#define DS1307_REG_HOUR  	0x02
#define DS1307_REG_DOW    	0x03
#define DS1307_REG_DATE   	0x04
#define DS1307_REG_MONTH  	0x05
#define DS1307_REG_YEAR   	0x06
#define DS1307_REG_CONTROL 	0x07
#define DS1307_REG_UTC_HR	0x08
#define DS1307_REG_UTC_MIN	0x09
#define DS1307_REG_CENT    	0x10
#define DS1307_REG_RAM   	0x11
#define DS1307_TIMEOUT		0xFFFFF
/*----------------------------------------------------------------------------*/

typedef enum {
	DS1307_DISABLED = 0,
	DS1307_ENABLED = 1
} DS1307_SquareWaveEnable;

typedef enum {
	DS1307_1HZ = 0x00,
	DS1307_4096HZ = 0x01,
	DS1307_8192HZ = 0x02,
	DS1307_32768HZ = 0x03
} DS1307_Rate;

/* Initialize (Khong can truyen handle I2C vi dung cung I2C1) */
void DS1307_Init(void);

/* Halt / Chay dong ho */
void DS1307_SetClockHalt(uint8_t halt);
uint8_t DS1307_GetClockHalt(void);

/* Truy xuat thanh ghi tho */
void DS1307_SetRegByte(uint8_t regAddr, uint8_t val);
uint8_t DS1307_GetRegByte(uint8_t regAddr);

/* Pulse vuong Output */
void DS1307_SetEnableSquareWave(DS1307_SquareWaveEnable mode);
void DS1307_SetInterruptRate(DS1307_Rate rate);

/* Function GET */
uint8_t DS1307_GetDayOfWeek(void);
uint8_t DS1307_GetDate(void);
uint8_t DS1307_GetMonth(void);
uint16_t DS1307_GetYear(void);
uint8_t DS1307_GetHour(void);
uint8_t DS1307_GetMinute(void);
uint8_t DS1307_GetSecond(void);
int8_t DS1307_GetTimeZoneHour(void);
uint8_t DS1307_GetTimeZoneMin(void);

/* Function SET */
void DS1307_SetDayOfWeek(uint8_t dayOfWeek);
void DS1307_SetDate(uint8_t date);
void DS1307_SetMonth(uint8_t month);
void DS1307_SetYear(uint16_t year);
void DS1307_SetHour(uint8_t hour_24mode);
void DS1307_SetMinute(uint8_t minute);
void DS1307_SetSecond(uint8_t second);
void DS1307_SetTimeZone(int8_t hr, uint8_t min);

/* BCD Decode/Encode */
uint8_t DS1307_DecodeBCD(uint8_t bin);
uint8_t DS1307_EncodeBCD(uint8_t dec);

#endif /* __DRV_DS1307_H */
