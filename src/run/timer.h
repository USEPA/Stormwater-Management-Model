/*
 *  timer.h - Provides simple timer with millisec resolution
 *
 *  Created on: October 8, 2020
 *  Updated on:
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/CESER
 */

#ifndef TIMER_H
#define TIMER_H


// System includes
#ifdef _WIN32
  #include <Windows.h>
#else
  #include <sys/time.h>
#endif

#include <stdio.h>


#define TIMER_LEN 8


#if defined(__cplusplus)
extern "C" {
#endif


long current_time_millis(void);

char *format_time(char *time, long msec);


#if defined(__cplusplus)
}
#endif


#endif //TIMER_H
