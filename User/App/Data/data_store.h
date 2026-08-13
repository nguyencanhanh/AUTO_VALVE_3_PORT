/*****************************************************************************
 * File Name   : data_store.h
 * Layer       : App / Data
 * Description : Luu tru cac thong so cai dat vao EEPROM AT24C64.
 *               Cung cap API don gian de doc/ghi tung tham so hoac
 *               doc/ghi toan bo struct 1 lan.
 *
 *   0x0000 - 0x000F : Settings struct (16 bytes)
 *   0x0010 - 0x0013 : Filter total minutes (u32, 4 bytes)
 *   0x0014 - 0x0017 : Flow total liters (u32, 4 bytes)
 *   0x0020 - 0x0023 : Magic key (4 bytes) - de nhan biet EEPROM da init
 *   0x0024          : Wash phase (1 byte) - 0=idle, 1=BW, 2=FR
 *   0x0025          : Wash remain_count (1 byte)
 *   0x0100+          : Reserved for future use
 *****************************************************************************/
#ifndef __DATA_STORE_H
#define __DATA_STORE_H

#include "BSP/bsp_config.h"

/*============================================================================
 * EEPROM Address Map
 *============================================================================*/
#define EEPROM_ADDR_SETTINGS     0x0000  /* Settings struct base */
#define EEPROM_ADDR_FILTER_MIN   0x0010  /* Filter total minutes (u32) */
#define EEPROM_ADDR_FLOW_LITERS  0x0014  /* Flow total liters (u32) */
#define EEPROM_ADDR_MAGIC        0x0020  /* Magic key for first-boot detect */
#define EEPROM_ADDR_WASH_PHASE   0x0024  /* Wash phase: 0=idle, 1=BW, 2=FR */
#define EEPROM_ADDR_WASH_REMAIN  0x0025  /* Wash remain_count */

/* Wash phase values */
#define WASH_PHASE_IDLE  0x00u
#define WASH_PHASE_BW    0x01u   /* Dang suc rua nguoc (Backwash) */
#define WASH_PHASE_FR    0x02u   /* Dang xa nhanh (Fast Rinse) */

#define EEPROM_MAGIC_KEY         0xA5C6  /* Nhan biet EEPROM da duoc ghi lan dau */

/*============================================================================
 * Settings struct - Chua toan bo thong so cai dat
 *============================================================================*/
typedef struct {
    u8  wash_sched_hour;     /* Gio bat dau suc rua (0-23) */
    u8  wash_sched_min;      /* Phut bat dau suc rua (0-59) */
    u8  wash_count;          /* So lan rua trong 1 chu ky (1-99) */
    u8  wash_period_hours;   /* Chu ky rua (gio, 1-99) */
    u8  backwash_sec;        /* Thoi gian rua nguoc (giay, 1-99) */
    u8  fast_rinse_sec;      /* Thoi gian xa nhanh (giay, 1-99) */
    u8  relay_mode;          /* Che do relay: 0=NO, 1=NC */
    u8  reserved;            /* Du phong, giup struct chan 2 byte */
} AppSettings_t;

/*============================================================================
 * Default values - Dung khi EEPROM chua duoc ghi lan nao
 *============================================================================*/
#define DEFAULT_WASH_SCHED_HOUR    6
#define DEFAULT_WASH_SCHED_MIN     0
#define DEFAULT_WASH_COUNT         3
#define DEFAULT_WASH_PERIOD_HOURS  24
#define DEFAULT_BACKWASH_SEC       30
#define DEFAULT_FAST_RINSE_SEC     15
#define DEFAULT_RELAY_MODE         0

/*============================================================================
 * API
 *============================================================================*/

/* Khoi tao: doc EEPROM, neu chua co magic key thi ghi gia tri mac dinh */
void Data_Init(void);

/* Doc/Ghi toan bo struct settings 1 lan */
void Data_LoadAll(AppSettings_t *cfg);
void Data_SaveAll(const AppSettings_t *cfg);

/* Lay con tro toi struct settings hien tai (RAM cache) */
AppSettings_t* Data_GetSettings(void);

/* Ghi tung tham so rieng le (tu dong cap nhat RAM + EEPROM) */
void Data_SetWashSchedule(u8 hour, u8 min);
void Data_SetWashCount(u8 count);
void Data_SetWashPeriod(u8 hours);
void Data_SetBackwashTime(u8 sec);
void Data_SetFastRinseTime(u8 sec);
void Data_SetRelayMode(u8 mode);

/* Filter runtime counter (tong so phut loc da chay) */
u32  Data_GetFilterMinutes(void);
void Data_SetFilterMinutes(u32 minutes);

/* Flow counter (tong so lit da chay qua) */
u32  Data_GetFlowLiters(void);
void Data_SetFlowLiters(u32 liters);

/* Wash state persistence (phuc hoi sau mat dien) */
void Data_SaveWashState(u8 phase, u8 remain_count);
void Data_LoadWashState(u8 *phase, u8 *remain_count);
void Data_ClearWashState(void);

#endif /* __DATA_STORE_H */
