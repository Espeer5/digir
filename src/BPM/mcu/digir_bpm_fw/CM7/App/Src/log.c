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

// 7/31/26  Edward Speer  Initial Revision
// 8/10/26  Edward Speer  Support format print, timestamps

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <stdio.h>
#include <stdarg.h>

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

void emit_log(LOG_LEVEL_E log_level, const char *fmt, va_list args)
{
    if (logging_handle == NULL || fmt == NULL)
    {
        return;
    }

    char message[LOG_MSG_MAX_LEN];
    int offset = 0;

    uint32_t timestamp = HAL_GetTick();

    offset += snprintf(message + offset, sizeof(message) - offset, "[%010lu][%s] ",
                       (unsigned long)timestamp, log_level_to_str(log_level));

    offset += vsnprintf(message + offset, sizeof(message) - offset, fmt, args);

    offset += snprintf(message + offset, sizeof(message - offset), "\r\n");

    if (offset < 0)
    {
        return;
    }

    // Log out to uart3
    size_t transmit_len = (size_t)offset >= sizeof(message) ? sizeof(message) - 1 : (size_t)offset; 
    HAL_UART_Transmit(logging_handle, (const uint8_t *)message, transmit_len, HAL_MAX_DELAY);
}

void log_crit(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    emit_log(LOG_LEVEL_CRIT, fmt, args);
    va_end(args);
}

void log_warn(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    emit_log(LOG_LEVEL_WARN, fmt, args);
    va_end(args);
}

void log_info(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    emit_log(LOG_LEVEL_INFO, fmt, args);
    va_end(args);
}

void log_debug(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    emit_log(LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
}

