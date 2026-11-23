/*
 * global.h
 *
 * Shared system states, traffic timing parameters, timer channel indices,
 * and cross-module shared variables (owned by global.c).
 *
 * ARCHITECTURAL RULE: Pure C only — NO HAL headers here.
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <stdint.h>

/* ========================================================================= */
/* 1. System FSM States                                                      */
/* ========================================================================= */
typedef enum {
    SYS_INIT = 0,

    /* Automatic mode states */
    AUTO_INIT,
    AUTO_RED1_GREEN2,
    AUTO_RED1_AMBER2,
    AUTO_GREEN1_RED2,
    AUTO_AMBER1_RED2,

    /* Manual mode states */
    MAN_INIT,
    MAN_RED1_GREEN2,
    MAN_RED1_AMBER2,
    MAN_GREEN1_RED2,
    MAN_AMBER1_RED2,
    MAN_ALL_RED,

    /* Tuning mode states */
    TUN_INIT,
    TUN_RED,
    TUN_GREEN,
    TUN_AMBER
} SystemState_t;

/* ========================================================================= */
/* 2. Traffic Light Timing Defaults & Bounds (seconds)                       */
/* ========================================================================= */
#define DEFAULT_TIME_RED_S      5U
#define DEFAULT_TIME_GREEN_S    3U
#define DEFAULT_TIME_AMBER_S    2U

#define MIN_LIGHT_TIME_S        1U
#define MAX_LIGHT_TIME_S        99U

#define SEC_TO_MS(s)            ((uint32_t)(s) * 1000U)

/* ========================================================================= */
/* 3. Software Timer Channel Allocation                                      */
/* ========================================================================= */
typedef enum {
    TIMER_TRAFFIC = 0,      /* Main traffic phase duration timer */
    TIMER_COUNTDOWN = 1,    /* 1-second countdown display tick */
    TIMER_BLINK = 2,        /* Blinking timer for Tuning / warning mode */
    TIMER_LCD = 3,          /* Periodic LCD refresh timer */
    TIMER_MANUAL_TRANS = 4  /* Safe transition timer (amber before green) */
} TimerChannel_t;

/* ========================================================================= */
/* 4. Shared Global Variables (Defined in global.c)                          */
/* ========================================================================= */
extern SystemState_t status;

/* Active traffic phase durations (seconds). Invariant: RED == GREEN + AMBER */
extern uint16_t time_red_s;
extern uint16_t time_green_s;
extern uint16_t time_amber_s;

/* Temporary values edited during TUNING before validation & commit */
extern uint16_t temp_time_red_s;
extern uint16_t temp_time_green_s;
extern uint16_t temp_time_amber_s;

/* Remaining phase countdown for LCD display (seconds) */
extern uint16_t countdown_lane1;
extern uint16_t countdown_lane2;

/* Latest environment readings updated by dht20 module */
extern float env_temperature;
extern float env_humidity;
extern uint8_t env_data_valid;

/**
 * @brief Initialize all shared global variables to safe default values.
 */
void global_init(void);

#endif /* INC_GLOBAL_H_ */
