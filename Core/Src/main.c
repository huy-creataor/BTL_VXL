/*
 * main.c
 *
 * System integration for CO3009 Traffic Light Controller (XNUCLEO-F103RB).
 * Initializes HAL peripherals, drivers, O(1) scheduler, and dispatches tasks.
 */

#include "main.h"
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

I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);

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

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM2_Init();

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

    while (1) {
        SCH_Dispatch_Tasks();
    }
}

void SystemClock_Config(void)
{
    /* Configured by STM32CubeMX */
}

static void MX_GPIO_Init(void)
{
    /* Configured by STM32CubeMX */
}

static void MX_I2C1_Init(void)
{
    /* Configured by STM32CubeMX: PB8 (SCL), PB9 (SDA), 100 kHz */
}

static void MX_TIM2_Init(void)
{
    /* Configured by STM32CubeMX: 10 ms period interrupt */
}
