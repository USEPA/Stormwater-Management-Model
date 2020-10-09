/*
 *  shared/csio_helper.h - Provides C stdio helper functions
 *
 *  Created on: October 8, 2020
 *  Updated on:
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/CESER
 */

#include <stdarg.h>
#include <stdio.h>

#include "csio_helper.h"


int csio_sprintf(char *str, size_t n, const char *format, ...)
{
#ifdef _MSC_VER
  #define VSPRINTF vsprintf_s
#else
  #define VSPRINTF vsnprintf
#endif

    int error;
    va_list args;
    va_start(args, format);

    error = VSPRINTF(str, n, format, args);

    va_end(args);
    return error;
}
