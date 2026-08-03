//#################################################################################################
//  DIGIR BPM MCU GLOBAL STATE CACHE
//
//  VERSION 0.1
//#################################################################################################

// This module declares the global state cache which collects all data needed for decision making
// and mode execution for the DigiR BPM firmware.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 8/2/26  Edward Speer  Initial revision

#ifndef GLOBAL_STATE_H
#define GLOBAL_STATE_H 

//#################################################################################################
//  INCLUDES 
//#################################################################################################

#include "nmea_parser.h"

//#################################################################################################
//  STRUCTURES
//#################################################################################################

// Global GNSS state including time and location
typedef struct {
    int64_t unix_ts;
    bool    unix_ts_valid;
    int64_t lat_deg_e7;
    int64_t lon_deg_e7;
    bool    loc_valid;
} global_gnss_state_t;

// Global state container
typedef struct {
    global_gnss_state_t gnss_state;
} global_state_t;

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

// Get a pointer to the global state cache
global_state_t *get_global_state(void);

// Initialize the global state cache
void global_state_init(global_state_t *state);

// Update the global GNSS state
void update_global_gnss_state(global_state_t *global_state, nmea_sentence_t sentence);

#endif // #ifndef GLOBAL_STATE_H

