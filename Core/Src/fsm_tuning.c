/*
 * fsm_tuning.c
 *
 * Finite State Machine for TUNING traffic light durations (RED / GREEN / AMBER).
 * ARCHITECTURAL RULE: Must NOT contain HAL_* calls or include main.h / pin_config.h.
 */

#include "fsm_tuning.h"
#include "button.h"
#include "traffic_light.h"
#include "software_timer.h"
#include "lcd.h"

#define TUNING_BLINK_MS     250U
#define TUNING_LCD_MS       200U

static uint8_t blink_on = 1U;

static uint16_t *tuning_get_active_ptr(SystemState_t state)
{
    switch (state) {
        case TUN_RED:   return &temp_time_red_s;
        case TUN_GREEN: return &temp_time_green_s;
        case TUN_AMBER: return &temp_time_amber_s;
        default:        return &temp_time_red_s;
    }
}

static TrafficColor_t tuning_get_active_color(SystemState_t state)
{
    switch (state) {
        case TUN_RED:   return RED;
        case TUN_GREEN: return GREEN;
        case TUN_AMBER: return AMBER;
        default:        return RED;
    }
}

static const char *tuning_get_color_label(SystemState_t state)
{
    switch (state) {
        case TUN_RED:   return "RED";
        case TUN_GREEN: return "GREEN";
        case TUN_AMBER: return "AMBER";
        default:        return "RED";
    }
}

uint8_t fsm_tuning_validate(uint16_t red_s, uint16_t green_s, uint16_t amber_s)
{
    if (red_s < MIN_LIGHT_TIME_S || red_s > MAX_LIGHT_TIME_S) {
        return 0U;
    }
    if (green_s < MIN_LIGHT_TIME_S || green_s > MAX_LIGHT_TIME_S) {
        return 0U;
    }
    if (amber_s < MIN_LIGHT_TIME_S || amber_s > MAX_LIGHT_TIME_S) {
        return 0U;
    }
    return (red_s == (uint16_t)(green_s + amber_s)) ? 1U : 0U;
}

void fsm_tuning_init(void)
{
    temp_time_red_s = time_red_s;
    temp_time_green_s = time_green_s;
    temp_time_amber_s = time_amber_s;

    status = TUN_RED;
    blink_on = 1U;
    traffic_set_both(RED, RED);

    setTimer(TIMER_BLINK, TUNING_BLINK_MS);
    setTimer(TIMER_LCD, TUNING_LCD_MS);
    button_clear_all();

    lcd_display_tuning("RED", temp_time_red_s, temp_time_red_s, temp_time_green_s, temp_time_amber_s);
}

void fsm_tuning_run(void)
{
    if (status == TUN_INIT) {
        fsm_tuning_init();
        return;
    }

    if (status != TUN_RED && status != TUN_GREEN && status != TUN_AMBER) {
        return;
    }

    /* Exit tuning without saving if MODE is pressed */
    if (isButtonPressed(BTN_MODE) != 0U) {
        traffic_all_red();
        status = AUTO_INIT;
        return;
    }

    /* Blink active color on both lanes */
    if (isTimerExpired(TIMER_BLINK) != 0U) {
        blink_on = (blink_on == 0U) ? 1U : 0U;
        if (blink_on != 0U) {
            TrafficColor_t c = tuning_get_active_color(status);
            traffic_set_both(c, c);
        } else {
            traffic_all_off();
        }
        setTimer(TIMER_BLINK, TUNING_BLINK_MS);
    }

    uint16_t *active_val = tuning_get_active_ptr(status);

    /* Increment / Decrement active duration */
    if (isButtonPressed(BTN_INC) != 0U || isButtonLongPressed(BTN_INC) != 0U) {
        *active_val = (*active_val >= MAX_LIGHT_TIME_S) ? MIN_LIGHT_TIME_S : (uint16_t)(*active_val + 1U);
    }
    if (isButtonPressed(BTN_DEC) != 0U || isButtonLongPressed(BTN_DEC) != 0U) {
        *active_val = (*active_val <= MIN_LIGHT_TIME_S) ? MAX_LIGHT_TIME_S : (uint16_t)(*active_val - 1U);
    }

    /* Advance field or validate & commit */
    if (isButtonPressed(BTN_SET) != 0U) {
        if (status == TUN_RED) {
            status = TUN_GREEN;
            blink_on = 1U;
            traffic_set_both(GREEN, GREEN);
        } else if (status == TUN_GREEN) {
            status = TUN_AMBER;
            blink_on = 1U;
            traffic_set_both(AMBER, AMBER);
        } else {
            if (fsm_tuning_validate(temp_time_red_s, temp_time_green_s, temp_time_amber_s) != 0U) {
                time_red_s = temp_time_red_s;
                time_green_s = temp_time_green_s;
                time_amber_s = temp_time_amber_s;
                traffic_all_red();
                status = AUTO_INIT;
                return;
            } else {
                lcd_display_error("R != G + A");
                status = TUN_RED;
                blink_on = 1U;
                traffic_set_both(RED, RED);
                setTimer(TIMER_LCD, 800U);
                return;
            }
        }
        setTimer(TIMER_BLINK, TUNING_BLINK_MS);
    }

    /* Periodic LCD refresh */
    if (isTimerExpired(TIMER_LCD) != 0U) {
        lcd_display_tuning(tuning_get_color_label(status), *active_val,
                           temp_time_red_s, temp_time_green_s, temp_time_amber_s);
        setTimer(TIMER_LCD, TUNING_LCD_MS);
    }
}
