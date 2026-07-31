//#################################################################################################
//  DIGIR BPM FIRMWARE LOGGING ROUTINES
//#################################################################################################

// This module desclares logging routines used to output useful log messages via UART3 to the dev
// host PC. These logs are useful for debugging and monitoring the system during bring-up.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/31/26  Edward Speer  Initial revision

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include "stm32h7xx_hal.h"

//#################################################################################################
//  CONSTANTS
//#################################################################################################

#define LOG_MSG_MAX_LEN 127

//#################################################################################################
//  STRUCTURES
//#################################################################################################

typedef enum
{
    LOG_LEVEL_CRIT,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
} LOG_LEVEL_E;

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

// Set the global logging handle
void set_global_log_handle(UART_HandleTypeDef *uart_handle);

// Emit a critical level log over UART3
void log_crit(char *msg);

// Emit a warning level log over UART3
void log_warn(char *msg);

// Emit an info level log over UART3
void log_info(char *msg);

// Emit a debug level log over UART3
void log_debug(char *msg);

