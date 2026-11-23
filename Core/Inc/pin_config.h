/*
 * pin_config.h
 *
 * Unified hardware pinout, I2C addresses, and HAL peripheral handles
 * for XNUCLEO-F103RB (STM32F103RB) + Grove Expansion Shield.
 *
 * ARCHITECTURAL RULE:
 *   ONLY hardware driver .c files (button.c, traffic_light.c, dht20.c, lcd.c)
 *   and main.c may include this file. Never include pin_config.h or main.h
 *   inside scheduler.c or fsm_*.c.
 */

#ifndef INC_PIN_CONFIG_H_
#define INC_PIN_CONFIG_H_

#include "main.h"

/* ========================================================================= */
/* 1. Fallback GPIO Definitions (if CubeMX User Labels differ or not yet set) */
/* ========================================================================= */

/* Traffic Light Module 1: Arduino D3 (PB3), D4 (PB5) */
#ifndef TL1_A_Pin
#define TL1_A_Pin               GPIO_PIN_3
#define TL1_A_GPIO_Port         GPIOB
#endif

#ifndef TL1_B_Pin
#define TL1_B_Pin               GPIO_PIN_5
#define TL1_B_GPIO_Port         GPIOB
#endif

/* Traffic Light Module 2: Arduino D5 (PB4), D6 (PB10) */
#ifndef TL2_A_Pin
#define TL2_A_Pin               GPIO_PIN_4
#define TL2_A_GPIO_Port         GPIOB
#endif

#ifndef TL2_B_Pin
#define TL2_B_Pin               GPIO_PIN_10
#define TL2_B_GPIO_Port         GPIOB
#endif

/* Button Module 1: Arduino D7 (PA8), D8 (PA9) */
#ifndef BTN_1_Pin
#define BTN_1_Pin               GPIO_PIN_8
#define BTN_1_GPIO_Port         GPIOA
#endif

#ifndef BTN_2_Pin
#define BTN_2_Pin               GPIO_PIN_9
#define BTN_2_GPIO_Port         GPIOA
#endif

/* Button Module 2: Arduino D9 (PC7), D10 (PB6) */
#ifndef BTN_3_Pin
#define BTN_3_Pin               GPIO_PIN_7
#define BTN_3_GPIO_Port         GPIOC
#endif

#ifndef BTN_4_Pin
#define BTN_4_Pin               GPIO_PIN_6
#define BTN_4_GPIO_Port         GPIOB
#endif

/* Onboard LEDs & User Button: PA5, PC9, PC13 */
#ifndef LED_USER1_Pin
#define LED_USER1_Pin           GPIO_PIN_5
#define LED_USER1_GPIO_Port     GPIOA
#endif

#ifndef LED_USER2_Pin
#define LED_USER2_Pin           GPIO_PIN_9
#define LED_USER2_GPIO_Port     GPIOC
#endif

#ifndef BTN_USER_Pin
#define BTN_USER_Pin            GPIO_PIN_13
#define BTN_USER_GPIO_Port      GPIOC
#endif

/* ========================================================================= */
/* 2. Timing, Button Logic & I2C Addresses (Verify in Step 3 Hardware Tests) */
/* ========================================================================= */

/* Scheduler tick period (ms) configured on TIM2 */
#define SCH_TICK_MS             10U

/* Active logic level when a button is pressed (confirm in Step 3.3) */
#define BTN_PRESSED_STATE       GPIO_PIN_RESET
#define BTN_RELEASED_STATE      GPIO_PIN_SET

/* DHT20 I2C 7-bit address (default 0x38) and 8-bit HAL address */
#define DHT20_I2C_ADDR_7BIT     0x38U
#define DHT20_I2C_ADDR          (DHT20_I2C_ADDR_7BIT << 1)

/* LCD I2C 7-bit address (update to 0x27 or 0x3F after Step 3.4 I2C scan) */
#define LCD_I2C_ADDR_7BIT       0x27U
#define LCD_I2C_ADDR            (LCD_I2C_ADDR_7BIT << 1)

/* ========================================================================= */
/* 3. Traffic Light 2-Bit Signal Encoding (TODO: Fill after Step 3.2 test)   */
/* ========================================================================= */
/*
 * Run Step 3.2 in main.c to observe which (A, B) combination maps to
 * OFF, RED, AMBER, GREEN on the Grove traffic light module, then update:
 */
#define TL_SIG_OFF_A            GPIO_PIN_RESET
#define TL_SIG_OFF_B            GPIO_PIN_RESET

#define TL_SIG_RED_A            GPIO_PIN_SET
#define TL_SIG_RED_B            GPIO_PIN_RESET

#define TL_SIG_AMBER_A          GPIO_PIN_RESET
#define TL_SIG_AMBER_B          GPIO_PIN_SET

#define TL_SIG_GREEN_A          GPIO_PIN_SET
#define TL_SIG_GREEN_B          GPIO_PIN_SET

/* ========================================================================= */
/* 4. External HAL Peripheral Handles (Defined in CubeMX main.c)             */
/* ========================================================================= */
extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim2;

#endif /* INC_PIN_CONFIG_H_ */
