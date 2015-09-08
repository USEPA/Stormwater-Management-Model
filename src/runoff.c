//-----------------------------------------------------------------------------
//   runoff.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//             09/15/14   (Build 5.1.007)
//   Author:   L. Rossman
//
//   Runoff analysis functions.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "headers.h"
#include "odesolve.h"

//-----------------------------------------------------------------------------
// Shared variables
//-----------------------------------------------------------------------------
static char  IsRaining;                // TRUE if precip.falls on study area
static char  HasRunoff;                // TRUE if study area generates runoff
static char  HasSnow;                  // TRUE if any snow cover on study area
static int   Nsteps;                   // number of runoff time steps taken
static int   MaxSteps;                 // final number of runoff time steps
static long  MaxStepsPos;              // position in Runoff interface file
                                       //    where MaxSteps is saved

//-----------------------------------------------------------------------------
//  Exportable variables (shared with subcatch.c)
//-----------------------------------------------------------------------------
double* OutflowLoad;         // outflow pollutant mass from a subcatchment 
double* WashoffLoad;         // washoff pollutant mass from landuses

//-----------------------------------------------------------------------------
//  Imported variables
//-----------------------------------------------------------------------------
extern float* SubcatchResults;         // Results vector defined in OUTPUT.C

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
// runoff_open     (called from swmm_start in swmm5.c)
// runoff_execute  (called from swmm_step in swmm5.c)
// runoff_close    (called from swmm_end in swmm5.c)

//-----------------------------------------------------------------------------
// Local functions
//-----------------------------------------------------------------------------
static double runoff_getTimeStep(DateTime currentDate);
static void   runoff_initFile(void);
static void   runoff_readFromFile(void);
static void   runoff_saveToFile(float tStep);


//=============================================================================

int runoff_open()
//
//  Input:   none
//  Output:  returns the global error code
//  Purpose: opens the runoff analyzer.
//
{
    IsRaining = FALSE;
    HasRunoff = FALSE;
    HasSnow = FALSE;
    Nsteps = 0;

    // --- open the Ordinary Differential Equation solver
    if ( !odesolve_open(MAXODES) ) report_writeErrorMsg(ERR_ODE_SOLVER, "");

    // --- allocate memory for pollutant washoff loads
    OutflowLoad = NULL;
    WashoffLoad = NULL;
    if ( Nobjects[POLLUT] > 0 )
    {
        OutflowLoad = (double *) calloc(Nobjects[POLLUT], sizeof(double));
        if ( !OutflowLoad ) report_writeErrorMsg(ERR_MEMORY, "");
        WashoffLoad = (double *) calloc(Nobjects[POLLUT], sizeof(double));
        if ( !WashoffLoad ) report_writeErrorMsg(ERR_MEMORY, "");
    }

    // --- see if a runoff interface file should be opened
    switch ( Frunoff.mode )
    {
      case USE_FILE:
        if ( (Frunoff.file = fopen(Frunoff.name, "r+b")) == NULL)
            report_writeErrorMsg(ERR_RUNOFF_FILE_OPEN, Frunoff.name);
        else runoff_initFile();
        break;
      case SAVE_FILE:
        if ( (Frunoff.file = fopen(Frunoff.name, "w+b")) == NULL)
            report_writeErrorMsg(ERR_RUNOFF_FILE_OPEN, Frunoff.name);
        else runoff_initFile();
        break;
    }

////  Call to climate_openFile() moved to climate_validate().  ////            //(5.1.007)
    return ErrorCode;
}

//=============================================================================

void runoff_close()
//
//  Input:   none
//  Output:  none
//  Purpose: closes the runoff analyzer.
//
{
    // --- close the ODE solver
    odesolve_close();

    // --- free memory for pollutant washoff loads
    FREE(OutflowLoad);
    FREE(WashoffLoad);

    // --- close runoff interface file if in use
    if ( Frunoff.file )
    {
        // --- write to file number of time steps simulated
        if ( Frunoff.mode == SAVE_FILE )
        {
            fseek(Frunoff.file, MaxStepsPos, SEEK_SET);
            fwrite(&Nsteps, sizeof(int), 1, Frunoff.file);
        }
        fclose(Frunoff.file);
    }

    // --- close climate file if in use
    if ( Fclimate.file ) fclose(Fclimate.file);
}

//=============================================================================

void runoff_execute()
//
//  Input:   none
//  Output:  none
//  Purpose: computes runoff from each subcatchment at current runoff time.
//
{
    int      j;                        // object index
    int      day;                      // day of calendar year
    double    runoffStep;              // runoff time step (sec)
    double    runoff;                  // subcatchment runoff (ft/sec)
    DateTime currentDate;              // current date/time 
    char     canSweep;                 // TRUE if street sweeping can occur

    if ( ErrorCode ) return;

    // --- convert elapsed runoff time in milliseconds to a calendar date
    currentDate = getDateTime(NewRunoffTime);

    // --- update climatological conditions
    climate_setState(currentDate);

    // --- if no subcatchments then simply update runoff elapsed time
    if ( Nobjects[SUBCATCH] == 0 )
    {
        OldRunoffTime = NewRunoffTime;
        NewRunoffTime += (double)(1000.0 * DryStep);
        return;
    }

    // --- update current rainfall at each raingage
    //     NOTE: must examine gages in sequential order due to possible
    //     presence of co-gages (gages that share same rain time series).
    IsRaining = FALSE;
    for (j = 0; j < Nobjects[GAGE]; j++)
    {
        gage_setState(j, currentDate);
        if ( Gage[j].rainfall > 0.0 ) IsRaining = TRUE;
    }

    // --- read runoff results from interface file if applicable
    if ( Frunoff.mode == USE_FILE )
    {
        runoff_readFromFile();
        return;
    }

    // --- see if street sweeping can occur on current date
    day = datetime_dayOfYear(currentDate);
    if ( day >= SweepStart && day <= SweepEnd ) canSweep = TRUE;
    else canSweep = FALSE;

    // --- get runoff time step (in seconds)
    runoffStep = runoff_getTimeStep(currentDate);
    if ( runoffStep <= 0.0 )
    {
        ErrorCode = ERR_TIMESTEP;
        return;
    }

    // --- update runoff time clock (in milliseconds)
    OldRunoffTime = NewRunoffTime;
    NewRunoffTime += (double)(1000.0 * runoffStep);

    // --- update old state of each subcatchment, 
    for (j = 0; j < Nobjects[SUBCATCH]; j++) subcatch_setOldState(j);

    // --- determine runon from upstream subcatchments, and implement snow removal
    for (j = 0; j < Nobjects[SUBCATCH]; j++)
    {
        subcatch_getRunon(j);
        if ( !IgnoreSnowmelt ) snow_plowSnow(j, runoffStep);
    }
    
    // --- determine runoff and pollutant buildup/washoff in each subcatchment
    HasSnow = FALSE;
    HasRunoff = FALSE;
    for (j = 0; j < Nobjects[SUBCATCH]; j++)
    {
        // --- find total runoff rate (in ft/sec) over the subcatchment
        //     (the amount that actually leaves the subcatchment (in cfs)
        //     is also computed and is stored in Subcatch[j].newRunoff)
        runoff = subcatch_getRunoff(j, runoffStep);

        // --- update state of study area surfaces
        if ( runoff > 0.0 ) HasRunoff = TRUE;
        if ( Subcatch[j].newSnowDepth > 0.0 ) HasSnow = TRUE;

        // --- skip pollutant buildup/washoff if quality ignored
        if ( IgnoreQuality ) continue;

        // --- now assign 'runoff' to runoff that leaves the subcatchment
        if (Subcatch[j].area > 0.0)
            runoff = Subcatch[j].newRunoff / Subcatch[j].area;

        // --- add to pollutant buildup if runoff is negligible
        if ( runoff < MIN_RUNOFF ) subcatch_getBuildup(j, runoffStep); 

        // --- reduce buildup by street sweeping
        if ( canSweep && Subcatch[j].rainfall <= MIN_RUNOFF)
            subcatch_sweepBuildup(j, currentDate);

        // --- compute pollutant washoff 
        subcatch_getWashoff(j, runoff, runoffStep);
    }

    // --- update tracking of system-wide max. runoff rate
    stats_updateMaxRunoff();

    // --- save runoff results to interface file if one is used
    Nsteps++;
    if ( Frunoff.mode == SAVE_FILE )
    {
        runoff_saveToFile((float)runoffStep);
    }

    // --- reset subcatchment runon to 0
    for (j = 0; j < Nobjects[SUBCATCH]; j++) Subcatch[j].runon = 0.0;
}

//=============================================================================

double runoff_getTimeStep(DateTime currentDate)
//
//  Input:   currentDate = current simulation date/time
//  Output:  time step (sec)
//  Purpose: computes a time step to use for runoff calculations.
//
{
    int  j;
    long timeStep;
    long maxStep = DryStep;

    // --- find shortest time until next evaporation or rainfall value
    //     (this represents the maximum possible time step)
    timeStep = datetime_timeDiff(climate_getNextEvap(currentDate), currentDate);
    if ( timeStep < maxStep ) maxStep = timeStep;
    for (j = 0; j < Nobjects[GAGE]; j++)
    {
        timeStep = datetime_timeDiff(gage_getNextRainDate(j, currentDate),
                   currentDate);
        if ( timeStep > 0 && timeStep < maxStep ) maxStep = timeStep;
    }

    // --- determine whether wet or dry time step applies
    if ( IsRaining || HasSnow || HasRunoff ) timeStep = WetStep;
    else timeStep = DryStep;

    // --- limit time step if necessary
    if ( timeStep > maxStep ) timeStep = maxStep;
    return (double)timeStep;
}

//=============================================================================

void runoff_initFile(void)
//
//  Input:   none
//  Output:  none
//  Purpose: initializes a Runoff Interface file for saving results.
//
{
    int   nSubcatch;
    int   nPollut;
    int   flowUnits;
    char  fileStamp[] = "SWMM5-RUNOFF";
    char  fStamp[] = "SWMM5-RUNOFF";

    MaxSteps = 0;
    if ( Frunoff.mode == SAVE_FILE )
    {
        // --- write file stamp, # subcatchments & # pollutants to file
        nSubcatch = Nobjects[SUBCATCH];
        nPollut = Nobjects[POLLUT];
        flowUnits = FlowUnits;
        fwrite(fileStamp, sizeof(char), strlen(fileStamp), Frunoff.file);
        fwrite(&nSubcatch, sizeof(int), 1, Frunoff.file);
        fwrite(&nPollut, sizeof(int), 1, Frunoff.file);
        fwrite(&flowUnits, sizeof(int), 1, Frunoff.file);
        MaxStepsPos = ftell(Frunoff.file); 
        fwrite(&MaxSteps, sizeof(int), 1, Frunoff.file);
    }

    if ( Frunoff.mode == USE_FILE )
    {
        // --- check that interface file contains proper header records
        fread(fStamp, sizeof(char), strlen(fileStamp), Frunoff.file);
        if ( strcmp(fStamp, fileStamp) != 0 )
        {
            report_writeErrorMsg(ERR_RUNOFF_FILE_FORMAT, "");
            return;
        }
        nSubcatch = -1;
        nPollut = -1;
        flowUnits = -1;
        fread(&nSubcatch, sizeof(int), 1, Frunoff.file);
        fread(&nPollut, sizeof(int), 1, Frunoff.file);
        fread(&flowUnits, sizeof(int), 1, Frunoff.file);
        fread(&MaxSteps, sizeof(int), 1, Frunoff.file);
        if ( nSubcatch != Nobjects[SUBCATCH]
        ||   nPollut   != Nobjects[POLLUT]
        ||   flowUnits != FlowUnits
        ||   MaxSteps  <= 0 )
        {
             report_writeErrorMsg(ERR_RUNOFF_FILE_FORMAT, "");
        }
    }
}

//=============================================================================

void  runoff_saveToFile(float tStep)
//
//  Input:   tStep = runoff time step (sec)
//  Output:  none
//  Purpose: saves current runoff results to Runoff Interface file.
//
{
    int j;
    int n = MAX_SUBCATCH_RESULTS + Nobjects[POLLUT] - 1;
    

    fwrite(&tStep, sizeof(float), 1, Frunoff.file);
    for (j=0; j<Nobjects[SUBCATCH]; j++)
    {
        subcatch_getResults(j, 1.0, SubcatchResults);
        fwrite(SubcatchResults, sizeof(float), n, Frunoff.file);
    }
}

//=============================================================================

void  runoff_readFromFile(void)
//
//  Input:   none
//  Output:  none
//  Purpose: reads runoff results from Runoff Interface file for current time.
//
{
    int    i, j;
    int    nResults;                   // number of results per subcatch.
    int    kount;                      // count of items read from file
    float  tStep;                      // runoff time step (sec)
    TGroundwater* gw;                  // ptr. to Groundwater object

    // --- make sure not past end of file
    if ( Nsteps > MaxSteps )
    {
         report_writeErrorMsg(ERR_RUNOFF_FILE_END, "");
         return;
    }

    // --- replace old state with current one for all subcatchments
    for (j = 0; j < Nobjects[SUBCATCH]; j++) subcatch_setOldState(j);

    // --- read runoff time step
    kount = 0;
    kount += fread(&tStep, sizeof(float), 1, Frunoff.file);

    // --- compute number of results saved for each subcatchment
    nResults = MAX_SUBCATCH_RESULTS + Nobjects[POLLUT] - 1;

    // --- for each subcatchment
    for (j = 0; j < Nobjects[SUBCATCH]; j++)
    {
        // --- read vector of saved results
        kount += fread(SubcatchResults, sizeof(float), nResults, Frunoff.file);

        // --- extract hydrologic results, converting units where necessary
        //     (results were saved to file in user's units)
        Subcatch[j].newSnowDepth = SubcatchResults[SUBCATCH_SNOWDEPTH] /
                                   UCF(RAINDEPTH);
        Subcatch[j].evapLoss     = SubcatchResults[SUBCATCH_EVAP] /
                                   UCF(RAINFALL);
        Subcatch[j].infilLoss    = SubcatchResults[SUBCATCH_INFIL] /
                                   UCF(RAINFALL);
        Subcatch[j].newRunoff    = SubcatchResults[SUBCATCH_RUNOFF] /
                                   UCF(FLOW);
        gw = Subcatch[j].groundwater;
        if ( gw )
        {
            gw->newFlow    = SubcatchResults[SUBCATCH_GW_FLOW] / UCF(FLOW);
            gw->lowerDepth = Aquifer[gw->aquifer].bottomElev -
                             (SubcatchResults[SUBCATCH_GW_ELEV] / UCF(LENGTH));
            gw->theta      = SubcatchResults[SUBCATCH_SOIL_MOIST];
        }

        // --- extract water quality results
        for (i = 0; i < Nobjects[POLLUT]; i++)
        {
            Subcatch[j].newQual[i] = SubcatchResults[SUBCATCH_WASHOFF + i];
        }
    }

    // --- report error if not enough values were read
    if ( kount < 1 + Nobjects[SUBCATCH] * nResults )
    {
         report_writeErrorMsg(ERR_RUNOFF_FILE_READ, "");
         return;
    }

    // --- update runoff time clock
    OldRunoffTime = NewRunoffTime;
    NewRunoffTime = OldRunoffTime + (double)(tStep)*1000.0;
    Nsteps++;
}

//=============================================================================
