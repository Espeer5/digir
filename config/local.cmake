###################################################################################################
#  DIGIR LOCAL CMAKE CONFIGURATION
###################################################################################################

# This file defines site-specific variables requiring configuration for the host system.

# Path to OSS-CAD installation on the host machine
set(
    OSS_CAD_ROOT
    "/opt/oss-cad-suite"
    CACHE PATH
    "Path to the OSS CAD Suite installation"
)

### FPGA SPECS ###

# ECP5 device identifier
set(FPGA_DEVICE 85k)

# ECP5 package
set(FPGA_PACKAGE CABGA381)

# ECP5 speed grade
set(FPGA_SPEED_GRADE 8)

