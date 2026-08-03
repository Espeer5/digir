//#################################################################################################
//  DIGIR BPM MCU GLOBAL STATE CACHE
//
//  VERSION 0.1
//#################################################################################################

// This module defines the global state cache which collects all data needed for decision making
// and mode execution for the DigiR BPM firmware.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 8/2/26  Edward Speer  Initial revision

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <stdbool.h>

#include "global_state.h"

//#################################################################################################
//  CONSTANTS
//#################################################################################################

global_state_t global_state;

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

global_state_t *get_global_state()
{
    return &global_state;
}

static void init_gnss_state(global_state_t *global_state)
{
    global_state->gnss_state.unix_ts       = 0;
    global_state->gnss_state.unix_ts_valid = false;
    global_state->gnss_state.lat_deg_e7    = 0;
    global_state->gnss_state.lon_deg_e7    = 0;
    global_state->gnss_state.loc_valid     = false;
}

void global_state_init(global_state_t *state)
{
    global_state_t *global_state = get_global_state();
    init_gnss_state(global_state);
}

void update_global_gnss_state(global_state_t *global_state, nmea_sentence_t sentence)
{
    if (sentence.ts_valid)
    {
        global_state->gnss_state.unix_ts = sentence.unix_second;
        global_state->gnss_state.unix_ts_valid = true;
    }
    else
    {
        // unix TS aged out -- should be applied before this update is processed
        global_state->gnss_state.unix_ts_valid = false;
    }

    // Note: unlike timestamp, if loc invalid, we assume the previous loc still applies
    if (sentence.loc_valid)
    {
        global_state->gnss_state.lat_deg_e7 = sentence.lat_deg_e7;
        global_state->gnss_state.lon_deg_e7 = sentence.lon_deg_e7;
        global_state->gnss_state.loc_valid  = true;
    }
}

