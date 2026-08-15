//#################################################################################################
//  DIGIR BPM SPI IP
//
//  VERSION 0.1
//#################################################################################################

// This module defines a general purpose SPI peripheral used to communicate with SPI peripherals

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 8/12/26  Edward Speer  Initial revision

//#################################################################################################
//  MODULE DEFINITIONS
//#################################################################################################

module spi_slave_phy (
    input  logic       reset,

    // 4 WIRE SPI
    input  logic       spi_sclk,
    input  logic       spi_cs_n,
    input  logic       spi_mosi,
    output logic       spi_miso,

    // INTERNAL OUTPUTS
    output logic [7:0] rx_byte,
    output logic       rx_valid,

    // INTERNAL INPUS
    input  logic [7:0] tx_byte,
);

    logic [7:0] rx_shift;
    logic [7:0] tx_shift;
    logic [2:0] bit_count;

    // READING: MOSI sampled on rising edge of SPI clk.
    always_ff @(posedge spi_sclk or posedge spi_cs_n) begin
        if (spi_cs_n) begin
            rx_shift  <= '0;
            bit_count <= '0;
            rx_valid  <= 1'b0;
        end
        else begin
            rx_shift <= {rx_shift[6:0], spi_mosi};
            rx_valid <= 1'b0;

            if (bit_count == 3'd7) begin
                rx_byte  <= {rx_shift[6:0], spi_mosi};
                rx_valid <= 1'b1;
                bit_count <= 3'd0;
            end
            else begin
                bit_count <= bit_count + 1'b1;
            end
        end
    end

    // WRITING: MISO updated at falling edge of SPI clk
    always_ff @(negedge spi_sclk or posedge spi_cs_n) begin
        if (spi_cs_n) begin
            tx_shift <= tx_byte;
            spi_miso <= 1'b0;
        end
        else begin
            spi_miso <= tx_shift[7];
            tx_shift <= {tx_shift[6:0], 1'b0};
        end
    end

endmodule

