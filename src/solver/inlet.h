//-----------------------------------------------------------------------------
//   inlet.h
//
//   Project: EPA SWMM5
//   Version: 5.2
//   Date:    11/01/21 (Build 5.2.0)
//   Author:  L. Rossman
//
//   Street/Channel Inlet Functions
//
//-----------------------------------------------------------------------------
#ifndef INLET_H
#define INLET_H

typedef struct TInlet TInlet;

int    inlet_create(int nInlets);
void   inlet_delete();
int    inlet_readDesignParams(char* tok[], int ntoks);
int    inlet_readUsageParams(char* tok[], int ntoks);
void   inlet_validate();

void   inlet_findCapturedFlows(double tStep);
void   inlet_adjustQualInflows();
void   inlet_adjustQualOutflows();

void   inlet_writeStatsReport();
double inlet_capturedFlow(int link);

#endif
