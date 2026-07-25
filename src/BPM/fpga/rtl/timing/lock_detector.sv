//#################################################################################################
//  DIGIR BPM TIMING LOCK DETECTOR IP
//
//  VERSION 0.1
//#################################################################################################

// This module defines a statistical lock detector which evaluates whether or not the time servo is
// locked to the GNSS PPS. This lock status is the authoratative measure of whether or not the
// DigiR time base is tracking GNSS time sufficiently for complete transceiver operation.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/25/26  Edward Speer  Initial revision

//#################################################################################################
//  MODULE DEFINITION
//#################################################################################################

module lock_detector #(
    parameter int unsigned REF_CLK_HZ       = 100000000,
    parameter int unsigned PHASE_ERR_WIDTH  = (REF_CLK_HZ > 1) ? $clog2(REF_CLK_HZ) : 1,
    parameter int unsigned IIR_FILTER_SHIFT = 3,
    parameter int unsigned LOCK_THRESHOLD   = 10, // Expressed in phase error units
    parameter int unsigned LOCK_SAMPLES     = 10  // Consecutive samples under threshold for lock
)(
    input  logic                              reset,
    input  logic                              clk,
    input  logic signed [PHASE_ERR_WIDTH-1:0] phase_err,
    input  logic                              phase_err_valid,
    input  logic                              time_valid,

    output logic                              lock,
    output logic [PHASE_ERR_WIDTH-1:0]        filtered_abs_phase_err
);
    localparam int unsigned LOCK_COUNT_WIDTH = (LOCK_SAMPLES > 1) ? $clog2(LOCK_SAMPLES + 1) : 1;

    // Use extended signals internall to avoid overflow during subtraction
    logic [PHASE_ERR_WIDTH:0] abs_phase_err_extended;
    logic [PHASE_ERR_WIDTH:0] filtered_extended;

    logic signed [PHASE_ERR_WIDTH+1:0] filter_difference;
    logic signed [PHASE_ERR_WIDTH+1:0] filter_adjustment;
    logic signed [PHASE_ERR_WIDTH+1:0] filtered_next_signed;

    logic [LOCK_COUNT_WIDTH-1:0] lock_count;

    // Take the absolute value of the input phase error
    always_comb begin
        if (phase_err[PHASE_ERR_WIDTH-1]) begin
            abs_phase_err_extended = {1'b0, ~phase_error} + {{PHASE_ERR_WIDTH{1'b0}}, 1'b1};
        end
        else begin
            abs_phase_err_extended = {1'b0, phase_err};
        end
    end 

    // filtered_next = filtered + (abs_phase_err - filtered) / 2**FILTER_SHIFT
    always_comb begin
        filter_difference = $signed({1'b0, abs_phase_err_extended}) -
            $signed({1'b0, filtered_extended});

        filter_adjustment = filter_difference >>> FILTER_SHIFT;

        filtered_next_signed = $signed({1'b0, filtered_extended}) + filter_adjustment;
    end

    always_ff @(posedge clk) begin
        if (reset) begin
            filtered_extended <= '0;
            lock              <= 1'b0;
            lock_count        <= '0;
        end
        else begin
            // look-ahead -- use the new filtered value on each clock cycle rather than the prev
            if (filtered_next_signed < $signed(LOCK_THRESHOLD)) begin
                if (!locked) begin
                    if (lock_count == LOCK_SAMPLES - 1) begin
                        lock_count <= lock_count;
                        lock       <= time_valid;
                    end
                    else begin
                        lock_count <= lock_count + 1'b1;
                    end
                end
            end
            else begin
                lock_count <= '0;
                lock       <= 1'b0;
            end
        end
    end

    assign filtered_abs_phase_err = filtered_extended[PHAS_ERR_WIDTH-1:0];

endmodule

