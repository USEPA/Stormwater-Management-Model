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

// OWA EDIT ##################################################################################
// TInlet and TInletStats struct defs moved to inlet.h to be shared by toolkit.c
typedef struct
{
    int       flowPeriods;        // # periods with approach flow
    int       capturePeriods;     // # periods with captured flow
    int       backflowPeriods;    // # periods with backflow
    double    peakFlow;           // peak flow seen by inlet (cfs)
    double    peakFlowCapture;    // capture efficiency at peak flow
    double    avgFlowCapture;     // average capture efficiency
    double    bypassFreq;         // frequency of bypass flow
} TInletStats;

// Inlet list object
struct TInlet
{
    int         linkIndex;        // index of conduit link with the inlet
    int         designIndex;      // index of inlet's design
    int         nodeIndex;        // index of node receiving captured flow
    int         numInlets;        // # inlets on each side of street or in channel
    int         placement;        // whether inlet is on-grade or on-sag
    double      clogFactor;       // fractional degree of inlet clogging
    double      flowLimit;        // inlet flow restriction (cfs)
    double      localDepress;     // local gutter depression (ft)
    double      localWidth;       // local depression width (ft)

    double      flowFactor;       // flow = flowFactor * (flow spread)^2.67
    double      flowCapture;      // captured flow rate (cfs)
    double      backflow;         // backflow from capture node (cfs)
    double      backflowRatio;    // inlet backflow / capture node overflow
    TInletStats stats;            // inlet performance statistics
    TInlet *    nextInlet;        // next inlet in list
};
// ###########################################################################################

#endif
