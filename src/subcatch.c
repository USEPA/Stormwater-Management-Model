//-----------------------------------------------------------------------------
//   subcatch.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/19/14  (Build 5.1.000)
//             04/19/14  (Build 5.1.006)
//   Author:   L. Rossman
//
//   Subcatchment runoff & quality functions.
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
// Shared variables   
//-----------------------------------------------------------------------------
static  double     Losses;        // subcatch evap. + infil. loss rate (ft/sec)
static  double     Outflow;       // subcatch outflow rate (ft/sec)

// Volumes as either total (ft3) or per unit area (ft) depending on context
static  double     Vrain;         // subcatch rain volume over a time step
static  double     Vevap;         // subcatch evap. volume over a time step
static  double     Vinfil;        // subcatch infil. volume over a time step

static  double     Vrunon;        // subcatch runon volume over a time step (ft3)
static  double     Vponded;       // volume of ponded water over subcatch (ft3)
static  double     Voutflow;      // subcatch outflow depth (ft3)

static  TSubarea* theSubarea;     // subarea to which getDdDt() is applied
static  char *RunoffRoutingWords[] = { w_OUTLET,  w_IMPERV, w_PERV, NULL};

//-----------------------------------------------------------------------------
//  Imported variables (declared in RUNOFF.C)
//-----------------------------------------------------------------------------
extern  double*    OutflowLoad;   // outflow pollutant mass from a subcatchment
extern  double*    WashoffLoad;   // washoff pollutant mass from landuses

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
//  subcatch_getRunoff         (called from runoff_execute)
//  subcatch_getWashoff        (called from runoff_execute)
//  subcatch_getBuildup        (called from runoff_execute)
//  subcatch_sweepBuildup      (called from runoff_execute)
//  subcatch_hadRunoff         (called from runoff_execute)

//  subcatch_getFracPerv       (called from gwater_initState)
//  subcatch_getStorage        (called from massbal_getRunoffError)

//  subcatch_getWtdOutflow     (called from addWetWeatherInflows in routing.c)
//  subcatch_getWtdWashoff     (called from addWetWeatherInflows in routing.c)

//  subcatch_getResults        (called from output_saveSubcatchResults)

//-----------------------------------------------------------------------------
// Function declarations
//-----------------------------------------------------------------------------
static void   getNetPrecip(int j, double* netPrecip, double tStep);
static void   getSubareaRunoff(int subcatch, int subarea, double rainfall,
              double evap, double tStep);
static double getSubareaInfil(int j, TSubarea* subarea, double precip,
              double tStep);
static void   findSubareaRunoff(TSubarea* subarea, double tRunoff);
static void   updatePondedDepth(TSubarea* subarea, double* tx);
static void   getDdDt(double t, double* d, double* dddt);
static void   updatePondedQual(int j, double wUp[], double pondedQual[],
	          double tStep);

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
//    Name  RainGage  Outlet  Area  %Imperv  Width  Slope CurbLength  Snowmelt  
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
    Subcatch[j].fracImperv  = x[4] / 100.0;
    Subcatch[j].width       = x[5] / UCF(LENGTH);
    Subcatch[j].slope       = x[6] / 100.0;
    Subcatch[j].curbLength  = x[7];

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
        if ( area > 0.0 && Subcatch[j].subArea[i].N > 0.0 )
        {
            Subcatch[j].subArea[i].alpha = MCOEFF * Subcatch[j].width / area *
                sqrt(Subcatch[j].slope) / Subcatch[j].subArea[i].N;
        }
    }
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
    int    p;                          // pollutant index

    // --- initialize rainfall, runoff, & snow depth
    Subcatch[j].rainfall = 0.0;
    Subcatch[j].oldRunoff = 0.0;
    Subcatch[j].newRunoff = 0.0;
    Subcatch[j].oldSnowDepth = 0.0;
    Subcatch[j].newSnowDepth = 0.0;
    Subcatch[j].runon = 0.0;

    // --- set isUsed property of subcatchment's rain gage
    i = Subcatch[j].gage;
    if ( i >= 0 )
    {
        Gage[i].isUsed = TRUE;
        if ( Gage[i].coGage >= 0 ) Gage[Gage[i].coGage].isUsed = TRUE;
    }

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
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        Subcatch[j].oldQual[p] = 0.0;
        Subcatch[j].newQual[p] = 0.0;
        Subcatch[j].pondedQual[p] = 0.0;
    }

    // --- initialize pollutant buildup
	landuse_getInitBuildup(Subcatch[j].landFactor,  Subcatch[j].initBuildup,
		Subcatch[j].area, Subcatch[j].curbLength);
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
    int    i;                          // subarea index
    int    k;                          // outlet subcatchment index
    double q;                          // runon to outlet subcatchment (ft/sec)
    double q1, q2;                     // runoff from imperv. areas (ft/sec)
    double pervArea;                   // subcatchment pervious area (ft2)

    // --- add previous period's runoff from this subcatchment to the
    //     runon of the outflow subcatchment, if it exists
    k = Subcatch[j].outSubcatch;
    if ( k >= 0 && k != j && Subcatch[k].area > 0.0 )
    {
        // --- distribute previous runoff from subcatch j (in cfs)
        //     uniformly over area of subcatch k (ft/sec)
        q = Subcatch[j].oldRunoff / Subcatch[k].area;
        Subcatch[k].runon += q;

        // --- assign this flow to the 3 types of subareas
        for (i = IMPERV0; i <= PERV; i++)
        {
            Subcatch[k].subArea[i].inflow += q;
        }

        // --- add runoff mass load (in mass/sec) to receiving subcatch,
        //     storing it in Subcatch[].newQual for now
        for (i = 0; i < Nobjects[POLLUT]; i++)
        {
            Subcatch[k].newQual[i] += (Subcatch[j].oldRunoff *
                                       Subcatch[j].oldQual[i] * LperFT3);
        }
    }

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
        if ( pervArea > 0.0 ) Subcatch[j].subArea[PERV].inflow +=
            lid_getFlowToPerv(j) / pervArea;
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
{
    int    i;                          // subarea index
    double area;                       // portion of subcatchment area (ft2)
    double netPrecip[3];               // subarea net precipitation (ft/sec)
    double rainVol     = 0.0;          // rain volume (ft3)
    double evapVol     = 0.0;          // evaporation volume (ft3)
    double infilVol    = 0.0;          // infiltration volume (ft3)
    double outflowVol  = 0.0;          // runoff volume leaving subcatch (ft3)
    double outflow     = 0.0;          // runoff rate leaving subcatch (cfs)
    double runoff      = 0.0;          // total runoff rate on subcatch (ft/sec)
    double pervEvapVol = 0.0;          // evaporation over pervious area (ft3)
    double evapRate    = 0.0;          // max. evaporation rate (ft/sec)

    // NOTE: The 'runoff' value returned by this function is the total runoff
    //       generated (in ft/sec) by the subcatchment before any internal
    //       re-routing is applied. It is used in the Exponential Washoff
    //       function to compute pollutant washoff. The 'outflow' value
    //       computed here (in cfs) is the runoff that actually leaves the
    //       subcatchment (which can be reduced by internal re-routing and
    //       LID controls) and is saved to Subcatch[j].newRunoff.

    // --- save current depth of ponded water over entire subcatchment
	Vponded = subcatch_getDepth(j) * Subcatch[j].area;

    // --- get net precipitation (rainfall + snowmelt) on subcatchment
    getNetPrecip(j, netPrecip, tStep);
    if ( Evap.dryOnly && Subcatch[j].rainfall > 0.0 ) evapRate = 0.0;
    else evapRate = Evap.rate;

    // --- initialize runoff rates
    outflow = 0.0;
    runoff = 0.0;

    // --- examine each type of sub-area
    for (i = IMPERV0; i <= PERV; i++)
    {
        // --- check that sub-area type exists
        area = (Subcatch[j].area - Subcatch[j].lidArea) *
                Subcatch[j].subArea[i].fArea;
        if ( area > 0.0 )
        {
            // --- get runoff rate from sub-area
            getSubareaRunoff(j, i, netPrecip[i], evapRate, tStep);
            runoff += Subcatch[j].subArea[i].runoff * area;

            // --- update components of volumetric water balance (in ft3)
            //Subcatch[j].losses += Losses * area;
            rainVol    += netPrecip[i] * tStep * area;
            outflow    += Outflow * area;
            evapVol    += Vevap * area;
            infilVol   += Vinfil * area;

            // --- save evap losses from pervious area
            //     (needed for groundwater modeling)
            if ( i == PERV ) pervEvapVol += Vevap * area;
        }
    }

    // --- evaluate LID treatment as if it were another type of sub-area
    //     while updating outflow, evap volumes, & infil volumes
    if ( Subcatch[j].lidArea > 0.0 )
        runoff += lid_getRunoff(j, &outflow, &evapVol, &pervEvapVol,
                                &infilVol, tStep);

    // --- update groundwater levels & flows if applicable
    if (!IgnoreGwater && Subcatch[j].groundwater )
        gwater_getGroundwater(j, pervEvapVol, infilVol, tStep);

    // --- save subcatchment's outflow (cfs) & total loss rates (ft/s)
    area = Subcatch[j].area;
    Subcatch[j].newRunoff = outflow;
    Subcatch[j].evapLoss = evapVol / tStep / area;
    Subcatch[j].infilLoss = infilVol / tStep / area;

    // --- save volumes (ft3) for use in pollutant washoff calculation
    Vrain = rainVol;
    Vevap = evapVol;
    Vinfil = infilVol;
	Voutflow = outflow * tStep;
	Vrunon = Subcatch[j].runon * tStep * area;

    // --- compute water flux volumes over the time step
    rainVol = Subcatch[j].rainfall * tStep * area;
    stats_updateSubcatchStats(j, rainVol, Vrunon, Vevap, Vinfil,
                              Voutflow, outflow);

    // --- update system flow balance
    //     (system outflow is 0 if outlet is another subcatch)
    outflowVol = Voutflow;
    if ( Subcatch[j].outNode == -1 && Subcatch[j].outSubcatch != j )
    {
        outflowVol = 0.0;
    }
    massbal_updateRunoffTotals(rainVol, evapVol, infilVol, outflowVol);

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
//  Output:  returns average depth of water (ft)
//  Purpose: finds average depth of water over a subcatchment
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

    if ( Subcatch[j].lidArea > 0.0 ) 
    {
        depth = (depth * (Subcatch[j].area - Subcatch[j].lidArea) +
            lid_getSurfaceDepth(j) * Subcatch[j].lidArea) / Subcatch[j].area;
    }
    return depth;
}

//=============================================================================

void subcatch_getBuildup(int j, double tStep)
//
//  Input:   j = subcatchment index
//           tStep = time step (sec)
//  Output:  none
//  Purpose: adds to pollutant buildup on subcatchment.
//
{
    int     i;                         // land use index
    int     p;                         // pollutant index
    double  f;                         // land use fraction
    double  area;                      // land use area (acres or hectares)
    double  curb;                      // land use curb length (user units)
    double  oldBuildup;                // buildup at start of time step
    double  newBuildup;                // buildup at end of time step

    // --- consider each landuse
    for (i = 0; i < Nobjects[LANDUSE]; i++)
    {
        // --- skip landuse if not in subcatch
        f = Subcatch[j].landFactor[i].fraction;
        if ( f == 0.0 ) continue;

        // --- get land area (in acres or hectares) & curb length
        area = f * Subcatch[j].area * UCF(LANDAREA);
        curb = f * Subcatch[j].curbLength;

        // --- examine each pollutant
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            // --- see if snow-only buildup is in effect
            if (Pollut[p].snowOnly 
            && Subcatch[j].newSnowDepth < 0.001/12.0) continue;

            // --- use land use's buildup function to update buildup amount
            oldBuildup = Subcatch[j].landFactor[i].buildup[p];        
            newBuildup = landuse_getBuildup(i, p, area, curb, oldBuildup,
                         tStep);
            newBuildup = MAX(newBuildup, oldBuildup);
            Subcatch[j].landFactor[i].buildup[p] = newBuildup;
            massbal_updateLoadingTotals(BUILDUP_LOAD, p, 
                                       (newBuildup - oldBuildup));
       }
    }
}

//=============================================================================

void subcatch_sweepBuildup(int j, DateTime aDate)
//
//  Input:   j = subcatchment index
//  Output:  none
//  Purpose: reduces pollutant buildup over a subcatchment if sweeping occurs.
//
{
    int     i;                         // land use index
    int     p;                         // pollutant index
    double  oldBuildup;                // buildup before sweeping (lbs or kg)
    double  newBuildup;                // buildup after sweeping (lbs or kg)

    // --- no sweeping if there is snow on plowable impervious area
    if ( Subcatch[j].snowpack != NULL &&
         Subcatch[j].snowpack->wsnow[IMPERV0] > MIN_TOTAL_DEPTH ) return;

    // --- consider each land use
    for (i = 0; i < Nobjects[LANDUSE]; i++)
    {
        // --- skip land use if not in subcatchment 
        if ( Subcatch[j].landFactor[i].fraction == 0.0 ) continue;

        // --- see if land use is subject to sweeping
        if ( Landuse[i].sweepInterval == 0.0 ) continue;

        // --- see if sweep interval has been reached
        if ( aDate - Subcatch[j].landFactor[i].lastSwept >=
            Landuse[i].sweepInterval )
        {
        
            // --- update time when last swept
            Subcatch[j].landFactor[i].lastSwept = aDate;

            // --- examine each pollutant
            for (p = 0; p < Nobjects[POLLUT]; p++)
            {
                // --- reduce buildup by the fraction available
                //     times the sweeping effic.
                oldBuildup = Subcatch[j].landFactor[i].buildup[p];
                newBuildup = oldBuildup * (1.0 - Landuse[i].sweepRemoval *
                             Landuse[i].washoffFunc[p].sweepEffic);
                newBuildup = MIN(oldBuildup, newBuildup);
                newBuildup = MAX(0.0, newBuildup);
                Subcatch[j].landFactor[i].buildup[p] = newBuildup;

                // --- update mass balance totals
                massbal_updateLoadingTotals(SWEEPING_LOAD, p,
                                            oldBuildup - newBuildup);
            }
        }
    }
}

//=============================================================================

void  subcatch_getWashoff(int j, double runoff, double tStep)
//
//  Input:   j = subcatchment index
//           runoff = total subcatchment runoff (ft/sec)
//           tStep = time step (sec)
//  Output:  none
//  Purpose: computes new runoff quality for subcatchment.
//
//  Considers two separate pollutant generating streams that are combined
//  together:
//  1. complete mix mass balance of pollutants in surface ponding due to
//     runon, wet deposition, infil., & evap.
//  2. washoff of pollutant buildup as described by the project's land
//     use washoff functions.
//
{
    int    i, p;
    double massLoad;

    // --- return if there is no area or no pollutants
    if ( Nobjects[POLLUT] == 0 || Subcatch[j].area == 0.0 ) return;

    // --- intialize outflow loads to zero
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        WashoffLoad[p] = 0.0;      // load just from washoff function
        OutflowLoad[p] = 0.0;      // Washoff load + ponded water load
    }

    // --- add outflow of pollutants in ponded water to outflow loads
    //     (Note: at this point, Subcatch.newQual contains mass inflow
    //      from any upstream subcatchments draining to this one)
    updatePondedQual(j, Subcatch[j].newQual, Subcatch[j].pondedQual, tStep);

    // --- add washoff loads from landuses to outflow loads
    if ( runoff >= MIN_RUNOFF )
    {
        for (i = 0; i < Nobjects[LANDUSE]; i++)
        {
            if ( Subcatch[j].landFactor[i].fraction > 0.0 )
            {
                landuse_getWashoff(i, Subcatch[j].area, Subcatch[j].landFactor,
				    runoff, tStep, WashoffLoad);
            }
        }

        // --- compute contribution from any co-pollutant
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            WashoffLoad[p] += landuse_getCoPollutLoad(p, WashoffLoad);
            OutflowLoad[p] += WashoffLoad[p];
        }

    }

    // --- switch from internal runoff (used in washoff functions) to
    //     runoff that actually leaves the subcatchment
    runoff = Subcatch[j].newRunoff;

    // --- for each pollutant
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        // --- update subcatchment's total runoff load in lbs (or kg)
        massLoad = OutflowLoad[p] * Pollut[p].mcf;
        Subcatch[j].totalLoad[p] += massLoad;

        // --- update overall runoff mass balance if runoff goes to
        //     conveyance system
        if ( Subcatch[j].outNode >= 0 || Subcatch[j].outSubcatch == j ) 
            massbal_updateLoadingTotals(RUNOFF_LOAD, p, massLoad);
        
        // --- save new outflow runoff concentration (in mass/L)
        if ( runoff > MIN_RUNOFF )
            Subcatch[j].newQual[p] = OutflowLoad[p] / (runoff * tStep * LperFT3);
        else Subcatch[j].newQual[p] = 0.0;
    }
}

//=============================================================================

void updatePondedQual(int j, double wRunon[], double pondedQual[], double tStep)
{
    int    p;
    double c;
    double vIn;
    double wPpt, wInfil, w1;
    double bmpRemoval;
    int    isDry;

    // --- total inflow volume
    vIn = Vrain + Vrunon;

    // --- for dry conditions
    if ( Vponded + vIn == 0.0 ) isDry = 1;
    else isDry = 0;

    // --- analyze each pollutant
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        // --- update mass balance for direct deposition
        wPpt = Pollut[p].pptConcen * LperFT3 * Vrain;                          //(5.1.006 - MT)
        massbal_updateLoadingTotals(DEPOSITION_LOAD, p, wPpt * Pollut[p].mcf);

        // --- surface is dry and has no inflow -- add any remaining mass
        //     to overall mass balance's FINAL_LOAD category
        if ( isDry )
        {
            massbal_updateLoadingTotals(FINAL_LOAD, p,
                pondedQual[p] * Pollut[p].mcf);
            pondedQual[p] = 0.0;
            OutflowLoad[p] = 0.0;
        }
        else
        {
            // --- find concen. of ponded water
            w1 = pondedQual[p] + wPpt + wRunon[p]*tStep;
            c = w1 / (Vponded + vIn);

            // --- mass lost to infiltration
            wInfil = c * Vinfil;
            wInfil = MIN(wInfil, w1);
            massbal_updateLoadingTotals(INFIL_LOAD, p, wInfil * Pollut[p].mcf);
            w1 -= wInfil;

            // --- mass lost to outflow
            OutflowLoad[p] = MIN(w1, (c*Voutflow));
            w1 -= OutflowLoad[p];

            // --- reduce outflow load by average BMP removal
            bmpRemoval = landuse_getAvgBmpEffic(j, p) * OutflowLoad[p];
            massbal_updateLoadingTotals(BMP_REMOVAL_LOAD, p,
                bmpRemoval*Pollut[p].mcf);
            OutflowLoad[p] -= bmpRemoval;

            // --- update ponded mass
            pondedQual[p] = c * subcatch_getDepth(j) * Subcatch[j].area;       //(5.1.006)
        }
    }
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

double subcatch_getWtdWashoff(int j, int p, double f)
//
//  Input:   j = subcatchment index
//           p = pollutant index
//           f = weighting factor
//  Output:  returns pollutant washoff value
//  Purpose: finds wtd. combination of old and new washoff for a pollutant.
//
{
    return (1.0 - f) * Subcatch[j].oldRunoff * Subcatch[j].oldQual[p] +
           f * Subcatch[j].newRunoff *Subcatch[j].newQual[p];
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
    if ( runoff < MIN_RUNOFF_FLOW ) runoff = 0.0;
    x[SUBCATCH_RUNOFF] = (float)(runoff* UCF(FLOW));

    // --- retrieve groundwater results
    gw = Subcatch[j].groundwater;
    if ( gw )
    {
        z = (f1 * gw->oldFlow + f * gw->newFlow) * Subcatch[j].area * UCF(FLOW);
        x[SUBCATCH_GW_FLOW] = (float)z;
        z = (Aquifer[gw->aquifer].bottomElev + gw->lowerDepth) * UCF(LENGTH);
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
        if ( runoff < MIN_RUNOFF_FLOW ) z = 0.0;
        else z = f1 * Subcatch[j].oldQual[p] + f * Subcatch[j].newQual[p];
        x[SUBCATCH_WASHOFF+p] = (float)z;
    }
}


//=============================================================================
//                              SUB-AREA METHODS
//=============================================================================

void getSubareaRunoff(int j, int i, double precip, double evap, double tStep)
//
//  Purpose: computes runoff & losses from a subarea over the current time step.
//  Input:   j = subcatchment index
//           i = subarea index
//           precip = rainfall + snowmelt over subarea (ft/sec)
//           evap = evaporation (ft/sec)
//           tStep = time step (sec)
//  Output:  none
//
{
    double    tRunoff;                 // time over which runoff occurs (sec)
    double    oldRunoff;               // runoff from previous time period
    double    surfMoisture;            // surface water available (ft/sec)
    double    surfEvap;                // evap. used for surface water (ft/sec)
    double    infil;                   // infiltration rate (ft/sec)
    TSubarea* subarea;                 // pointer to subarea being analyzed

    // --- assign pointer to current subarea
    subarea = &Subcatch[j].subArea[i];

    // --- assume runoff occurs over entire time step
    tRunoff = tStep;

    // --- initialize runoff & losses
    oldRunoff = subarea->runoff;
    subarea->runoff = 0.0;
    infil    = 0.0;
    Vevap    = 0.0;
    Vinfil   = 0.0;
    Voutflow = 0.0;
    Losses   = 0.0;
    Outflow  = 0.0;

    // --- no runoff if no area
    if ( subarea->fArea == 0.0 ) return;

    // --- determine evaporation loss rate
    surfMoisture = subarea->depth / tStep;
    surfEvap = MIN(surfMoisture, evap);

    // --- compute infiltration loss rate
    if ( i == PERV ) infil = getSubareaInfil(j, subarea, precip, tStep);

    // --- add precip to other subarea inflows
    subarea->inflow += precip;
    surfMoisture += subarea->inflow;

    // --- save volumes lost to evaporation & infiltration
    Vevap = surfEvap * tStep;
    Vinfil = infil * tStep;

    // --- if losses exceed available moisture then no ponded water remains
    Losses = surfEvap + infil;
    if ( Losses >= surfMoisture )
    {
        Losses = surfMoisture;
        subarea->depth = 0.0;
    }

    // --- otherwise update depth of ponded water
    //     and time over which runoff occurs
    else updatePondedDepth(subarea, &tRunoff);

    // --- compute runoff based on updated ponded depth
    findSubareaRunoff(subarea, tRunoff);

    // --- compute runoff volume leaving subcatchment for mass balance purposes
    //     (fOutlet is the fraction of this subarea's runoff that goes to the
    //     subcatchment outlet as opposed to another subarea of the subcatchment)
    if ( subarea->fOutlet > 0.0 )
    {
        Voutflow = 0.5 * (oldRunoff + subarea->runoff) * tRunoff
                  * subarea->fOutlet;
        Outflow = subarea->fOutlet * subarea->runoff;
    }
}

//=============================================================================

double getSubareaInfil(int j, TSubarea* subarea, double precip, double tStep)
//
//  Purpose: computes infiltration rate at current time step.
//  Input:   j = subcatchment index
//           subarea = ptr. to a subarea
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

void findSubareaRunoff(TSubarea* subarea, double tRunoff)
//
//  Purpose: computes runoff (ft/s) from subarea after current time step.
//  Input:   subarea = ptr. to a subarea
//           tRunoff = time step over which runoff occurs (sec)
//  Output:  none
//
{
    double xDepth = subarea->depth - subarea->dStore;

    if ( xDepth > ZERO )
    {
        // --- case where nonlinear routing is used
        if ( subarea->N > 0.0 )
        {
            subarea->runoff = subarea->alpha * pow(xDepth, MEXP);
        }

        // --- case where no routing is used (Mannings N = 0)
        else
        {
            subarea->runoff = xDepth / tRunoff;
            subarea->depth = subarea->dStore;
        }
    }
    else
    {    
        subarea->runoff = 0.0;
    }
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
    double ix;                         // excess inflow to subarea (ft/sec)
    double dx;                         // depth above depression storage (ft)
    double tx = *dt;                   // time over which dx > 0 (sec)

    // --- excess inflow = total inflow - losses
    ix = subarea->inflow - Losses;

    // --- see if not enough inflow to fill depression storage (dStore)
    if ( subarea->depth + ix*tx <= subarea->dStore )
    {
        subarea->depth += ix * tx;
    }

    // --- otherwise use the ODE solver to integrate flow depth
    else
    {
        // --- if depth < dStore then fill up dStore & reduce time step
        dx = subarea->dStore - subarea->depth;
        if ( dx > 0.0 && ix > 0.0 )
        {
            tx -= dx / ix;
            subarea->depth = subarea->dStore;
        }

        // --- now integrate depth over remaining time step tx
        if ( subarea->alpha > 0.0 && tx > 0.0 )
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
    double ix = theSubarea->inflow - Losses;
    double rx = *d - theSubarea->dStore;
    if ( rx < 0.0 )
    {
        rx = 0.0;
    }
    else
    {
        rx = theSubarea->alpha * pow(rx, MEXP);
    }
    *dddt = ix - rx;
}

//=============================================================================
