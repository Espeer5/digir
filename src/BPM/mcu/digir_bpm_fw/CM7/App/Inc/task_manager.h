//#################################################################################################
//  DIGIR CPM MCU TASK MANAGER
//
//  VERSION 0.1
//#################################################################################################

// This module contains declarations of structures and functions for main-thread task handling in
// the DigiR BPM MCU.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/31/26  Edward Speer  Initial revision
// 8/1/26   Edward Speer  Task mask made atomic

#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <stdatomic.h>
#include <stdint.h>
#include <stdbool.h>

//#################################################################################################
//  STRUCTURES
//#################################################################################################

typedef enum {
    TASK_ID_GNSS_UPDATE,
} TASK_ID_E;

typedef struct {
    atomic_uint_least32_t task_mask;
} task_manager_t;

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

// Get the pointer to the global task manager
task_manager_t *get_global_task_manager(void);

// Initialize a task manager
void task_manager_init(task_manager_t *task_manager);

// Schedules the task identified by the task ID to occur on the next loop iteration
void task_manager_set_task(task_manager_t *task_manager, TASK_ID_E task);

// Handles all currently scheduled tasks. Should be called in the main loop.
void task_manager_run(task_manager_t *task_manager);

#endif // #ifndef TASK_MANAGER_H

