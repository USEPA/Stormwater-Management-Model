/*
 *  shared/cstr_helper.c - Provides C string helper functions
 *
 *  Created on: April 2, 2019
 *  Updated on: Dec 2, 2019
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/NRMRL
 */

#include <stdlib.h>
#include <string.h>

#include "cstr_helper.h"


int cstr_duplicate(char **dest, const char *source)
// Duplicates source string
{
    size_t size = 1 + strlen(source);
    *dest = (char *) calloc(size, sizeof(char));

    if (*dest == NULL)
        return -1;
    else {
#ifdef _MSC_VER
        strncpy_s(*dest, size, source, size);
#else
        strncpy(*dest, source, size);
#endif
    }
    return 0;
}


bool cstr_isvalid(const char *element_id)
// Determines if invalid characters are present in an element id string
{
    const char *invalid_chars = " \";";

    // if invalid char is present a pointer to it is returned else NULL
    if (strpbrk(element_id, invalid_chars))
        return false;
    else
        return true;
}


bool cstr_isnullterm(const char *source)
// Determines if the string passed is null terminated or not
{
    if (strchr(source, '\0'))
        return true;
    else
        return false;
}
