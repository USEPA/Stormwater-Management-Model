/*
 *  shared/csio_helper.h - Provides C stdio helper functions
 *
 *  Created on: October 8, 2020
 *  Updated on:
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/CESER
 */

#ifndef CSIO_HELPER_H_
#define CSIO_HELPER_H_


#include <stdio.h>
#include <stdarg.h>

#ifdef _MSC_VER
  #define RESTRICT
#else
  #define RESTRICT restrict
#endif


#if defined(__cplusplus)
extern "C" {
#endif


inline int csio_snprintf(char *str, size_t n, const char *format, ...)
{
#ifdef _MSC_VER
  #define SNPRINTF vsprintf_s
#else
  #define SNPRINTF vsnprintf
#endif

    va_list args;
    va_start(args, format);

    int error = SNPRINTF(str, n, format, args);

    va_end(args);
    return error;
}

inline int csio_fprintf(FILE *RESTRICT stream, const char *RESTRICT format, ...)
{
#ifdef _MSC_VER
  #define FPRINTF vfprintf_s
#else
  #define FPRINTF vfprintf
#endif

    va_list args;
    va_start(args, format);

    int error = FPRINTF(stream, format, args);

    va_end(args);
    return error;
}

inline int csio_printf(const char *RESTRICT format, ...)
{
#ifdef _MSC_VER
  #define PRINTF vprintf_s
#else
  #define PRINTF vprintf
#endif

    va_list args;
    va_start(args, format);

    int error = PRINTF(format, args);

    va_end(args);
    return error;
}


#if defined(__cplusplus)
}
#endif


#endif /* CSIO_HELPER_H_ */
