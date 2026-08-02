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
// 7/31/26  Edward Speer  Use task handling workflow
// 8/1/26   Edward Speer  Double buffered byte array outputs

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

    reader->reader_state      = GNSS_UART_STATE_WAITING;
    reader->uart_handle       = uart_handle;
    reader->nmea_index        = 0;
    reader->nmea_buffer_index = false;

    reader->nmea_buffers[0] = calloc(NMEA_SENTENCE_MAX_LEN, sizeof(uint8_t));
    reader->nmea_buffers[1] = calloc(NMEA_SENTENCE_MAX_LEN, sizeof(uint8_t));

    if (reader->nmea_buffers[0] == NULL || reader->nmea_buffers[1] == NULL)
    {
        log_crit("Failed to allocate nmea buffers");
        fatal_err();
    }

    // Arm the GNSS UART interrupt
    if (HAL_UART_Receive_IT(uart_handle, &reader->read_byte, 1U) != HAL_OK)
    {
        log_crit("GNSS UART IT arm failed");
        fatal_err();
    }
}

void gnss_uart_reader_free(gnss_uart_reader_t *reader)
{
    free(reader->nmea_buffers[0]);
    free(reader->nmea_buffers[1]);
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
        if (new_byte  == NMEA_START_BYTE)
        {
            // Begin reading a new sentence
            memset(reader->nmea_buffers[reader->nmea_buffer_index], '\n', NMEA_SENTENCE_MAX_LEN);

            reader->nmea_index   = 0;
            reader->reader_state = GNSS_UART_STATE_READING;

            reader->nmea_buffers[reader->nmea_buffer_index][reader->nmea_index] = new_byte;
            return;
        }
        // Not currently reading anything -- in between sentences
        return;
    }

    // Read byte into current NMEA sentence
    if (++reader->nmea_index >= NMEA_SENTENCE_MAX_LEN)
    {
        log_warn("GNSS reader received sentence longer than max length");
    }
    else
    {
        reader->nmea_buffers[reader->nmea_buffer_index][reader->nmea_index] = new_byte;
    }

    if (new_byte == NMEA_END_BYTE)
    {
        reader->reader_state = GNSS_UART_STATE_WAITING;
        reader->nmea_buffer_index = !reader->nmea_buffer_index;
        task_manager_set_task(get_global_task_manager(), TASK_ID_GNSS_UPDATE); 
    } 

}

void gnss_uart_reader_get_sentence(gnss_uart_reader_t *reader, uint8_t *output)
{
    // CRITICAL SECTION -- IRQs can overwrite index and change sentence during cpy
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    memcpy(output, reader->nmea_buffers[!reader->nmea_buffer_index], NMEA_SENTENCE_MAX_LEN);

    __set_PRIMASK(primask);
    // END CRITICAL SECTION
}

