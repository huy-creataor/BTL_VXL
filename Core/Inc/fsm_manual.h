/*
 * fsm_manual.h
 *
 * Finite State Machine for MANUAL button-controlled traffic light mode.
 *
 * ARCHITECTURAL RULE: Must NOT contain HAL_* calls or include main.h.
 */

#ifndef INC_FSM_MANUAL_H_
#define INC_FSM_MANUAL_H_

#include "global.h"

/**
 * @brief Safely initialize/enter MANUAL mode (clears timers and sets safe state).
 */
void fsm_manual_init(void);

/**
 * @brief Periodic scheduler task running the MANUAL mode FSM.
 */
void fsm_manual_run(void);

#endif /* INC_FSM_MANUAL_H_ */
