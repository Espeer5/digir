//#################################################################################################
//  DIGIR BPM PHASE ERROR CAPTURE IP TESTBENCH
//
//  VERSION 0.1
//#################################################################################################

// This file contains a test bench which verifies that the phase err measurement module produces
// the expected output under different input conditions.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/24/26  Edward Speer  Initial revision

//#################################################################################################
//  TESTBENCH
//#################################################################################################

// Slow time by 1E3
`timescale 1ns/1ps

module phase_err_capture_tb;

    // ### PARAMETERS ###
    localparam int unsigned REF_CLK_HZ = 100;
    localparam int unsigned FRAC_WIDTH = $clog2(REF_CLK_HZ);

    localparam time CLK_PERIOD     = 10ns;
    localparam int  NUM_TEST_CASES = 9;

    // ### DUT CONNECTIONS ###

    logic                         reset;
    logic                         clk;
    logic                         gnss_pps;
    logic [FRAC_WIDTH-1:0]        fractional_ticks;
    logic signed [FRAC_WIDTH-1:0] phase_err;
    logic                         phase_err_valid;


    // ### DUT ###

    phase_err_capture #(
        .REF_CLK_HZ(REF_CLK_HZ)
    ) dut (
        .reset(reset),
        .clk(clk),
        .gnss_pps(gnss_pps),
        .fractional_ticks(fractional_ticks),
        .phase_err(phase_err),
        .phase_err_valid(phase_err_valid)
    );


    // ### TEST CASE ###

    integer test_ticks      [0:NUM_TEST_CASES-1];
    integer expected_error  [0:NUM_TEST_CASES-1];


    // ### CLOCK GEN ###

    always #(CLK_PERIOD / 2) clk = ~clk;


    // ### REPEATED TASKS ###

    task automatic apply_reset;
        reset            = 1'b1;
        gnss_pps         = 1'b0;
        fractional_ticks = '0;

        repeat (4) @(posedge clk);

        reset = 1'b0;
        repeat (2) @(posedge clk);
    endtask

    task automatic pulse_pps;
        // Keep the PPS asserted long enough to be picked up by the 2-flop sync mechanism
        @(negedge clk);
        gnss_pps = 1'b1;
 
        repeat (2) @(negedge clk);

        gnss_pps = 1'b0;
    endtask

    task automatic run_test_case(
        input int unsigned test_number,
        input integer      ticks,
        input integer      expected
    );
        int unsigned timeout_cycles;
        fractional_ticks = ticks;

        pulse_pps();

        // Wait for the DUT to report a valid phase error measurement with a timeout
        timeout_cycles = 0;
        while (!phase_err_valid && timeout_cycles < 10) begin
            @(posedge clk);
            timeout_cycles++;
        end

        if (!phase_err_valid) begin
            $error(
                "Test %0d timed out: fractional_ticks=%0d",
                test_number,
                ticks
            );
            return;
        end

        if ($signed(phase_err) !== expected) begin
            $error(
                "Test %0d failed: fractional ticks=%0d expected=%0d actual=%0d",
                test_number,
                ticks,
                expected,
                $signed(phase_err)
            );
        end

        // Ensure phase_err_valid assertion returns low for the next clock cycle
        @(posedge clk);

        if (phase_err_valid) begin
            $error(
                "Test %0d failed: phase_err_valid remained asserted",
                test_number
            );
        end

        repeat (2) @(posedge clk);
    endtask


    // ### RUN TESTS ###

    initial begin
        clk              = 1'b0;
        reset            = 1'b0;
        gnss_pps         = 1'b0;
        fractional_ticks = '0;

        test_ticks[0]     = 0;
        expected_error[0] = 0;

        test_ticks[1]     = 1;
        expected_error[1] = 1;

        test_ticks[2]     = 10;
        expected_error[2] = 10;

        test_ticks[3]     = 49;
        expected_error[3] = 49;

        test_ticks[4]     = 50;
        expected_error[4] = -50;

        test_ticks[5]     = 51;
        expected_error[5] = -49;

        test_ticks[6]     = 75;
        expected_error[6] = -25;

        test_ticks[7]     = 98;
        expected_error[7] = -2;

        test_ticks[8]     = 99;
        expected_error[8] = -1;

        apply_reset();

        for (int unsigned i = 0; i < NUM_TEST_CASES; i++) begin
            run_test_case(i, test_ticks[i], expected_error[i]);
        end

        $finish;
    end

endmodule

