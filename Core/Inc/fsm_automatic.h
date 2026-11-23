/*
 * fsm_automatic.h
 *
 * Finite State Machine for AUTOMATIC traffic light mode.
 *
 * ARCHITECTURAL RULE: Must NOT contain HAL_* calls or include main.h.
 */

#ifndef INC_FSM_AUTOMATIC_H_
#define INC_FSM_AUTOMATIC_H_

#include "global.h"

/**
 * @brief Safely initialize/enter AUTOMATIC mode (starts at AUTO_RED1_GREEN2).
 */
void fsm_automatic_init(void);

/**
 * @brief Periodic scheduler task running the AUTOMATIC mode FSM.
 */
void fsm_automatic_run(void);

#endif /* INC_FSM_AUTOMATIC_H_ */
