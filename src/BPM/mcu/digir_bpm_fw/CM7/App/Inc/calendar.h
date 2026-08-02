//#################################################################################################
//  DIGIR BPM CALENDAR HANDLING
//
//  VERSION 0.1
//#################################################################################################

// Handling of the calendar and its relation to the UTC second in the unix epoch for DigiR

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 8/1/2026  Edward Speer  Initial revision

#ifndef CALENDAR_H
#define CALENDAR_H

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

// Convert a UTC time with date and ToD to unix epoch timestamp in seconds
bool utc_to_unix_seconds(
    int     year,
    int     month,
    int     day,
    int64_t tod, // in seconds
    int64_t *unix_seconds);

#endif // #ifndef CALENDAR_H

