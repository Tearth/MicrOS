#include "../time.h"

time_t time(time_t* timer)
{
    micros_rtc_time rtc_time;
    micros_rtc_read_time(&rtc_time);
    
    tm time;
    time.tm_year = rtc_time.year;
    time.tm_mon = rtc_time.month;
    time.tm_mday = rtc_time.day;
    time.tm_hour = rtc_time.hour;
    time.tm_min = rtc_time.minute;
    time.tm_sec = rtc_time.second;
    
    time_t current_time = mktime(&time);
    if(timer != 0)
    {
        *timer = current_time;
    }
    
    return mktime(&time);
}