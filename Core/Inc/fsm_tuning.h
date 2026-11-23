/*
 * fsm_tuning.h
 *
 * Finite State Machine for TUNING traffic light durations (RED / GREEN / AMBER).
 *
 * ARCHITECTURAL RULE: Must NOT contain HAL_* calls or include main.h.
 */

#ifndef INC_FSM_TUNING_H_
#define INC_FSM_TUNING_H_

#include "global.h"

/**
 * @brief Safely initialize/enter TUNING mode (loads temp durations & sets blink timer).
 */
void fsm_tuning_init(void);

/**
 * @brief Periodic scheduler task running the TUNING mode FSM.
 */
void fsm_tuning_run(void);

/**
 * @brief Validate candidate timing parameters before committing.
 * @return 1 if valid (within bounds and red_s == green_s + amber_s), 0 otherwise.
 */
uint8_t fsm_tuning_validate(uint16_t red_s, uint16_t green_s, uint16_t amber_s);

#endif /* INC_FSM_TUNING_H_ */
