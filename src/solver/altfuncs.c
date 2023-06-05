//-----------------------------------------------------------------------------
//   altfuncs.c
//
//   Project:  EPA SWMM5
//   Version:  5.2
//   Date:     06/05/23   
//   Author:   Issac Gardner
//
//   Alternate functions to speed up processing
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include "headers.h"
#include "math.h"

// Borrowed from xsect.c (lookup), should become global or library
static double alt_lookup(double x, double *table, int nItems);

//=============================================================================

// Faster version of pow(x, 1.33333)
double faster_pow_1_33333(double val)
//
{
    return alt_lookup(val/4.0, pow_1_33333, N_altfuncs);
}

//=============================================================================

// Faster version of pow(x, 0.66666)
double faster_pow_0_66666(double val)
//
{
    return alt_lookup(val/4.0, pow_0_66666, N_altfuncs);
}

//=============================================================================

int  altfuncs_open(void)
//
//  Input:   none
//  Output:  returns TRUE if successful, FALSE if not
//  Purpose: allocates memory for storing alternate function tables.
//
{
  N_altfuncs = 2001;
  pow_1_33333 = (double *) calloc(N_altfuncs, sizeof(double));
  pow_0_66666 = (double *) calloc(N_altfuncs, sizeof(double));
  if (pow_1_33333 == NULL || pow_0_66666 == NULL)
  {
    report_writeErrorMsg(ERR_MEMORY, "");
    return FALSE;
  }

  // Creates a table of values, assuming input starts at 0ft and ends at 4ft.
  for(int i = 0; i < N_altfuncs; i++){
    double maxVal = 4.0;

    double f = i/(N_altfuncs-1.0) * maxVal;
    pow_1_33333[i] = pow(f, 1.33333);
    pow_0_66666[i] = pow(f, 2./3.);
  }
  
  return TRUE;
}

//=============================================================================

void altfuncs_close(void)
//
//  Input:   none
//  Purpose: frees alternate function table memory.
//
{
    FREE(pow_1_33333);
    FREE(pow_0_66666);
}

//=============================================================================

double alt_lookup(double x, double *table, int nItems)
//
// Borrowed from xsect.c (lookup), should become global or library
//
//  Input:   x = value of independent variable in an alternate function table
//           table = ptr. to alternate function table
//           nItems = number of equally spaced items in table
//  Output:  returns value of dependent table variable
//  Purpose: looks up a value in an alternate function table (i.e., finds y given x).
//
{
    double  delta, x0, x1, y, y2;
    int     i;

    // --- find which segment of table contains x
    delta = 1.0 / ((double)nItems-1);
    i = (int)(x / delta);
    if ( i >= nItems - 1 ) return table[nItems-1];

    // --- compute x at start and end of segment
    x0 = i * delta;
    x1 = ((double)i+1) * delta;

    // --- linearly interpolate a y-value
    y = table[i] + (x - x0) * (table[i+1] - table[i]) / delta;

    // --- use quadratic interpolation for low x value
    if ( i < 2 )
    {
        y2 = y + (x - x0) * (x - x1) / (delta*delta) *
             (table[i]/2.0 - table[i+1] + table[i+2]/2.0) ;
        if ( y2 > 0.0 ) y = y2;
    }
    if ( y < 0.0 ) y = 0.0;
    return y;
}