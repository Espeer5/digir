###################################################################################################
#  BASEBAND PROCESSING MODULE CMAKELISTS
###################################################################################################

# This file contains the build directives for the baseband processing module

###################################################################################################
#  CHANGE LOG
###################################################################################################

# 7/23/26  Edward Speer  Initial revision
# 7/27/26  Edward Speer  Add STM32 firmware build

###################################################################################################
#  BUILD DIRECTIVES
###################################################################################################

# All FPGA build logic is under this CMake project tree
add_subdirectory(fpga)

# The STM32 compilation needs its own compiler setup and is included as an external project
set(STM32_FW_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mcu/digir_bpm_fw)

set(STM32_FW_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/stm32_firmware)

set(STM32_TOOLCHAIN_FILE ${STM32_FW_SOURCE_DIR}/gcc-arm-none-eabi.cmake)

ExternalProject_Add(
    digir_bpm_firmware_external

    SOURCE_DIR
        "${STM32_FW_SOURCE_DIR}"

    BINARY_DIR
        "${STM32_FW_BINARY_DIR}"

    CMAKE_ARGS
        "-DCMAKE_TOOLCHAIN_FILE=${STM32_TOOLCHAIN_FILE}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"

    INSTALL_COMMAND
        ""

    USES_TERMINAL_CONFIGURE
        TRUE

    USES_TERMINAL_BUILD
        TRUE
)

add_custom_target(
    firmware
    DEPENDS digir_bpm_firmware_external
)

