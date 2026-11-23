/*
 * software_timer.h
 *
 * Multi-channel software timer driven by a 10 ms scheduler task.
 *
 * ARCHITECTURAL RULE: Pure C only — NO HAL headers here.
 */

#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include <stdint.h>

#define MAX_TIMERS  8U

/* Timeout flags for each channel (1 = expired, 0 = running/cleared) */
extern uint8_t timer_flag[MAX_TIMERS];

/**
 * @brief Reset all software timer counters and flags.
 */
void timer_init(void);

/**
 * @brief Start or restart a software timer channel.
 * @param index        Timer channel index (0 .. MAX_TIMERS - 1).
 * @param duration_ms  Duration in milliseconds (converted to 10 ms ticks).
 */
void setTimer(uint8_t index, uint32_t duration_ms);

/**
 * @brief Check whether a timer channel has expired (non-destructive).
 * @param index  Timer channel index.
 * @return 1 if expired, 0 otherwise.
 */
uint8_t isTimerExpired(uint8_t index);

/**
 * @brief Clear a timer channel counter and timeout flag.
 * @param index  Timer channel index.
 */
void clearTimer(uint8_t index);

/**
 * @brief Update all active timer channels; register as a 10 ms scheduler task.
 */
void timer_run(void);

#endif /* INC_SOFTWARE_TIMER_H_ */
