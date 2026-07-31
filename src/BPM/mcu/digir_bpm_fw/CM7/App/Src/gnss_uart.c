//#################################################################################################
//  DIGIR BPM GNSS UART HANDLER
//
//  VERSION 0.1
//#################################################################################################

// This module defines the functions used to capture NMEA 0183 sentences from the GNSS module over
// UART.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/29/26  Edward Speer  Initial Revision
// 7/30/26  Edward Speer  Add NMEA parsing
// 7/31/26  Edward Speer  Add error handling

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <string.h>

#include "log.h"
#include "gnss_uart.h"
#include "system_control.h"

//#################################################################################################
//  GLOBALS
//#################################################################################################

gnss_uart_reader_t global_gnss_uart_reader;

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

gnss_uart_reader_t *get_gnss_uart_reader()
{
    return &global_gnss_uart_reader;
}

void gnss_uart_reader_init(gnss_uart_reader_t *reader, UART_HandleTypeDef *uart_handle)
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

    reader->reader_state = GNSS_UART_STATE_WAITING;
    reader->uart_handle  = uart_handle;
    reader->nmea_index   = 0;

    memset(&reader->nmea_buffer, '\n', NMEA_SENTENCE_MAX_LEN);

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

    bool read_byte = true;

    if (reader->reader_state == GNSS_UART_STATE_WAITING)
    {
        if (reader->read_byte == NMEA_START_BYTE)
        {
            // Begin reading a new sentence
            reader->new_sentence = false;
            reader->nmea_index   = -1;
            memset(&reader->nmea_buffer, '\n', NMEA_SENTENCE_MAX_LEN);
            reader->reader_state = GNSS_UART_STATE_READING;
        } else 
        {
            read_byte = false;
        }
    }
    else if (reader->reader_state == GNSS_UART_STATE_READING && reader->read_byte == NMEA_END_BYTE)
    {
        reader->reader_state      = GNSS_UART_STATE_WAITING;
        reader->prev_sentence     = nmea_bytes_to_sentence(reader->nmea_buffer);
        reader->new_sentence      = true;
    } 

    // Read byte into current NMEA sentence
    if (read_byte)
    {
        if (++reader->nmea_index >= NMEA_SENTENCE_MAX_LEN)
        {
            log_warn("GNSS reader received sentence longer than max length");
            return;
        }
        reader->nmea_buffer[reader->nmea_index] = reader->read_byte;
    }

    // Re-arm the GNSS UART interrupt
    if (HAL_UART_Receive_IT(reader->uart_handle, &reader->read_byte, 1U) != HAL_OK)
    {
        log_crit("Re-arming GNSS UART IT failed");
        fatal_err();
    }
}

bool gnss_uart_reader_new_sentence(gnss_uart_reader_t *reader)
{
    return reader->new_sentence;
}

nmea_sentence_t gnss_uart_reader_get_sentence(gnss_uart_reader_t *reader)
{
    reader->new_sentence = false;
    return reader->prev_sentence;
}

