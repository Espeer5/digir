//#################################################################################################
//  DIGIR BPM NMEA PARSER
//
//  VERSION 0.1
//#################################################################################################

// This module contains definitions for functions used to parse gnss nmea sentences into time and
// location data.

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 7/29/26  Edward Speer  Initial revision
// 7/30/26  Edward Speer  Add main parse sentence function

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <string.h>

#include "nmea_parser.h"

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

static NMEA_TALKER_ID_E talker_id_bytes_to_talker_id(uint8_t *bytes)
{
    if (memcmp(bytes, "BD", NMEA_TALKER_ID_LEN_BYTES) == 0 ||
        memcmp(bytes, "GB", NMEA_TALKER_ID_LEN_BYTES) == 0)
    {
        return NMEA_TALKER_ID_BEIDOU;
    }
    if (memcmp(bytes, "GA", NMEA_TALKER_ID_LEN_BYTES) == 0)
    {
        return NMEA_TALKER_ID_GALILEO;
    }
    if (memcmp(bytes, "GI", NMEA_TALKER_ID_LEN_BYTES) == 0)
    {
        return NMEA_TALKER_ID_NAVIC;
    }
    if (memcmp(bytes, "GL", NMEA_TALKER_ID_LEN_BYTES) == 0)
    {
        return NMEA_TALKER_ID_GLONASS;
    }
    if (memcmp(bytes, "GP", NMEA_TALKER_ID_LEN_BYTES) == 0)
    {
        return NMEA_TALKER_ID_GPS;
    }
    if (memcmp(bytes, "GQ", NMEA_TALKER_ID_LEN_BYTES) == 0)
    {
        return NMEA_TALKER_ID_QZSS;
    }
    return NMEA_TALKER_ID_UNKNOWN;
}

static NMEA_SENTENCE_TYPE_E sentence_type_bytes_to_sentence_type(uint8_t *bytes)
{
    if (memcmp(bytes, "GGA", NMEA_SENTENCE_TYPE_LEN_BYTES) == 0)
    {
        return NMEA_SENTENCE_TYPE_FIXED_DATA;
    }
    if (memcmp(bytes, "GLL", NMEA_SENTENCE_TYPE_LEN_BYTES) == 0)
    {
        return NMEA_SENTENCE_TYPE_LAT_LON;
    }
    if (memcmp(bytes, "GSA", NMEA_SENTENCE_TYPE_LEN_BYTES) == 0)
    {
        return NMEA_SENTENCE_TYPE_DOP;
    }
    if (memcmp(bytes, "GSV", NMEA_SENTENCE_TYPE_LEN_BYTES) == 0)
    {
        return NMEA_SENTENCE_TYPE_SAT_VIEW;
    }
    if (memcmp(bytes, "RMC", NMEA_SENTENCE_TYPE_LEN_BYTES) == 0)
    {
        return NMEA_SENTENCE_TYPE_MIN_GPS_DATA;
    }
    if (memcmp(bytes, "VTG", NMEA_SENTENCE_TYPE_LEN_BYTES) == 0)
    {
        return NMEA_SENTENCE_TYPE_COURSE_SPEED;
    }
    return NMEA_SENTENCE_TYPE_UNKNOWN;
}

nmea_sentence_t nmea_bytes_to_sentence(uint8_t bytes[NMEA_SENTENCE_MAX_LEN])
{
    nmea_sentence_t sentence = {0};

    if (bytes == NULL)
    {
        return sentence;
    }

    sentence.talker_id = talker_id_bytes_to_talker_id(&bytes[1]);
    sentence.sentence_type = sentence_type_bytes_to_sentence_type(&bytes[3]);

    return sentence;
}

