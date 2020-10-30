/*
 *  timer.h - Provides simple timer with millisec resolution
 *
 *  Created on: October 8, 2020
 *  Updated on:
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/CESER
 */

#include "timer.h"


long current_time_millis(void)
{
#ifdef _WIN32
    // Be advised, this counter has low resolution
    return (long)GetTickCount();

#else
    static long sec = -1;
    struct timeval tv;

    gettimeofday(&tv, (void *)NULL);

    if (sec < 0)
        sec = tv.tv_sec;

    return (tv.tv_sec - sec)*1000 + tv.tv_usec/1000;
#endif
}

char *format_time(char *time, long ms)
{
    size_t n = (size_t)(TIMER_LEN + 1);

    if (ms < 1)
        snprintf(time, n, "0.00s");
    else if ( ms < 1000 )
        snprintf(time, n, "< 1.00s");
    else {
        // Compute hours elapsed
        long hrs = ms/(3600*1000);
        ms -= hrs*3600*1000;
        // Compute minutes elapsed
        long mins = ms/(60*1000);
        ms -= mins*60*1000;
        // Compute seconds elapsed
        double secs = ms*0.001;

        if (hrs > 0)
            snprintf(time, n, "%2ld:%2ld:%02.0f", hrs, mins, secs);
        else if (mins > 0)
            snprintf(time, n, "%2ld:%02.0f", mins, secs);
        else
            snprintf(time, n, "%3.2fs", secs);
    }

    return time;
}
