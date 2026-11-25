/*
 * global.c
 *
 * Definitions and safe initialization for shared system states,
 * traffic timing parameters, countdowns, and environment readings.
 *
 * ARCHITECTURAL RULE: Pure C only — NO HAL headers here.
 */

#include "global.h"

SystemState_t status = SYS_INIT;

uint16_t time_red_s = DEFAULT_TIME_RED_S;
uint16_t time_green_s = DEFAULT_TIME_GREEN_S;
uint16_t time_amber_s = DEFAULT_TIME_AMBER_S;

uint16_t temp_time_red_s = DEFAULT_TIME_RED_S;
uint16_t temp_time_green_s = DEFAULT_TIME_GREEN_S;
uint16_t temp_time_amber_s = DEFAULT_TIME_AMBER_S;

uint16_t countdown_lane1 = 0U;
uint16_t countdown_lane2 = 0U;

float env_temperature = 0.0f;
float env_humidity = 0.0f;
uint8_t env_data_valid = 0U;

void global_init(void)
{
    status = SYS_INIT;

    time_red_s = DEFAULT_TIME_RED_S;
    time_green_s = DEFAULT_TIME_GREEN_S;
    time_amber_s = DEFAULT_TIME_AMBER_S;

    temp_time_red_s = DEFAULT_TIME_RED_S;
    temp_time_green_s = DEFAULT_TIME_GREEN_S;
    temp_time_amber_s = DEFAULT_TIME_AMBER_S;

    countdown_lane1 = 0U;
    countdown_lane2 = 0U;

    env_temperature = 0.0f;
    env_humidity = 0.0f;
    env_data_valid = 0U;
}
