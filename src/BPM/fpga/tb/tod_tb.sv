//#################################################################################################
//  DIGIR BPM TOD IP TESTBENCH
//
//  VERSION 0.1
//#################################################################################################

// This testbench verifies the ToD counter IP for the DigiR BPM FPGA deisgn

//#################################################################################################
// CHANGE LOG
//#################################################################################################

// 7/24/26  Edward Speer  Initial revision

//#################################################################################################
//  TESTBENCH
//#################################################################################################

// Slow time by 1E3
`timescale 1ns / 1ps

module tod_tb;

// ### PARAMTERS ###

localparam int REF_CLK_HZ = 100;
localparam int SEC_WIDTH  = 48;

localparam logic [SEC_WIDTH-1:0] TEST_UTC_SECONDS = 48'd123456;

// ### DUT CONNECTIONS ###

logic clk;
logic reset;

logic jam;
logic [SEC_WIDTH-1:0] jam_seconds;

logic [SEC_WIDTH-1:0] utc_seconds;
logic [$clog2(REF_CLK_HZ)-1:0] fractional_ticks;
logic rollover;
logic time_valid;


// ### DUT ###

digir_tod #(
    .REF_CLK_HZ(REF_CLK_HZ),
    .SEC_WIDTH(SEC_WIDTH)
) dut (
    .REF_CLK(clk),
    .reset(reset),

    .jam(jam),
    .jam_seconds(jam_seconds),

    .utc_seconds(utc_seconds),
    .fractional_ticks(fractional_ticks),
    .rollover(rollover),
    .time_valid(time_valid)
);


// ### CLOCK GEN ###

initial begin
    clk = 0;

    forever begin
        #5 clk = ~clk;
    end
end

// ### TEST PROCEDURE ###

initial begin : test_sequence

    int unsigned expected_fraction;

    // Begin holding the DUT in reset and verify outputs hold at 0
    reset       = 1;
    jam         = 0;
    jam_seconds = 0;
    
    repeat (54) begin
        @(negedge clk);

        assert (utc_seconds === '0)
            else $fatal(
                1,
                "RESET TEST FAILED: utc_seconds incremented during reset",
                utc_seconds
            );

        assert (fractional_ticks === '0)
            else $fatal(
                1,
                "RESET TEST FAILED: fractional_ticks incremented during reset",
                fractional_ticks
            );

        assert (time_valid === 1'b0)
            else $fatal(
                1,
                "RESET TEST FAILED: time_valid must be cleared during reset"
            );

        assert (rollover === 1'b0)
            else $fatal(
                1,
                "RESET TEST FAILED: rollover must be cleared during reset"
            );
    end

    $display("PASS: RESET TEST");

    // Enable the counter
    @(negedge clk);
    reset = 0;

    // Perform a UTC jam and validate outputs
    @(posedge clk);

    jam         = 1;
    jam_seconds = TEST_UTC_SECONDS;

    @(negedge clk);
    
    assert (utc_seconds === TEST_UTC_SECONDS)
        else $fatal(
            1,
            "JAM TEST FAILED: expected seconds=%0d actual=%0d",
            TEST_UTC_SECONDS,
            utc_seconds
        );

    assert (fractional_ticks === '0)
        else $fatal(
            1,
            "JAM TEST FAILED: No phase reset on jam"
        );

    assert (time_valid === 1'b1)
        else $fatal(
            1,
            "JAM TEST FAILED: time_valid not asserted on jam"
        );

    assert (rollover === 1'b0)
        else $fatal(
            1,
            "JAM TEST FAILED: rollover asserted on jam"
        );

    $display("PASS: JAM TEST");

    // jam complete, enable normal operation
    jam = 0;

    // Run the counter and verify fractional portion increments properly
    for (
        expected_fraction = 1;
        expected_fraction < REF_CLK_HZ;
        expected_fraction++
    ) begin
        @(negedge clk)

        assert (fractional_ticks === expected_fraction)
            else $fatal(
                1,
                "FRACTION TEST FAILED: expected_fraction not incremented on REF_CLK"
            );

        assert (utc_seconds === TEST_UTC_SECONDS)
            else $fatal(
                1,
                "FRACTION TEST FAILED: utc_seconds changed mid-epoch"
            );

        assert (rollover === 1'b0)
            else $fatal(
                1,
                "FRACTION TEST FAILED: rollover asserted mid-epoch"
            );
    end

    $display("PASS: FRACTION TEST");

    // Execute and validate rollover behavior
    @(negedge clk);

    assert (fractional_ticks === '0)
        else $fatal(
            1,
            "ROLLOVER TEST FAILED: fractional_ticks did not reset"
        );

    assert (utc_seconds === TEST_UTC_SECONDS + 1'b1)
        else $fatal(
            1,
            "ROLLOVER TEST FAILED: utc_seconds did not increments"
        );

    assert (rollover === 1'b1)
        else $fatal(
            1,
            "ROLLOVER TEST FAILED: rollover not asserted"
        );

    @(negedge clk);

    assert (fractional_ticks === 1'b1)
        else $fatal(
            1,
            "ROLLOVER TEST FAILED: fractional_ticks not incremented after rollover"
        );

    assert (utc_seconds === TEST_UTC_SECONDS + 1'b1)
        else $fatal(
            1,
            "ROLLOVER TEST FAILED: utc_seconds incremented unexpectedly"
        );

    assert (rollover === 1'b0)
        else $fatal(
            1,
            "ROLLOVER TEST FAILED: rollover asserted for multiple clock cycles"
        );

    $display("PASS: ROLLOVER TEST");

    $finish;

end

// Prevent test bench from running forever
initial begin : simulation_timout
    #10us;

    $fatal(
        1,
        "TESTBENCH TIMEOUT: simulation did not complete"
    );
end

endmodule

