//#################################################################################################
//  DIGIR BPM NMEA PARSER
//
//  VERSION 0.1
//#################################################################################################

// This module contains declarations for structures and functions used to parse gnss nmea sentences
// into time and location data.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/29/26  Edward Speer  Initial revision
// 7/30/26  Edward Speer  Internal functions are static

#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H

//#################################################################################################
//  INCLDUES
//#################################################################################################

#include <stdint.h>

//#################################################################################################
//  CONSTANTS
//#################################################################################################

#define NMEA_SENTENCE_MAX_LEN        82
#define NMEA_START_BYTE              0x24
#define NMEA_END_BYTE                0x0A
#define NMEA_TALKER_ID_LEN_BYTES     2
#define NMEA_SENTENCE_TYPE_LEN_BYTES 3
#define NMEA_DATA_MAX_LEN            (NMEA_SENTENCE_MAX_LEN - NMEA_TALKER_ID_LEN_BYTES - \
                                      NMEA_SENTENCE_TYPE_LEN_BYTES)

//#################################################################################################
//  STRUCTURES
//#################################################################################################

typedef enum 
{
    NMEA_TALKER_ID_UNKNOWN,
    NMEA_TALKER_ID_BEIDOU,
    NMEA_TALKER_ID_GALILEO,
    NMEA_TALKER_ID_NAVIC,
    NMEA_TALKER_ID_GLONASS,
    NMEA_TALKER_ID_GPS,
    NMEA_TALKER_ID_QZSS,
    NUM_NMEA_TALKER_IDS,
} NMEA_TALKER_ID_E;

typedef enum
{
    NMEA_SENTENCE_TYPE_UNKNOWN,
    NMEA_SENTENCE_TYPE_FIXED_DATA,
    NMEA_SENTENCE_TYPE_LAT_LON,
    NMEA_SENTENCE_TYPE_DOP,
    NMEA_SENTENCE_TYPE_SAT_VIEW,
    NMEA_SENTENCE_TYPE_MIN_GPS_DATA,
    NMEA_SENTENCE_TYPE_COURSE_SPEED,
    NUM_NMEA_SENTENCE_TYPES,
} NMEA_SENTENCE_TYPE_E;

typedef struct
{
    NMEA_TALKER_ID_E     talker_id;
    NMEA_SENTENCE_TYPE_E sentence_type;
    uint64_t             utc_second;
    int64_t              lat_deg_e7;
    int64_t              lon_deg_e7;
} nmea_sentence_t;


//#################################################################################################
//  FUNCTIONS
//#################################################################################################

nmea_sentence_t nmea_bytes_to_sentence(uint8_t bytes[NMEA_SENTENCE_MAX_LEN]);

#endif // #ifndef NMEA_PARSER_H

