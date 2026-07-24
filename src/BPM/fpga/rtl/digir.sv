//#################################################################################################
//  DIGIR FPGA TOP-LEVEL MODULE
//
//  VERSION 0.1 
//#################################################################################################

// This file defines the top-level module for the FPGA in the DigiR baseband processing module.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/23/26  Edward Speer  Initial Revision

//#################################################################################################
//  MODULE DEFINITION
//#################################################################################################

module digir_bpm_top #(
    parameter int unsigned SAMPLE_WIDTH = 16,
) (
    // RESET LINES
    input  logic                    reset,        // global reset

    // CLOCK TREE
    input  logic                    pps,          // GNSS PPS
    input  logic                    ref_clk,      // reference OCXO

    // MCU SPI INTERFACE
    input  logic                    mcu_spi_sclk, // SPI serial clock
    input  logic                    mcu_spi_cs,   // SPI chip select
    input  logic                    mcu_spi_mosi, // SPI MOSI
    input  logic                    mcu_spi_miso, // SPI MISO

    // MCU IRQ line
    output logic                    mcu_irq,      // MCU IRQ 

    // RF SAMPLE INTERFACE
    output logic                    sample_clk,   // RF sample clock
    output logic [SAMPLE_WIDTH-1:0] sample_out,   // RF sample output
    output logic                    out_valid,    // RF sample output valid?
);

endmodule

