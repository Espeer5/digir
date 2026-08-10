//#################################################################################################
//  DIGIR BPM MCU GNSS UART HANDLER
//
//  VERSION 0.1
//#################################################################################################

// This module declares the structures and functions used to capture data from the GNSS module over
// UART.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/29/26  Edward Speer  Initial revision
// 7/20/26  Edward Speer  Add NMEA sentence parsing
// 8/1/26   Edward Speer  Double buffered byte array outputs
// 8/10/26  Edward Speer  Make GNSS input format generic

#ifndef GNSS_UART_H
#define GNSS_UART_H

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <stdbool.h>

#include "stm32h7xx_hal.h"
#include "nmea_parser.h"
#include "ringbuffer.h"

//#################################################################################################
//  CONSTANTS
//#################################################################################################

// Set to the largest input allowed for any support GNSS UART format
#define GNSS_INPUT_MAX_LEN 92

// Capacity of ringbuffer holding GNSS inputs
#define GNSS_INPUT_BUFF_CAPACITY 7

//#################################################################################################
//  STRUCTURES
//#################################################################################################

// Supported input formats for GNSS data
typedef enum
{
    GNSS_INPUT_FORMAT_NMEA,
} GNSS_INPUT_FORMAT_E;

// The GNSS UART reader operates as an FSM with two states
typedef enum
{
    GNSS_UART_STATE_WAITING, // Waiting to start reading a new input
    GNSS_UART_STATE_READING, // Reading until end of input
    NUM_GNSS_UART_STATES,
} GNSS_UART_STATE_E;

// Declare a struct around the input data buffer to allow copy-by-value
typedef struct
{
    uint8_t data_buffer[GNSS_INPUT_MAX_LEN];
} gnss_input_t;

// Declare a ring buffer of GNSS inputs to push each commpleted read to
RBUFF_DEF(gnss_input_queue_t, gnss_input_t, GNSS_INPUT_BUFF_CAPACITY);

// The GNSS UART reader
typedef struct {
    GNSS_UART_STATE_E   reader_state;
    GNSS_INPUT_FORMAT_E input_format;
    UART_HandleTypeDef  *uart_handle;
    uint8_t             read_byte;
    uint8_t             input_index;
    gnss_input_t        input_buffer;
    gnss_input_queue_t  input_queue;
} gnss_uart_reader_t;

//#################################################################################################
//  PUBLIC FUNCTIONS
//#################################################################################################

// Get the handle to the global GNSS UART reader
gnss_uart_reader_t *get_gnss_uart_reader(void);

// Initialize a new GNSS UART reader on the specified UART handle with the specified format.
void gnss_uart_reader_init(gnss_uart_reader_t *reader, UART_HandleTypeDef *uart_handle,
                           GNSS_INPUT_FORMAT_E format);

// Handle a single byte received over UART from the GNSS module
void gnss_uart_reader_handle_byte(gnss_uart_reader_t *reader);

// Get a sentence from the UART reader if one is available.
bool gnss_uart_reader_get_sentence(gnss_uart_reader_t *reader, gnss_input_t *output);

#endif // #ifndef GNSS_UART_H

