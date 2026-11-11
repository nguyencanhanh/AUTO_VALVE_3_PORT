/*****************************************************************************
 * File Name   : bsp_config.h
 * Layer       : BSP - Board Support Package
 * Project     : AUTO_VALVE_MAIN_V1_3
 * MCU         : CH32X035C8T6 @ 48MHz (TSSOP48)
 * Description : Centralize entire define pin (PIN MAP) va hang so
 *               he thong. Modify this file when changing wiring diagram.
 *
 * ======================== SO DO PIN (from schematic U1) ====================
 *
 *  BUTTONS  (Active LOW, pull-up ngoai):
 *    PA15 - BTN_MENU       PA16 - BTN_UP
 *    PA17 - BTN_DOWN       PA18 - BTN_SELECT
 *
 *  RELAY & BUZZER (Active HIGH, NPN transistor):
 *    PC17 - MOTOR_RELAY  : Relay valve main
 *    PC16 - PUMP_RELAY   : Relay pump sub
 *    PB11 - BUZZER       : Buzzer (via NPN)
 *
 *  SENSORS (Input):
 *    PA6  - FLOW_SENSOR  : Sensor flow (EXTI pulse)
 *    PA7  - POS_SENSOR   : Sensor vi tri valve (HIGH = already open)
 *
 *  7-SEGMENT DISPLAY (Common Anode, noi truc tiep, 4 digit):
 *    PB13 - SEG_A         PA12 - SEG_B (GPIOA)
 *    PB2  - SEG_C         PB1  - SEG_D
 *    PB3  - SEG_E         PB5  - SEG_F
 *    PB8  - SEG_G         PB4  - SEG_DP
 *    PB12 - DIG1 (left)   PB6  - DIG2
 *    PB0  - DIG3          PB9  - DIG4 (right)
 *
 *  LED INDICATORS (Active LOW):
 *    PC7  - LED1          PC6  - LED2
 *    PA7  - LED3          PA6  - LED4
 *    PA19 - LED5          PA23 - LED6
 *    PA0  - LED7          PA1  - LED8
 *    PA5  - LED9          PA4  - LED10
 *    PA3  - LED11         PA2  - LED12
 *
 *  I2C (RTC DS1307 + EEPROM AT24C64):
 *    PA10 - I2C_SCL
 *    PA11 - I2C_SDA
 *
 *****************************************************************************/
#ifndef __BSP_CONFIG_H
#define __BSP_CONFIG_H

#include "debug.h"

/*============================================================================
 * 1. BUTTONS (Active LOW, external pull-up)
 *============================================================================*/
#define BTN_MENU_PORT GPIOA
#define BTN_MENU_PIN GPIO_Pin_15

#define BTN_UP_PORT GPIOA
#define BTN_UP_PIN GPIO_Pin_18

#define BTN_DOWN_PORT GPIOA
#define BTN_DOWN_PIN GPIO_Pin_17

#define BTN_SELECT_PORT GPIOA
#define BTN_SELECT_PIN GPIO_Pin_16

/* All button on cung port GPIOA */
#define BTN_ALL_PINS (BTN_MENU_PIN | BTN_UP_PIN | BTN_DOWN_PIN | BTN_SELECT_PIN)

/*============================================================================
 * 2. RELAY & BUZZER (Active HIGH)
 *============================================================================*/
#define MOTOR_PORT GPIOC
#define MOTOR_PIN GPIO_Pin_17 /* Relay valve main */

#define PUMP_PORT GPIOC
#define PUMP_PIN GPIO_Pin_16 /* Relay pump sub */

#define BUZZER_PORT GPIOB
#define BUZZER_PIN GPIO_Pin_11 /* Buzzer via NPN */

/*============================================================================
 * 3. SENSORS (Input)
 *============================================================================*/
#define FLOW_PORT GPIOA
#define FLOW_PIN GPIO_Pin_8 /* Sensor flow (EXTI) */
#define FLOW_EXTI_LINE EXTI_Line8
#define FLOW_EXTI_PORTSRC GPIO_PortSourceGPIOA
#define FLOW_EXTI_PINSRC GPIO_PinSource8
#define FLOW_IRQn EXTI15_8_IRQn

#define POS_SENSOR_PORT GPIOC
#define POS_SENSOR_PIN GPIO_Pin_15 /* Vi tri valve: HIGH = open */

/*============================================================================
 * 4. 7-SEGMENT DISPLAY (Direct drive, Common Cathode)
 *    Segment = Active HIGH | Digit select = Active LOW
 *============================================================================*/
/* Segment pins - all on GPIOB */
#define SEG_A_PIN GPIO_Pin_13
#define SEG_B_PIN GPIO_Pin_12
#define SEG_C_PIN GPIO_Pin_2
#define SEG_D_PIN GPIO_Pin_1
#define SEG_E_PIN GPIO_Pin_3
#define SEG_F_PIN GPIO_Pin_5
#define SEG_G_PIN GPIO_Pin_8
// #define SEG_H_PIN GPIO_Pin_4
#define SEG_DP_PIN GPIO_Pin_4
#define SEG_PORT GPIOB

/* Total segment mask on GPIOB */
#define SEG_ALL_PINS                                                           \
  (SEG_A_PIN | SEG_B_PIN | SEG_C_PIN | SEG_D_PIN | SEG_E_PIN | SEG_F_PIN |     \
   SEG_G_PIN | SEG_DP_PIN)

/* Digit select - Active LOW (common cathode) */
#define DIG1_PIN GPIO_Pin_12 /* Digit left nhat */
#define DIG2_PIN GPIO_Pin_6
#define DIG3_PIN GPIO_Pin_0
#define DIG4_PIN GPIO_Pin_9 /* Digit right nhat */
#define DIG_PORT GPIOB
#define DIG_ALL_PINS (DIG1_PIN | DIG2_PIN | DIG3_PIN | DIG4_PIN)

/*============================================================================
 * 5. LED INDICATORS (Active HIGH, GPIOA)
 *============================================================================*/
#define LED1_PIN GPIO_Pin_7
#define LED2_PIN GPIO_Pin_6
#define LED3_PIN GPIO_Pin_7
#define LED4_PIN GPIO_Pin_6
#define LED5_PIN GPIO_Pin_19
#define LED6_PIN GPIO_Pin_23
#define LED7_PIN GPIO_Pin_0
#define LED8_PIN GPIO_Pin_1
#define LED9_PIN GPIO_Pin_5
#define LED10_PIN GPIO_Pin_4
#define LED11_PIN GPIO_Pin_3
#define LED12_PIN GPIO_Pin_2

#define LED_ALL_PINS                                                           \
  (LED3_PIN | LED4_PIN | LED5_PIN | LED6_PIN | LED7_PIN | LED8_PIN |           \
   LED9_PIN | LED10_PIN | LED11_PIN | LED12_PIN)

/*============================================================================
 * 6. I2C SOFTWARE (DS1307 RTC + AT24C64 EEPROM)
 *============================================================================*/
#define SCL_PORT GPIOA
#define SCL_PIN GPIO_Pin_10

#define SDA_PORT GPIOA
#define SDA_PIN GPIO_Pin_11

/*============================================================================
 * 7. I2C DEVICE ADDRESSES
 *============================================================================*/
#define DS1307_ADDR 0x68  /* RTC DS1307  (7-bit) */
#define AT24C64_ADDR 0x50 /* EEPROM AT24C64 (7-bit, A0=A1=A2=GND) */

/*============================================================================
 * 8. SYSTEM CONSTANTS
 *============================================================================*/
#define FLOW_PULSES_PER_LITER 210u /* ZJMAZ-20: approx 210 pulses/liter */
#define VALVE_TIMEOUT_MS 15000u    /* Timeout cho valve open/close (ms) */
#define DEBOUNCE_MS 20u            /* Debounce button bam (ms) */
#define LONG_PRESS_MS 2000u        /* Long press button (ms) */
#define DISPLAY_NUM_DIGITS 4       /* Number of digits 7-seg */

/*============================================================================
 * 9. PRODUCT INFO
 *============================================================================*/
#define PRODUCT_CODE_D3 7    /* Digit 1 (left) */
#define PRODUCT_CODE_D2 1    /* Digit 2 */
#define PRODUCT_CODE_D1 0x17 /* 'P' → index SEG_CHAR_P trong font */
#define PRODUCT_CODE_D0 1    /* Digit 4 (right) */

/*============================================================================
 * 10. BOOT TIMING (ms)
 *============================================================================*/
#define BOOT_BIP_LONG_MS 2000u /* B1a: Beep dai 2 seconds */
#define BOOT_LED_ON_MS 8000u   /* B1b: LED + 7seg ALL ON ~8 seconds */
#define BOOT_BIP2_MS 1000u     /* B1c: Beep boot ~1 seconds */
#define BOOT_PRODUCT_MS 2000u  /* B2:  Display product code ~2 seconds */
#define BOOT_SCAN_MS 1500u     /* B3:  Scan function ~1.5 seconds */

/*============================================================================
 * 11. INTERLOCK (Daisy-chain Multi-valve Synchronization)
 *     PA9  (Output): Gui tin hieu ban (busy) sang van tiep theo
 *     PC14 (Input) : Nhan tin hieu ban tu van phia truoc
 *============================================================================*/
#define INTERLOCK_IN_PORT      GPIOC
#define INTERLOCK_IN_PIN       GPIO_Pin_14   /* Input tu van truoc */

#define INTERLOCK_OUT_PORT     GPIOA
#define INTERLOCK_OUT_PIN      GPIO_Pin_9    /* Output den van sau */

/* 1 = Active HIGH: PA9=1 khi ban/dang suc, PC14=1 nghia la he thong ban
 * 0 = Active LOW : PA9=0 khi ban/dang suc, PC14=0 nghia la he thong ban
 */
#define INTERLOCK_BUSY_LEVEL   1

#endif /* __BSP_CONFIG_H */
