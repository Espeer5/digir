###################################################################################################
#  DIGIR TOP-LEVEL CMAKELISTS
###################################################################################################

# This file contains the top-level build directive for the full DigiR project.

###################################################################################################
#  CHANGE LOG
###################################################################################################

# 7/23/26  Edward Speer  Initial Revision

###################################################################################################
#  BUILD DIRECTIVES
###################################################################################################

cmake_minimum_required(VERSION 3.24)

project(
    DigiR
    VERSION 0.1.0
    LANGUAGES NONE
)

# Load the local config for the host system
set(
    DIGIR_LOCAL_CONFIG
    "${CMAKE_SOURCE_DIR}/config/local.cmake"
    CACHE FILEPATH
    "Path to the local DigiR build configuration"
)

if(EXISTS "${DIGIR_LOCAL_CONFIG}")
    message(STATUS "Loading local configuration: ${DIGIR_LOCAL_CONFIG}")
    include("${DIGIR_LOCAL_CONFIG}")
else()
    message(STATUS "No local configuration found at ${DIGIR_LOCAL_CONFIG}")
endif()

# Build subsystems
add_subdirectory(src)


