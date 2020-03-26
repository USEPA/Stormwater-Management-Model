//-----------------------------------------------------------------------------
//   subcatch.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/19/14  (Build 5.1.000)
//             04/19/14  (Build 5.1.006)
//             03/19/15  (Build 5.1.008)
//             04/30/15  (Build 5.1.009)
//             08/05/15  (Build 5.1.010)
//             08/01/16  (Build 5.1.011)
//             03/14/17  (Build 5.1.012)
//             05/10/18  (Build 5.1.013)
//   Author:   L. Rossman
//
//   Subcatchment runoff functions.
//
//   Build 5.1.008:
//   - Support added for keeping separate track of drain outflows from LIDs.
//   - Processing of inflow/outflow volumes over a time step was refactored. 
//   - Reported subcatchment runoff includes both surface runoff and LID
//     drain flows, even though latter can be routed elsewhere.
//   - Runon now distributed only over non-LID area of a subcatchment, unless
//     LID covers full area.
//   - Pollutant buildup and washoff functions were moved to surfqual.c.
//
//   Build 5.1.009:
//   - Runon for full LID subcatchment added to statistical summary.
//
//   Build 5.1.010:
//   - Fixed a bug introduced in 5.1.008 that forgot to include LID
//     exfiltration as inflow sent to GW routine.
//
//   Build 5.1.011:
//   - Subcatchment percent imperviousness not allowed to exceed 100.
//
//   Build 5.1.012:
//   - Subcatchment bottom elevation used instead of aquifer's when
//     saving water table value to results file.
//
//   Build 5.1.013:
//   - Rain gage isUsed property now set in subcatch_validate().
//   - Cumulative impervious and pervious area runoff volumes added
//     to subcatchment statistics.
//   - Support added for monthly adjustment of subcatchment's depression
//     storage, pervious N, and infiltration.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include <string.h>
#include "headers.h"
#include "lid.h"
#include "odesolve.h"

//-----------------------------------------------------------------------------
// Constants 
//-----------------------------------------------------------------------------
const double MCOEFF    = 1.49;              // constant in Manning Eq.
const double MEXP      = 1.6666667;         // exponent in Manning Eq.
const double ODETOL    = 0.0001;            // acceptable error for ODE solver

//-----------------------------------------------------------------------------
// Globally shared variables   
//-----------------------------------------------------------------------------
// Volumes (ft3) for a subcatchment over a time step
double     Vevap;         // evaporation
double     Vpevap;        // pervious area evaporation
double     Vinfil;        // non-LID infiltration
double     Vinflow;       // non-LID precip + snowmelt + runon + ponded water
double     Voutflow;      // non-LID runoff to subcatchment's outlet
double     VlidIn;        // impervious area flow to LID units
double     VlidInfil;     // infiltration from LID units
double     VlidOut;       // surface outflow from LID units
double     VlidDrain;     // drain outflow from LID units
double     VlidReturn;    // LID outflow returned to pervious area

//-----------------------------------------------------------------------------
// Locally shared variables   
//-----------------------------------------------------------------------------
static  TSubarea* theSubarea;     // subarea to which getDdDt() is applied
static  double    Dstore;         // monthly adjusted depression storage (ft)  //(5.1.013)
static  double    Alpha;          // monthly adjusted runoff coeff.            //
static  char *RunoffRoutingWords[] = { w_OUTLET,  w_IMPERV, w_PERV, NULL};

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)   
//-----------------------------------------------------------------------------
//  subcatch_readParams        (called from parseLine in input.c)
//  subcatch_readSubareaParams (called from parseLine in input.c)
//  subcatch_readLanduseParams (called from parseLine in input.c)
//  subcatch_readInitBuildup   (called from parseLine in input.c)

//  subcatch_validate          (called from project_validate)
//  subcatch_initState         (called from project_init)

//  subcatch_setOldState       (called from runoff_execute)
//  subcatch_getRunon          (called from runoff_execute)
//  subcatch_addRunon          (called from subcatch_getRunon,
//                              lid_addDrainRunon, & runoff_getOutfallRunon)
//  subcatch_getRunoff         (called from runoff_execute)
//  subcatch_hadRunoff         (called from runoff_execute)

//  subcatch_getFracPerv       (called from gwater_initState)
//  subcatch_getStorage        (called from massbal_getRunoffError)
//  subcatch_getDepth          (called from findPondedLoads in surfqual.c)

//  subcatch_getWtdOutflow     (called from addWetWeatherInflows in routing.c)
//  subcatch_getResults        (called from output_saveSubcatchResults)

//-----------------------------------------------------------------------------
// Function declarations
//-----------------------------------------------------------------------------
static void   getNetPrecip(int j, double* netPrecip, double tStep);
static double getSubareaRunoff(int subcatch, int subarea, double area,
              double rainfall, double evap, double tStep);
static double getSubareaInfil(int j, TSubarea* subarea, double precip,
              double tStep);
static double findSubareaRunoff(TSubarea* subarea, double tRunoff);
static void   updatePondedDepth(TSubarea* subarea, double* tx);
static void   getDdDt(double t, double* d, double* dddt);
static void   adjustSubareaParams(int subareaType, int subcatch);              //(5.1.013)

//=============================================================================

int  subcatch_readParams(int j, char* tok[], int ntoks)
//
//  Input:   j = subcatchment index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads subcatchment parameters from a tokenized  line of input data.
//
//  Data has format:
//    Name  RainGage  Outlet  Area  %Imperv  Width  Slope CurbLength  Snowpack  
//
{
    int    i, k, m;
    char*  id;
    double x[9];

    // --- check for enough tokens
    if ( ntoks < 8 ) return error_setInpError(ERR_ITEMS, "");

    // --- check that named subcatch exists
    id = project_findID(SUBCATCH, tok[0]);
    if ( id == NULL ) return error_setInpError(ERR_NAME, tok[0]);

    // --- check that rain gage exists
    k = project_findObject(GAGE, tok[1]);
    if ( k < 0 ) return error_setInpError(ERR_NAME, tok[1]);
    x[0] = k;

    // --- check that outlet node or subcatch exists
    m = project_findObject(NODE, tok[2]);
    x[1] = m;
    m = project_findObject(SUBCATCH, tok[2]);
    x[2] = m;
    if ( x[1] < 0.0 && x[2] < 0.0 )
        return error_setInpError(ERR_NAME, tok[2]);

    // --- read area, %imperv, width, slope, & curb length
    for ( i = 3; i < 8; i++)
    {
        if ( ! getDouble(tok[i], &x[i]) || x[i] < 0.0 )
            return error_setInpError(ERR_NUMBER, tok[i]);
    }

    // --- if snowmelt object named, check that it exists
    x[8] = -1;
    if ( ntoks > 8 )
    {
        k = project_findObject(SNOWMELT, tok[8]);
        if ( k < 0 ) return error_setInpError(ERR_NAME, tok[8]);
        x[8] = k;
    }

    // --- assign input values to subcatch's properties
    Subcatch[j].ID = id;
    Subcatch[j].gage        = (int)x[0];
    Subcatch[j].outNode     = (int)x[1];
    Subcatch[j].outSubcatch = (int)x[2];
    Subcatch[j].area        = x[3] / UCF(LANDAREA);
    Subcatch[j].fracImperv  = MIN(x[4], 100.0) / 100.0;
    Subcatch[j].width       = x[5] / UCF(LENGTH);
    Subcatch[j].slope       = x[6] / 100.0;
    Subcatch[j].curbLength  = x[7];
    Subcatch[j].nPervPattern  = -1;                                            //(5.1.013
    Subcatch[j].dStorePattern = -1;                                            //
    Subcatch[j].infilPattern  = -1;                                            //

    // --- create the snow pack object if it hasn't already been created
    if ( x[8] >= 0 )
    {
        if ( !snow_createSnowpack(j, (int)x[8]) )
            return error_setInpError(ERR_MEMORY, "");
    }
    return 0;
}

//=============================================================================

int subcatch_readSubareaParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads subcatchment's subarea parameters from a tokenized 
//           line of input data.
//
//  Data has format:
//    Subcatch  Imperv_N  Perv_N  Imperv_S  Perv_S  PctZero  RouteTo (PctRouted)
//
{
    int    i, j, k, m;
    double x[7];

    // --- check for enough tokens
    if ( ntoks < 7 ) return error_setInpError(ERR_ITEMS, "");

    // --- check that named subcatch exists
    j = project_findObject(SUBCATCH, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);

    // --- read in Mannings n, depression storage, & PctZero values
    for (i = 0; i < 5; i++)
    {
        if ( ! getDouble(tok[i+1], &x[i])  || x[i] < 0.0 )
            return error_setInpError(ERR_NAME, tok[i+1]);
    }

    // --- check for valid runoff routing keyword
    m = findmatch(tok[6], RunoffRoutingWords);
    if ( m < 0 ) return error_setInpError(ERR_KEYWORD, tok[6]);

    // --- get percent routed parameter if present (default is 100)
    x[5] = m;
    x[6] = 1.0;
    if ( ntoks >= 8 )
    {
        if ( ! getDouble(tok[7], &x[6]) || x[6] < 0.0 || x[6] > 100.0 )
            return error_setInpError(ERR_NUMBER, tok[7]);
        x[6] /= 100.0;
    }

    // --- assign input values to each type of subarea
    Subcatch[j].subArea[IMPERV0].N = x[0];
    Subcatch[j].subArea[IMPERV1].N = x[0];
    Subcatch[j].subArea[PERV].N    = x[1];

    Subcatch[j].subArea[IMPERV0].dStore = 0.0;
    Subcatch[j].subArea[IMPERV1].dStore = x[2] / UCF(RAINDEPTH);
    Subcatch[j].subArea[PERV].dStore    = x[3] / UCF(RAINDEPTH);

    Subcatch[j].subArea[IMPERV0].fArea  = Subcatch[j].fracImperv * x[4] / 100.0;
    Subcatch[j].subArea[IMPERV1].fArea  = Subcatch[j].fracImperv * (1.0 - x[4] / 100.0);
    Subcatch[j].subArea[PERV].fArea     = (1.0 - Subcatch[j].fracImperv);

    // --- assume that all runoff from each subarea goes to subcatch outlet
    for (i = IMPERV0; i <= PERV; i++)
    {
        Subcatch[j].subArea[i].routeTo = TO_OUTLET;
        Subcatch[j].subArea[i].fOutlet = 1.0;
    }

    // --- modify routing if pervious runoff routed to impervious area
    //     (fOutlet is the fraction of runoff not routed)
    
    k = (int)x[5];
    if ( Subcatch[j].fracImperv == 0.0
    ||   Subcatch[j].fracImperv == 1.0 ) k = TO_OUTLET;
    if ( k == TO_IMPERV && Subcatch[j].fracImperv )
    {
        Subcatch[j].subArea[PERV].routeTo = k;
        Subcatch[j].subArea[PERV].fOutlet = 1.0 - x[6];
    }

    // --- modify routing if impervious runoff routed to pervious area
    if ( k == TO_PERV )
    {
        Subcatch[j].subArea[IMPERV0].routeTo = k;
        Subcatch[j].subArea[IMPERV1].routeTo = k;
        Subcatch[j].subArea[IMPERV0].fOutlet = 1.0 - x[6];
        Subcatch[j].subArea[IMPERV1].fOutlet = 1.0 - x[6];
    }
    return 0;
}

//=============================================================================

int subcatch_readLanduseParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads assignment of landuses to subcatchment from a tokenized 
//           line of input data.
//
//  Data has format:
//    Subcatch  landuse  percent .... landuse  percent
//
{
    int     j, k, m;
    double  f;

    // --- check for enough tokens
    if ( ntoks < 3 ) return error_setInpError(ERR_ITEMS, "");

    // --- check that named subcatch exists
    j = project_findObject(SUBCATCH, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);

    // --- process each pair of landuse - percent items
    for ( k = 2; k <= ntoks; k = k+2)
    {
        // --- check that named land use exists and is followed by a percent
        m = project_findObject(LANDUSE, tok[k-1]);
        if ( m < 0 ) return error_setInpError(ERR_NAME, tok[k-1]);
        if ( k+1 > ntoks ) return error_setInpError(ERR_ITEMS, "");
        if ( ! getDouble(tok[k], &f) )
            return error_setInpError(ERR_NUMBER, tok[k]);

        // --- store land use fraction in subcatch's landFactor property
        Subcatch[j].landFactor[m].fraction = f/100.0;
    }
    return 0;
}

//=============================================================================

int subcatch_readInitBuildup(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads initial pollutant buildup on subcatchment from 
//           tokenized line of input data.
//
//  Data has format:
//    Subcatch  pollut  initLoad .... pollut  initLoad
//
{
    int     j, k, m;
    double  x;

    // --- check for enough tokens
    if ( ntoks < 3 ) return error_setInpError(ERR_ITEMS, "");

    // --- check that named subcatch exists
    j = project_findObject(SUBCATCH, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);

    // --- process each pair of pollutant - init. load items
    for ( k = 2; k <= ntoks; k = k+2)
    {
        // --- check for valid pollutant name and loading value
        m = project_findObject(POLLUT, tok[k-1]);
        if ( m < 0 ) return error_setInpError(ERR_NAME, tok[k-1]);
        if ( k+1 > ntoks ) return error_setInpError(ERR_ITEMS, "");
        if ( ! getDouble(tok[k], &x) )
            return error_setInpError(ERR_NUMBER, tok[k]);

        // --- store loading in subcatch's initBuildup property
        Subcatch[j].initBuildup[m] = x;
    }
    return 0;
}

//=============================================================================

void  subcatch_validate(int j)
//
//  Input:   j = subcatchment index
//  Output:  none
//  Purpose: checks for valid subcatchment input parameters.
//
{
    int     i;
    double  area;
    double  nonLidArea = Subcatch[j].area;

    // --- check for ambiguous outlet name
    if ( Subcatch[j].outNode >= 0 && Subcatch[j].outSubcatch >= 0 )
        report_writeErrorMsg(ERR_SUBCATCH_OUTLET, Subcatch[j].ID);

    // --- validate subcatchment's groundwater component 
    gwater_validate(j);

    // --- validate placement of LIDs in the subcatchment
    nonLidArea -= Subcatch[j].lidArea;

    // --- compute alpha (i.e. WCON in old SWMM) for overland flow
    //     NOTE: the area which contributes to alpha for both imperv
    //     subareas w/ and w/o depression storage is the total imperv area.
    for (i = IMPERV0; i <= PERV; i++)
    {
        if ( i == PERV )
        {
            area = (1.0 - Subcatch[j].fracImperv) * nonLidArea;
        }
        else
        {
             area = Subcatch[j].fracImperv * nonLidArea;
        }
        Subcatch[j].subArea[i].alpha = 0.0;

////  Possible change to how sub-area width should be assigned.  ////
////        area = nonLidArea; 
/////////////////////////////////////////////////////////////////////

        if ( area > 0.0 && Subcatch[j].subArea[i].N > 0.0 )
        {
            Subcatch[j].subArea[i].alpha = MCOEFF * Subcatch[j].width / area *
                sqrt(Subcatch[j].slope) / Subcatch[j].subArea[i].N;
        }
    }

    // --- set isUsed property of subcatchment's rain gage                     //(5.1.013)
    i = Subcatch[j].gage;                                                      //
    if (i >= 0) Gage[i].isUsed = TRUE;                                         //

}

//=============================================================================

void  subcatch_initState(int j)
//
//  Input:   j = subcatchment index
//  Output:  none
//  Purpose: Initializes the state of a subcatchment.
//
{
    int    i;

//// isUsed property of subcatchment's rain gage now set in subcatch_validate  //(5.1.013)

    // --- initialize rainfall, runoff, & snow depth
    Subcatch[j].rainfall = 0.0;
    Subcatch[j].oldRunoff = 0.0;
    Subcatch[j].newRunoff = 0.0;
    Subcatch[j].oldSnowDepth = 0.0;
    Subcatch[j].newSnowDepth = 0.0;
    Subcatch[j].runon = 0.0;
    Subcatch[j].evapLoss = 0.0;
    Subcatch[j].infilLoss = 0.0;

    // --- initialize state of infiltration, groundwater, & snow pack objects
    if ( Subcatch[j].infil == j )  infil_initState(j, InfilModel);
    if ( Subcatch[j].groundwater ) gwater_initState(j);
    if ( Subcatch[j].snowpack )    snow_initSnowpack(j);

    // --- initialize state of sub-areas
    for (i = IMPERV0; i <= PERV; i++)
    {
        Subcatch[j].subArea[i].depth  = 0.0;
        Subcatch[j].subArea[i].inflow = 0.0;
        Subcatch[j].subArea[i].runoff = 0.0;
    }

    // --- initialize runoff quality
    surfqual_initState(j);
}

//=============================================================================

void subcatch_setOldState(int j)
//
//  Input:   j = subcatchment index
//  Output:  none
//  Purpose: replaces old state of subcatchment with new state.
//
{
    int i;
    Subcatch[j].oldRunoff = Subcatch[j].newRunoff;
    Subcatch[j].oldSnowDepth = Subcatch[j].newSnowDepth;
    for (i = IMPERV0; i <= PERV; i++)
    {
        Subcatch[j].subArea[i].inflow = 0.0;
    }
    for (i = 0; i < Nobjects[POLLUT]; i++)
    {
        Subcatch[j].oldQual[i] = Subcatch[j].newQual[i];
        Subcatch[j].newQual[i] = 0.0;
    }
    lid_setOldGroupState(j);
}

//=============================================================================

double subcatch_getFracPerv(int j)
//
//  Purpose: determines what fraction of subcatchment area, including any LID
//           area, is pervious.
//  Input:   j = subcatchment index
//  Output:  returns fraction of area with pervious cover
//
{
    double fracPerv = 1.0 - Subcatch[j].fracImperv;

    if ( Subcatch[j].lidArea > 0.0 )
    {
        fracPerv = (fracPerv * (Subcatch[j].area - Subcatch[j].lidArea) + 
                    lid_getPervArea(j)) / Subcatch[j].area;
        fracPerv = MIN(fracPerv, 1.0);
    }
    return fracPerv;
}

//=============================================================================

double subcatch_getStorage(int j)
//
//  Input:   j = subcatchment index
//  Output:  returns total volume of stored water (ft3)
//  Purpose: finds total volume of water stored on a subcatchment's surface
//           and its LIDs at the current time.
//
{
    int    i;
    double v = 0.0;

    for ( i = IMPERV0; i <= PERV; i++)
    {
        v += Subcatch[j].subArea[i].depth * Subcatch[j].subArea[i].fArea;
    }
    return v * (Subcatch[j].area - Subcatch[j].lidArea) +
           lid_getStoredVolume(j);
}

//=============================================================================

void subcatch_getRunon(int j)
//
//  Input:   j = subcatchment index
//  Output:  none
//  Purpose: Routes runoff from a subcatchment to its outlet subcatchment
//           or between its subareas.
//
{
    int    k;                          // outlet subcatchment index
    int    p;                          // pollutant index
    double q;                          // runon to outlet subcatchment (ft/sec)
    double q1, q2;                     // runoff from imperv. areas (ft/sec)
    double pervArea;                   // subcatchment pervious area (ft2)

    // --- add previous period's runoff from this subcatchment to the
    //     runon of the outflow subcatchment, if it exists
    k = Subcatch[j].outSubcatch;
    q = Subcatch[j].oldRunoff;
    if ( k >= 0 && k != j )
    {
        subcatch_addRunonFlow(k, q);
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            Subcatch[k].newQual[p] += q * Subcatch[j].oldQual[p] * LperFT3;
        }
    }

    // --- add any LID underdrain flow sent from this subcatchment to
    //     other subcatchments
    if ( Subcatch[j].lidArea > 0.0 ) lid_addDrainRunon(j);

    // --- add to sub-area inflow any outflow from other subarea in previous period
    //     (NOTE: no transfer of runoff pollutant load, since runoff loads are
    //     based on runoff flow from entire subcatchment.)

    // --- Case 1: imperv --> perv
    if ( Subcatch[j].fracImperv < 1.0 &&
         Subcatch[j].subArea[IMPERV0].routeTo == TO_PERV )
    {
        // --- add area-wtd. outflow from imperv1 subarea to perv area inflow
        q1 = Subcatch[j].subArea[IMPERV0].runoff *
             Subcatch[j].subArea[IMPERV0].fArea;
        q2 = Subcatch[j].subArea[IMPERV1].runoff *
             Subcatch[j].subArea[IMPERV1].fArea;
        q = q1 + q2;
        Subcatch[j].subArea[PERV].inflow += q *
             (1.0 - Subcatch[j].subArea[IMPERV0].fOutlet) /
             Subcatch[j].subArea[PERV].fArea;
    }

    // --- Case 2: perv --> imperv
    if ( Subcatch[j].fracImperv > 0.0 &&
         Subcatch[j].subArea[PERV].routeTo == TO_IMPERV &&
         Subcatch[j].subArea[IMPERV1].fArea > 0.0 )
    {
        q = Subcatch[j].subArea[PERV].runoff;
        Subcatch[j].subArea[IMPERV1].inflow +=
            q * (1.0 - Subcatch[j].subArea[PERV].fOutlet) *
            Subcatch[j].subArea[PERV].fArea /
            Subcatch[j].subArea[IMPERV1].fArea;
    }

    // --- Add any return flow from LID units to pervious subarea
    if ( Subcatch[j].lidArea > 0.0 && Subcatch[j].fracImperv < 1.0 )
    {
        pervArea = Subcatch[j].subArea[PERV].fArea *
                   (Subcatch[j].area - Subcatch[j].lidArea);
        q = lid_getFlowToPerv(j);
        if ( pervArea > 0.0 )
        {
            Subcatch[j].subArea[PERV].inflow += q / pervArea;
        }
    }
}

//=============================================================================

void  subcatch_addRunonFlow(int k, double q)
//
//  Input:   k = subcatchment index
//           q = runon flow rate (cfs) to subcatchment k
//  Output:  none
//  Purpose: Updates the total runon flow (ft/s) seen by a subcatchment that
//           receives runon flow from an upstream subcatchment.
//
{
    int i;
    double nonLidArea;

    // --- distribute runoff from upstream subcatchment (in cfs)
    //     uniformly over the non-LID area of current subcatchment (ft/sec)
    if ( Subcatch[k].area <= 0.0 ) return;
    nonLidArea = Subcatch[k].area - Subcatch[k].lidArea; 
    if ( nonLidArea > 0.0 ) q = q / nonLidArea;
    else                    q = q / Subcatch[k].area;
    Subcatch[k].runon += q;

    // --- assign this flow to the 3 types of subareas
    for (i = IMPERV0; i <= PERV; i++)
    {
        Subcatch[k].subArea[i].inflow += q;
    }
}

//=============================================================================

double subcatch_getRunoff(int j, double tStep)
//
//  Input:   j = subcatchment index
//           tStep = time step (sec)
//  Output:  returns total runoff produced by subcatchment (ft/sec)
//  Purpose: Computes runoff & new storage depth for subcatchment.
//
//  The 'runoff' value returned by this function is the total runoff
//  generated (in ft/sec) by the subcatchment before any internal
//  re-routing is applied. It is used to compute pollutant washoff.
//
//  The 'outflow' value computed here (in cfs) is the surface runoff
//  that actually leaves the subcatchment after any LID controls are
//  applied and is saved to Subcatch[j].newRunoff. 
//
{
    int    i;                          // subarea index
    double nonLidArea;                 // non-LID portion of subcatch area (ft2)
    double area;                       // sub-area or subcatchment area (ft2)
    double netPrecip[3];               // subarea net precipitation (ft/sec)
    double vRain;                      // rainfall (+ snowfall) volume (ft3)
    double vRunon    = 0.0;            // runon volume from other areas (ft3)
    double vOutflow  = 0.0;            // runoff volume leaving subcatch (ft3)
    double runoff    = 0.0;            // total runoff flow on subcatch (cfs)
    double evapRate  = 0.0;            // potential evaporation rate (ft/sec)
    double subAreaRunoff;              // sub-area runoff rate (cfs)           //(5.1.013)
    double vImpervRunoff = 0.0;        // impervious area runoff volume (ft3)  //
    double vPervRunoff = 0.0;          // pervious area runoff volume (ft3)    //

    // --- initialize shared water balance variables
    Vevap     = 0.0;
    Vpevap    = 0.0;
    Vinfil    = 0.0;
    Voutflow  = 0.0;
    VlidIn    = 0.0;
    VlidInfil = 0.0;
    VlidOut   = 0.0;
    VlidDrain = 0.0;
    VlidReturn = 0.0;

    // --- find volume of inflow to non-LID portion of subcatchment as existing
    //     ponded water + any runon volume from upstream areas;
    //     rainfall and snowmelt will be added as each sub-area is analyzed
    nonLidArea = Subcatch[j].area - Subcatch[j].lidArea;
    vRunon = Subcatch[j].runon * tStep * nonLidArea;
    Vinflow = vRunon + subcatch_getDepth(j) * nonLidArea;

    // --- find LID runon only if LID occupies full subcatchment
    if ( nonLidArea == 0.0 )
        vRunon = Subcatch[j].runon * tStep * Subcatch[j].area;

    // --- get net precip. (rainfall + snowfall + snowmelt) on the 3 types
    //     of subcatchment sub-areas and update Vinflow with it
    getNetPrecip(j, netPrecip, tStep);

    // --- find potential evaporation rate
    if ( Evap.dryOnly && Subcatch[j].rainfall > 0.0 ) evapRate = 0.0;
    else evapRate = Evap.rate;

    // --- set monthly infiltration adjustment factor                          //(5.1.013)
    infil_setInfilFactor(j);                                                   //(5.1.013)

    // --- examine each type of sub-area (impervious w/o depression storage,
    //     impervious w/ depression storage, and pervious)
    if ( nonLidArea > 0.0 ) for (i = IMPERV0; i <= PERV; i++)
    {
        // --- get runoff from sub-area updating Vevap, Vpevap,
        //     Vinfil & Voutflow)
        area = nonLidArea * Subcatch[j].subArea[i].fArea;
        Subcatch[j].subArea[i].runoff =
            getSubareaRunoff(j, i, area, netPrecip[i], evapRate, tStep);
        subAreaRunoff = Subcatch[j].subArea[i].runoff * area;                  //(5.1.013)
        if (i == PERV) vPervRunoff = subAreaRunoff * tStep;                    //
        else           vImpervRunoff += subAreaRunoff * tStep;                 //
        runoff += subAreaRunoff;                                               //
    }

    // --- evaluate any LID treatment provided (updating Vevap,
    //     Vpevap, VlidInfil, VlidIn, VlidOut, & VlidDrain)
    if ( Subcatch[j].lidArea > 0.0 )
    {
        lid_getRunoff(j, tStep);
    }

    // --- update groundwater levels & flows if applicable
    if ( !IgnoreGwater && Subcatch[j].groundwater )
    {
        gwater_getGroundwater(j, Vpevap, Vinfil+VlidInfil, tStep);
    }

    // --- save subcatchment's total loss rates (ft/s)
    area = Subcatch[j].area;
    Subcatch[j].evapLoss = Vevap / tStep / area;
    Subcatch[j].infilLoss = (Vinfil + VlidInfil) / tStep / area;

    // --- find net surface runoff volume
    //     (VlidDrain accounts for LID drain flows)
    vOutflow = Voutflow      // runoff from all non-LID areas
               - VlidIn      // runoff treated by LID units
               + VlidOut;    // runoff from LID units
    Subcatch[j].newRunoff = vOutflow / tStep;

    // --- obtain external precip. volume (without any snowmelt)
    vRain = Subcatch[j].rainfall * tStep * area;

    // --- update the cumulative stats for this subcatchment
    stats_updateSubcatchStats(j, vRain, vRunon, Vevap, Vinfil + VlidInfil,
        vImpervRunoff, vPervRunoff, vOutflow + VlidDrain,                      //(5.1.013)
        Subcatch[j].newRunoff + VlidDrain/tStep);

    // --- include this subcatchment's contribution to overall flow balance
    //     only if its outlet is a drainage system node
    if ( Subcatch[j].outNode == -1 && Subcatch[j].outSubcatch != j )
    {
        vOutflow = 0.0;
    }

    // --- update mass balances
    massbal_updateRunoffTotals(RUNOFF_RAINFALL, vRain);
    massbal_updateRunoffTotals(RUNOFF_EVAP, Vevap);
    massbal_updateRunoffTotals(RUNOFF_INFIL, Vinfil+VlidInfil);
    massbal_updateRunoffTotals(RUNOFF_RUNOFF, vOutflow);

    // --- return area-averaged runoff (ft/s)
    return runoff / area;
}

//=============================================================================

void getNetPrecip(int j, double* netPrecip, double tStep)
{
//
//  Purpose: Finds combined rainfall + snowmelt on a subcatchment.
//  Input:   j = subcatchment index
//           tStep = time step (sec)
//  Output:  netPrecip = rainfall + snowmelt over each type of subarea (ft/s)
//
    int    i, k;
    double rainfall = 0.0;             // rainfall (ft/sec)
    double snowfall = 0.0;             // snowfall (ft/sec)

    // --- get current rainfall or snowfall from rain gage (in ft/sec)
    k = Subcatch[j].gage;
    if ( k >= 0 )
    {
        gage_getPrecip(k, &rainfall, &snowfall);
    }

    // --- assign total precip. rate to subcatch's rainfall property
    Subcatch[j].rainfall = rainfall + snowfall;

    // --- determine net precipitation input (netPrecip) to each sub-area

    // --- if subcatch has a snowpack, then base netPrecip on possible snow melt
    if ( Subcatch[j].snowpack && !IgnoreSnowmelt )
    {
        Subcatch[j].newSnowDepth = 
            snow_getSnowMelt(j, rainfall, snowfall, tStep, netPrecip);
    }

    // --- otherwise netPrecip is just sum of rainfall & snowfall
    else
    {
        for (i=IMPERV0; i<=PERV; i++) netPrecip[i] = rainfall + snowfall;
    }
}

//=============================================================================

double subcatch_getDepth(int j)
//
//  Input:   j = subcatchment index
//  Output:  returns average depth of ponded water (ft)
//  Purpose: finds average depth of water over the non-LID portion of a
//           subcatchment
//
{
    int    i;
    double fArea;
    double depth = 0.0;

    for (i = IMPERV0; i <= PERV; i++)
    {
        fArea = Subcatch[j].subArea[i].fArea;
        if ( fArea > 0.0 ) depth += Subcatch[j].subArea[i].depth * fArea;
    }
    return depth;
}

//=============================================================================

double subcatch_getWtdOutflow(int j, double f)
//
//  Input:   j = subcatchment index
//           f = weighting factor.
//  Output:  returns weighted runoff value
//  Purpose: computes wtd. combination of old and new subcatchment runoff.
//
{
    if ( Subcatch[j].area == 0.0 ) return 0.0;
    return (1.0 - f) * Subcatch[j].oldRunoff + f * Subcatch[j].newRunoff;
}

//=============================================================================

void  subcatch_getResults(int j, double f, float x[])
//
//  Input:   j = subcatchment index
//           f = weighting factor
//  Output:  x = array of results
//  Purpose: computes wtd. combination of old and new subcatchment results.
//
{
    int    p;                          // pollutant index
    int    k;                          // rain gage index
    double f1 = 1.0 - f;
    double z;
    double runoff;
    TGroundwater* gw;                  // ptr. to groundwater object

    // --- retrieve rainfall for current report period
    k = Subcatch[j].gage;
    if ( k >= 0 ) x[SUBCATCH_RAINFALL] = (float)Gage[k].reportRainfall;
    else          x[SUBCATCH_RAINFALL] = 0.0f;

    // --- retrieve snow depth
    z = ( f1 * Subcatch[j].oldSnowDepth +
          f * Subcatch[j].newSnowDepth ) * UCF(RAINDEPTH);
    x[SUBCATCH_SNOWDEPTH] = (float)z;

    // --- retrieve runoff and losses
    x[SUBCATCH_EVAP] = (float)(Subcatch[j].evapLoss * UCF(EVAPRATE));
    x[SUBCATCH_INFIL] = (float)(Subcatch[j].infilLoss * UCF(RAINFALL));
    runoff = f1 * Subcatch[j].oldRunoff + f * Subcatch[j].newRunoff;

    // --- add any LID drain flow to reported runoff
    if ( Subcatch[j].lidArea > 0.0 )
    {
        runoff += f1 * lid_getDrainFlow(j, PREVIOUS) +
                  f * lid_getDrainFlow(j, CURRENT);
    }

    // --- if runoff is really small, report it as zero
    if ( runoff < MIN_RUNOFF * Subcatch[j].area ) runoff = 0.0;
    x[SUBCATCH_RUNOFF] = (float)(runoff * UCF(FLOW));

    // --- retrieve groundwater results
    gw = Subcatch[j].groundwater;
    if ( gw )
    {
        z = (f1 * gw->oldFlow + f * gw->newFlow) * Subcatch[j].area * UCF(FLOW);
        x[SUBCATCH_GW_FLOW] = (float)z;
        z = (gw->bottomElev + gw->lowerDepth) * UCF(LENGTH);
        x[SUBCATCH_GW_ELEV] = (float)z;
        z = gw->theta;
        x[SUBCATCH_SOIL_MOIST] = (float)z;
    }
    else
    {
        x[SUBCATCH_GW_FLOW] = 0.0f;
        x[SUBCATCH_GW_ELEV] = 0.0f;
        x[SUBCATCH_SOIL_MOIST]  = 0.0f;
    }

    // --- retrieve pollutant washoff
    if ( !IgnoreQuality ) for (p = 0; p < Nobjects[POLLUT]; p++ )
    {
        if ( runoff == 0.0 ) z = 0.0;
        else z = f1 * Subcatch[j].oldQual[p] + f * Subcatch[j].newQual[p];
        x[SUBCATCH_WASHOFF+p] = (float)z;
    }
}


//=============================================================================
//                              SUB-AREA METHODS
//=============================================================================

double getSubareaRunoff(int j, int i, double area, double precip, double evap,
    double tStep)
//
//  Purpose: computes runoff & losses from a subarea over the current time step.
//  Input:   j = subcatchment index
//           i = subarea index
//           area = sub-area area (ft2)
//           precip = rainfall + snowmelt over subarea (ft/sec)
//           evap = evaporation (ft/sec)
//           tStep = time step (sec)
//  Output:  returns runoff rate from the sub-area (cfs);
//           updates shared variables Vinflow, Vevap, Vpevap, Vinfil & Voutflow.
//
{
    double    tRunoff;                 // time over which runoff occurs (sec)
    double    surfMoisture;            // surface water available (ft/sec)
    double    surfEvap;                // evap. used for surface water (ft/sec)
    double    infil = 0.0;             // infiltration rate (ft/sec)
    double    runoff = 0.0;            // runoff rate (ft/sec)
    TSubarea* subarea;                 // pointer to subarea being analyzed

    // --- no runoff if no area
    if ( area == 0.0 ) return 0.0;

    // --- assign pointer to current subarea
    subarea = &Subcatch[j].subArea[i];

    // --- assume runoff occurs over entire time step
    tRunoff = tStep;

    // --- determine evaporation loss rate
    surfMoisture = subarea->depth / tStep;
    surfEvap = MIN(surfMoisture, evap);

    // --- compute infiltration loss rate
    if ( i == PERV ) infil = getSubareaInfil(j, subarea, precip, tStep);

    // --- add precip to other subarea inflows
    subarea->inflow += precip;
    surfMoisture += subarea->inflow;

    // --- update total inflow, evaporation & infiltration volumes
    Vinflow += precip * area * tStep;
    Vevap += surfEvap * area * tStep;
    if ( i == PERV ) Vpevap += Vevap;
    Vinfil += infil * area * tStep;

    // --- assign adjusted runoff coeff. & storage to shared variables         //(5.1.013)
    Alpha = subarea->alpha;                                                    //
    Dstore = subarea->dStore;                                                  //
    adjustSubareaParams(i, j);                                                 // 

    // --- if losses exceed available moisture then no ponded water remains
    if ( surfEvap + infil >= surfMoisture )
    {
        subarea->depth = 0.0;
    }

    // --- otherwise reduce inflow by losses and update depth
    //     of ponded water and time over which runoff occurs
    else
    {
        subarea->inflow -= surfEvap + infil;
        updatePondedDepth(subarea, &tRunoff);
    }

    // --- compute runoff based on updated ponded depth
    runoff = findSubareaRunoff(subarea, tRunoff);

    // --- compute runoff volume leaving subcatchment for mass balance purposes
    //     (fOutlet is the fraction of this subarea's runoff that goes to the
    //     subcatchment outlet as opposed to another subarea of the subcatchment)
    Voutflow += subarea->fOutlet * runoff * area * tStep;
    return runoff;
}

//=============================================================================

double getSubareaInfil(int j, TSubarea* subarea, double precip, double tStep)
//
//  Purpose: computes infiltration rate at current time step.
//  Input:   j = subcatchment index
//           subarea = ptr. to a subarea
//           precip = rainfall + snowmelt over subarea (ft/sec)
//           tStep = time step (sec)
//  Output:  returns infiltration rate (ft/s)
//
{
    double infil = 0.0;                     // actual infiltration rate (ft/sec)

    // --- compute infiltration rate 
    infil = infil_getInfil(j, InfilModel, tStep, precip,
                           subarea->inflow, subarea->depth);

    // --- limit infiltration rate by available void space in unsaturated
    //     zone of any groundwater aquifer
    if ( !IgnoreGwater && Subcatch[j].groundwater )
    {
        infil = MIN(infil,
                    Subcatch[j].groundwater->maxInfilVol/tStep);
    }
    return infil;
}

//=============================================================================

double findSubareaRunoff(TSubarea* subarea, double tRunoff)
//
//  Purpose: computes runoff (ft/s) from subarea after current time step.
//  Input:   subarea = ptr. to a subarea
//           tRunoff = time step over which runoff occurs (sec)
//  Output:  returns runoff rate (ft/s)
//
{
    double xDepth = subarea->depth - Dstore;                                   //(5.1.013)
    double runoff = 0.0;

    if ( xDepth > ZERO )
    {
        // --- case where nonlinear routing is used
        if ( subarea->N > 0.0 )
        {
            runoff = Alpha * pow(xDepth, MEXP);                                //(5.1.013)
        }

        // --- case where no routing is used (Mannings N = 0)
        else
        {
            runoff = xDepth / tRunoff;
            subarea->depth = Dstore;                                           //(5.1.013)
        }
    }
    else
    {    
        runoff = 0.0;
    }
    return runoff;
}

//=============================================================================

void updatePondedDepth(TSubarea* subarea, double* dt)
//
//  Input:   subarea = ptr. to a subarea,
//           dt = time step (sec)
//  Output:  dt = time ponded depth is above depression storage (sec)
//  Purpose: computes new ponded depth over subarea after current time step.
//
{
    double ix = subarea->inflow;       // excess inflow to subarea (ft/sec)
    double dx;                         // depth above depression storage (ft)
    double tx = *dt;                   // time over which dx > 0 (sec)
    
    // --- see if not enough inflow to fill depression storage (dStore)
    if ( subarea->depth + ix*tx <= Dstore )                                    //(5.1.013)
    {
        subarea->depth += ix * tx;
    }

    // --- otherwise use the ODE solver to integrate flow depth
    else
    {
        // --- if depth < Dstore then fill up Dstore & reduce time step        //(5.1.013)
        dx = Dstore - subarea->depth;                                          //
        if ( dx > 0.0 && ix > 0.0 )
        {
            tx -= dx / ix;
            subarea->depth = Dstore;                                           //(5.1.013)
        }

        // --- now integrate depth over remaining time step tx
        if ( Alpha > 0.0 && tx > 0.0 )                                         //(5.1.013)
        {
            theSubarea = subarea;
            odesolve_integrate(&(subarea->depth), 1, 0, tx, ODETOL, tx,
                               getDdDt);
        }
        else
        {
            if ( tx < 0.0 ) tx = 0.0;
            subarea->depth += ix * tx;
        }
    }

    // --- do not allow ponded depth to go negative
    if ( subarea->depth < 0.0 ) subarea->depth = 0.0;

    // --- replace original time step with time ponded depth
    //     is above depression storage
    *dt = tx;
}

//=============================================================================

void  getDdDt(double t, double* d, double* dddt)
//
//  Input:   t = current time (not used)
//           d = stored depth (ft)
//  Output   dddt = derivative of d with respect to time
//  Purpose: evaluates derivative of stored depth w.r.t. time
//           for the subarea whose runoff is being computed.
//
{
    double ix = theSubarea->inflow;
    double rx = *d - Dstore;                                                   //(5.1.013)
    if ( rx < 0.0 )
    {
        rx = 0.0;
    }
    else
    {
        rx = Alpha * pow(rx, MEXP);                                            //(5.1.013)
    }
    *dddt = ix - rx;
}

//=============================================================================

////  New function added to release 5.1.013.  ////                             //(5.1.013)

void adjustSubareaParams(int i, int j)
//
//  Input:   i = type of subarea being analyzed
//           j = index of current subcatchment being analyzed
//  Output   adjusted values of module-level variables Dstore & Alpha
//  Purpose: adjusts a subarea's depression storage and its pervious
//           runoff coeff. by month of the year.
//
{
    int p;              // monthly pattern index
    int m;              // current month of the year
    double f;           // adjustment factor

     // --- depression storage adjustment
     p = Subcatch[j].dStorePattern;
     if (p >= 0 && Pattern[p].type == MONTHLY_PATTERN)
     {
         m = datetime_monthOfYear(getDateTime(OldRunoffTime)) - 1;
         f = Pattern[p].factor[m];
         if (f >= 0.0) Dstore *= f;
     }

    // --- pervious area roughness
    p = Subcatch[j].nPervPattern;
    if (i == PERV && p >= 0 && Pattern[p].type == MONTHLY_PATTERN)
    {
         m = datetime_monthOfYear(getDateTime(OldRunoffTime)) - 1;
         f = Pattern[p].factor[m];
         if (f <= 0.0) Alpha = 0.0;
         else          Alpha /= f;
     }
}
