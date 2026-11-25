/*
 * fsm_automatic.c
 *
 * Finite State Machine for AUTOMATIC 2-lane traffic light mode.
 *
 * ARCHITECTURAL RULE: Must NOT contain HAL_* calls or include main.h/pin_config.h.
 */

#include "fsm_automatic.h"
#include "fsm_manual.h"
#include "software_timer.h"
#include "button.h"
#include "traffic_light.h"

static void auto_sanitize_timings(void)
{
    uint8_t valid = 1U;

    if (time_red_s < MIN_LIGHT_TIME_S || time_red_s > MAX_LIGHT_TIME_S ||
        time_green_s < MIN_LIGHT_TIME_S || time_green_s > MAX_LIGHT_TIME_S ||
        time_amber_s < MIN_LIGHT_TIME_S || time_amber_s > MAX_LIGHT_TIME_S) {
        valid = 0U;
    } else if ((uint32_t)time_red_s != ((uint32_t)time_green_s + (uint32_t)time_amber_s)) {
        valid = 0U;
    }

    if (valid == 0U) {
        time_red_s = DEFAULT_TIME_RED_S;
        time_green_s = DEFAULT_TIME_GREEN_S;
        time_amber_s = DEFAULT_TIME_AMBER_S;
        temp_time_red_s = DEFAULT_TIME_RED_S;
        temp_time_green_s = DEFAULT_TIME_GREEN_S;
        temp_time_amber_s = DEFAULT_TIME_AMBER_S;
    }
}

static void enter_auto_state(SystemState_t next_state)
{
    status = next_state;

    switch (next_state) {
        case AUTO_RED1_GREEN2:
            traffic_set_both(RED, GREEN);
            countdown_lane1 = time_red_s;
            countdown_lane2 = time_green_s;
            setTimer(TIMER_TRAFFIC, SEC_TO_MS(time_green_s));
            setTimer(TIMER_COUNTDOWN, 1000U);
            break;

        case AUTO_RED1_AMBER2:
            traffic_set_both(RED, AMBER);
            countdown_lane1 = time_amber_s;
            countdown_lane2 = time_amber_s;
            setTimer(TIMER_TRAFFIC, SEC_TO_MS(time_amber_s));
            setTimer(TIMER_COUNTDOWN, 1000U);
            break;

        case AUTO_GREEN1_RED2:
            traffic_set_both(GREEN, RED);
            countdown_lane1 = time_green_s;
            countdown_lane2 = time_red_s;
            setTimer(TIMER_TRAFFIC, SEC_TO_MS(time_green_s));
            setTimer(TIMER_COUNTDOWN, 1000U);
            break;

        case AUTO_AMBER1_RED2:
            traffic_set_both(AMBER, RED);
            countdown_lane1 = time_amber_s;
            countdown_lane2 = time_amber_s;
            setTimer(TIMER_TRAFFIC, SEC_TO_MS(time_amber_s));
            setTimer(TIMER_COUNTDOWN, 1000U);
            break;

        default:
            traffic_all_red();
            countdown_lane1 = 0U;
            countdown_lane2 = 0U;
            break;
    }
}

void fsm_automatic_init(void)
{
    clearTimer(TIMER_TRAFFIC);
    clearTimer(TIMER_COUNTDOWN);
    clearTimer(TIMER_BLINK);
    clearTimer(TIMER_LCD);
    clearTimer(TIMER_MANUAL_TRANS);
    button_clear_all();

    auto_sanitize_timings();
    enter_auto_state(AUTO_RED1_GREEN2);
}

void fsm_automatic_run(void)
{
    switch (status) {
        case SYS_INIT:
        case AUTO_INIT:
            fsm_automatic_init();
            break;

        case AUTO_RED1_GREEN2:
        case AUTO_RED1_AMBER2:
        case AUTO_GREEN1_RED2:
        case AUTO_AMBER1_RED2:
            if (isButtonPressed(BTN_MODE)) {
                clearTimer(TIMER_TRAFFIC);
                clearTimer(TIMER_COUNTDOWN);
                fsm_manual_init();
                return;
            }

            if (isButtonPressed(BTN_USER_IDX)) {
                time_red_s = DEFAULT_TIME_RED_S;
                time_green_s = DEFAULT_TIME_GREEN_S;
                time_amber_s = DEFAULT_TIME_AMBER_S;
                temp_time_red_s = DEFAULT_TIME_RED_S;
                temp_time_green_s = DEFAULT_TIME_GREEN_S;
                temp_time_amber_s = DEFAULT_TIME_AMBER_S;
                fsm_automatic_init();
                return;
            }

            /* Check phase expiry before countdown tick so phase rollover is immediate */
            if (isTimerExpired(TIMER_TRAFFIC)) {
                if (status == AUTO_RED1_GREEN2) {
                    enter_auto_state(AUTO_RED1_AMBER2);
                } else if (status == AUTO_RED1_AMBER2) {
                    enter_auto_state(AUTO_GREEN1_RED2);
                } else if (status == AUTO_GREEN1_RED2) {
                    enter_auto_state(AUTO_AMBER1_RED2);
                } else {
                    enter_auto_state(AUTO_RED1_GREEN2);
                }
                return;
            }

            if (isTimerExpired(TIMER_COUNTDOWN)) {
                setTimer(TIMER_COUNTDOWN, 1000U);
                if (countdown_lane1 > 1U) {
                    countdown_lane1--;
                }
                if (countdown_lane2 > 1U) {
                    countdown_lane2--;
                }
            }
            break;

        default:
            break;
    }
}
