/*****************************************************************************
 * File Name   : bsp_gpio.c
 * Layer       : BSP - Board Support Package
 * Description : Initialize entire GPIO for AUTO VALVE
 *               - LED indicators (PA0-5, PA20-23)
 *               - Buttons     PA15-PA19  (Input Pull-Up, Active LOW)
 *               - Relay       PC18(Motor), PC17(Pump) (Output PP)
 *               - Buzzer      PB4         (Output PP, Active HIGH)
 *               - Flow sensor PA6         (Input Pull-Up, EXTI separate)
 *               - Pos sensor  PA7         (Input Pull-Up)
 *               - 7-segment   PB0-PB13    (Output PP)
 *               - Soft I2C    PC5(SCL), PC2(SDA) (Output Open-Drain)
 *****************************************************************************/
#include "bsp_gpio.h"
#include "bsp_config.h"

/*============================================================================
 * BSP_GPIO_Init - Initialize entire GPIO at startup
 *============================================================================*/
void BSP_GPIO_Init(void) {
  GPIO_InitTypeDef g = {0};

  /* Turn on clock for all port su dung */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                             RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,
                         ENABLE);

  /* ----------------------------------------------------------------
   * GPIOA - LED (output) + Buttons (input) + Sensors (input)
   * ---------------------------------------------------------------- */
  /* LED: PC7, PC6 → Output Push-Pull */
  g.GPIO_Pin = LED1_PIN | LED2_PIN;
  g.GPIO_Mode = GPIO_Mode_Out_PP;
  g.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &g);
  GPIO_SetBits(GPIOC, LED1_PIN | LED2_PIN); /* LED OFF (Active LOW) */

  g.GPIO_Pin = LED_ALL_PINS;
  g.GPIO_Mode = GPIO_Mode_Out_PP;
  g.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &g);
  GPIO_SetBits(GPIOA, LED_ALL_PINS); /* LED OFF (Active LOW) */
  /* Buttons: PA15-PA19 → Input Pull-Up */
  g.GPIO_Pin = BTN_ALL_PINS;
  g.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(BTN_MENU_PORT, &g);

  /* Sensors: PA6 (flow) + PA7 (position) → Input Pull-Up */
  g.GPIO_Pin = FLOW_PIN | POS_SENSOR_PIN;
  g.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &g);

  /* ----------------------------------------------------------------
   * GPIOB - 7-Segment Segments + Digit select + Buzzer
   * ---------------------------------------------------------------- */
  /* Segment lines + digit select → Output Push-Pull */
  g.GPIO_Pin = SEG_ALL_PINS | DIG_ALL_PINS;
  g.GPIO_Mode = GPIO_Mode_Out_PP;
  g.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(SEG_PORT, &g);
  GPIO_ResetBits(SEG_PORT, SEG_ALL_PINS); /* Segments OFF (Level LOW) */
  GPIO_SetBits(DIG_PORT, DIG_ALL_PINS);   /* Digits OFF (Level HIGH) */

  g.GPIO_Pin = SEG_B_PIN;
  g.GPIO_Mode = GPIO_Mode_Out_PP;
  g.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &g);
  GPIO_ResetBits(GPIOA, SEG_B_PIN); /* Segment OFF (Level LOW) */

  /* Buzzer: PB4 → Output Push-Pull */
  g.GPIO_Pin = BUZZER_PIN;
  g.GPIO_Mode = GPIO_Mode_Out_PP;
  g.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(BUZZER_PORT, &g);
  GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN); /* Buzzer OFF */

  /* ----------------------------------------------------------------
   * GPIOC - Relay (output) + Soft I2C (open-drain)
   * ---------------------------------------------------------------- */
  /* Motor relay (PC18) + Pump relay (PC17) → Output Push-Pull */
  g.GPIO_Pin = MOTOR_PIN | PUMP_PIN;
  g.GPIO_Mode = GPIO_Mode_Out_PP;
  g.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &g);
  GPIO_ResetBits(GPIOC, MOTOR_PIN | PUMP_PIN); /* Relay OFF = valve close */


}

/*============================================================================
 * LED Control
 *============================================================================*/
void BSP_LED_AllOn(void) {
    GPIO_ResetBits(GPIOC, LED1_PIN | LED2_PIN);
    GPIO_ResetBits(GPIOA, LED_ALL_PINS); 
}

void BSP_LED_AllOff(void) {
    GPIO_SetBits(GPIOC, LED1_PIN | LED2_PIN);
    GPIO_SetBits(GPIOA, LED_ALL_PINS); 
}

/*============================================================================
 * Buzzer Direct GPIO (dung for boot sequence, khong via pattern)
 *============================================================================*/
void BSP_Buzzer_On(void) { GPIO_SetBits(BUZZER_PORT, BUZZER_PIN); }

void BSP_Buzzer_Off(void) { GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN); }

/*============================================================================
 * Relay Control
 *============================================================================*/
void BSP_Motor_On(void) { GPIO_SetBits(MOTOR_PORT, MOTOR_PIN); }
void BSP_Motor_Off(void) { GPIO_ResetBits(MOTOR_PORT, MOTOR_PIN); }
void BSP_Pump_On(void) { GPIO_SetBits(PUMP_PORT, PUMP_PIN); }
void BSP_Pump_Off(void) { GPIO_ResetBits(PUMP_PORT, PUMP_PIN); }

/*============================================================================
 * Sensor Read
 *============================================================================*/
u8 BSP_Sensor_ReadPosition(void) {
  return (u8)GPIO_ReadInputDataBit(POS_SENSOR_PORT, POS_SENSOR_PIN);
}

u8 BSP_Button_ReadRaw(uint32_t pin) {
  return (GPIO_ReadInputDataBit(BTN_MENU_PORT, pin) == Bit_RESET) ? 1u : 0u;
}
