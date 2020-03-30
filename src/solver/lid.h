//-----------------------------------------------------------------------------
//   lid.h
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    03/20/14   (Build 5.1.001)
//            03/19/15   (Build 5.1.008)
//            08/01/16   (Build 5.1.011)
//            03/14/17   (Build 5.1.012)
//            05/10/18   (Build 5.1.013)
//   Author:  L. Rossman (US EPA)
//
//   Public interface for LID functions.
//
//   Build 5.1.008:
//   - Support added for Roof Disconnection LID.
//   - Support added for separate routing of LID drain flows.
//   - Detailed LID reporting modified.
//
//   Build 5.1.011:
//   - Water depth replaces moisture content for LID's pavement layer. 
//   - Arguments for lidproc_saveResults() modified.
//
//   Build 5.1.012:
//   - Redefined meaning of wasDry in TLidRptFile structure.
//
//   Build 5.1.013:
//   - New member fromPerv added to TLidUnit structure to allow LID
//     units to also treat pervious area runoff.
//   - New members hOpen and hClose addded to TDrainLayer to open/close
//     drain when certain heads are reached.
//   - New member qCurve added to TDrainLayer to allow underdrain flow to
//     be adjusted by a curve of multiplier v. head.
//   - New array drainRmvl added to TLidProc to allow for underdrain
//     pollutant removal values.
//   - New members added to TPavementLayer and TLidUnit to support
//     unclogging permeable pavement at fixed intervals.
//
//-----------------------------------------------------------------------------

#ifndef LID_H
#define LID_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "infil.h"

//-----------------------------------------------------------------------------
//  Enumerations
//-----------------------------------------------------------------------------
enum LidTypes {
    BIO_CELL,                // bio-retention cell
    RAIN_GARDEN,             // rain garden 
    GREEN_ROOF,              // green roof 
    INFIL_TRENCH,            // infiltration trench
    POROUS_PAVEMENT,         // porous pavement
    RAIN_BARREL,             // rain barrel
    VEG_SWALE,               // vegetative swale
    ROOF_DISCON};            // roof disconnection

enum TimePeriod {
    PREVIOUS,                // previous time period
    CURRENT};                // current time period

//-----------------------------------------------------------------------------
//  Data Structures
//-----------------------------------------------------------------------------
#define MAX_LAYERS 4

// LID Surface Layer
typedef struct
{
    double    thickness;          // depression storage or berm ht. (ft)
    double    voidFrac;           // available fraction of storage volume
    double    roughness;          // surface Mannings n 
    double    surfSlope;          // land surface slope (fraction)
    double    sideSlope;          // swale side slope (run/rise)
    double    alpha;              // slope/roughness term in Manning eqn.
    char      canOverflow;        // 1 if immediate outflow of excess water
}  TSurfaceLayer;

// LID Pavement Layer
typedef struct
{
    double   thickness;           // layer thickness (ft)
    double   voidFrac;            // void volume / total volume
    double   impervFrac;          // impervious area fraction
    double   kSat;                // permeability (ft/sec)
    double   clogFactor;          // clogging factor
    double   regenDays;           // clogging regeneration interval (days)     //(5.1.013)
    double   regenDegree;         // degree of clogging regeneration           //
}  TPavementLayer;

// LID Soil Layer
typedef struct
{
    double    thickness;          // layer thickness (ft)
    double    porosity;           // void volume / total volume
    double    fieldCap;           // field capacity
    double    wiltPoint;          // wilting point
    double    suction;            // suction head at wetting front (ft)
    double    kSat;               // saturated hydraulic conductivity (ft/sec)
    double    kSlope;             // slope of log(K) v. moisture content curve
}  TSoilLayer;

// LID Storage Layer
typedef struct
{
    double    thickness;          // layer thickness (ft)
    double    voidFrac;           // void volume / total volume
    double    kSat;               // saturated hydraulic conductivity (ft/sec)
    double    clogFactor;         // clogging factor
}  TStorageLayer;

// Underdrain System (part of Storage Layer)
typedef struct
{
    double    coeff;              // underdrain flow coeff. (in/hr or mm/hr)
    double    expon;              // underdrain head exponent (for in or mm)
    double    offset;             // offset height of underdrain (ft)
    double    delay;              // rain barrel drain delay time (sec)
    double    hOpen;              // head when drain opens (ft)                //(5.1.013)
    double    hClose;             // head when drain closes (ft)               //
    int       qCurve;             // curve controlling flow rate (optional)    //
}  TDrainLayer;

// Drainage Mat Layer (for green roofs)
typedef struct
{
    double    thickness;          // layer thickness (ft)
    double    voidFrac;           // void volume / total volume
    double    roughness;          // Mannings n for green roof drainage mats
    double    alpha;              // slope/roughness term in Manning equation
}  TDrainMatLayer;

// LID Process - generic LID design per unit of area
typedef struct
{
    char*          ID;            // identifying name
    int            lidType;       // type of LID
    TSurfaceLayer  surface;       // surface layer parameters
    TPavementLayer pavement;      // pavement layer parameters
    TSoilLayer     soil;          // soil layer parameters
    TStorageLayer  storage;       // storage layer parameters
    TDrainLayer    drain;         // underdrain system parameters
    TDrainMatLayer drainMat;      // drainage mat layer
    double*        drainRmvl;     // underdrain pollutant removals             //(5.1.013)
}  TLidProc;

// Water Balance Statistics
typedef struct
{
    double         inflow;        // total inflow (ft)
    double         evap;          // total evaporation (ft)
    double         infil;         // total infiltration (ft)
    double         surfFlow;      // total surface runoff (ft)
    double         drainFlow;     // total underdrain flow (ft)
    double         initVol;       // initial stored volume (ft)
    double         finalVol;      // final stored volume (ft)
}  TWaterBalance;

// LID Report File
typedef struct
{
    FILE*     file;               // file pointer
    int       wasDry;             // number of successive dry periods
    char      results[256];       // results for current time period
}   TLidRptFile;

// LID Unit - specific LID process applied over a given area
typedef struct
{
    int      lidIndex;       // index of LID process
    int      number;         // number of replicate units
    double   area;           // area of single replicate unit (ft2)
    double   fullWidth;      // full top width of single unit (ft)
    double   botWidth;       // bottom width of single unit (ft)
    double   initSat;        // initial saturation of soil & storage layers
    double   fromImperv;     // fraction of impervious area runoff treated
    double   fromPerv;       // fraction of pervious area runoff treated       //(5.1.013)
    int      toPerv;         // 1 if outflow sent to pervious area; 0 if not
    int      drainSubcatch;  // subcatchment receiving drain flow
    int      drainNode;      // node receiving drain flow
    TLidRptFile* rptFile;    // pointer to detailed report file

    TGrnAmpt soilInfil;      // infil. object for biocell soil layer 
    double   surfaceDepth;   // depth of ponded water on surface layer (ft)
    double   paveDepth;      // depth of water in porous pavement layer
    double   soilMoisture;   // moisture content of biocell soil layer
    double   storageDepth;   // depth of water in storage layer (ft)

    // net inflow - outflow from previous time step for each LID layer (ft/s)
    double   oldFluxRates[MAX_LAYERS];
                                     
    double   dryTime;        // time since last rainfall (sec)
    double   oldDrainFlow;   // previous drain flow (cfs)
    double   newDrainFlow;   // current drain flow (cfs)
    double   volTreated;     // total volume treated (ft)                      //(5.1.013)
    double   nextRegenDay;   // next day when unit regenerated                 //
    TWaterBalance  waterBalance;     // water balance quantites
}  TLidUnit;

//-----------------------------------------------------------------------------
//   LID Methods
//-----------------------------------------------------------------------------
void     lid_create(int lidCount, int subcatchCount);
void     lid_delete(void);

int      lid_readProcParams(char* tok[], int ntoks);
int      lid_readGroupParams(char* tok[], int ntoks);

void     lid_validate(void);
void     lid_initState(void);
void     lid_setOldGroupState(int subcatch);

double   lid_getPervArea(int subcatch);
double   lid_getFlowToPerv(int subcatch);
double   lid_getDrainFlow(int subcatch, int timePeriod);
double   lid_getStoredVolume(int subcatch);
void     lid_addDrainLoads(int subcatch, double c[], double tStep);
void     lid_addDrainRunon(int subcatch);
void     lid_addDrainInflow(int subcatch, double f);
void     lid_getRunoff(int subcatch, double tStep);
void     lid_writeSummary(void);
void     lid_writeWaterBalance(void);

//-----------------------------------------------------------------------------

void     lidproc_initWaterBalance(TLidUnit *lidUnit, double initVol);

double   lidproc_getOutflow(TLidUnit* lidUnit, TLidProc* lidProc,
         double inflow, double evap, double infil, double maxInfil,
         double tStep, double* lidEvap, double* lidInfil, double* lidDrain);

void     lidproc_saveResults(TLidUnit* lidUnit, double ucfRainfall,
         double ucfRainDepth);

#endif
