//#################################################################################################
//  DIGIR BPM CALENDAR HANDLING
//
//  VERSION 0.1
//#################################################################################################

// Handling of the calendar and its relation to the UTC second in the unix epoch for DigiR

//#################################################################################################
//  CHANGE LOG
//#################################################################################################

// 8/1/26  Edward Speer  Initial revision

//#################################################################################################
//  INCLUDES
//#################################################################################################

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "calendar.h"

//#################################################################################################
//  FUNCTIONS
//#################################################################################################

static bool is_leap_year(int year)
{
    return (year % 4 == 0) & ((year % 100 != 0) || (year % 400 == 0));
}

static int days_in_month(int year, int month)
{
    static const uint8_t month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month == 2 && is_leap_year(year))
    {
        return 29;
    }

   return month_days[month - 1];
}

bool utc_to_unix_seconds(
    int year,
    int month,
    int day,
    int64_t tod, // in seconds
    int64_t *unix_seconds)
{
    if (unix_seconds == NULL)
    {
        return false;
    }

    if (year < 1970 ||
        month < 1 || month > 12 ||
        day < 1 || day > days_in_month(year, month) ||
        tod < 0 || tod > 60 * 60 * 24 + 1) // Allow for 23:59:60 on leap second
    {
        return false;
    }

    int64_t days = 0;
    for (int y = 1970; y < year; y++)
    {
        days += is_leap_year(y) ? 366 : 365;
    }

    for (int m = 1; m < month; m++)
    {
        days += (int64_t)days_in_month(year, m);
    }

    days += (int64_t)(day - 1);

    // clamp 23:59:60
    int64_t normalized_tod = tod == 60 * 60 * 24 ? tod - 1 : tod;

    *unix_seconds = days * 24 * 60 * 60 + normalized_tod;
    return true;
}

