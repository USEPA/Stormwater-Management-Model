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


#ifdef _MSC_VER
  #define RESTRICT
#else
  #define RESTRICT restrict
#endif

#if defined(__cplusplus)
extern "C" {
#endif


int csio_snprintf(char *str, size_t n, const char *format, ...);
int csio_fprintf(FILE *RESTRICT stream, const char *RESTRICT format, ...);


#if defined(__cplusplus)
}
#endif


#endif /* CSIO_HELPER_H_ */
