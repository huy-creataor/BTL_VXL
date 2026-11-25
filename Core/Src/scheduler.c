/*
 * scheduler.c
 *
 * Cooperative O(1) Task Scheduler using a static relative-delay (delta-queue)
 * sorted array, aligned with the CO3009 O(1) RTOS Scheduler specification.
 *
 * ARCHITECTURAL RULE: Pure C only — NO HAL headers here.
 */

#include "scheduler.h"
#include <stddef.h>

static sTask SCH_tasks_G[SCH_MAX_TASKS];
static volatile uint32_t SCH_count = 0U;
static uint32_t SCH_next_task_id = 1U;

/* Save/restore PRIMASK on ARM Cortex-M so nested calls stay safe; no-op on host */
#if defined(__GNUC__) && (defined(__arm__) || defined(__thumb__))
static inline uint32_t SCH_Lock(void)
{
    uint32_t primask;
    __asm__ volatile (
        "mrs %0, primask\n"
        "cpsid i\n"
        : "=r" (primask)
        :
        : "memory"
    );
    return primask;
}

static inline void SCH_Unlock(uint32_t primask)
{
    __asm__ volatile (
        "msr primask, %0\n"
        :
        : "r" (primask)
        : "memory"
    );
}
#else
static inline uint32_t SCH_Lock(void)
{
    return 0U;
}

static inline void SCH_Unlock(uint32_t primask)
{
    (void)primask;
}
#endif

static void sch_clear_slot(uint32_t idx)
{
    SCH_tasks_G[idx].pTask = NULL;
    SCH_tasks_G[idx].Delay = 0U;
    SCH_tasks_G[idx].Period = 0U;
    SCH_tasks_G[idx].RunMe = 0U;
    SCH_tasks_G[idx].TaskID = NO_TASK_ID;
}

static uint32_t Get_New_Task_ID(void)
{
    uint32_t candidate = SCH_next_task_id;

    for (uint32_t attempt = 0U; attempt < (SCH_MAX_TASKS + 1U); attempt++) {
        if (candidate == NO_TASK_ID) {
            candidate = 1U;
        }

        uint8_t in_use = 0U;
        for (uint32_t i = 0U; i < SCH_count; i++) {
            if (SCH_tasks_G[i].TaskID == candidate) {
                in_use = 1U;
                break;
            }
        }

        if (in_use == 0U) {
            SCH_next_task_id = candidate + 1U;
            if (SCH_next_task_id == NO_TASK_ID) {
                SCH_next_task_id = 1U;
            }
            return candidate;
        }
        candidate++;
    }

    return NO_TASK_ID;
}

/*
 * Sorted insertion into the relative-delay delta list.
 * Preserves FIFO order for equal due times (<= condition).
 */
static uint32_t Insert(void (*pF)(void), uint32_t delay, uint32_t period, uint32_t id)
{
    uint32_t m = SCH_Lock();

    if (pF == NULL || SCH_count >= SCH_MAX_TASKS || id == NO_TASK_ID) {
        SCH_Unlock(m);
        return NO_TASK_ID;
    }

    uint32_t idx = 0U;
    uint32_t sum = 0U;

    while (idx < SCH_count && SCH_tasks_G[idx].Delay <= (delay - sum)) {
        sum += SCH_tasks_G[idx++].Delay;
    }

    for (uint32_t i = SCH_count; i > idx; i--) {
        SCH_tasks_G[i] = SCH_tasks_G[i - 1U];
    }

    sTask *t = &SCH_tasks_G[idx];
    t->pTask = pF;
    t->Delay = delay - sum;
    t->Period = period;
    t->TaskID = id;
    t->RunMe = (idx == 0U && t->Delay == 0U) ? 1U : 0U;

    if (idx < SCH_count) {
        SCH_tasks_G[idx + 1U].Delay -= t->Delay;
    }

    SCH_count++;
    SCH_Unlock(m);
    return id;
}

void SCH_Init(void)
{
    uint32_t m = SCH_Lock();

    for (uint32_t i = 0U; i < SCH_MAX_TASKS; i++) {
        sch_clear_slot(i);
    }
    SCH_count = 0U;
    SCH_next_task_id = 1U;

    SCH_Unlock(m);
}

uint32_t SCH_Add_Task(void (*pFunc)(void), uint32_t DELAY, uint32_t PERIOD)
{
    uint32_t m = SCH_Lock();

    if (pFunc == NULL || SCH_count >= SCH_MAX_TASKS) {
        SCH_Unlock(m);
        return NO_TASK_ID;
    }

    uint32_t id = Get_New_Task_ID();
    uint32_t result = Insert(pFunc, DELAY, PERIOD, id);

    SCH_Unlock(m);
    return result;
}

uint8_t SCH_Delete_Task(uint32_t taskID)
{
    if (taskID == NO_TASK_ID) {
        return 0U;
    }

    uint32_t m = SCH_Lock();

    uint32_t i = SCH_MAX_TASKS;
    for (uint32_t k = 0U; k < SCH_count; k++) {
        if (SCH_tasks_G[k].TaskID == taskID) {
            i = k;
            break;
        }
    }

    if (i >= SCH_count) {
        SCH_Unlock(m);
        return 0U;
    }

    if ((i + 1U) < SCH_count) {
        SCH_tasks_G[i + 1U].Delay += SCH_tasks_G[i].Delay;
        SCH_tasks_G[i + 1U].RunMe = (i == 0U && SCH_tasks_G[i + 1U].Delay == 0U) ? 1U : 0U;
    }

    for (uint32_t j = i; (j + 1U) < SCH_count; j++) {
        SCH_tasks_G[j] = SCH_tasks_G[j + 1U];
    }

    SCH_count--;
    sch_clear_slot(SCH_count);

    SCH_Unlock(m);
    return 1U;
}

void SCH_Update(void)
{
    if (SCH_count == 0U) {
        return;
    }
    if (SCH_tasks_G[0].Delay > 0U) {
        SCH_tasks_G[0].Delay--;
    }
    if (SCH_tasks_G[0].Delay == 0U) {
        SCH_tasks_G[0].RunMe = 1U;
    }
}

void SCH_Dispatch_Tasks(void)
{
    while (1) {
        uint32_t m = SCH_Lock();
        if (SCH_count == 0U || SCH_tasks_G[0].RunMe == 0U) {
            SCH_Unlock(m);
            break;
        }
        sTask t = SCH_tasks_G[0];
        SCH_Unlock(m);

        if (t.pTask != NULL) {
            t.pTask();
        }

        if (SCH_Delete_Task(t.TaskID) && t.Period > 0U) {
            (void)Insert(t.pTask, t.Period, t.Period, t.TaskID);
        }
    }
}
