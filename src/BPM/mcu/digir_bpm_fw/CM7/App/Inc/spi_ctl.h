//#################################################################################################
//  DIGIR BPM MCU SPI CONTROLLER
//
//  VERSION 0.1
//#################################################################################################

// This module declares structures and functions for SPI control logic for any SPI peripheral.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 08/10/26  Edward Speer  Initial revision

#ifndef SPI_CTL_H
#define SPI_CTL_H

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include "stm32h7xx_hal.h"
#include "system_control.h"
#include "log.h"

//#################################################################################################
//  STRUCTURES
//#################################################################################################

typedef struct {
    SPI_HandleTypeDef *spi_handle;
    GPIO_TypeDef      *cs_handle;
    uint16_t          cs_pin;
} spi_controller_t;

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

// Initialize a SPI controller with the specified hardware characteristics
void spi_controller_init(spi_controller_t *controller, SPI_HandleTypeDef *spi_handle,
                         GPIO_TypeDef *cs_handle, uint16_t cs_pin);

// Write num_bytes out over the SPI
void spi_controller_tx(spi_controller_t *controller, size_t len, uint8_t *bytes);

#endif // #ifndef SPI_CTL_H

