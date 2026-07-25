//#################################################################################################
//  DIGIR BPM PHASE ERR CAPTURE IP
//
//  VERSION 0.1
//#################################################################################################

// This module is used to capture the phase error of the GNSS PPS with the internal ToD counter.
// This phase error is used to measure the drift of the oscillator and generate a VCTCXO control
// signal.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/24/26  Edward Speer  Initial revision

//#################################################################################################
//  MODULE DEFINITION
//#################################################################################################

module phase_err_capture #(
    parameter int unsigned REF_CLK_HZ = 100000000,
    parameter int unsigned FRAC_WIDTH = (REF_CLK_HZ > 1) ? $clog2(REF_CLK_HZ) : 1
)(
    input  logic                         reset,
    input  logic                         clk,

    input  logic                         gnss_pps,
    input  logic [FRAC_WIDTH-1:0]        fractional_ticks,

    output logic signed [FRAC_WIDTH-1:0] phase_err,
    output logic                         phase_err_valid
);

    logic                       pps_rising_edge;
    logic signed [FRAC_WIDTH:0] phase_err_extended; // One extra bit to capture overflow

    // Synchronize and detect the rising edge of the PPS using an edge detector
    edge_detector pps_edge_detector (
        .reset(reset),
        .clk(clk),
        .async_signal(gnss_pps),
        .rising_edge(pps_rising_edge)
    );

    always_comb begin
        if (fractional_ticks < REF_CLK_HZ / 2) begin
            // If err is less than 500ms, assume it's positive (ToD > 1PPS)
            phase_err_extended = $signed({1'b0, fractional_ticks});
        end
        else begin
            // If err is greater than 500ms, assume it's negative (ToD < 1PPS)
            phase_err_extended = $signed({1'b0, fractional_ticks}) - REF_CLK_HZ;
        end
    end

    always_ff @(posedge clk) begin
        if (reset) begin
            phase_err_valid <= 1'b0;
            phase_err       <= '0;
        end
        else begin
            // If we aren't on a PPS rising edge, the err sample is not valid
            phase_err_valid <= 1'b0;

            // On the PPS rising edge, latch the captured phase error into the output
            if (pps_rising_edge) begin
                phase_err_valid <= 1'b1;
                phase_err       <= phase_err_extended[FRAC_WIDTH-1:0];
            end
        end
    end

endmodule

