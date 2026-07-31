//#################################################################################################
//  DIGIR BPM FIRMWARE LOGGING ROUTINES
//
//  VERSION 0.1 
//#################################################################################################

// This module defines logging reoutines used to output useful log messages over UART3 to the dev
// host PC. These logs are useful for debugging and monitoring the system during bring-up.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/31/26  Edward Speer Initial Revision

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <stdio.h>

#include "log.h"

//#################################################################################################
//  GLOBALS
//#################################################################################################

UART_HandleTypeDef *logging_handle;

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

static const char *log_level_to_str(LOG_LEVEL_E level)
{
    switch(level)
    {
        case LOG_LEVEL_CRIT:
        {
            return "CRIT";
        }
        case LOG_LEVEL_WARN:
        {
            return "WARN";
        }
        case LOG_LEVEL_INFO:
        {
            return "INFO";
        }
        case LOG_LEVEL_DEBUG:
        {
            return "DBUG";
        }
        default:
        {
            return "UNKN";
        }
    }
}

void set_global_log_handle(UART_HandleTypeDef *uart_handle)
{
    logging_handle = uart_handle;
}

void emit_log(LOG_LEVEL_E log_level, char *msg)
{
    if (logging_handle == NULL || msg == NULL)
    {
        return;
    }

    char message[LOG_MSG_MAX_LEN];
    int len = snprintf(message, sizeof(message), "[%s] %s\r\n", log_level_to_str(log_level), msg);

    if (len < 0)
    {
        return;
    }

    // Log out to uart3
    size_t transmit_len = (size_t)len >= sizeof(message) ? sizeof(message) - 1U : (size_t)len; 
    HAL_UART_Transmit(logging_handle, (const uint8_t *)message, transmit_len, HAL_MAX_DELAY);
}

void log_crit(char *msg)
{
    emit_log(LOG_LEVEL_CRIT, msg);
}

void log_warn(char *msg)
{
    emit_log(LOG_LEVEL_WARN, msg);
}

void log_info(char *msg)
{
    emit_log(LOG_LEVEL_INFO, msg);
}

void log_debug(char *msg)
{
    emit_log(LOG_LEVEL_DEBUG, msg);
}

