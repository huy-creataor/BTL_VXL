/*
 * traffic_light.h
 *
 * Hardware abstraction layer for the 2-lane Grove traffic light modules.
 *
 * ARCHITECTURAL RULE: Pure C only — NO HAL headers here.
 */

#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

#include <stdint.h>

typedef enum {
    OFF = 0,
    RED,
    AMBER,
    GREEN
} TrafficColor_t;

typedef enum {
    LANE_1 = 1,
    LANE_2 = 2
} TrafficLane_t;

/**
 * @brief Initialize traffic light outputs to a safe default state (ALL RED or OFF).
 */
void traffic_init(void);

/**
 * @brief Set the light color for a single traffic lane.
 * @param lane   1 (LANE_1) or 2 (LANE_2).
 * @param color  OFF, RED, AMBER, or GREEN.
 */
void traffic_set(uint8_t lane, TrafficColor_t color);

/**
 * @brief Atomically set both lanes while enforcing the safety invariant.
 * @note  Safety invariant: NEVER allows both lane1 and lane2 to be GREEN
 *        simultaneously; if attempted, falls back to safe ALL RED.
 * @param lane1_color  Color for Lane 1.
 * @param lane2_color  Color for Lane 2.
 */
void traffic_set_both(TrafficColor_t lane1_color, TrafficColor_t lane2_color);

/**
 * @brief Turn off both traffic light modules (used for blinking in Tuning).
 */
void traffic_all_off(void);

/**
 * @brief Force both traffic light modules to RED (safe fallback state).
 */
void traffic_all_red(void);

#endif /* INC_TRAFFIC_LIGHT_H_ */
