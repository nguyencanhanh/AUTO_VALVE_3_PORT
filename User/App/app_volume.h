/*****************************************************************************
 * File Name   : app_volume.h
 * Layer       : App
 * Description : Volume Mode - Auto valve close when target volume is reached.
 *               Flow sensor (PA6) counts pulses → calculates volume in ml.
 *               User sets target volume (liters). Valve closes automatically.
 *****************************************************************************/
#ifndef __APP_VOLUME_H
#define __APP_VOLUME_H

#include "BSP/bsp_config.h"

/**
 * @brief Enter volume mode.
 *        Display target volume or flow progress.
 */
void APP_Volume_Enter(void);

/**
 * @brief Run volume mode logic.
 *        - BTN_VALVE  → open valve and start counting flow pulses
 *        - Monitor g_flow_pulses → calculate ml = pulses * 1000 / FLOW_PULSES_PER_LITER
 *        - When flow_ml >= target_ml → auto close valve → play BUZ_DONE melody
 *        - Display: current flow volume on 7-seg (x.xxx L)
 *        - BTN_UP / BTN_DOWN → adjust target volume while valve closed
 *        - BTN_MENU   → return to home
 */
void APP_Volume_Run(void);

/**
 * @brief Exit volume mode.
 */
void APP_Volume_Exit(void);

/**
 * @brief Set target volume in milliliters.
 * @param target_ml Target volume (ml)
 */
void APP_Volume_SetTarget(uint32_t target_ml);

/**
 * @brief Get accumulated flow volume for current session (ml).
 */
uint32_t APP_Volume_GetFlow_ml(void);

#endif /* __APP_VOLUME_H */
