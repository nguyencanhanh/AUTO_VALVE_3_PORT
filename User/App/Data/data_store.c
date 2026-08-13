/*****************************************************************************
 * File Name   : data_store.c
 * Layer       : App / Data
 * Description : Luu tru cac thong so cai dat vao EEPROM AT24C64.
 *               RAM cache + ghi EEPROM khi thay doi.
 *****************************************************************************/
#include "data_store.h"
#include "Drivers/drv_at24c64.h"
#include "App/app_error.h"
#include "debug.h"

/*============================================================================
 * RAM cache
 *============================================================================*/
static AppSettings_t s_settings;

/*============================================================================
 * Private helpers
 *============================================================================*/

/* Ghi 1 byte vao EEPROM tai vi tri offset trong struct */
static void prv_SaveByte(u16 offset, u8 value) {
    AT24Cxx_ERR_t err = DRV_AT24C64_WriteByte(EEPROM_ADDR_SETTINGS + offset, value);
    if (err != AT24_OK) {
        PRINT("[DATA] ERROR: WriteByte offset=%d err=%d\r\n", offset, err);
        APP_Error_Set(ERR_EEPROM);  /* E4: EEPROM loi */
    }
}

/* Doc magic key tu EEPROM */
static u16 prv_ReadMagic(void) {
    u8 buf[2] = {0, 0};
    AT24Cxx_ERR_t err = DRV_AT24C64_ReadBuffer(EEPROM_ADDR_MAGIC, buf, 2);
    if (err != AT24_OK) {
        PRINT("[DATA] ERROR: ReadMagic err=%d\r\n", err);
        APP_Error_Set(ERR_EEPROM);  /* E4: EEPROM loi */
    }
    return (u16)((buf[0] << 8) | buf[1]);
}

/* Ghi magic key vao EEPROM */
static void prv_WriteMagic(void) {
    u8 buf[2];
    buf[0] = (u8)(EEPROM_MAGIC_KEY >> 8);
    buf[1] = (u8)(EEPROM_MAGIC_KEY & 0xFF);
    AT24Cxx_ERR_t err = DRV_AT24C64_WriteBuffer(EEPROM_ADDR_MAGIC, buf, 2);
    if (err != AT24_OK) {
        PRINT("[DATA] ERROR: WriteMagic err=%d\r\n", err);
        APP_Error_Set(ERR_EEPROM);  /* E4: EEPROM loi */
    }
}

/* Gan gia tri mac dinh cho struct */
static void prv_SetDefaults(AppSettings_t *cfg) {
    cfg->wash_sched_hour   = DEFAULT_WASH_SCHED_HOUR;
    cfg->wash_sched_min    = DEFAULT_WASH_SCHED_MIN;
    cfg->wash_count        = DEFAULT_WASH_COUNT;
    cfg->wash_period_hours = DEFAULT_WASH_PERIOD_HOURS;
    cfg->backwash_sec      = DEFAULT_BACKWASH_SEC;
    cfg->fast_rinse_sec    = DEFAULT_FAST_RINSE_SEC;
    cfg->relay_mode        = DEFAULT_RELAY_MODE;
    cfg->reserved          = 0;
}

/*============================================================================
 * Data_Init - Goi 1 lan khi khoi dong
 *============================================================================*/
void Data_Init(void) {
    u16 magic = prv_ReadMagic();
    PRINT("[DATA] Magic read=0x%04X, expected=0x%04X\r\n", magic, EEPROM_MAGIC_KEY);

    if (magic != EEPROM_MAGIC_KEY) {
        /* EEPROM chua duoc ghi lan nao -> ghi gia tri mac dinh */
        PRINT("[DATA] EEPROM empty, writing defaults\r\n");
        prv_SetDefaults(&s_settings);
        Data_SaveAll(&s_settings);
        Data_SetFilterMinutes(0);
        Data_SetFlowLiters(0);
        prv_WriteMagic();

        /* Verify: doc lai magic key sau khi ghi */
        u16 verify = prv_ReadMagic();
        PRINT("[DATA] Verify magic after write=0x%04X %s\r\n",
              verify, (verify == EEPROM_MAGIC_KEY) ? "OK" : "FAIL!");
    } else {
        /* Doc cai dat tu EEPROM vao RAM cache */
        PRINT("[DATA] Loading settings from EEPROM\r\n");
        Data_LoadAll(&s_settings);
    }

    PRINT("[DATA] Schedule=%02d:%02d Count=%d Period=%dh BW=%ds FR=%ds Relay=%d\r\n",
          s_settings.wash_sched_hour, s_settings.wash_sched_min,
          s_settings.wash_count, s_settings.wash_period_hours,
          s_settings.backwash_sec, s_settings.fast_rinse_sec,
          s_settings.relay_mode);
}

/*============================================================================
 * Doc/Ghi toan bo struct
 *============================================================================*/
void Data_LoadAll(AppSettings_t *cfg) {
    DRV_AT24C64_ReadBuffer(EEPROM_ADDR_SETTINGS, (u8*)cfg, sizeof(AppSettings_t));
}

void Data_SaveAll(const AppSettings_t *cfg) {
    DRV_AT24C64_WriteBuffer(EEPROM_ADDR_SETTINGS, (u8*)cfg, sizeof(AppSettings_t));
}

/*============================================================================
 * Lay con tro toi RAM cache
 *============================================================================*/
AppSettings_t* Data_GetSettings(void) {
    return &s_settings;
}

/*============================================================================
 * Ghi tung tham so - cap nhat ca RAM + EEPROM
 *============================================================================*/
void Data_SetWashSchedule(u8 hour, u8 min) {
    s_settings.wash_sched_hour = hour;
    s_settings.wash_sched_min  = min;
    prv_SaveByte(0, hour);  /* offset 0 = wash_sched_hour */
    prv_SaveByte(1, min);   /* offset 1 = wash_sched_min */
    PRINT("[DATA] Saved schedule: %02d:%02d\r\n", hour, min);
}

void Data_SetWashCount(u8 count) {
    s_settings.wash_count = count;
    prv_SaveByte(2, count); /* offset 2 = wash_count */
    PRINT("[DATA] Saved wash count: %d\r\n", count);
}

void Data_SetWashPeriod(u8 hours) {
    s_settings.wash_period_hours = hours;
    prv_SaveByte(3, hours); /* offset 3 = wash_period_hours */
    PRINT("[DATA] Saved wash period: %dh\r\n", hours);
}

void Data_SetBackwashTime(u8 sec) {
    s_settings.backwash_sec = sec;
    prv_SaveByte(4, sec);   /* offset 4 = backwash_sec */
    PRINT("[DATA] Saved backwash: %ds\r\n", sec);
}

void Data_SetFastRinseTime(u8 sec) {
    s_settings.fast_rinse_sec = sec;
    prv_SaveByte(5, sec);   /* offset 5 = fast_rinse_sec */
    PRINT("[DATA] Saved fast rinse: %ds\r\n", sec);
}

void Data_SetRelayMode(u8 mode) {
    s_settings.relay_mode = mode;
    prv_SaveByte(6, mode);  /* offset 6 = relay_mode */
    PRINT("[DATA] Saved relay mode: %d\r\n", mode);
}

/*============================================================================
 * Filter runtime counter (u32 = 4 bytes)
 *============================================================================*/
u32 Data_GetFilterMinutes(void) {
    u8 buf[4];
    DRV_AT24C64_ReadBuffer(EEPROM_ADDR_FILTER_MIN, buf, 4);
    return ((u32)buf[0] << 24) | ((u32)buf[1] << 16) |
           ((u32)buf[2] << 8)  | (u32)buf[3];
}

void Data_SetFilterMinutes(u32 minutes) {
    u8 buf[4];
    buf[0] = (u8)(minutes >> 24);
    buf[1] = (u8)(minutes >> 16);
    buf[2] = (u8)(minutes >> 8);
    buf[3] = (u8)(minutes & 0xFF);
    DRV_AT24C64_WriteBuffer(EEPROM_ADDR_FILTER_MIN, buf, 4);
}

/*============================================================================
 * Flow counter (u32 = 4 bytes)
 *============================================================================*/
u32 Data_GetFlowLiters(void) {
    u8 buf[4];
    DRV_AT24C64_ReadBuffer(EEPROM_ADDR_FLOW_LITERS, buf, 4);
    return ((u32)buf[0] << 24) | ((u32)buf[1] << 16) |
           ((u32)buf[2] << 8)  | (u32)buf[3];
}

void Data_SetFlowLiters(u32 liters) {
    u8 buf[4];
    buf[0] = (u8)(liters >> 24);
    buf[1] = (u8)(liters >> 16);
    buf[2] = (u8)(liters >> 8);
    buf[3] = (u8)(liters & 0xFF);
    DRV_AT24C64_WriteBuffer(EEPROM_ADDR_FLOW_LITERS, buf, 4);
}

/*============================================================================
 * Wash state persistence - Phuc hoi sau mat dien
 * Ghi 2 byte: phase (BW/FR/IDLE) + remain_count
 *============================================================================*/
void Data_SaveWashState(u8 phase, u8 remain_count) {
    DRV_AT24C64_WriteByte(EEPROM_ADDR_WASH_PHASE, phase);
    DRV_AT24C64_WriteByte(EEPROM_ADDR_WASH_REMAIN, remain_count);
    PRINT("[DATA] Saved wash state: phase=%d remain=%d\r\n", phase, remain_count);
}

void Data_LoadWashState(u8 *phase, u8 *remain_count) {
    DRV_AT24C64_ReadByte(EEPROM_ADDR_WASH_PHASE, phase);
    DRV_AT24C64_ReadByte(EEPROM_ADDR_WASH_REMAIN, remain_count);
    PRINT("[DATA] Loaded wash state: phase=%d remain=%d\r\n", *phase, *remain_count);
}

void Data_ClearWashState(void) {
    DRV_AT24C64_WriteByte(EEPROM_ADDR_WASH_PHASE, WASH_PHASE_IDLE);
    DRV_AT24C64_WriteByte(EEPROM_ADDR_WASH_REMAIN, 0);
    PRINT("[DATA] Cleared wash state\r\n");
}
