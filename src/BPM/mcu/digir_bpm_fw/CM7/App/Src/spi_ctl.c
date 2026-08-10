//#################################################################################################
//  DIGIR BPM MCU SPI CONTROLLER
//
//  VERSION 0.1
//#################################################################################################

// This module defines functions for SPI control for any SPI peripheral

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 8/10/26  Edward Speer  Initial revision

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include "spi_ctl.h"

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

void spi_controller_init(spi_controller_t *controller, SPI_HandleTypeDef *spi_handle,               
                         GPIO_TypeDef *cs_handle, uint16_t cs_pin)                                 
{                                                                                                   
    if (controller == NULL || spi_handle == NULL || cs_handle == NULL)                              
    {                                                                                               
        log_crit("NULL pointer in SPI controller initialization");                                  
        fatal_err();                                                                                
    }                                                                                               
                                                                                                    
    controller->spi_handle = spi_handle;                                                            
    controller->cs_handle  = cs_handle;                                                             
    controller->cs_pin     = cs_pin;                                                                
}

void spi_controller_tx(spi_controller_t *controller, size_t len, uint8_t *bytes)
{
    if (controller == NULL)
    {
        log_crit("SPI controller NULL in TX");
        fatal_err();
    }

    HAL_GPIO_WritePin(controller->cs_handle, controller->cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(controller->spi_handle, bytes, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(controller->cs_handle, controller->cs_pin, GPIO_PIN_SET);
}


