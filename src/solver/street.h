//-----------------------------------------------------------------------------
//   street.h
//
//   Project: EPA SWMM5
//   Version: 5.2
//   Date:    03/24/21  (Build 5.2.0)
//   Author:  L. Rossman
//
//   Definition of the Street Cross-Section Object
//
//-----------------------------------------------------------------------------
#ifndef STREET_H
#define STREET_H

#include "headers.h"

// Street backing structure
typedef struct
{
    double slope;
    double width;
    double roughness;
} TBacking;

// Street object
typedef struct
{
    char*        ID;                // name of street section
    int          sides;             // 1 or 2 sided street
    double       slope;             // cross slope (Sx)
    double       width;             // distance from curb to crown (ft) (Tmax)
    double       curbHeight;        // curb height incl. depression (ft) (Hc)
    double       gutterDepression;  // gutter depression (ft) (a)
    double       gutterWidth;       // gutter width (ft) (W)
    double       roughness;         // street's Manning n
    TBacking     backing;           // street backing
    TTransect    transect;          // street's transect   
}   TStreet;

// Shared street variables
int     StreetCount;                // number of defined Street cross-sections
TStreet *Street;                    // array of defined Street cross-sections

// Shared street functions
int   street_create(int nStreets);
void  street_delete();
int   street_readParams(char* tok[], int ntoks);

#endif
