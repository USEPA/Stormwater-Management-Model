/*
 *  shared/csio_helper.h - Provides C stdio helper functions
 *
 *  Created on: October 8, 2020
 *  Updated on:
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/CESER
 */

#include "csio_helper.h"


#ifdef _MSC_VER
  #define RESTRICT
#else
  #define RESTRICT restrict
#endif


extern inline int csio_snprintf(char *str, size_t n, const char *format, ...);

extern inline int csio_fprintf(FILE *RESTRICT stream, const char *RESTRICT format, ...);

extern inline int csio_printf(const char *RESTRICT format, ...);
