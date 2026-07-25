//#################################################################################################
//  DIGIR BPM FPGA TOD IP
//
//  VERSION 0.1
//#################################################################################################

// This module defines the ToD IP, consisting of a clocked counter at the precision of the
// reference oscillator which is aligned to UTC time based on the GNSS counter. The ToD counter
// support jamming to UTC second at startup, and direct reading to enable phase error computation
// with the GNSS PPS.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/24/26  Edward Speer  Initial Revision

//#################################################################################################
//  MODULE DEFINITION
//#################################################################################################

module digir_tod #(
    parameter int unsigned REF_CLK_HZ = 100000000,
    parameter int unsigned SEC_WIDTH  = 48,
    parameter int unsigned FRAC_WIDTH = (REF_CLK_HZ > 1) ? $clog2(REF_CLK_HZ) : 1
)(
    // EXTERNAL INPUTS
    input  logic                  REF_CLK,          // Reference OCXO
    input  logic                  reset,            // Reset counter
    
    // JAM INTERFACE
    input  logic                  jam,              // Jam ToD counter if high
    input  logic [SEC_WIDTH-1:0]  jam_seconds,      // Seconds value to jam counter to

    // TOD
    output logic [SEC_WIDTH-1:0]  utc_seconds,      // Whole UTC seconds count
    output logic [FRAC_WIDTH-1:0] fractional_ticks, // Clocked sub-second counter
    output logic                  rollover,         // High at ToD 1 second rollover
    output logic                  time_valid        // ToD aligned to utc?
);

    // SEQUENTIAL LOGIC
    always_ff @(posedge REF_CLK) begin

        // The rollover should be held low unless fractional counter wraps
        rollover <= 1'b0;

        if (reset) begin
            // The reset line holds the counter at 0
            utc_seconds      <= '0;
            fractional_ticks <= '0;
            time_valid       <= 1'b0;
        end
        else if (jam) begin
            // If we're told to jam, we need to write the specified utc_second to the seconds
            // counter and phase reset the fractional counter to 0. We consider the UTC time to
            // be valid once a jam has been completed.
            utc_seconds      <= jam_seconds;
            fractional_ticks <= '0;
            time_valid       <= 1'b1;
        end
        else begin
            // In normal operation, increment the fractional counter on each clock edge, until a
            // second elapses, then increment the seconds counter, reset the fractional counter,
            // and assert the rollover line.
            if (fractional_ticks == REF_CLK_HZ - 1) begin
                utc_seconds      <= utc_seconds + 1'b1;
                fractional_ticks <= '0;
                rollover         <= 1'b1;
            end
            else begin
                fractional_ticks <= fractional_ticks + 1'b1;
            end
        end
    end

endmodule


