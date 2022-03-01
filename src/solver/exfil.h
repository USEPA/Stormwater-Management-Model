//-----------------------------------------------------------------------------
//   exfil.h
//
//   Project: EPA SWMM5
//   Version: 5.2
//   Date:    11/01/21  (Build 5.2.0)
//   Author:  L. Rossman
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
