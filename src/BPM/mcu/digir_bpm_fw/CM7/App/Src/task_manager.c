//#################################################################################################
//  DIGIR BPM MCU TASK MANAGER
//
//  VERSION 0.1
//#################################################################################################

// This module contains definitions of functions for main-thread task handling in the DigiR BPM MCU

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/31/26  Edward Speer  Initial revision
// 8/1/26   Edward Speer  Parse NMEA snapshots from GNSS
// 8/2/26   Edward Speer  Mutate global state cache
// 8/10/26  Edward Speer  GNSS input type is generic

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include "task_manager.h"
#include "log.h"
#include "nmea_parser.h"
#include "gnss_uart.h"
#include "system_control.h"
#include "global_state.h"

//#################################################################################################
//  GLOBALS
//#################################################################################################

// Global task manager
task_manager_t global_task_manager;

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

static bool task_manager_check_task(uint_least32_t mask, TASK_ID_E task)
{
    return (mask & (0b1 << task)) != 0;
}

task_manager_t *get_global_task_manager()
{
    return &global_task_manager;
}

void task_manager_init(task_manager_t *task_manager)
{
    atomic_init(&task_manager->task_mask, 0);
}

void task_manager_set_task(task_manager_t *task_manager, TASK_ID_E task)
{
    const uint_least32_t mask = 0b1 << task;
    atomic_fetch_or_explicit(&task_manager->task_mask, mask, memory_order_release);
}

void task_manager_run(task_manager_t *task_manager)
{
    if (task_manager == NULL)
    {
        log_crit("Task manager is null");
        fatal_err();
    }

    const uint_least32_t pending_tasks = atomic_exchange_explicit(&task_manager->task_mask, 0,
                                                                  memory_order_acquire);

    if (task_manager_check_task(pending_tasks, TASK_ID_GNSS_UPDATE))
    {
        log_debug("Task manager handling GNSS update");

        gnss_uart_reader_t *reader = get_gnss_uart_reader();

        switch (reader->input_format)
        {
            case GNSS_INPUT_FORMAT_NMEA:
            {
                gnss_input_t nmea_sentence_bytes;
                while (gnss_uart_reader_get_sentence(reader, &nmea_sentence_bytes))
                { 
                    nmea_sentence_t sentence = nmea_bytes_to_sentence(nmea_sentence_bytes.data_buffer);
                    update_global_gnss_state_nmea(get_global_state(), sentence);
                }
                break;
            }
        }
    }
}

