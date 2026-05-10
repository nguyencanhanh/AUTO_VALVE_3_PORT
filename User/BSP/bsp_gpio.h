/*****************************************************************************
 * File Name   : bsp_gpio.h
 * Layer       : BSP - Board Support Package
 * Description : Initialize va dieu whenen GPIO entire hardware
 *****************************************************************************/
#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "bsp_config.h"

/* Initialize all GPIO mot time at startup */
void BSP_GPIO_Init(void);

/* LED control */
void BSP_LED_AllOn(void);
void BSP_LED_AllOff(void);

/* Buzzer direct GPIO (for boot sequence - khong dung pattern) */
void BSP_Buzzer_On(void);
void BSP_Buzzer_Off(void);

/* Relay control */
void BSP_Motor_On(void);
void BSP_Motor_Off(void);
void BSP_Pump_On(void);
void BSP_Pump_Off(void);

/* Sensor read */
u8   BSP_Sensor_ReadPosition(void);   /* 1 = valve currently open */

/* Button raw read (active LOW → return 1 when currently nhan) */
u8   BSP_Button_ReadRaw(uint32_t pin);

#endif /* __BSP_GPIO_H */
