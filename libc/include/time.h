#ifndef _TIME_H
#define _TIME_H 1

#include <sys/cdefs.h>
#include <stddef.h>

#define EOF (-1)

struct tm {
   int tm_sec;         /* seconds,  range 0 to 59          */
   int tm_min;         /* minutes, range 0 to 59           */
   int tm_hour;        /* hours, range 0 to 23             */
   int tm_mday;        /* day of the month, range 1 to 31  */
   int tm_month;         /* month, range 0 to 11             */
   int tm_year;        /* The number of years since 1900   */
   int tm_wday;        /* day of the week, range 0 to 6    */
   int tm_yday;        /* day in the year, range 0 to 365  */
   int tm_isdst;       /* daylight saving time             */
};

#ifdef __cplusplus
extern "C" {
#endif
/*Supports only: %d,%m,%y,%S,%M,%H,%Y identifiers.*/
size_t strftime(char *s, size_t max, const char *format,
                       const struct tm *tm); 

#ifdef __cplusplus
}
#endif

#endif
