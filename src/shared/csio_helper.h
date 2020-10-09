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


#if defined(__cplusplus)
extern "C" {
#endif


int csio_sprintf(char *str, size_t n, const char *format, ...);


#if defined(__cplusplus)
}
#endif


#endif /* CSIO_HELPER_H_ */
