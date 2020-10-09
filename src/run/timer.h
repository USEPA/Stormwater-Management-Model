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


#define TIMER_LEN 8

long current_time_millis(void);
char *format_time(char *time, long msec);


#endif //TIMER_H
