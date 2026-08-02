//#################################################################################################
//  DIGIR BPM MCU GNSS UART HANDLER
//
//  VERSION 0.1
//#################################################################################################

// This module declares the structures and functions used to capture and parse NMEA 0183 sentences
// from the GNSS module over UART.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/29/26  Edward Speer  Initial revision
// 7/20/26  Edward Speer  Add NMEA sentence parsing
// 8/1/26   Edward Speer  Double buffered byte array outputs

#ifndef GNSS_UART_H
#define GNSS_UART_H

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <stdbool.h>

#include "stm32h7xx_hal.h"
#include "nmea_parser.h"

//#################################################################################################
//  STRUCTURES
//#################################################################################################

// The GNSS UART reader operates as an FSM with two states
typedef enum
{
    GNSS_UART_STATE_WAITING, // Waiting to encounter NMEA start character
    GNSS_UART_STATE_READING, // Reading until NMEA line ending
    NUM_GNSS_UART_STATES,
} GNSS_UART_STATE_E;

// The GNSS UART reader
typedef struct {
    GNSS_UART_STATE_E  reader_state;
    UART_HandleTypeDef *uart_handle;
    uint8_t            read_byte;
    uint8_t            nmea_index;
    bool               nmea_buffer_index;
    uint8_t            *nmea_buffers[NMEA_SENTENCE_MAX_LEN];
} gnss_uart_reader_t;

//#################################################################################################
//  PUBLIC FUNCTIONS
//#################################################################################################

// Get the handle to the global GNSS UART reader
gnss_uart_reader_t *get_gnss_uart_reader(void);

// Initialize a new GNSS UART reader on the specified UART handle. 
void gnss_uart_reader_init(gnss_uart_reader_t *reader, UART_HandleTypeDef *uart_handle);

// Destroy GNSS UART reader and free all resources
void gnss_uart_reader_free(gnss_uart_reader_t *reader);

// Handle a singal byte received over UART from the GNSS module
void gnss_uart_reader_handle_byte(gnss_uart_reader_t *reader);

// Get the current sentence from the GNSS UART reader. 
void gnss_uart_reader_get_sentence(gnss_uart_reader_t *reader, uint8_t *output);

#endif // #ifndef GNSS_UART_H

