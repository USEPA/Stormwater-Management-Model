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


#ifdef _MSC_VER
  #define RESTRICT __restrict
#else
  #define RESTRICT restrict
#endif


int csio_snprintf(char *str, size_t n, const char *format, ...)
{
#ifdef _MSC_VER
  #define VSNPRINTF vsprintf_s
#else
  #define VSNPRINTF vsnprintf
#endif

    int error;
    va_list args;
    va_start(args, format);

    error = VSNPRINTF(str, n, format, args);

    va_end(args);
    return error;
}

int csio_fprintf(FILE *RESTRICT stream, const char *RESTRICT format, ...)
{
#ifdef _MSC_VER
  #define VFPRINTF vfprintf_s
#else
  #define VFPRINTF vfprintf
#endif

    int error;
    va_list args;
    va_start(args, format);

    error = VFPRINTF(stream, format, args);

    va_end(args);
    return error;
}
