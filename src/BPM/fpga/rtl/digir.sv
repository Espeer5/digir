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
// 7/25/26  Edward Speer  Add timing core instantiation

//#################################################################################################
//  MODULE DEFINITION
//#################################################################################################

module digir_bpm_top #(
    parameter int unsigned SAMPLE_WIDTH       = 16,
    parameter int unsigned REF_CLK_HZ         = 100000000,
    parameter int unsigned PHASE_ERR_WIDTH    = (REF_CLK_HZ > 1) ? $clog2(REF_CLK_HZ) : 1,
    parameter int unsigned INTERNAL_DAC_WIDTH = 12
)(
    // RESET LINES
    input  logic                       reset,                  // global reset

    // CLOCK TREE
    input  logic                       pps,                    // GNSS PPS
    input  logic                       ref_clk,                // reference OCXO
    output logic                       lock,                   // Time locked to gnss?
    output logic [PHASE_ERR_WIDTH-1:0] filtered_abs_phase_err, // Clock sync quality metric


    // MCU SPI INTERFACE
    input  logic                       mcu_spi_sclk,           // SPI serial clock
    input  logic                       mcu_spi_cs,             // SPI chip select
    input  logic                       mcu_spi_mosi,           // SPI MOSI
    input  logic                       mcu_spi_miso,           // SPI MISO

    // MCU IRQ lines
    output logic                       mcu_irq,                // MCU IRQ 

    // RF SAMPLE INTERFACE
    output logic                       sample_clk,             // RF sample clock
    output logic [SAMPLE_WIDTH-1:0]    sample_out,             // RF sample output
    output logic                       out_valid               // RF sample output valid?
);
    // Internal signals
    logic [INTERNAL_DAC_WIDTH-1:0] nominal_dac_code;
    logic [INTERNAL_DAC_WIDTH-1:0] max_dac_code;
    logic [INTERNAL_DAC_WIDTH-1:0] min_dac_code;

    // Wire up internal modules

    timing_core #(
        .REF_CLK_HZ(REF_CLK_HZ)
    ) timing_core_inst (
        .reset(reset),
        .REF_CLK(ref_clk),
        .gnss_pps(pps),
        .nominal_dac_code(nominal_dac_code),
        .min_dac_code(min_dac_code),
        .max_dac_code(max_dac_code),
        .tod_rollover(mcu_irq),
        .lock(lock),
        .filtered_abs_phase_err(filtered_abs_phase_err)
    );

endmodule

