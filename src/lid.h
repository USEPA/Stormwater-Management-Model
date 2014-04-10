//-----------------------------------------------------------------------------
//   lid.h
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    03/20/14   (Build 5.1.001)
//   Author:  L. Rossman (US EPA)
//
//   Public interface for LID functions.
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
    VEG_SWALE};              // vegetative swale

//-----------------------------------------------------------------------------
//  Data Structures
//-----------------------------------------------------------------------------
#define MAX_STATE_VARS 3

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
    double    lastReportTime;     // next reporting time (msec)
}   TLidRptFile;

// LID Unit - specific LID process applied over a given area
typedef struct
{
    int            lidIndex;      // index of LID process
    int            number;        // number of replicate units
    double         area;          // area of single replicate unit (ft2)
    double         fullWidth;     // full top width of single unit (ft)
    double         botWidth;      // bottom width of single unit (ft)
    double         initSat;       // initial saturation of soil & storage layers
    double         fromImperv;    // fraction of impervious area runoff treated
    int            toPerv;        // 1 if outflow sent to pervious area; 0 if not
    TLidRptFile*   rptFile;       // pointer to detailed report file

    TGrnAmpt       soilInfil;     // infil. object for biocell soil layer 
    double         surfaceDepth;  // depth of ponded water on surface layer (ft)
    double         soilMoisture;  // moisture content of biocell soil layer
    double         storageDepth;  // depth of water in storage layer (ft)
    double         oldFluxRates[MAX_STATE_VARS];  // net inflow - outflow from previous
                                     // time step for surface/soil(pavement)/
                                     // storage layers (ft/sec)
    double         dryTime;          // time since last rainfall (sec)
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
double   lid_getPervArea(int subcatch);
double   lid_getFlowToPerv(int subcatch);
double   lid_getStoredVolume(int subcatch);
double   lid_getSurfaceDepth(int subcatch);
double   lid_getDepthOnPavement(int subcatch, double impervDepth);
double   lid_getRunoff(int subcatch, double *outflow, double *evapVol,
         double *pervEvapVol, double *infilVol, double tStep);
void     lid_writeSummary(void);
void     lid_writeWaterBalance(void);
//-----------------------------------------------------------------------------
void     lidproc_initWaterBalance(TLidUnit *lidUnit, double initVol);
double   lidproc_getOutflow(TLidUnit* theUnit, TLidProc* theProc, double inflow,
	     double rain, double evap, double infil, double maxInfil, double tStep,
         double* lidEvap, double* lidInfil);
void     lidproc_saveResults(TLidUnit* theUnit, int saveResults,
         double ucfRainfall, double ucfRainDepth);

#endif
