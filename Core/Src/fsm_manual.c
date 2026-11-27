/*
 * fsm_manual.c
 *
 * Finite State Machine for MANUAL button-controlled traffic light mode.
 * Transitions between opposing GREEN phases always pass through an AMBER
 * clearance interval to prevent unsafe instantaneous conflicts.
 *
 * ARCHITECTURAL RULE: Must NOT contain HAL_* calls or include main.h/pin_config.h.
 */

#include "fsm_manual.h"
#include "fsm_automatic.h"
#include "software_timer.h"
#include "button.h"
#include "traffic_light.h"

static uint16_t manual_amber_duration_s(void)
{
    if (time_amber_s >= MIN_LIGHT_TIME_S && time_amber_s <= MAX_LIGHT_TIME_S) {
        return time_amber_s;
    }
    return DEFAULT_TIME_AMBER_S;
}

static void enter_manual_all_red(void)
{
    clearTimer(TIMER_MANUAL_TRANS);
    traffic_all_red();
    countdown_lane1 = 0U;
    countdown_lane2 = 0U;
    status = MAN_ALL_RED;
}

static void safe_exit_to_tuning(void)
{
    clearTimer(TIMER_TRAFFIC);
    clearTimer(TIMER_COUNTDOWN);
    clearTimer(TIMER_BLINK);
    clearTimer(TIMER_LCD);
    clearTimer(TIMER_MANUAL_TRANS);
    traffic_all_red();
    countdown_lane1 = 0U;
    countdown_lane2 = 0U;
    button_clear_all();
    status = TUN_INIT;
}

void fsm_manual_init(void)
{
    clearTimer(TIMER_TRAFFIC);
    clearTimer(TIMER_COUNTDOWN);
    clearTimer(TIMER_BLINK);
    clearTimer(TIMER_LCD);
    clearTimer(TIMER_MANUAL_TRANS);
    button_clear_all();
    enter_manual_all_red();
}

void fsm_manual_run(void)
{
    switch (status) {
        case MAN_INIT:
            fsm_manual_init();
            break;

        case MAN_ALL_RED:
            if (isButtonPressed(BTN_MODE)) {
                safe_exit_to_tuning();
                return;
            }
            if (isButtonPressed(BTN_SET)) {
                fsm_automatic_init();
                return;
            }
            (void)isButtonPressed(BTN_DEC);
            if (isButtonPressed(BTN_INC)) {
                traffic_set_both(RED, GREEN);
                countdown_lane1 = 0U;
                countdown_lane2 = 0U;
                status = MAN_RED1_GREEN2;
            }
            break;

        case MAN_RED1_GREEN2:
            if (isButtonPressed(BTN_MODE)) {
                safe_exit_to_tuning();
                return;
            }
            if (isButtonPressed(BTN_SET)) {
                fsm_automatic_init();
                return;
            }
            if (isButtonPressed(BTN_DEC)) {
                enter_manual_all_red();
                return;
            }
            if (isButtonPressed(BTN_INC)) {
                traffic_set_both(RED, AMBER);
                setTimer(TIMER_MANUAL_TRANS, SEC_TO_MS(manual_amber_duration_s()));
                status = MAN_RED1_AMBER2;
            }
            break;

        case MAN_RED1_AMBER2:
            if (isButtonPressed(BTN_MODE)) {
                safe_exit_to_tuning();
                return;
            }
            if (isButtonPressed(BTN_SET)) {
                clearTimer(TIMER_MANUAL_TRANS);
                fsm_automatic_init();
                return;
            }
            if (isButtonPressed(BTN_DEC)) {
                enter_manual_all_red();
                return;
            }
            /* Ignore phase toggle spam while amber clearance is active */
            (void)isButtonPressed(BTN_INC);

            if (isTimerExpired(TIMER_MANUAL_TRANS)) {
                clearTimer(TIMER_MANUAL_TRANS);
                traffic_set_both(GREEN, RED);
                status = MAN_GREEN1_RED2;
            }
            break;

        case MAN_GREEN1_RED2:
            if (isButtonPressed(BTN_MODE)) {
                safe_exit_to_tuning();
                return;
            }
            if (isButtonPressed(BTN_SET)) {
                fsm_automatic_init();
                return;
            }
            if (isButtonPressed(BTN_DEC)) {
                enter_manual_all_red();
                return;
            }
            if (isButtonPressed(BTN_INC)) {
                traffic_set_both(AMBER, RED);
                setTimer(TIMER_MANUAL_TRANS, SEC_TO_MS(manual_amber_duration_s()));
                status = MAN_AMBER1_RED2;
            }
            break;

        case MAN_AMBER1_RED2:
            if (isButtonPressed(BTN_MODE)) {
                safe_exit_to_tuning();
                return;
            }
            if (isButtonPressed(BTN_SET)) {
                clearTimer(TIMER_MANUAL_TRANS);
                fsm_automatic_init();
                return;
            }
            if (isButtonPressed(BTN_DEC)) {
                enter_manual_all_red();
                return;
            }
            /* Ignore phase toggle spam while amber clearance is active */
            (void)isButtonPressed(BTN_INC);

            if (isTimerExpired(TIMER_MANUAL_TRANS)) {
                clearTimer(TIMER_MANUAL_TRANS);
                traffic_set_both(RED, GREEN);
                status = MAN_RED1_GREEN2;
            }
            break;

        default:
            break;
    }
}
