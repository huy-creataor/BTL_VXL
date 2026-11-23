/*
 * button.h
 *
 * Debounced button polling driver supporting short press and long press.
 * Intended to be sampled every 10 ms by the scheduler.
 *
 * ARCHITECTURAL RULE: Pure C only — NO HAL headers here.
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <stdint.h>

/* Unified logical button mapping across all FSMs */
typedef enum {
    BTN_MODE = 0,       /* D7  (PA8):  Switch AUTO / MANUAL / TUNING */
    BTN_INC = 1,        /* D8  (PA9):  Next phase (Manual) / +1s (Tuning) */
    BTN_DEC = 2,        /* D9  (PC7):  All-red/warn (Manual) / -1s (Tuning) */
    BTN_SET = 3,        /* D10 (PB6):  Confirm/next field (Tuning) / Back to Auto */
    BTN_USER_IDX = 4,   /* Onboard PC13: Optional reset to defaults */
    NUM_OF_BUTTONS = 5
} ButtonIndex_t;

#define BTN_LONG_PRESS_TICKS    100U    /* 100 * 10 ms = 1000 ms */
#define BTN_AUTO_REPEAT_TICKS   25U     /* 25 * 10 ms = 250 ms */

/**
 * @brief Initialize button debounce registers and event flags.
 */
void button_init(void);

/**
 * @brief Sample and debounce all buttons; register as a 10 ms scheduler task.
 */
void button_read(void);

/**
 * @brief Check and consume a single short-press event for the given button.
 * @param index  Button index (ButtonIndex_t).
 * @return 1 if newly pressed since last check, 0 otherwise.
 */
uint8_t isButtonPressed(uint8_t index);

/**
 * @brief Check and consume a long-press / auto-repeat event.
 * @param index  Button index (ButtonIndex_t).
 * @return 1 if long-pressed / repeated, 0 otherwise.
 */
uint8_t isButtonLongPressed(uint8_t index);

/**
 * @brief Check whether the button is currently held down (debounced state).
 * @param index  Button index (ButtonIndex_t).
 * @return 1 if currently held down, 0 otherwise.
 */
uint8_t isButtonHeld(uint8_t index);

/**
 * @brief Clear all pending button press flags (useful on mode transitions).
 */
void button_clear_all(void);

#endif /* INC_BUTTON_H_ */
