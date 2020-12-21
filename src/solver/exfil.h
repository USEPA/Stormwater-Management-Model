//-----------------------------------------------------------------------------
//   exfil.h
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    09/15/14   (Build 5.1.007)
//   Author:  L. Rossman (US EPA)
//
//   Public interface for exfiltration functions.
//-----------------------------------------------------------------------------

#ifndef EXFIL_H
#define EXFIL_H

//----------------------------
// EXFILTRATION OBJECT
//----------------------------
typedef struct
{
    TGrnAmpt*  btmExfil;
    TGrnAmpt*  bankExfil;
    double     btmArea;
    double     bankMinDepth;
    double     bankMaxDepth;
    double     bankMaxArea;
}   TExfil;

//-----------------------------------------------------------------------------
//   Exfiltration Methods
//-----------------------------------------------------------------------------
int    exfil_readStorageParams(int k, char* tok[], int ntoks, int n);
void   exfil_initState(int k);
double exfil_getLoss(TExfil* exfil, double tStep, double depth, double area);

#endif
