/*
 * scheduler.h
 *
 * Cooperative O(1) Task Scheduler supporting periodic and one-shot tasks.
 * SCH_Update() runs in O(1) inside the 10 ms TIM2 interrupt by maintaining
 * a relative-delay sorted task list.
 *
 * ARCHITECTURAL RULE: Pure C only — NO HAL headers here.
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>

#define SCH_MAX_TASKS           16U
#define SCH_TICK_PERIOD_MS      10U
#define SCH_INVALID_TASK_ID     0xFFFFFFFFU

#define MS_TO_TICKS(ms)         ((uint32_t)(ms) / SCH_TICK_PERIOD_MS)

typedef struct {
    void (*pTask)(void);    /* Pointer to task function */
    uint32_t Delay;         /* Relative delay in ticks before task runs */
    uint32_t Period;        /* Period in ticks; 0 = one-shot task */
    uint8_t RunMe;          /* Flag set by SCH_Update() when ready to run */
    uint32_t TaskID;        /* Unique task identifier */
} sTask;

/**
 * @brief Initialize the scheduler task list and internal counters.
 */
void SCH_Init(void);

/**
 * @brief Add a periodic or one-shot task to the sorted relative-delay list.
 * @param pFunc   Pointer to task callback function.
 * @param DELAY   Initial delay in scheduler ticks (1 tick = 10 ms).
 * @param PERIOD  Repeat period in ticks (0 for one-shot task).
 * @return Assigned TaskID, or SCH_INVALID_TASK_ID if list is full/invalid.
 */
uint32_t SCH_Add_Task(void (*pFunc)(void), uint32_t DELAY, uint32_t PERIOD);

/**
 * @brief Delete a task by its TaskID while preserving relative delays.
 * @param taskID  Identifier returned by SCH_Add_Task().
 * @return 1 if deleted successfully, 0 if not found.
 */
uint8_t SCH_Delete_Task(uint32_t taskID);

/**
 * @brief O(1) tick update function; call inside the 10 ms timer ISR.
 */
void SCH_Update(void);

/**
 * @brief Dispatch ready tasks (RunMe > 0); call inside main while(1).
 */
void SCH_Dispatch_Tasks(void);

#endif /* INC_SCHEDULER_H_ */
