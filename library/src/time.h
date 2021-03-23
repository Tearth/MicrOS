#ifndef TIME_H
#define TIME_H

//! Clock ticks per second
#define CLOCKS_PER_SEC 1000

#include <stdint.h>
#include "micros.h"

//! Clock type
typedef uint32_t clock_t;

//! Time type
typedef uint64_t time_t;

//! Time structure
typedef struct tm
{
    //! Seconds after the minute
    uint32_t tm_sec;
    
    //! Minutes after the hour
    uint32_t tm_min;
    
    //! Hours since midnight
    uint32_t tm_hour;
    
    //! Day of the month
    uint32_t tm_mday;
    
    //! Months since January
    uint32_t tm_mon;
    
    //! Years since 1900
    uint32_t tm_year;
    
    //! Days since Sunday
    uint32_t tm_wday;
    
    //! Days since January 1
    uint32_t tm_yday;
    
    //! Daylight Saving Time flag
    uint32_t tm_isdst;
} tm;

//! Internal variable used during time conversions
tm converted_time;

#ifdef __cplusplus
extern "C" {
#endif

//! Clock program
/*!
    Copies the values of num bytes from the location pointed to by source directly to the memory block pointed to by destination.
    \return Destination is returned.
*/
clock_t clock();

//! Return difference between two times
/*!
    Calculates the difference in seconds between beginning and end.
    \param end Higher bound of the time interval whose length is calculated.
    \param beginning Lower bound of the time interval whose length is calculated. If this describes a time point later than end, the result is negative.
    \return The result of (end-beginning) in seconds as a floating-point value of type double.
*/
double difftime(time_t end, time_t beginning);

//! Convert tm structure to time_t
/*!
    Returns the value of type time_t that represents the local time described by the tm structure pointed by timeptr (which may be modified).
    \param timeptr Pointer to a tm structure that contains a calendar time broken down into its components (see struct tm).
    \return A time_t value corresponding to the calendar time passed as argument. If the calendar time cannot be represented, a value of -1 is returned.
*/
time_t mktime(tm * timeptr);

//! Get current time
/*!
    Get the current calendar time as a value of type time_t.
    \param timer Pointer to an object of type time_t, where the time value is stored.
    \return The current calendar time as a time_t object.
*/
time_t time(time_t* timer);

//! Convert time_t to tm as UTC time
/*!
    Uses the value pointed by timer to fill a tm structure with the values that represent the corresponding time, expressed as a UTC time (i.e., the time at the GMT timezone).
    \param timer Pointer to an object of type time_t that contains a time value.
    \return A pointer to a tm structure with its members filled with the values that correspond to the UTC time representation of timer.
*/
tm *gmtime(const time_t *timer);

//! Calculates time of the year
/*!
    Calculates time of the year based on the passed date.
    \param day Day in the date to be calculated.
    \param month Month in the date to be calculated.
    \param year Year in the date to be calculated.
    \return Day of the year.
*/
uint32_t __time_get_day_of_year(uint32_t day, uint32_t month, uint32_t year);

//! Calculates time of the week
/*!
    Calculates time of the week based on the passed date.
    \param day Day in the date to be calculated.
    \param month Month in the date to be calculated.
    \param year Year in the date to be calculated.
    \return Day of the week.
*/
uint32_t __time_get_day_of_week(uint32_t day, uint32_t month, uint32_t year);

#ifdef __cplusplus
}
#endif

#endif