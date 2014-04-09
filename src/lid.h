//-----------------------------------------------------------------------------
//   lid.h
//
//   Project: EPA SWMM5
//   Version: 5.0
//   Date:    07/30/10   (Build 5.0.019)
//   Author:  L. Rossman (US EPA)
//
//   Public interface for LID functions.
//
//-----------------------------------------------------------------------------

#ifndef LID_H
#define LID_H

//-----------------------------------------------------------------------------
//   LID Methods
//-----------------------------------------------------------------------------
void     lid_create(int lidCount, int subcatchCount);
void     lid_delete(void);
int      lid_readProcParams(char* tok[], int ntoks);
int      lid_readGroupParams(char* tok[], int ntoks);
void     lid_validate(void);
void     lid_initState(void);
double   lid_getPervArea(int subcatch);
double   lid_getFlowToPerv(int subcatch);
double   lid_getStoredVolume(int subcatch);
double   lid_getSurfaceDepth(int subcatch);
double   lid_getDepthOnPavement(int subcatch, double impervDepth);
double   lid_getRunoff(int subcatch, double *outflow, double *evapVol,
         double *pervEvapVol, double *infilVol, double tStep);
void     lid_writeSummary(void);
void     lid_writeWaterBalance(void);

#endif
