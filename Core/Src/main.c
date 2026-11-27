/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pin_config.h"
#include "global.h"
#include "scheduler.h"
#include "software_timer.h"
#include "button.h"
#include "traffic_light.h"
#include "dht20.h"
#include "lcd.h"
#include "fsm_automatic.h"
#include "fsm_manual.h"
#include "fsm_tuning.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void system_fsm_task(void)
{
    if (status == SYS_INIT) {
        status = AUTO_INIT;
    }

    switch (status) {
        case AUTO_INIT:
        case AUTO_RED1_GREEN2:
        case AUTO_RED1_AMBER2:
        case AUTO_GREEN1_RED2:
        case AUTO_AMBER1_RED2:
            fsm_automatic_run();
            break;

        case MAN_INIT:
        case MAN_RED1_GREEN2:
        case MAN_RED1_AMBER2:
        case MAN_GREEN1_RED2:
        case MAN_AMBER1_RED2:
        case MAN_ALL_RED:
            fsm_manual_run();
            break;

        case TUN_INIT:
        case TUN_RED:
        case TUN_GREEN:
        case TUN_AMBER:
            fsm_tuning_run();
            break;

        default:
            status = AUTO_INIT;
            break;
    }
}

static void lcd_update_task(void)
{
    switch (status) {
        case AUTO_RED1_GREEN2:
        case AUTO_RED1_AMBER2:
        case AUTO_GREEN1_RED2:
        case AUTO_AMBER1_RED2:
            lcd_display_auto(countdown_lane1, countdown_lane2,
                             env_temperature, env_humidity, env_data_valid);
            break;

        case MAN_RED1_GREEN2:
            lcd_display_manual("R1 - G2", env_temperature, env_humidity, env_data_valid);
            break;
        case MAN_RED1_AMBER2:
            lcd_display_manual("R1 - A2", env_temperature, env_humidity, env_data_valid);
            break;
        case MAN_GREEN1_RED2:
            lcd_display_manual("G1 - R2", env_temperature, env_humidity, env_data_valid);
            break;
        case MAN_AMBER1_RED2:
            lcd_display_manual("A1 - R2", env_temperature, env_humidity, env_data_valid);
            break;
        case MAN_ALL_RED:
            lcd_display_manual("ALL RED", env_temperature, env_humidity, env_data_valid);
            break;

        case TUN_INIT:
        case TUN_RED:
        case TUN_GREEN:
        case TUN_AMBER:
            /* LCD is managed directly by fsm_tuning.c (including error message) */
            break;

        default:
            break;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        SCH_Update();
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  global_init();
  SCH_Init();
  timer_init();
  button_init();
  traffic_init();
  dht20_init();
  lcd_init();

  SCH_Add_Task(timer_run, 0U, 1U);        /* 10 ms */
  SCH_Add_Task(button_read, 0U, 1U);      /* 10 ms */
  SCH_Add_Task(system_fsm_task, 1U, 1U);  /* 10 ms */
  SCH_Add_Task(dht20_task, 2U, 5U);       /* 50 ms */
  SCH_Add_Task(lcd_update_task, 5U, 20U); /* 50 ms offset, 200 ms period */

  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    SCH_Dispatch_Tasks();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: source line number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
