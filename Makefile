###################################################################################################
#  DIGIR MAKEFILE
###################################################################################################

# This file contains common recipes used in DigiR development

###################################################################################################
#  CHANGE LOG
###################################################################################################

# 7/23/26  Edward Speer  Initial revision
# 7/24/26  Edward Speer  Add test recipe
# 7/27/26  Edward Speer  Add firmware recipe

###################################################################################################
#  RECIPES
###################################################################################################

BUILD_DIR := build
CONFIGURE := cmake -S . -B $(BUILD_DIR)
BUILD     := cmake --build $(BUILD_DIR) 

.PHONY: all configure synth pnr bitstream program_fpga flash_fpga clean

all: bitstream firmware

clean:
	rm -rf $(BUILD_DIR)
	rm -rf src/BPM/mcu/digir_bpm_fw/CM4/build
	rm -rf src/BPM/mcu/digir_bpm_fw/CM7/build

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

test: configure
	$(BUILD) --target fpga_test

firmware: configure
	$(BUILD) --target firmware

