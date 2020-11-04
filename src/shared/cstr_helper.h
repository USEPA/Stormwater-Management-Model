/*
 *  shared/cstr_helper.h - Provides C string helper functions
 *
 *  Created on: April 2, 2019
 *  Updated on: Dec 2, 2019
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/NRMRL
 */

#ifndef CSTR_HELPER_H_
#define CSTR_HELPER_H_


#if (_MSC_VER <= 1600)
  #define bool  int
  #define true  1
  #define false 0
#else
  #include <stdbool.h>
#endif


#if defined(__cplusplus)
extern "C" {
#endif


int cstr_duplicate(char **dest, const char *source);

bool cstr_isvalid(const char *element_id);

bool cstr_isnullterm(const char *source);


#if defined(__cplusplus)
}
#endif


#endif /* CSTR_HELPER_H_ */
