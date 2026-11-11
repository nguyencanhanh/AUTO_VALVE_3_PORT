/*****************************************************************************
 * File Name   : app_interlock.h
 * Layer       : App
 * Description : Multi-valve Daisy-chain Interlock & Queue Manager.
 *               Ensures only 1 valve washes at any time.
 *               PC14 (Input)  : Signal from previous valve
 *               PA9  (Output) : Signal to next valve
 *****************************************************************************/
#ifndef __APP_INTERLOCK_H
#define __APP_INTERLOCK_H

#include "debug.h"

/* Initialize interlock state */
void APP_Interlock_Init(void);

/* Process daisy chain forwarding and blanking timers (run in main loop) */
void APP_Interlock_Run(void);

/* Check if the chain is busy (another valve or local valve is washing) */
u8   APP_Interlock_IsBusy(void);

/* Check if local valve can acquire the lock and start washing */
u8   APP_Interlock_CanStartWash(void);

/* Local valve acquires lock and starts washing (drives PA9 busy) */
void APP_Interlock_OnWashStart(void);

/* Local valve finishes washing and releases lock (drives PA9 idle) */
void APP_Interlock_OnWashFinished(void);

/* Queue management */
void APP_Interlock_RequestWash(void);
u8   APP_Interlock_IsWashPending(void);
void APP_Interlock_ClearPending(void);

#endif /* __APP_INTERLOCK_H */
