//#################################################################################################
//  DIGIR BPM TIMING SERVO IP
//
//  VERSION 0.1
//#################################################################################################

// This module defines the controller which takes in the fractional phase error of the reference
// clock against the GPNSS PPS, and generates an output control signal to discipline the VCTCXO.
// The control signal must be output at the correct bit width for the BPM internal DAC.

// For the loop constants, we currently use a power of two model to allow shifting instead of
// hardware multiplier usage. Changing to multiplication could enable additional precision, but
// multipliers are currently being reserved for DSP usage. If the performance proves to be too poor
// on the hardware, we can reconsider.

// Note that the sign convention for the phase error and control signal may need to be adjusted for
// the real oscillator. The phase error is defined as oscillator phase - GNSS phase, so if the clk
// is fast, we will see a positive sign error. Currently, a positive sign error produces a negative
// control signal. A parameter for this polarity is included, called POLARITY. It is defined so
// that if high, the control signal will have the opposite sign of the phase error.

// When tuning, note the units at play here. Phase error is expressed in ticks of the oscillator.
// The control output units depend on characteristics of both the DAC and the oscillator. The
// unit of the proportional control constant is DAC codes / clk tick, and the unit of the integral
// constant is thus DAC codes / clk tick / second. The shift parameters in the module allow tuning
// of these constants to produce appropriate voltage per error level dependent on the
// characteristics of the hardware. For bench testing it may later become useful to be able to
// write in different control constants from the MCU, so the shifts may later become input signals.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/25/26  Edward Speer  Initial revision

//#################################################################################################
//  MODULE DEFINITION
//#################################################################################################

module servo #(
    parameter int unsigned REF_CLK_HZ  = 100000000,
    parameter int unsigned PHASE_WIDTH = (REF_CLK_HZ > 1) ? $clog2(REF_CLK_HZ) : 1,
    parameter int unsigned DAC_WIDTH   = 12,
    parameter int unsigned ACC_WIDTH   = 48, // Width of internal accumulator
    parameter int unsigned FRAC_WIDTH  = 16, // Width of fixed point fractional part
    parameter int unsigned KP_SHIFT    = 8,
    parameter int unsigned KI_SHIFT    = 16,
    parameter int unsigned POLARITY    = 1
)(
    input  logic                          reset,
    input  logic                          clk,

    input  logic signed [PHASE_WIDTH-1:0] phase_err,        // From phase err capture block
    input  logic                          phase_err_valid,

    input  logic [DAC_WIDTH-1:0]          nominal_dac_code, // DAC code at center of tuning range
    input  logic [DAC_WIDTH-1:0]          min_dac_code,     // Minimum allowable control output
    input  logic [DAC_WIDTH-1:0]          max_dac_code,     // Maximum allowable control output

    output logic [DAC_WIDTH-1:0]          dac_code,         // Output control signal
    output logic                          dac_code_valid,

    output logic [ACC_WIDTH-1:0]          integrator_state, // Exposed for tuning and debug
    output logic                          output_saturated  // Desired control is outside bounds
);
    logic [ACC_WIDTH-1:0] phase_err_extended;
    logic [ACC_WIDTH-1:0] phase_err_scaled;
    logic [ACC_WIDTH-1:0] p_term;
    logic [ACC_WIDTH-1:0] i_term;
    logic [ACC_WIDTH-1:0] dac_nominal_scaled;
    logic [ACC_WIDTH-1:0] dac_min_scaled;
    logic [ACC_WIDTH-1:0] dac_max_scaled;
    logic [ACC_WIDTH-1:0] integrator_proposed;
    logic [ACC_WIDTH-1:0] output_proposed;
    logic [ACC_WIDTH-1:0] integrator_selected;
    logic [ACC_WIDTH-1:0] output_selected;
    logic [ACC_WIDTH-1:0] output_limited;

    logic integrator_update_allowed; // Do not update integrator state when output saturated

    always_comb begin
        // Default control state
        output_saturated          = 1'b0;
        integrator_update_allowed = 1'b1;

        // Convert input phase error into internal fixed point representation
        phase_err_extended = {{(ACC_WIDTH-PHASE_WIDTH){phase_err[PHASE_WIDTH-1]}}, phase_err};
        phase_err_scaled   = phase_err_extended <<< FRAC_WIDTH;

        // Compute PI terms
        p_term = phase_err_scaled >>> KP_SHIFT;
        i_term = phase_err_scaled >>> KI_SHIFT;

        // Express DAC parameters in internal fixed point representation
        dac_nominal_scaled = $signed({1'b0, nominal_dac_code}) <<< FRAC_WIDTH;
        dac_min_scaled     = $signed({1'b0, min_dac_code}) <<< FRAC_WIDTH;
        dac_max_scaled     = $signed({1'b0, max_dac_code}) <<< FRAC_WIDTH;

        // Compute the proposed new integrator and control output
        integrator_proposed = i_term + integrator_state;
        if (POLARITY) begin
            output_proposed = nominal_scaled - integrator_proposed - p_term;
        end
        else begin
            output_proposed = nominal_scaled + integrator_proposed + p_term;
        end

        // Prevent integrator windup by capping at saturation
        if ((output_proposed > dac_max_scaled) && (i_term > 0)) begin
            integrator_update_allowed = 1'b0;
        end
        if ((output_proposed < dac_min_scaled) && (i_term < 0)) begin
            integrator_update_allowed = 1'b0;
        end
        if (integrator_update_allowed) begin
            integrator_selected = integrator_proposed;
        end
        else begin
            integrator_selected = integrator_state;
        end

        // Recalculate the output using the selected integrator
        if (POLARITY) begin
            output_selected = nominal_scaled - integrator_selected - p_term;
        end
        else begin
            output_selected = nominal_scaled + integrator_selected + p_term;
        end

        // Saturate control at DAC limits
        if (output_selected > dac_max_scaled) begin
            output_limited   = dac_max_scaled;
            output_saturated = 1'b1;
        end
        if (output_selected < dac_min_scaled) begin
            output_limited = dac_min_scaled;
            output_saturated = 1'b1;
        end
        else begin
            output_limited = output_selected;
        end
    end

    always_ff @(posedge clk) begin
        if (reset) begin
            integrator_state <= '0;
            dac_code         <= nominal_dac_code;
            dac_code_valid   <= 1'b0;
        end
        else begin
            // DAC code output is updated only after a fresh phase err measurement is produced
            dac_code_valid <= 1'b0;

            if (phase_err_valid) begin
                dac_code_valid   <= 1'b1;
                integrator_state <= integrator_selected;
                dac_code         <= output_limited >>> FRAC_WIDTH;
            end
        end
    end

endmodule

