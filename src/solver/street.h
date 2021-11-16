//-----------------------------------------------------------------------------
//   street.h
//
//   Project: EPA SWMM5
//   Version: 5.2
//   Date:    11/01/21  (Build 5.2.0)
//   Author:  L. Rossman
//
//   Definition of the Street Cross-Section Object
//
//-----------------------------------------------------------------------------
#ifndef STREET_H
#define STREET_H

// Shared street functions
int    street_create(int nStreets);
void   street_delete();
int    street_readParams(char* tok[], int ntoks);
double street_getExtentFilled(int link);

#endif
