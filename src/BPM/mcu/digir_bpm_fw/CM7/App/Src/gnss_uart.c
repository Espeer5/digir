//#################################################################################################
//  DIGIR BPM GNSS UART HANDLER
//
//  VERSION 0.1
//#################################################################################################

// This module defines the functions used to capture data from the GNSS module over UART.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/29/26  Edward Speer  Initial Revision
// 7/30/26  Edward Speer  Add NMEA parsing
// 7/31/26  Edward Speer  Add error handling
// 7/31/26  Edward Speer  Use task handling workflow
// 8/1/26   Edward Speer  Double buffered byte array outputs
// 8/10/26  Edward Speer  Make GNSS input format generic

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <string.h>
#include <stdlib.h>

#include "log.h"
#include "gnss_uart.h"
#include "system_control.h"
#include "task_manager.h"

//#################################################################################################
//  GLOBALS
//#################################################################################################

gnss_uart_reader_t global_gnss_uart_reader;

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

// Check if the given byte is the start byte for the specified input format
static bool is_start_byte(GNSS_INPUT_FORMAT_E format, uint8_t byte)
{
    switch (format)
    {
        case GNSS_INPUT_FORMAT_NMEA:
        {
            return byte == NMEA_START_BYTE;
        }
        default:
        {
            return false;
        }
    }
}

// Check if the given byte is the stop byte for the specified input format
static bool is_stop_byte(GNSS_INPUT_FORMAT_E format, uint8_t byte)
{
    switch (format)
    {
        case GNSS_INPUT_FORMAT_NMEA:
        {
            return byte == NMEA_END_BYTE;
        }
        default:
        {
            return false;
        }
    }
}

gnss_uart_reader_t *get_gnss_uart_reader()
{
    return &global_gnss_uart_reader;
}

void gnss_uart_reader_init(gnss_uart_reader_t *reader, UART_HandleTypeDef *uart_handle,
                           GNSS_INPUT_FORMAT_E format)
{
    if (reader == NULL)
    {
        log_crit("GNSS UART handle was NULL");
        fatal_err();
    }

    if (uart_handle == NULL)
    {
        log_crit("GNSS UART handle was NULL");
        fatal_err();
    }

    reader->input_format       = format;
    reader->reader_state       = GNSS_UART_STATE_WAITING;
    reader->uart_handle        = uart_handle;
    reader->input_index        = 0;

    gnss_input_queue_t_init(&reader->input_queue);

    // Arm the GNSS UART interrupt
    if (HAL_UART_Receive_IT(uart_handle, &reader->read_byte, 1U) != HAL_OK)
    {
        log_crit("GNSS UART IT arm failed");
        fatal_err();
    }
}

void gnss_uart_reader_handle_byte(gnss_uart_reader_t *reader)
{   
    if (reader == NULL)
    {
        log_crit("GNSS UART reader handle was NULL");
        fatal_err();
    }

    uint8_t new_byte = reader->read_byte; 

    // Re-arm the GNSS UART interrupt
    if (HAL_UART_Receive_IT(reader->uart_handle, &reader->read_byte, 1U) != HAL_OK)
    {
        log_crit("Re-arming GNSS UART IT failed");
        fatal_err();
    }

    if (reader->reader_state == GNSS_UART_STATE_WAITING)
    {
        if (is_start_byte(reader->input_format, new_byte))
        {
            // Begin reading a new sentence
            memset(reader->input_buffer.data_buffer, '\n', GNSS_INPUT_MAX_LEN);

            reader->input_index  = 0;
            reader->reader_state = GNSS_UART_STATE_READING;

            reader->input_buffer.data_buffer[reader->input_index] = new_byte;
            return;
        }
        // Not currently reading anything -- in between sentences
        return;
    }

    // Read byte into current input buffer
    if (++reader->input_index >= GNSS_INPUT_MAX_LEN)
    {
        log_warn("GNSS reader received input longer than max length");

        // In this case, reset the reader
        reader->reader_state = GNSS_UART_STATE_WAITING;
    }
    else
    {
        reader->input_buffer.data_buffer[reader->input_index] = new_byte;
    }

    if (is_stop_byte(reader->input_format, new_byte))
    {
        reader->reader_state = GNSS_UART_STATE_WAITING;
        gnss_input_queue_t_push(&reader->input_queue, &reader->input_buffer, true);
        task_manager_set_task(get_global_task_manager(), TASK_ID_GNSS_UPDATE); 
    }
}

bool gnss_uart_reader_get_sentence(gnss_uart_reader_t *reader, gnss_input_t *output)
{
    // CRITICAL SECTION -- IRQs can overwrite index and change sentence during cpy
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    bool sentence = gnss_input_queue_t_pop(&reader->input_queue, output);

    __set_PRIMASK(primask);
    // END CRITICAL SECTION

    return sentence;
}

