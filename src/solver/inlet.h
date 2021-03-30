//-----------------------------------------------------------------------------
//   inlet.h
//
//   Project: EPA SWMM5
//   Version: 5.2
//   Date:    03/24/21 (Build 5.2.0)
//   Author:  L. Rossman
//
//   Definition of Street/Channel Inlet Objects
//
//-----------------------------------------------------------------------------
#ifndef INLET_H
#define INLET_H

// Grate inlet
typedef struct
{
    int       type;               // type of grate used
    double    length;             // length (parallel to flow) (ft)
    double    width;              // width (perpendicular to flow) (ft)
    double    fracOpenArea;       // fraction of grate area that is open
    double    splashVeloc;        // splash-over velocity (ft/s)
} TGrateInlet;

// Slotted drain inlet
typedef struct
{
    double    length;             // length (parallel to flow) (ft)
    double    width;              // width (perpendicular to flow) (ft)
} TSlottedInlet;

// Curb opening inlet
typedef struct
{
    double    length;             // length of curb opening (ft)
    double    height;             // height of curb opening (ft)
    int       throatAngle;        // type of throat angle
} TCurbInlet;

// Custom inlet
typedef struct
{
    int       onGradeCurve;       // flow diversion curve index
    int       onSagCurve;         // flow rating curve index
} TCustomInlet;

// Inlet design object
typedef struct
{
    char *         ID;            // name assigned to inlet design
    int            type;          // type of inlet used (grate, curb, etc)
    TGrateInlet    grateInlet;    // length = 0 if not used
    TSlottedInlet  slottedInlet;  // length = 0 if not used
    TCurbInlet     curbInlet;     // length = 0 if not used
    TCustomInlet   customInlet;   // curve index = -1 if not used
} TInletDesign;

// Inlet performance statistics
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

// Inlet usage object
typedef struct TInlet TInlet;
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
    double      outflow;          // captured flow rate (cfs)
    double      backflow;         // backflow into inlet (cfs)
    TInletStats stats;            // inlet performance statistics
    TInlet *    nextInlet;        // next inlet in list
};

// Shared inlet variables
TInletDesign * InletDesigns;      // array of available inlet designs
int            InletDesignCount;  // number of inlet designs
int            UsesInlets;        // TRUE if project uses inlets

// Shared inlet functions
int    inlet_create(int nInlets);
void   inlet_delete();
int    inlet_readDesignParams(char* tok[], int ntoks);
int    inlet_readUsageParams(char* tok[], int ntoks);
void   inlet_validate();
void   inlet_findInletFlows(double tStep);
void   inlet_convertOverflows();
void   inlet_writeStatsReport();
double inlet_capturedFlow(int link);

#endif
