/*
 * software_timer.c
 *
 * Multi-channel software timer updated every 10 ms tick (TICK = 10 ms).
 *
 * ARCHITECTURAL RULE: Pure C only — NO HAL headers here.
 */

#include "software_timer.h"
#include "scheduler.h"

#define TICK    SCH_TICK_PERIOD_MS  /* 10 ms per tick */

uint8_t timer_flag[MAX_TIMERS];
static uint32_t timer_counter[MAX_TIMERS];

void timer_init(void)
{
    for (uint8_t i = 0U; i < MAX_TIMERS; i++) {
        timer_counter[i] = 0U;
        timer_flag[i] = 0U;
    }
}

void setTimer(uint8_t index, uint32_t duration_ms)
{
    if (index >= MAX_TIMERS) {
        return;
    }

    if (duration_ms == 0U) {
        timer_counter[index] = 0U;
        timer_flag[index] = 1U;
        return;
    }

    uint32_t ticks = duration_ms / TICK;
    if (ticks == 0U) {
        ticks = 1U;
    }

    timer_counter[index] = ticks;
    timer_flag[index] = 0U;
}

uint8_t isTimerExpired(uint8_t index)
{
    if (index >= MAX_TIMERS) {
        return 0U;
    }
    return (timer_flag[index] != 0U) ? 1U : 0U;
}

void clearTimer(uint8_t index)
{
    if (index >= MAX_TIMERS) {
        return;
    }
    timer_counter[index] = 0U;
    timer_flag[index] = 0U;
}

void timer_run(void)
{
    for (uint8_t i = 0U; i < MAX_TIMERS; i++) {
        if (timer_counter[i] > 0U) {
            timer_counter[i]--;
            if (timer_counter[i] == 0U) {
                timer_flag[i] = 1U;
            }
        }
    }
}
