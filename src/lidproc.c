//-----------------------------------------------------------------------------
//   lidproc.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/12   (Build 5.1.001)
//             05/19/14   (Build 5.1.006)
//             09/15/14   (Build 5.1.007)
//   Author:   L. Rossman (US EPA)
//
//   This module computes the hydrologic performance of an LID (Low Impact
//   Development) unit at a given point in time.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lid.h"
#include "headers.h"

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
#define STOPTOL  0.00328     //integration error tolerance in ft (= 1 mm)
#define MINFLOW  2.3e-8      //flow cutoff for dry conditions (= 0.001 in/hr)

//-----------------------------------------------------------------------------
//  Enumerations
//-----------------------------------------------------------------------------
enum LidLayerTypes {
    SURF,                    // surface layer
    SOIL,                    // soil layer
    STOR,                    // storage layer
    PAVE,                    // pavement layer
    DRAIN};                  // underdrain system

enum LidRptVars {
    SURF_INFLOW,             // inflow to surface layer
    TOTAL_EVAP,              // evaporation rate from all layers
    SURF_INFIL,              // infiltration into surface layer
    SOIL_PERC,               // percolation through soil layer
    STOR_INFIL,              // infiltration from storage layer
    SURF_OUTFLOW,            // outflow from surface layer
    STOR_DRAIN,              // outflow from storage layer
    SURF_DEPTH,              // ponded depth on surface layer
    SOIL_MOIST,              // moisture content of soil layer
    STOR_DEPTH,              // water level in storage layer
    MAX_RPT_VARS};

//-----------------------------------------------------------------------------
//  Local Variables
//-----------------------------------------------------------------------------
static TLidUnit*  theLidUnit;          // ptr. to a subcatchment's LID unit
static TLidProc*  theLidProc;          // ptr. to a LID process

static double     Tstep;               // current time step (sec)
static double     Rainfall;            // current rainfall rate (ft/s)
static double     EvapRate;            // evaporation rate (ft/s)
static double     NativeInfil;         // native soil infil. rate (ft/s)
static double     MaxNativeInfil;      // native soil infil. rate limit (ft/s)

static double     SurfaceInflow;       // precip. + runon to LID unit (ft/s)
static double     SurfaceInfil;        // infil. rate from surface layer (ft/s)
static double     SurfaceEvap;         // evap. rate from surface layer (ft/s)
static double     SurfaceOutflow;      // outflow from surface layer (ft/s)
static double     SurfaceVolume;       // volume in surface storage (ft)

static double     SoilEvap;            // evap. from soil layer (ft/s)
static double     SoilPerc;            // percolation from soil layer (ft/s)
static double     SoilVolume;          // volume in soil/pavement storage (ft)

static double     StorageInflow;       // inflow rate to storage layer (ft/s)
static double     StorageInfil;        // infil. rate from storage layer (ft/s)
static double     StorageEvap;         // evap.rate from storage layer (ft/s)
static double     StorageDrain;        // underdrain flow rate from storage layer (ft/s)
static double     StorageVolume;       // volume in storage layer (ft)

static double     Xold[MAX_STATE_VARS];

//-----------------------------------------------------------------------------
//  External Functions (declared in lid.h)
//-----------------------------------------------------------------------------
// lidproc_initWaterBalance  (called by lid_initState)
// lidproc_getOutflow        (called by evalLidUnit in lid.c)
// lidproc_saveResults       (called by evalLidUnit in lid.c)

//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
static void   barrelFluxRates(double x[], double f[]);
static void   biocellFluxRates(double x[], double f[]);
static void   greenRoofFluxRates(double x[], double f[]);
static void   pavementFluxRates(double x[], double f[]);
static void   trenchFluxRates(double x[], double f[]);
static void   swaleFluxRates(double x[], double f[]);

static double getSurfaceOutflowRate(double depth);
static double getSurfaceOverflowRate(double* surfaceDepth);
static double getPavementPermRate(void);
static double getSoilPercRate(double theta);                                   //(5.1.007)
static double getStorageInfilRate(void);
static double getStorageDrainRate(double head);
static double getDrainMatOutflow(double depth);
static void   getEvapRates(double surfaceVol, double soilVol,
                           double storageVol);

static void   updateWaterBalance(TLidUnit *lidUnit, double inflow,
                                 double evap, double infil, double surfFlow,
                                 double drainFlow, double storage);

static int    modpuls_solve(int n, double* x, double* xOld, double* xPrev,
                            double* xMin, double* xMax, double* xTol,
                            double* qOld, double* q, double dt, double omega,  //(5.1.007)
                            void (*derivs)(double*, double*));


//=============================================================================

void lidproc_initWaterBalance(TLidUnit *lidUnit, double initVol)
//
//  Purpose: initializes the water balance components of a LID unit.
//  Input:   lidUnit = a particular LID unit
//           initVol = initial water volume stored in the unit (ft)
//  Output:  none
//
{
    lidUnit->waterBalance.inflow = 0.0;
    lidUnit->waterBalance.evap = 0.0;
    lidUnit->waterBalance.infil = 0.0;
    lidUnit->waterBalance.surfFlow = 0.0;
    lidUnit->waterBalance.drainFlow = 0.0;
    lidUnit->waterBalance.initVol = initVol;
    lidUnit->waterBalance.finalVol = lidUnit->waterBalance.initVol;
}

//=============================================================================

double lidproc_getOutflow(TLidUnit* lidUnit, TLidProc* lidProc, double inflow,
                          double rain, double evap, double infil,
                          double maxInfil, double tStep, double* lidEvap,
                          double* lidInfil)
//
//  Purpose: computes runoff outflow from a single LID unit.
//  Input:   theUnit  = ptr. to specific LID unit being analyzed
//           theProc  = ptr. to generic LID process of the LID unit
//           inflow   = runoff rate captured by LID unit (ft/s)
//           rain     = current rainfall on LID unit (ft/s)
//           evap     = potential evaporation rate (ft/s)
//           infil    = infiltration rate of native soil (ft/s)
//           maxInfil = max. infiltration rate to native soil (ft/s)
//           tStep    = time step (sec)
//  Output:  lidEvap  = evaporation rate for LID unit (ft/s)
//           lidInfil = infiltration rate for LID unit (ft/s)
//           returns surface runoff rate from the LID unit (ft/s)
//
{
    int    i;
    double x[MAX_STATE_VARS];        // layer moisture levels
    double xOld[MAX_STATE_VARS];     // work vector 
    double xPrev[MAX_STATE_VARS];    // work vector
    double xMin[MAX_STATE_VARS];     // lower limit on moisture levels
    double xMax[MAX_STATE_VARS];     // upper limit on moisture levels
    double fOld[MAX_STATE_VARS];     // previously computed flux rates
    double f[MAX_STATE_VARS];        // newly computed flux rates
    double xTol[] =                  // convergence tolerance on moisture
        {STOPTOL, STOPTOL, STOPTOL}; // levels (ft, moisture fraction , ft)
    double omega = 0.0;              // integration time weighting             //(5.1.007)

    //... define a pointer to function that computes flux rates through the LID
    void (*fluxRates) (double *, double *) = NULL;

    //... save references to the LID process and LID unit
    theLidProc = lidProc;
    theLidUnit = lidUnit;

    //... save rain, evap, max. infil. & time step to shared variables
    Rainfall = rain;
    EvapRate = evap;
    MaxNativeInfil = maxInfil;
    Tstep = tStep;

    //... store current moisture levels in vector x
    x[SURF] = theLidUnit->surfaceDepth;
    x[SOIL] = theLidUnit->soilMoisture;
    x[STOR] = theLidUnit->storageDepth;

    //... initialize layer flux rates and moisture limits
    SurfaceInflow  = inflow;
    SurfaceInfil   = 0.0;
    SurfaceEvap    = 0.0;
    SurfaceOutflow = 0.0;
    SoilEvap       = 0.0;
    SoilPerc       = 0.0;
    StorageInflow  = 0.0;
    StorageInfil   = 0.0;
    StorageEvap    = 0.0;
    StorageDrain   = 0.0;
    for (i = 0; i < MAX_STATE_VARS; i++)
    {
        f[i] = 0.0;
        fOld[i] = theLidUnit->oldFluxRates[i];
        xMin[i] = 0.0;
        xMax[i] = BIG;
        Xold[i] = x[i];
    }

    //... find Green-Ampt infiltration from surface layer
    if ( theLidUnit->soilInfil.Ks > 0.0 )
    {
        SurfaceInfil =
            grnampt_getInfil(&theLidUnit->soilInfil, Tstep,
                             SurfaceInflow, theLidUnit->surfaceDepth);
    }
    else SurfaceInfil = infil;

    //... set moisture limits for soil & storage layers
    if ( theLidProc->soil.thickness > 0.0 )
    {
        xMin[SOIL] = theLidProc->soil.wiltPoint;
        xMax[SOIL] = theLidProc->soil.porosity;
    }
    if ( theLidProc->pavement.thickness > 0.0 )
    {
        xMax[SOIL] = theLidProc->pavement.voidFrac;
    }
    if ( theLidProc->storage.thickness > 0.0 )
    {
        xMax[STOR] = theLidProc->storage.thickness;
    }
    if ( theLidProc->lidType == GREEN_ROOF )
    {
        xMax[STOR] = theLidProc->drainMat.thickness;
    }

    //... determine which flux rate function to use
    switch (theLidProc->lidType)
    {
    case BIO_CELL:
    case RAIN_GARDEN:     fluxRates = &biocellFluxRates;  break;
    case GREEN_ROOF:      fluxRates = &greenRoofFluxRates; break;
    case INFIL_TRENCH:    fluxRates = &trenchFluxRates;   break;
    case POROUS_PAVEMENT: fluxRates = &pavementFluxRates; break;
    case RAIN_BARREL:     fluxRates = &barrelFluxRates;   break;
    case VEG_SWALE:       fluxRates = &swaleFluxRates;
                          omega = 0.5;                                         //(5.1.007)
                          break;
    default:              return 0.0;
    }

    //... update moisture levels and flux rates over the time step
    i = modpuls_solve(MAX_STATE_VARS, x, xOld, xPrev, xMin, xMax, xTol,
                     fOld, f, tStep, omega, fluxRates);                        //(5.1.007)

/** For debugging only ********************************************
    if  (i == 0) 
    {
        fprintf(Frpt.file,
        "\n  WARNING 09: integration failed to converge at %s %s",
            theDate, theTime);
        fprintf(Frpt.file,
        "\n              for LID %s placed in subcatchment %s.",
            theLidProc->ID, theSubcatch->ID);
    }
*******************************************************************/

    //... add any surface overflow to surface outflow
    if ( theLidProc->surface.canOverflow || theLidUnit->fullWidth == 0.0 )
    {
        SurfaceOutflow += getSurfaceOverflowRate(&x[SURF]);
    }

    //... save updated results
    theLidUnit->surfaceDepth = x[SURF];
    theLidUnit->soilMoisture = x[SOIL];
    theLidUnit->storageDepth = x[STOR];
    for (i = 0; i < MAX_STATE_VARS; i++) theLidUnit->oldFluxRates[i] = f[i];

    //... assign values to LID unit evaporation & infiltration
    *lidEvap = SurfaceEvap + SoilEvap + StorageEvap;
    *lidInfil = StorageInfil;

    //... return total outflow (per unit area) from unit
    return SurfaceOutflow + StorageDrain;
}    

//=============================================================================

void lidproc_saveResults(TLidUnit* lidUnit, int saveResults,
                         double ucfRainfall, double ucfRainDepth)
//
//  Purpose: updates the mass balance for an LID unit and saves
//           current flux rates to the LID report file.
//  Input:   lidUnit = ptr. to LID unit
//           runoffTime = current elapsed time for simulating runoff (msec)
//           reportStep = project's reporting time step (sec)
//           ucfRainfall = units conversion factor for rainfall rate
//           ucfDepth = units conversion factor for rainfall depth
//  Output:  none
//
{
    int    i;
    double prevReportTime;             // previous report time (msec) 
    double ucf;                        // units conversion factor
    double totalEvap;                  // total evaporation rate (ft/s)
    double totalVolume;                // total volume stored in LID (ft)
    double rptVars[MAX_RPT_VARS];      // array of reporting variables

    //... find total evap. rate and stored volume
    totalEvap = SurfaceEvap + SoilEvap + StorageEvap;
    totalVolume = SurfaceVolume + SoilVolume + StorageVolume;

    //... update mass balance totals
    updateWaterBalance(theLidUnit, SurfaceInflow, totalEvap, StorageInfil,
                       SurfaceOutflow, StorageDrain, totalVolume); 

    //... write results to LID report file
    if ( lidUnit->rptFile && saveResults )
    {
        //... check if dry-weather conditions hold
        if ( SurfaceInflow  < MINFLOW &&
             SurfaceOutflow < MINFLOW &&
             StorageDrain   < MINFLOW &&
             StorageInfil   < MINFLOW &&
             totalEvap      < MINFLOW ) return;

        //... convert rate results to original units (in/hr or mm/hr)
        ucf = ucfRainfall;
        rptVars[SURF_INFLOW]  = SurfaceInflow*ucf;
        rptVars[TOTAL_EVAP]   = totalEvap*ucf;
        rptVars[SURF_INFIL]   = SurfaceInfil*ucf;
        rptVars[SOIL_PERC]    = SoilPerc*ucf;
        rptVars[STOR_INFIL]   = StorageInfil*ucf;
        rptVars[SURF_OUTFLOW] = SurfaceOutflow*ucf;
        rptVars[STOR_DRAIN]   = StorageDrain*ucf;

        //... convert storage results to original units (in or mm)
        ucf = ucfRainDepth;
        rptVars[SURF_DEPTH] = theLidUnit->surfaceDepth*ucf;
        rptVars[SOIL_MOIST] = theLidUnit->soilMoisture;
        rptVars[STOR_DEPTH] = theLidUnit->storageDepth*ucf;

        //... one reporting period prior to current one
        prevReportTime = NewRunoffTime - (double)(ReportStep*1000);            //(5.1.006)

        //... if last reported time is prior to previous time
        //    then add a blank line to break the time series                   //(5.1.006)
        if ( theLidUnit->rptFile->lastReportTime < prevReportTime )           
            fprintf(theLidUnit->rptFile->file, "\n");                          //(5.1.006)                   

        //... write results to file
       // fprintf(theLidUnit->rptFile->file, "\n%7.3f\t",
       //         NewRunoffTime/1000.0/3600.0);
        fprintf(theLidUnit->rptFile->file, "\n%7.0f\t",
                NewRunoffTime/1000.0/60.0);
        fprintf(theLidUnit->rptFile->file, " %8.2f\t %8.4f\t",                 //(5.1.006)
            rptVars[SURF_INFLOW], rptVars[TOTAL_EVAP]);                        //(5.1.006)
        for ( i = SURF_INFIL; i <= STOR_DRAIN; i++)                            //(5.1.006)
                fprintf(theLidUnit->rptFile->file, " %8.2f\t", rptVars[i]);
        for ( i = SURF_DEPTH; i <= STOR_DEPTH; i++)
                fprintf(theLidUnit->rptFile->file, " %8.2f\t", rptVars[i]);
		theLidUnit->rptFile->lastReportTime = NewRunoffTime;
    }
}

//=============================================================================

////  This function was re-written for release 5.1.007.  ////                  //(5.1.007)

void greenRoofFluxRates(double x[], double f[])
//
//  Purpose: computes flux rates from the layers of a green roof.
//  Input:   x = vector of storage levels
//  Output:  f = vector of flux rates
//
{
    double surfaceDepth;
    double soilTheta;
    double storageDepth;
    double availVolume;
    double maxRate;

    //... retrieve state variables from work vector
    surfaceDepth = x[SURF];
    soilTheta = x[SOIL];
    storageDepth = x[STOR];

    //... convert state variables to volumes
    SurfaceVolume = surfaceDepth * theLidProc->surface.voidFrac;
    SoilVolume = soilTheta * theLidProc->soil.thickness;
    StorageVolume = storageDepth * theLidProc->storage.voidFrac;

    //... get ET rates
    availVolume = SoilVolume - theLidProc->soil.wiltPoint *
                   theLidProc->soil.thickness;
    getEvapRates(SurfaceVolume, availVolume, StorageVolume);

    //... no storage evap if soil layer saturated
    if ( soilTheta >= theLidProc->soil.porosity ) StorageEvap = 0.0;

    //... find soil layer perc rate
    SoilPerc = getSoilPercRate(soilTheta);

    //... find storage (drain mat) outflow rate
    StorageInfil = 0.0;
    StorageDrain = getDrainMatOutflow(storageDepth);

    //... both storage & soil layers are saturated
    if ( storageDepth >= theLidProc->storage.thickness &&
        soilTheta >= theLidProc->soil.porosity )
    {
        //... soil perc can't exceed storage outflow
        if ( SoilPerc > StorageDrain ) SoilPerc = StorageDrain;

        //... storage outflow can't exceed soil perc
        else StorageDrain = MIN(StorageDrain, SoilPerc);
    }

    //... storage and/or soil layers not saturated
    else
    {
        //... limit underdrain flow by volume above drain offset
        if ( StorageDrain > 0.0 )
        {
            maxRate = (storageDepth - theLidProc->drain.offset) *
                      theLidProc->storage.voidFrac / Tstep;
            StorageDrain = MIN(StorageDrain, maxRate);
        }

        //... limit soil perc by available storage volume
        availVolume = (theLidProc->storage.thickness - storageDepth) *
            theLidProc->storage.voidFrac;
        maxRate = availVolume/Tstep + StorageEvap + StorageDrain;
        SoilPerc = MIN(SoilPerc, maxRate);
    }

    //... limit surface infil. by available soil pore volume
    maxRate = (theLidProc->soil.porosity - soilTheta) * 
        theLidProc->soil.thickness / Tstep + SoilPerc;
    SurfaceInfil = MIN(SurfaceInfil, maxRate);

    // ... find surface outflow rate
    SurfaceOutflow = getSurfaceOutflowRate(surfaceDepth);

    // ... find net fluxes for each layer
    f[SURF] = (SurfaceInflow - SurfaceEvap - SurfaceInfil - SurfaceOutflow) /
              theLidProc->surface.voidFrac;
    f[SOIL] = (SurfaceInfil - SoilEvap - SoilPerc) /
              theLidProc->soil.thickness;
    f[STOR] = (SoilPerc - StorageEvap - StorageDrain) /
              theLidProc->storage.voidFrac;
}

//=============================================================================

////  This function was re-written for release 5.1.007.  ////                  //(5.1.007)

void biocellFluxRates(double x[], double f[])
//
//  Purpose: computes flux rates from the layers of a bio-retention cell LID.
//  Input:   x = vector of storage levels
//  Output:  f = vector of flux rates
//
{
    double surfaceDepth;
    double soilTheta;
    double storageDepth;
    double head;
    double availVolume;
    double maxRate;

    //... retrieve state variables from work vector
    surfaceDepth = x[SURF];
    soilTheta = x[SOIL];
    storageDepth = x[STOR];

    //... convert state variables to volumes
    SurfaceVolume = surfaceDepth * theLidProc->surface.voidFrac;
    SoilVolume = soilTheta * theLidProc->soil.thickness;
    StorageVolume = storageDepth * theLidProc->storage.voidFrac;

    //... get ET rates
    availVolume = SoilVolume - theLidProc->soil.wiltPoint *
                   theLidProc->soil.thickness;
    getEvapRates(SurfaceVolume, availVolume, StorageVolume);

    //... no storage evap if soil layer saturated
    if ( soilTheta >= theLidProc->soil.porosity ) StorageEvap = 0.0;

    //... find soil layer perc rate
    SoilPerc = getSoilPercRate(soilTheta);
    
    //... find infiltration rate out of storage layer
    StorageInfil = getStorageInfilRate();

    //... find underdrain flow rate
    StorageDrain = 0.0;
    head = storageDepth - theLidProc->drain.offset;
    if ( theLidProc->drain.coeff > 0.0 && head >= 0.0 )
    {
        if ( storageDepth >= theLidProc->storage.thickness )
        {
            head += (soilTheta - theLidProc->soil.fieldCap) /
                    (theLidProc->soil.porosity - theLidProc->soil.fieldCap) *
                    theLidProc->soil.thickness;
            if ( soilTheta >= theLidProc->soil.porosity ) head += surfaceDepth;
        }
        StorageDrain =  getStorageDrainRate(head);
    }

    //... special case of no storage layer present
    if ( theLidProc->storage.thickness == 0.0 )
    {
        StorageEvap = 0.0;
        maxRate = MIN(StorageInfil, SoilPerc);
        SoilPerc = maxRate;
        StorageInfil = maxRate;
    }

    //... both storage & soil layers are saturated
    else if ( storageDepth >= theLidProc->storage.thickness &&
        soilTheta >= theLidProc->soil.porosity )
    {
        //... soil perc can't exceed storage outflow
        maxRate = StorageDrain + StorageInfil;
        if ( SoilPerc > maxRate ) SoilPerc = maxRate;

        //... storage outflow can't exceed soil perc
        else
        {
            //... use up available drain capacity first
            StorageDrain = MIN(StorageDrain, SoilPerc);
            StorageInfil = SoilPerc - StorageDrain;
        }
    }

    //... layers not saturated
    else
    {
        //... limit underdrain flow by volume above drain offset
        if ( StorageDrain > 0.0 )
        {
            maxRate = (storageDepth - theLidProc->drain.offset) *
                      theLidProc->storage.voidFrac / Tstep;
            StorageDrain = MIN(StorageDrain, maxRate);
        }

        //... limit storage infil. by remaining volume
        maxRate = StorageVolume / Tstep - StorageDrain - StorageEvap;
        maxRate = MAX(0.0, maxRate);
        StorageInfil = MIN(StorageInfil, maxRate);

        //... limit soil perc by available storage volume
        availVolume = (theLidProc->storage.thickness - storageDepth) *
            theLidProc->storage.voidFrac;
        maxRate = availVolume/Tstep + StorageEvap + StorageDrain + StorageInfil;
        maxRate = MAX(maxRate, 0.0);
        SoilPerc = MIN(SoilPerc, maxRate);
    }

    //... limit surface infil. by available soil pore volume
    maxRate = (theLidProc->soil.porosity - soilTheta) * 
        theLidProc->soil.thickness / Tstep + SoilPerc;
    SurfaceInfil = MIN(SurfaceInfil, maxRate);

    //... find surface layer outflow rate
    SurfaceOutflow = getSurfaceOutflowRate(surfaceDepth);

    //... compute overall layer flux rates
    f[SURF] = (SurfaceInflow - SurfaceEvap - SurfaceInfil - SurfaceOutflow) /
              theLidProc->surface.voidFrac;
    f[SOIL] = (SurfaceInfil - SoilEvap - SoilPerc) /
              theLidProc->soil.thickness;
    f[STOR] = (SoilPerc - StorageEvap - StorageInfil - StorageDrain) /
              theLidProc->storage.voidFrac;
}

//=============================================================================

////  This function was re-written for release 5.1.007.  ////                  //(5.1.007)

void trenchFluxRates(double x[], double f[])
//
//  Purpose: computes flux rates from the layers of an infiltration trench LID.
//  Input:   x = vector of storage levels
//  Output:  f = vector of flux rates
//
{
    double surfaceDepth;
    double storageDepth;
    double head;
    double availVolume;
    double maxRate;

    //... extract zone depth levels from work vector
    surfaceDepth = x[SURF];
    storageDepth = x[STOR];

    //... convert depths to volumes
    SurfaceVolume = surfaceDepth * theLidProc->surface.voidFrac;
    SoilVolume = 0.0;
    StorageVolume = storageDepth * theLidProc->storage.voidFrac;
    availVolume = (theLidProc->storage.thickness - storageDepth) *
                  theLidProc->storage.voidFrac;

    //... nominal storage inflow
    StorageInflow = SurfaceInflow + SurfaceVolume / Tstep;

    //... get ET rate loss for each zone 
    getEvapRates(SurfaceVolume, 0.0, StorageVolume);

    //... no storage evap if surface ponded
    if ( surfaceDepth > 0.0 ) StorageEvap = 0.0;

    //... find infiltration rate out of storage layer
   StorageInfil = getStorageInfilRate();

    //... find underdrain flow rate
    StorageDrain = 0.0;
    head = storageDepth - theLidProc->drain.offset;
    if ( theLidProc->drain.coeff > 0.0 && head >= 0.0 )
    {
        if ( storageDepth >= theLidProc->storage.thickness )
        {
            head += surfaceDepth;
        }
        StorageDrain =  getStorageDrainRate(head);
    }

    //... limit underdrain flow by volume above drain offset
    if ( StorageDrain > 0.0 )
    {
        maxRate = (storageDepth - theLidProc->drain.offset) *
                  theLidProc->storage.voidFrac / Tstep;
        //... add on storage inflow if storage is full
        if ( storageDepth >= theLidProc->storage.thickness )
            maxRate += StorageInflow;
        StorageDrain = MIN(StorageDrain, maxRate);
    }

    //... limit storage infil. by remaining volume
    maxRate = StorageVolume / Tstep - StorageDrain - StorageEvap;
    maxRate = MAX(0.0, maxRate);
    StorageInfil = MIN(StorageInfil, maxRate);

    //... limit storage inflow by available storage volume
    availVolume = (theLidProc->storage.thickness - storageDepth) *
        theLidProc->storage.voidFrac;
    maxRate = availVolume/Tstep + StorageEvap + StorageDrain + StorageInfil;
    maxRate = MAX(maxRate, 0.0);
    StorageInflow = MIN(StorageInflow, maxRate);

    //... equate surface infil to storage inflow
    SurfaceInfil = StorageInflow;

    //... find surface outflow rate
    SurfaceOutflow = getSurfaceOutflowRate(surfaceDepth);

    // ... find net fluxes for each layer
    f[SURF] = SurfaceInflow - SurfaceEvap - StorageInflow - SurfaceOutflow /
              theLidProc->surface.voidFrac;;
    f[STOR] = (StorageInflow - StorageEvap - StorageInfil - StorageDrain) /
              theLidProc->storage.voidFrac;
    f[SOIL] = 0.0;
}

//=============================================================================

////  This function was re-written for release 5.1.007.  ////                  //(5.1.007)

void pavementFluxRates(double x[], double f[])
//
//  Purpose: computes flux rates from the layers of a porous pavement LID.
//  Input:   x = vector of storage levels
//  Output:  f = vector of flux rates
//
{
    double surfaceDepth;     // depth of water stored on surface (ft)
    double paveTheta;        // moisture content of pavement voids
    double storageDepth;     // depth of water in storage layer (ft)
    double pervVolume;       // volume/unit area of pervious pavement (ft)
    double pavePorosity;     // pavement porosity
    double availVolume;
    double maxRate;
    double head;

    //... retrieve state variables from work vector
    surfaceDepth = x[SURF];
    paveTheta = x[SOIL];
    storageDepth = x[STOR];
    pavePorosity = theLidProc->pavement.voidFrac;

    //... convert state variables to volumes
    //    (SoilVolume refers to pavement layer)
    SurfaceVolume = surfaceDepth * theLidProc->surface.voidFrac;
    pervVolume = theLidProc->pavement.thickness *
                 (1.0 - theLidProc->pavement.impervFrac);
    SoilVolume = paveTheta * pervVolume;
    StorageVolume = storageDepth * theLidProc->storage.voidFrac;

    //... get ET rates (arguments are stored volumes in ft)
    getEvapRates(SurfaceVolume, SoilVolume, StorageVolume);

    //... no storage evap if pavement layer saturated
    if ( paveTheta >= pavePorosity ) StorageEvap = 0.0;

    //... find nominal rate of surface infiltration into pavement
    SurfaceInfil = SurfaceInflow + (SurfaceVolume / Tstep);

    //... find pavement layer permeability (referred to as SoilPerc)
    SoilPerc = getPavementPermRate();

    //... limit pavement permeability to stored water + surface infil.
    maxRate = SoilVolume/Tstep + SurfaceInfil;
    SoilPerc = MIN(SoilPerc, maxRate);

    //... find infiltration rate out of storage layer
    StorageInfil = getStorageInfilRate();

    //... find underdrain flow rate
    StorageDrain = 0.0;
    head = storageDepth - theLidProc->drain.offset;
    if ( theLidProc->drain.coeff > 0.0 && head >= 0.0 )
    {
        if ( storageDepth >= theLidProc->storage.thickness )
        {
            head += (paveTheta) / pavePorosity * theLidProc->pavement.thickness;
            if ( paveTheta >= pavePorosity ) head += surfaceDepth;
        }
        StorageDrain =  getStorageDrainRate(head);
    }

    //... unit is flooded
    if ( storageDepth >= theLidProc->storage.thickness &&
        paveTheta >= pavePorosity && SurfaceInfil > MIN_RUNOFF )
    {
        //... pavement outflow can't exceed surface infil
        SoilPerc = MIN(SurfaceInfil, SoilPerc);

        //... pavement outflow can't exceed storage outflow
        maxRate = StorageEvap + StorageDrain + StorageInfil;
        if ( SoilPerc > maxRate )
        {
            SoilPerc = maxRate;
            SurfaceInfil = SoilPerc;
        }

        //... storage outflow can't exceed pavement perm.
        else
        {
            StorageDrain = MIN(StorageDrain, SoilPerc);
            StorageInfil = SoilPerc - StorageDrain;
        }
    }

    //... unit not flooded
    else
    {
        //... limit underdrain flow by volume above drain offset
        if ( StorageDrain > 0.0 )
        {
            maxRate = (storageDepth - theLidProc->drain.offset) *
                      theLidProc->storage.voidFrac / Tstep;
            StorageDrain = MIN(StorageDrain, maxRate);
        }

        //... limit storage infil. by remaining volume
        maxRate = StorageVolume / Tstep - StorageDrain - StorageEvap;
        maxRate = MAX(0.0, maxRate);
        StorageInfil = MIN(StorageInfil, maxRate);

        //... limit pavement outflow by available storage volume
        availVolume = (theLidProc->storage.thickness - storageDepth) *
            theLidProc->storage.voidFrac;
        maxRate = availVolume/Tstep + StorageEvap + StorageDrain + StorageInfil;
        maxRate = MAX(maxRate, 0.0);
        SoilPerc = MIN(SoilPerc, maxRate);

        //... limit pavement inflow by available pavement volume
        availVolume = (pavePorosity - paveTheta) * pervVolume;
        maxRate = availVolume / Tstep + SoilPerc;
        SurfaceInfil = MIN(SurfaceInfil, maxRate);
    }

    //... surface outflow
    SurfaceOutflow = getSurfaceOutflowRate(surfaceDepth);

    //... compute overall layer flux rates
    f[SURF] = SurfaceInflow - SurfaceEvap - SurfaceInfil - SurfaceOutflow;
    f[SOIL] = (SurfaceInfil - SoilEvap - SoilPerc) / pervVolume;
    f[STOR] = (SoilPerc - StorageEvap - StorageInfil - StorageDrain) /
              theLidProc->storage.voidFrac;
}

//=============================================================================

void swaleFluxRates(double x[], double f[])
//
//  Purpose: computes flux rates from a vegetative swale LID.
//  Input:   x = vector of storage levels
//  Output:  f = vector of flux rates
//
{
    double depth;            // depth of surface water in swale (ft)
    double topWidth;         // top width of full swale (ft)
    double botWidth;         // bottom width of swale (ft)
    double length;           // length of swale (ft)
    double surfInflow;       // inflow rate to swale (cfs)
    double surfWidth;        // top width at current water depth (ft)
    double surfArea;         // surface area of current water depth (ft2)
    double flowArea;         // x-section flow area (ft2)
    double lidArea;          // surface area of full swale (ft2)
    double hydRadius;        // hydraulic radius for current depth (ft)
    double slope;            // slope of swale side wall (run/rise)
    double volume;           // swale volume at current water depth (ft3)
    double dVdT;             // change in volume w.r.t. time (cfs)
    double dStore;           // depression storage depth (ft)
    double xDepth;           // depth above depression storage (ft)
    
    //... retrieve state variable from work vector
    depth = x[SURF];
    depth = MIN(depth, theLidProc->surface.thickness);

    //... depression storage depth
    dStore = 0.0;

    //... get swale's bottom width
    //    (0.5 ft minimum to avoid numerical problems)
    slope = theLidProc->surface.sideSlope;
    topWidth = theLidUnit->fullWidth;
    topWidth = MAX(topWidth, 0.5);
    botWidth = topWidth - 2.0 * slope * theLidProc->surface.thickness;
    if ( botWidth < 0.5 )
    {
        botWidth = 0.5;
        slope = 0.5 * (topWidth - 0.5) / theLidProc->surface.thickness;
    }

    //... swale's length
    lidArea = theLidUnit->area;
    length = lidArea / topWidth;

    //... top width, surface area and flow area of current ponded depth
    surfWidth = botWidth + 2.0 * slope * depth;
    surfArea = length * surfWidth;
    flowArea = (depth * (botWidth + slope * depth)) *
               theLidProc->surface.voidFrac;

    //... wet volume and effective depth
    volume = length * flowArea;

    //... surface inflow into swale (cfs)
    surfInflow = SurfaceInflow * lidArea;

    //... ET rate in cfs
    SurfaceEvap = EvapRate * surfArea;
    SurfaceEvap = MIN(SurfaceEvap, volume/Tstep);

    //... infiltration rate to native soil in cfs
    StorageInfil = SurfaceInfil * surfArea;

    //... no surface outflow if depth below depression storage
    xDepth = depth - dStore;
    if ( xDepth <= ZERO ) SurfaceOutflow = 0.0;

    //... otherwise compute a surface outflow
    else
    {
        //... modify flow area to remove depression storage,
        flowArea -= (dStore * (botWidth + slope * dStore)) *
                     theLidProc->surface.voidFrac;
        if ( flowArea < ZERO ) SurfaceOutflow = 0.0;
        else
        {
            //... compute hydraulic radius
            botWidth = botWidth + 2.0 * dStore * slope;
            hydRadius = botWidth + 2.0 * xDepth * sqrt(1.0 + slope*slope);
            hydRadius = flowArea / hydRadius;

            //... use Manning Eqn. to find outflow rate in cfs
            SurfaceOutflow = theLidProc->surface.alpha * flowArea *
                             pow(hydRadius, 2./3.);
        }
    }

    //... net flux rate (dV/dt) in cfs 
    dVdT = surfInflow - SurfaceEvap - StorageInfil - SurfaceOutflow;

    //... when full, any net positive inflow becomes spillage
    if ( depth == theLidProc->surface.thickness && dVdT > 0.0 )
    {
        SurfaceOutflow += dVdT;
        dVdT = 0.0;
    }

    //... convert flux rates to ft/s
    SurfaceEvap /= lidArea;
    StorageInfil /= lidArea;
    SurfaceOutflow /= lidArea;
    f[SURF] = dVdT / surfArea;
    f[SOIL] = 0.0;
    f[STOR] = 0.0;

    //... assign values to layer volumes
    SurfaceVolume = volume / lidArea;
    SoilVolume = 0.0;
    StorageVolume = 0.0;
}

//=============================================================================

////  This function was re-written for release 5.1.007.  ////                  //(5.1.007)

void barrelFluxRates(double x[], double f[])
//
//  Purpose: computes flux rates for a rain barrel LID.
//  Input:   x = vector of storage levels
//  Output:  f = vector of flux rates
//
{
    double storageDepth = x[STOR];
	double head;
    double maxValue;

    //... assign values to layer volumes
    SurfaceVolume = 0.0;
    SoilVolume = 0.0;
    StorageVolume = storageDepth;

    //... initialize flows
    SurfaceInfil = 0.0;
    SurfaceOutflow = 0.0;
    StorageDrain = 0.0;
    
    //... compute outflow if time since last rain exceeds drain delay
    //    (dryTime is updated in lid.evalLidUnit at each time step)
    if ( theLidProc->drain.delay == 0.0 || 
	     theLidUnit->dryTime >= theLidProc->drain.delay )
	{
	    head = storageDepth - theLidProc->drain.offset;
		if ( head > 0.0 )
	    {
	        StorageDrain = getStorageDrainRate(head);
		    maxValue = (head/Tstep);
			StorageDrain = MIN(StorageDrain, maxValue);
		}
	}

    //... limit inflow to available storage
    StorageInflow = SurfaceInflow;
    maxValue = (theLidProc->storage.thickness - storageDepth) / Tstep +
        StorageDrain;
    StorageInflow = MIN(StorageInflow, maxValue);
    SurfaceInfil = StorageInflow;

    //... assign values to layer flux rates
    f[SURF] = SurfaceInflow - StorageInflow;
    f[STOR] = StorageInflow - StorageDrain;
    f[SOIL] = 0.0;
}

//=============================================================================

double getSurfaceOutflowRate(double depth)
//
//  Purpose: computes outflow rate from a LID's surface layer.
//  Input:   depth = depth of ponded water on surface layer (ft)
//  Output:  returns outflow from surface layer (ft/s)
//
//  Note: this function should not be applied to swales or rain barrels.
//
{
    double delta;
    double outflow;

    //... no outflow if ponded depth below storage depth
    delta = depth - theLidProc->surface.thickness;
    if ( delta < 0.0 ) return 0.0;

    //... compute outflow from overland flow Manning equation
    outflow = theLidProc->surface.alpha * pow(delta, 5.0/3.0) *
              theLidUnit->fullWidth / theLidUnit->area;
    outflow = MIN(outflow, delta / Tstep);
    return outflow;
}

//=============================================================================

double getPavementPermRate()
//
//  Purpose: computes reduced permeability of a pavement layer due to
//           clogging.
//  Input:   none
//  Output:  returns the reduced permeability of the pavement layer (ft/s).
//
{
    double permRate;
    double permReduction;

    permReduction = theLidProc->pavement.clogFactor;
    if ( permReduction > 0.0 )
    {
        permReduction = theLidUnit->waterBalance.inflow / permReduction;
        permReduction = MIN(permReduction, 1.0);
    }
    permRate = theLidProc->pavement.kSat * (1.0 - permReduction);
    return permRate;
}

//=============================================================================
    
////  This function was modified for release 5.1.007.  ////                    //(5.1.007)

double getSoilPercRate(double theta)
//
//  Purpose: computes percolation rate of water through a LID's soil layer.
//  Input:   theta = moisture content (fraction)
//  Output:  returns percolation rate within soil layer (ft/s)
//
{
    double percRate;         // percolation rate (ft/s)
    double delta;            // moisture deficit
    double maxValue;         // max. allowable perc. rate (ft/s)

    // ... max. drainable soil moisture 
    maxValue = (theta - theLidProc->soil.fieldCap) * 
               theLidProc->soil.thickness / Tstep;
    if ( maxValue <= 0.0 ) return 0.0;

    // ... perc rate = unsaturated hydraulic conductivity
    delta = theLidProc->soil.porosity - theta;
    percRate = theLidProc->soil.kSat * exp(-delta * theLidProc->soil.kSlope);

    //... rate limited by drainable moisture content
    percRate = MIN(percRate, maxValue);
    return percRate;
}

//=============================================================================

double getStorageInfilRate()
//
//  Purpose: computes infiltration rate between storage zone and
//           native soil beneath a LID.
//  Input:   depth = depth of water storage zone (ft)
//  Output:  returns infiltration rate (ft/s)
//
{
    double infil = 0.0;
    double clogFactor = 0.0;

    if ( theLidProc->storage.kSat == 0.0 ) return 0.0;
    if ( MaxNativeInfil == 0.0 ) return 0.0;

    //... reduction due to clogging
    clogFactor = theLidProc->storage.clogFactor;
    if ( clogFactor > 0.0 )
    {
        clogFactor = theLidUnit->waterBalance.inflow / clogFactor;
        clogFactor = MIN(clogFactor, 1.0);
    }

    //... infiltration rate = storage Ksat reduced by any clogging
    infil = theLidProc->storage.kSat * (1.0 - clogFactor);

    //... limit infiltration rate by any groundwater-imposed limit
    return MIN(infil, MaxNativeInfil);
}

//=============================================================================

////  This function was modified for release 5.1.007.  ////                    //(5.1.007)

double  getStorageDrainRate(double head)
//
//  Purpose: computes underdrain flow rate in a LID's storage layer.
//  Input:   head = head of water above underdrain (ft)
//           inflow = rate of inflow to storage zone (ft/s)
//  Output:  returns flow in underdrain (ft/s)
//
//  Note:    drain eqn. is evaluated in user's units.
{
    double outflow = 0.0;

    if ( theLidProc->drain.coeff > 0.0 && head > ZERO )
    {
        // ... evaluate underdrain flow rate equation
        head *= UCF(RAINDEPTH);
        outflow = theLidProc->drain.coeff *
                  pow(head, theLidProc->drain.expon);
        outflow /= UCF(RAINFALL);
    }
    return outflow;
}

//=============================================================================

////  This function was modified for release 5.1.007.  ////                    //(5.1.007)

double getDrainMatOutflow(double depth)
{
    double result = SoilPerc;
    if ( theLidProc->drainMat.alpha > 0.0 )
    {
        depth *=  theLidProc->drainMat.voidFrac;
        result = theLidProc->drainMat.alpha * pow(depth, 5.0/3.0) *
                 theLidUnit->fullWidth / theLidUnit->area;
    }
    return result;
}

//=============================================================================

////  This function was re-written for release 5.1.007.  ////                  //(5.1.007)

void getEvapRates(double surfaceVol, double soilVol, double storageVol)
//
//  Purpose: computes surface, soil, and storage evaporation rates.
//  Input:   surfaceVol = volume of ponded water on surface layer (ft)
//           soilVol    = volume of water in soil (or pavement) pores (ft)
//           storageVol = volume of water in storage layer (ft)
//  Output:  none
//
{
    double availEvap;

    //... surface evaporation flux
    availEvap = EvapRate;
    SurfaceEvap = MIN(availEvap, surfaceVol/Tstep);
    SurfaceEvap = MAX(0.0, SurfaceEvap);
    availEvap = MAX(0.0, (availEvap - SurfaceEvap));

    //... no subsurface evap if water is infiltrating
    if ( SurfaceInfil > 0.0 )
    {
        SoilEvap = 0.0;
        StorageEvap = 0.0;
    }
    else
    {
        //... soil evaporation flux
        SoilEvap = MIN(availEvap, soilVol / Tstep);
        availEvap = MAX(0.0, (availEvap - SoilEvap));
    
        //... storage evaporation flux
        StorageEvap = MIN(availEvap, storageVol / Tstep);
    }
}

//=============================================================================

double getSurfaceOverflowRate(double* surfaceDepth)
//
//  Purpose: finds surface overflow rate from a LID unit.
//  Input:   surfaceDepth = depth of water stored in surface layer (ft)
//  Output:  returns the overflow rate (ft/s)
//
{
    double delta = *surfaceDepth - theLidProc->surface.thickness;
    if (  delta <= 0.0 ) return 0.0;
    *surfaceDepth = theLidProc->surface.thickness;
    return delta * theLidProc->surface.voidFrac / Tstep;
}

//=============================================================================

void updateWaterBalance(TLidUnit *lidUnit, double inflow, double evap,
    double infil, double surfFlow, double drainFlow, double storage)
//
//  Purpose: updates components of the water mass balance for a LID unit
//           over the current time step.
//  Input:   lidUnit   = a particular LID unit
//           inflow    = runon + rainfall to the LID unit (ft/s)
//           evap      = evaporation rate from the unit (ft/s)
//           infil     = infiltration out the bottom of the unit (ft/s)
//           surfFlow  = surface runoff from the unit (ft/s)
//           drainFlow = underdrain flow from the unit
//           storage   = volume of water stored in the unit (ft)
//  Output:  none
//
{
    lidUnit->waterBalance.inflow += inflow * Tstep;
    lidUnit->waterBalance.evap += evap * Tstep;
    lidUnit->waterBalance.infil += infil * Tstep;
    lidUnit->waterBalance.surfFlow += surfFlow * Tstep;
    lidUnit->waterBalance.drainFlow += drainFlow * Tstep;
    lidUnit->waterBalance.finalVol = storage;
}

//=============================================================================

int modpuls_solve(int n, double* x, double* xOld, double* xPrev,
                  double* xMin, double* xMax, double* xTol,
                  double* qOld, double* q, double dt, double omega,            //(5.1.007)
                  void (*derivs)(double*, double*))
//
//  Purpose: solves system of equations dx/dt = q(x) for x at end of time step
//           dt using a modified Puls method.
//  Input:   n = number of state variables
//           x = vector of state variables
//           xOld = state variable values at start of time step
//           xPrev = state variable values from previous iteration
//           xMin = lower limits on state variables
//           xMax = upper limits on state variables
//           xTol = convergence tolerances on state variables
//           qOld = flux rates at start of time step
//           q = flux rates at end of time step
//           dt = time step (sec)
//           omega = time weighting parameter (use 0 for Euler method          //(5.1.007)
//                   or 0.5 for modified Puls method)                          //(5.1.007)
//           derivs = pointer to function that computes flux rates q as a
//                    function of state variables x
//  Output:  returns number of steps required for convergence (or 0 if 
//           process doesn't converge)
//
{
    int i;
    int canStop;
    int steps = 1;
    int maxSteps = 20;

    //... initialize state variable values
    for (i=0; i<n; i++) 
    {
        xOld[i] = x[i];
        xPrev[i] = x[i];
    }

    //... repeat until convergence achieved
    while (steps < maxSteps)
    {
        //... compute flux rates for current state levels 
        canStop = 1;
        derivs(x, q);

        //... update state levels based on current flux rates
        for (i=0; i<n; i++)
        {
            x[i] = xOld[i] + (omega*qOld[i] + (1.0 - omega)*q[i]) * dt;
            x[i] = MIN(x[i], xMax[i]);
            x[i] = MAX(x[i], xMin[i]);

            if ( omega > 0.0 &&                                                //(5.1.007)
                 fabs(x[i] - xPrev[i]) > xTol[i] ) canStop = 0;
            xPrev[i] = x[i];
        }

        //... return if process converges
        if (canStop) return steps;
        steps++;
    }

    //... no convergence so return 0
    return 0;
}
