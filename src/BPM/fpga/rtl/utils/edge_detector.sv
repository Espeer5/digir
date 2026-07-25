//#################################################################################################
//  DIGIR BPM FPGA EDGE DETECTOR IP
//
//  VERSION 0.1
//#################################################################################################

// This module defines a reusable edge detector on asynchronous signals defined as as a two-flop
// synchronizer followed by a simple edged detector. 

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/24/26  Edward Speer  Initial revision

//#################################################################################################
//  MODULE DEFINITION
//#################################################################################################

module edge_detector (
    input  logic reset,
    input  logic clk,
    input  logic async_signal, // Signal not in clock time base 

    output logic rising_edge  // Asserted if rising edge of input signal detected
);

    logic in_sample;
    logic in_sample_sync;
    logic cache_sample;

    always_ff @(posedge clk) begin
        if (reset) begin
            in_sample      <= 1'b0;
            in_sample_sync <= 1'b0;
            cache_sample   <= 1'b0;
        end
        else begin
            // On each clock edge, latch in the value of the async signal through two flip-flops to
            // sync it to the clock's timebase
            in_sample      <= async_signal;
            in_sample_sync <= in_sample;
            cache_sample   <= in_sample_sync;
        end
    end

    assign rising_edge = in_sample_sync && !cache_sample;

endmodule

