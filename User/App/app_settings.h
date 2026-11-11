/*****************************************************************************
 * File Name   : app_settings.h
 * Layer       : App
 * Description : Settings menu with integrated button handling.
 *
 * Flow:
 *   [Locked] --(combo unlock)--> [IDLE: home still showing]
 *   [IDLE]   --(ENTER)---------> [NAV: "-1-"~"-7-", LED8 ON]
 *   [NAV]    --(UP/DOWN)-------> scroll settings
 *   [NAV]    --(ENTER)---------> [EDIT: inside sub-screen]
 *   [EDIT]   --(Back)----------> [NAV]
 *   [NAV]    --(Back)----------> [IDLE: home display restored]
 *
 * Auto-lock after 1 minute idle applies in all unlocked states.
 *****************************************************************************/
#ifndef __APP_SETTINGS_H
#define __APP_SETTINGS_H

#include "BSP/bsp_config.h"

/*============================================================================
 * Button action types (used by settings and sub-screens)
 *============================================================================*/
typedef enum {
    BTN_ACT_NONE = 0,
    BTN_ACT_SHORT,          /* Short press (released before long threshold) */
    BTN_ACT_LONG,           /* Long press first trigger */
    BTN_ACT_REPEAT          /* Auto-repeat while held (after long threshold) */
} BtnAction_t;

typedef enum {
    BTN_IDX_MENU = 0,       /* ENTER / PA15 */
    BTN_IDX_UP,             /* UP    / PA18 */
    BTN_IDX_DOWN,           /* DOWN  / PA17 */
    BTN_IDX_SELECT,         /* BACK  / PA16 */
    BTN_IDX_COUNT
} BtnIdx_t;

/*============================================================================
 * Settings items
 *============================================================================*/
typedef enum {
    SETTINGS_TIME         = 0,   /* 1: Set RTC time            */
    SETTINGS_WASH_SCHED   = 1,   /* 2: Wash schedule time      */
    SETTINGS_WASH_COUNT   = 2,   /* 3: Wash cycles per period  */
    SETTINGS_WASH_PERIOD  = 3,   /* 4: Wash cycle period       */
    SETTINGS_BACKWASH     = 4,   /* 5: Backwash duration       */
    SETTINGS_FAST_RINSE   = 5,   /* 6: Fast rinse duration     */
    SETTINGS_RELAY_MODE   = 6,   /* 7: Relay signal mode       */
    SETTINGS_ITEM_COUNT          /* = 7 */
} SettingsItem_t;

/*============================================================================
 * API
 *============================================================================*/
void APP_Settings_Enter(void);
void APP_Settings_Run(void);
void APP_Settings_Exit(void);

/* Returns 1 when settings is actively controlling the display (NAV or EDIT) */
u8 APP_Settings_IsActive(void);

void APP_Settings_SetNavMode(void);

#endif /* __APP_SETTINGS_H */
