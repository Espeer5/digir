###################################################################################################
#  DIGIR MAKEFILE
###################################################################################################

# This file contains common recipes used in DigiR development

###################################################################################################
#  CHANGE LOG
###################################################################################################

# 7/23/26  Edward Speer  Initial revision

###################################################################################################
#  RECIPES
###################################################################################################

BUILD_DIR := build
CONFIGURE := cmake -S . -B $(BUILD_DIR)
BUILD     := cmake --build $(BUILD_DIR) 

.PHONY: all configure synth pnr bitstream program_fpga flash_fpga clean

all: bitstream

clean:
	rm -rf $(BUILD_DIR)

configure:
	$(CONFIGURE)

synth: configure
	$(BUILD) --target fpga_synth

pnr: configure
	$(BUILD) --target fpga_pnr

bitstream: configure
	$(BUILD) --target fpga_bitstream

program_fpga: configure
	$(BUILD) --target fpga_program

flash_fpga: configure
	$(BUILD) --target fpga_flash

