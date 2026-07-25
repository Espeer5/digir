//#################################################################################################
//  DIGIR BPM TIMING CORE IP
//
//  VERSION 0.1
//#################################################################################################

// This module contains the core timing logic used to synchronize and distribute time in the DigiR
// baseband processing unit.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/24/26  Edward Speer  Initial revision
// 7/25/26  Edward Speer  Add servo and lock detector

//#################################################################################################
//  MODULE DEFINITION
//#################################################################################################

module timing_core #(
    parameter int unsigned REF_CLK_HZ = 100000000,
    parameter int unsigned SEC_WIDTH  = 48,
    parameter int unsigned FRAC_WIDTH = (REF_CLK_HZ > 1) ? $clog2(REF_CLK_HZ) : 1,
    parameter int unsigned DAC_WIDTH  = 12,
    parameter int unsigned ACC_WIDTH  = 48 // Width of servo internal accumulator
)(
    // EXTERNAL INPUTS
    input  logic                  reset,
    input  logic                  REF_CLK,
    input  logic                  gnss_pps,
    input  logic [DAC_WIDTH-1:0]  nominal_dac_code,
    input  logic [DAC_WIDTH-1:0]  max_dac_code,
    input  logic [DAC_WIDTH-1:0]  min_dac_code,

    // EXTERNAL OUTPUTS
    output logic                  tod_rollover,
    output logic                  lock,
    output logic [FRAC_WIDTH-1:0] filtered_abs_phase_err
);

    // Internal signals
    logic                  jam;
    logic [SEC_WIDTH-1:0]  jam_seconds;
    logic [SEC_WIDTH-1:0]  utc_seconds;
    logic [FRAC_WIDTH-1:0] fractional_ticks;
    logic                  time_valid;
    logic [FRAC_WIDTH-1:0] phase_err;
    logic                  phase_err_valid;
    logic [DAC_WIDTH-1:0]  control_code;
    logic                  control_code_valid;
    logic [ACC_WIDTH-1:0]  integrator_state;   // Exposed for tuning/debug
    logic                  servo_saturated;    // Exposed for tuning/debug

    // Wire up internal modules

    digir_tod #(
        .REF_CLK_HZ(REF_CLK_HZ),
        .SEC_WIDTH(SEC_WIDTH)
    ) tod_counter (
        .REF_CLK(REF_CLK),
        .reset(reset),
        .jam(jam),
        .jam_seconds(jam_seconds),
        .utc_seconds(utc_seconds),
        .fractional_ticks(fractional_ticks),
        .rollover(tod_rollover),
        .time_valid(time_valid)
    );

    phase_err_capture #(
        .REF_CLK_HZ(REF_CLK_HZ)
    ) phase_err_capture_inst (
        .reset(reset),
        .clk(REF_CLK),
        .gnss_pps(gnss_pps),
        .fractional_ticks(fractional_ticks),
        .phase_err(phase_err),
        .phase_err_valid(phase_err_valid)
    );

    servo #(
        .REF_CLK_HZ(REF_CLK_HZ)
    ) time_servo_inst (
        .reset(reset),
        .clk(REF_CLK),
        .phase_err(phase_err),
        .phase_err_valid(phase_err_valid),
        .nominal_dac_code(nominal_dac_code),
        .min_dac_code(min_dac_code),
        .max_dac_code(max_dac_code),
        .dac_code(control_code),
        .dac_code_valid(control_code_valid),
        .integrator_state(integrator_state),
        .output_saturated(servo_saturated)
    );

    lock_detector #(
        .REF_CLK_HZ(REF_CLK_HZ)
    ) lock_detector_inst (
        .reset(reset),
        .clk(REF_CLK),
        .phase_err(phase_err),
        .phase_err_valid(phase_err_valid),
        .time_valid(time_valid),
        .lock(lock),
        .filtered_abs_phase_err(filtered_abs_phase_err)
    );

endmodule

