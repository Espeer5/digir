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
// 7/31/26  Edward Speer  Add extaction from each sentence type data

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <string.h>
#include <stdbool.h>

#include "nmea_parser.h"
#include "calendar.h"
#include "log.h"

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

// Expects an input like (ddmm.mmmm,a), and returns degrees e7. Advances the pointer beyond the 
// degree data.
static int compute_deg_e7(uint8_t **bytes, int num_deg_bytes)
{
    int bytes_read = 0;
    int64_t deg = 0;
    while (bytes_read++ < num_deg_bytes)
    {
        deg = deg * 10 + (*(*bytes)++ - '0');
    }

    int64_t min = 0;
    int scale = 1;
    bool decimal = false;
    // TODO: NEEDS A TIMEOUT
    while (**bytes != ',' && **bytes != '\0')
    {
        if (**bytes == '.')
        {
            decimal = true;
        }
        else
        {
            min = min * 10 + (**bytes - '0');
            if (decimal)
            {
                scale *= 10;
            }
        }
        (*bytes)++;
    }
    uint8_t a = *++(*bytes);
    (*bytes) += 2; // Advance past final comma
    int dir = (a == 'N' || a == 'E') ? 1 : -1;

    return dir * (deg * 10000000 + min * 10000000 / (60 * scale));
}

static bool extract_from_gga_data(nmea_sentence_t *sentence, uint8_t *data_bytes)
{
    // GGA data format: hhmmss.ss, ddmm.mm, a, ddmm.mm, a, quality, sat info
    // GGA doesn't include date info, so we don't use it for time, only location
    sentence->ts_valid = false;

    // First find the start of the latitude after the first comma
    uint8_t *pos = data_bytes;
    int iter_cnt = 0;
    while(*pos++ != ',')
    {
        if (iter_cnt++ >= NMEA_DATA_MAX_LEN)
        {
            log_warn("Failed to parse NMEA GGA -- couldn't locate latitude");
            return false;
        }
    }
 
    // Will advance the pos pointer to the end of the lat data
    sentence->lat_deg_e7 = compute_deg_e7(&pos, 2);
    sentence->lon_deg_e7 = compute_deg_e7(&pos, 3);
    sentence->loc_valid = true;

    return true;
}

static bool extract_from_rmc_data(nmea_sentence_t *sentence, uint8_t *data_bytes)
{
    // RMC data format: hhmmss.ss, A, ddmm.mm, a, ddmm.mm, a, x.x, x.x, ddmmyy
    //                  (UTC)    (valid)  (lat)       (lon)   (sp) (trk) (date)
    // RMC is the most useful sentence, giving full UTC timestamp and location

    uint8_t *pos = data_bytes;

    // Read the timestamp in and convert to seconds
    int64_t utc_seconds = 0;

    utc_seconds += 60 * 60 * 10 * (*pos++ - '0'); // 10's place hours
    utc_seconds += 60 * 60 * (*pos++ - '0');      // 1's place hours

    utc_seconds += 60 * 10 * (*pos++ - '0');      // 10's place minutes
    utc_seconds += 60 * (*pos++ - '0');           // 1's place minutes

    utc_seconds += 10 * (*pos++ - '0');           // 10's place seconds
    utc_seconds += *pos++ - '0';                  // 1's place seconds

    // advance the pointer past '.'
    pos++;

    int fractional_seconds = 0;
    fractional_seconds += 100 * (*pos++ - '0'); // tenths of seconds
    fractional_seconds += 10 * (*pos++ - '0');  // hundreths of seconds
    fractional_seconds += *pos++ - '0';         // thousandths of seconds
    sentence->fractional_seconds = fractional_seconds;

    // advance the pointer past the comma
    pos++;
    
    sentence->ts_valid = *pos++ == 'A' ? true : false;

    // advance the pointer past the comma
    pos++;

    // Will advance the pos pointer to the end of the lat data
    sentence->lat_deg_e7 = compute_deg_e7(&pos, 2);
    sentence->lon_deg_e7 = compute_deg_e7(&pos, 3);
    sentence->loc_valid = true;

    // advance the pointer past x.x,x.x,
    pos += 12;

    int year  = 2000;
    int month = 0;
    int day   = 0;

    day += 10 * (*pos++ - '0');   // 10's place days
    day += *pos++ - '0';          // 1's place days

    month += 10 * (*pos++ - '0'); // 10's place months
    month += *pos++ - '0';        // 1's place months

    year += 10 * (*pos++ - '0');  // 10's place years
    year += *pos++ - '0';         // 1's place years

    utc_to_unix_seconds(year, month, day, utc_seconds, &sentence->unix_second);
    sentence->ts_valid = true;
    return true;
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

    switch (sentence.sentence_type)
    {
        case NMEA_SENTENCE_TYPE_FIXED_DATA:   // GGA
        {
            extract_from_gga_data(&sentence, &bytes[7]);
            break;
        }
        case NMEA_SENTENCE_TYPE_DOP:          // GSA 
        {
            // DOP currently not useful
            break;
        }
        case NMEA_SENTENCE_TYPE_SAT_VIEW:     // GSV
        {
            // Sats in view curently not useful
            break;
        }
        case NMEA_SENTENCE_TYPE_MIN_GPS_DATA: // RMC
        {
            extract_from_rmc_data(&sentence, &bytes[7]);
            break;
        }
        default:
            log_warn("Unhandled sentence type encountered in NMEA parser");
    }

    return sentence;
}

