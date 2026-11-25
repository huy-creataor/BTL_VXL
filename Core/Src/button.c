/*
 * button.c
 *
 * Debounced button polling driver (3-sample shift filter = 30 ms at 10 ms tick)
 * with short-press, long-press (1000 ms), and auto-repeat (250 ms) detection.
 */

#include "button.h"
#include "pin_config.h"

/* Onboard Nucleo USER button (PC13) is active-LOW regardless of Grove module polarity */
#ifndef BTN_USER_PRESSED_STATE
#define BTN_USER_PRESSED_STATE  GPIO_PIN_RESET
#endif

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    GPIO_PinState pressed_state;
} ButtonHwConfig_t;

static const ButtonHwConfig_t btn_hw[NUM_OF_BUTTONS] = {
    [BTN_MODE]     = { BTN_1_GPIO_Port,    BTN_1_Pin,    BTN_PRESSED_STATE },
    [BTN_INC]      = { BTN_2_GPIO_Port,    BTN_2_Pin,    BTN_PRESSED_STATE },
    [BTN_DEC]      = { BTN_3_GPIO_Port,    BTN_3_Pin,    BTN_PRESSED_STATE },
    [BTN_SET]      = { BTN_4_GPIO_Port,    BTN_4_Pin,    BTN_PRESSED_STATE },
    [BTN_USER_IDX] = { BTN_USER_GPIO_Port, BTN_USER_Pin, BTN_USER_PRESSED_STATE }
};

static uint8_t key_reg0[NUM_OF_BUTTONS];
static uint8_t key_reg1[NUM_OF_BUTTONS];
static uint8_t key_reg2[NUM_OF_BUTTONS];
static uint8_t key_stable[NUM_OF_BUTTONS];

static uint16_t hold_counter[NUM_OF_BUTTONS];
static uint8_t press_flag[NUM_OF_BUTTONS];
static uint8_t long_press_flag[NUM_OF_BUTTONS];

void button_init(void)
{
    for (uint8_t i = 0U; i < NUM_OF_BUTTONS; i++) {
        key_reg0[i] = 0U;
        key_reg1[i] = 0U;
        key_reg2[i] = 0U;
        key_stable[i] = 0U;
        hold_counter[i] = 0U;
        press_flag[i] = 0U;
        long_press_flag[i] = 0U;
    }
}

void button_read(void)
{
    for (uint8_t i = 0U; i < NUM_OF_BUTTONS; i++) {
        key_reg2[i] = key_reg1[i];
        key_reg1[i] = key_reg0[i];

        GPIO_PinState raw = HAL_GPIO_ReadPin(btn_hw[i].port, btn_hw[i].pin);
        key_reg0[i] = (raw == btn_hw[i].pressed_state) ? 1U : 0U;

        if ((key_reg0[i] == key_reg1[i]) && (key_reg1[i] == key_reg2[i])) {
            uint8_t next_state = key_reg0[i];

            if (key_stable[i] != next_state) {
                key_stable[i] = next_state;
                if (next_state == 1U) {
                    press_flag[i] = 1U;
                    hold_counter[i] = BTN_LONG_PRESS_TICKS;
                } else {
                    hold_counter[i] = 0U;
                }
            } else if (next_state == 1U) {
                if (hold_counter[i] > 0U) {
                    hold_counter[i]--;
                    if (hold_counter[i] == 0U) {
                        long_press_flag[i] = 1U;
                        hold_counter[i] = BTN_AUTO_REPEAT_TICKS;
                    }
                }
            }
        }
    }
}

uint8_t isButtonPressed(uint8_t index)
{
    if (index >= NUM_OF_BUTTONS) {
        return 0U;
    }
    if (press_flag[index] != 0U) {
        press_flag[index] = 0U;
        return 1U;
    }
    return 0U;
}

uint8_t isButtonLongPressed(uint8_t index)
{
    if (index >= NUM_OF_BUTTONS) {
        return 0U;
    }
    if (long_press_flag[index] != 0U) {
        long_press_flag[index] = 0U;
        return 1U;
    }
    return 0U;
}

uint8_t isButtonHeld(uint8_t index)
{
    if (index >= NUM_OF_BUTTONS) {
        return 0U;
    }
    return key_stable[index];
}

void button_clear_all(void)
{
    for (uint8_t i = 0U; i < NUM_OF_BUTTONS; i++) {
        press_flag[i] = 0U;
        long_press_flag[i] = 0U;
    }
}
