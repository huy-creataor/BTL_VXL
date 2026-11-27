/*
 * traffic_light.c
 *
 * Hardware abstraction layer for the 2-lane Grove traffic light modules.
 * Enforces the safety invariant that Lane 1 and Lane 2 are never GREEN
 * simultaneously.
 */

#include "traffic_light.h"
#include "pin_config.h"

static TrafficColor_t current_color[2] = { OFF, OFF };

static void traffic_write_hw(uint8_t lane, TrafficColor_t color)
{
    GPIO_PinState sig_a;
    GPIO_PinState sig_b;

    switch (color) {
        case RED:
            sig_a = TL_SIG_RED_A;
            sig_b = TL_SIG_RED_B;
            break;
        case AMBER:
            sig_a = TL_SIG_AMBER_A;
            sig_b = TL_SIG_AMBER_B;
            break;
        case GREEN:
            sig_a = TL_SIG_GREEN_A;
            sig_b = TL_SIG_GREEN_B;
            break;
        case OFF:
        default:
            color = OFF;
            sig_a = TL_SIG_OFF_A;
            sig_b = TL_SIG_OFF_B;
            break;
    }

    if (lane == LANE_1) {
        HAL_GPIO_WritePin(TL1_A_GPIO_Port, TL1_A_Pin, sig_a);
        HAL_GPIO_WritePin(TL1_B_GPIO_Port, TL1_B_Pin, sig_b);
        current_color[0] = color;
    } else if (lane == LANE_2) {
        HAL_GPIO_WritePin(TL2_A_GPIO_Port, TL2_A_Pin, sig_a);
        HAL_GPIO_WritePin(TL2_B_GPIO_Port, TL2_B_Pin, sig_b);
        current_color[1] = color;
    }
}

void traffic_init(void)
{
    traffic_all_red();
}

void traffic_set(uint8_t lane, TrafficColor_t color)
{
    if (lane == LANE_1) {
        if (color == GREEN && current_color[1] == GREEN) {
            traffic_all_red();
            return;
        }
        traffic_write_hw(LANE_1, color);
    } else if (lane == LANE_2) {
        if (color == GREEN && current_color[0] == GREEN) {
            traffic_all_red();
            return;
        }
        traffic_write_hw(LANE_2, color);
    }
}

void traffic_set_both(TrafficColor_t lane1_color, TrafficColor_t lane2_color)
{
    if (lane1_color == GREEN && lane2_color == GREEN) {
        traffic_all_red();
        return;
    }

    /* Update the non-GREEN lane first to prevent transient dual-GREEN glitches */
    if (lane1_color != GREEN) {
        traffic_write_hw(LANE_1, lane1_color);
        traffic_write_hw(LANE_2, lane2_color);
    } else {
        traffic_write_hw(LANE_2, lane2_color);
        traffic_write_hw(LANE_1, lane1_color);
    }
}

void traffic_all_off(void)
{
    traffic_write_hw(LANE_1, OFF);
    traffic_write_hw(LANE_2, OFF);
}

void traffic_all_red(void)
{
    traffic_write_hw(LANE_1, RED);
    traffic_write_hw(LANE_2, RED);
}

void traffic_set_tuning(TrafficColor_t color)
{
    traffic_write_hw(LANE_1, color);
    traffic_write_hw(LANE_2, color);
}
