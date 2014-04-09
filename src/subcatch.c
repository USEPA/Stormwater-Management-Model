//-----------------------------------------------------------------------------
//   subcatch.c
//
//   Project:  EPA SWMM5
//   Version:  5.0
//   Date:     6/19/07   (Build 5.0.010)
//             7/16/07   (Build 5.0.011)
//             2/4/08    (Build 5.0.012)
//             1/21/09   (Build 5.0.014)
//             10/7/09   (Build 5.0.017)
//             07/30/10  (Build 5.0.019)
//             09/30/10  (Build 5.0.021)
//             04/20/11  (Build 5.0.022)
//   Author:   L. Rossman
//
//   Subcatchment runoff & quality functions.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include <string.h>
#include "headers.h"
#include "lid.h"                                                               //(5.0.019 - LR)
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
static  double     Vevap;         // subcatch evap. volume over a time step (ft)
static  double     Vinfil;        // subcatch infil. volume over a time step (ft)
static  double     Voutflow;      // subcatch outflow volume over a time step (ft)
static  double     Vponded;       // subcatch ponded volume (ft)
static  TSubarea* theSubarea;     // subarea to which getDdDt() is applied
static  char *RunoffRoutingWords[] = { w_OUTLET,  w_IMPERV, w_PERV, NULL};

//-----------------------------------------------------------------------------
//  Imported variables (declared in RUNOFF.C)
//-----------------------------------------------------------------------------
extern  double*    WashoffQual;   // washoff quality for a subcatchment (mass/ft3)
extern  double*    WashoffLoad;   // washoff loads for a landuse (mass/sec)

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

//  subcatch_getFracPerv       (called from gwater_initState)                  //(5.0.019 - LR)
//  subcatch_getStorage        (called from massbal_getRunoffError)            //(5.0.019 - LR)

//  subcatch_getWtdOutflow     (called from addWetWeatherInflows in routing.c)
//  subcatch_getWtdWashoff     (called from addWetWeatherInflows in routing.c)

//  subcatch_getResults        (called from output_saveSubcatchResults)

//-----------------------------------------------------------------------------
// Function declarations
//-----------------------------------------------------------------------------
static char   sweptSurfacesDry(int subcatch);
static void   getNetPrecip(int j, double* netPrecip, double tStep);            //(5.0.019 - LR)
static void   getSubareaRunoff(int subcatch, int subarea, double rainfall,
              double evap, double tStep);
static double getSubareaInfil(int j, TSubarea* subarea, double tStep);         //(5.0.021 - LR)
static void   findSubareaRunoff(TSubarea* subarea, double tRunoff);            //(5.0.019 - LR)
static void   updatePondedDepth(TSubarea* subarea, double* tx);
static void   getDdDt(double t, double* d, double* dddt);
static void   getPondedQual(double wUp[], double qUp, double qPpt,
              double qEvap, double qInfil, double v, double area,
              double tStep, double pondedQual[]);

// (getCstrQual function replaced with call to qualrout_getCstrQual.)          //(5.0.014 - LR)

static void   getWashoffQual(int j, double runoff, double tStep,
              double washoffQual[]);
static void   combineWashoffQual(int j, double pondedQual[],
              double washoffQual[], double tStep);
static double getBmpRemoval(int j, int p);

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
    if ( Subcatch[j].fracImperv == 0.0                                         //(5.0.019 - LR)
    ||   Subcatch[j].fracImperv == 1.0 ) k = TO_OUTLET;                        //(5.0.019 - LR)
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

////  This function was re-written for the SWMM5 LID edition  ////             //(5.0.019 - LR)

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
    TGroundwater* gw;                                                          //(5.0.022 - LR)

    // --- check for ambiguous outlet name
    if ( Subcatch[j].outNode >= 0 && Subcatch[j].outSubcatch >= 0 )
        report_writeErrorMsg(ERR_SUBCATCH_OUTLET, Subcatch[j].ID);

////  Added to release 5.0.022  ////                                           //(5.0.022 - LR)
    // --- check if surface elevation is above initial groundwater elevation
    gw = Subcatch[j].groundwater;
    if ( gw )
    {
        if ( gw->surfElev < Aquifer[gw->aquifer].waterTableElev )
            report_writeErrorMsg(ERR_GROUND_ELEV, Subcatch[j].ID);
    }
///////////////////////////////////

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
    double f;                          // fraction of total area
    double area;                       // area (ft2 or acre or ha)
    double curb;                       // curb length (users units)
    double startDrySeconds;            // antecedent dry period (sec)
    double buildup;                    // initial mass buildup (lbs or kg)

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

    // --- first convert antecedent dry days into seconds
    startDrySeconds = StartDryDays*SECperDAY;

    // --- then examine each land use
    for (i = 0; i < Nobjects[LANDUSE]; i++)
    {
        // --- initialize date when last swept
        Subcatch[j].landFactor[i].lastSwept =
            datetime_addSeconds(StartDateTime, -Landuse[i].sweepDays0*SECperDAY);

        // --- determine area and curb length covered by land use
        f = Subcatch[j].landFactor[i].fraction;
        area = f * Subcatch[j].area * UCF(LANDAREA);
        curb = f * Subcatch[j].curbLength;

        // --- examine each pollutant
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            // --- if an initial loading was supplied, then use it to
            //     find the starting buildup over the land use
            buildup = 0.0;
            if ( Subcatch[j].initBuildup[p] > 0.0 )
            {
                buildup = Subcatch[j].initBuildup[p] * area;
            }

            // --- otherwise use the land use's buildup function to 
            //     compute a buildup over the antecedent dry period
            else buildup = landuse_getBuildup(i, p, area, curb, buildup,
                           startDrySeconds);
            Subcatch[j].landFactor[i].buildup[p] = buildup;
        }
    }
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
    Subcatch[j].runon = 0.0;
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

////  This function was added for the SWMM5 LID edition  ////                  //(5.0.019 - LR)

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

////  This function was added for the SWMM5 LID edition  ////                  //(5.0.019 - LR)

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
        //if ( q < MIN_RUNOFF_FLOW ) q = 0.0;


        Subcatch[j].subArea[PERV].inflow += q *               //(q1 + q2) *
             (1.0 - Subcatch[j].subArea[IMPERV0].fOutlet) /
             Subcatch[j].subArea[PERV].fArea;
    }

    // --- Case 2: perv --> imperv
    if ( Subcatch[j].fracImperv > 0.0 &&
         Subcatch[j].subArea[PERV].routeTo == TO_IMPERV &&
         Subcatch[j].subArea[IMPERV1].fArea > 0.0 )
    {

        q = Subcatch[j].subArea[PERV].runoff;
        //if ( q < MIN_RUNOFF_FLOW ) q = 0.0;

        Subcatch[j].subArea[IMPERV1].inflow +=
            q *                                           //Subcatch[j].subArea[PERV].runoff * 
            (1.0 - Subcatch[j].subArea[PERV].fOutlet) *
            Subcatch[j].subArea[PERV].fArea /
            Subcatch[j].subArea[IMPERV1].fArea;
    }

////  Following section added for SWMM5 LID edition  ////                      //(5.0.019 - LR)
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

////  This function was re-written for the SWMM5 LID edition  ////             //(5.0.019 - LR)

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
    double rainVol;                    // rain volume (ft3)
    double runonVol    = 0.0;          // runon volume (ft3)
    double evapVol     = 0.0;          // evaporation volume (ft3)
    double infilVol    = 0.0;          // infiltration volume (ft3)
    double outflowVol  = 0.0;          // runoff volume leaving subcatch (ft3)
    double outflow     = 0.0;          // runoff rate leaving subcatch (cfs)
    double runoff      = 0.0;          // total runoff rate on subcatch (ft/sec)
    double pervEvapVol = 0.0;          // evaporation over pervious area (ft3)
    double evapRate    = 0.0;          // max. evaporation rate (ft/sec)       //(5.0.019 - LR)

    // NOTE: The 'runoff' value returned by this function is the total runoff
    //       generated (in ft/sec) by the subcatchment before any internal
    //       re-routing is applied. It is used in the Exponential Washoff
    //       function to compute pollutant washoff. The 'outflow' value
    //       computed here (in cfs) is the runoff that actually leaves the
    //       subcatchment (which can be reduced by internal re-routing and
    //       LID controls) and is saved to Subcatch[j].newRunoff.

    // --- save current depth of ponded water
    Vponded = subcatch_getDepth(j);

    // --- get net precipitation (rainfall + snowmelt) on subcatchment
    getNetPrecip(j, netPrecip, tStep);
    if ( Evap.dryOnly && Subcatch[j].rainfall > 0.0 ) evapRate = 0.0;          //(5.0.019 - LR)
    else evapRate = Evap.rate;                                                 //(5.0.019 - LR)

    // --- initialize loss rate & runoff rates
    Subcatch[j].losses = 0.0;
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
            getSubareaRunoff(j, i, netPrecip[i], evapRate, tStep);             //(5.0.019 - LR)
            runoff += Subcatch[j].subArea[i].runoff * area;

            // --- update components of volumetric water balance (in ft3)
            Subcatch[j].losses += Losses * area;
            outflow    += Outflow * area;
            evapVol    += Vevap * area;
            infilVol   += Vinfil * area;
            outflowVol += Voutflow * area;

            // --- save evap losses from pervious area
            //     (needed for groundwater modeling)
            if ( i == PERV ) pervEvapVol += Vevap * area;                      //(5.0.021 - LR)
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
    Subcatch[j].losses = (evapVol + infilVol) / tStep / area;

    // --- compute water flux volumes over the time step
    rainVol = Subcatch[j].rainfall * tStep * area;
    runonVol = Subcatch[j].runon * tStep * area;
    outflowVol = outflow * tStep;
    stats_updateSubcatchStats(j, rainVol, runonVol, evapVol, infilVol,
                              outflowVol, outflow);

    // --- update system flow balance
    //     (system outflow is 0 if outlet is another subcatch)
    if ( Subcatch[j].outNode == -1 && Subcatch[j].outSubcatch != j )
    {
        outflowVol = 0.0;
    }
    massbal_updateRunoffTotals(rainVol, evapVol, infilVol, outflowVol);

    // --- return area-averaged runoff (ft/s)
    return runoff / area;
}

//=============================================================================

////  This function was added for the SWMM5 LID edition  ////                  //(5.0.019 - LR)

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

////  This function was re-written for the SWMM5 LID edition  ////             //(5.0.019 - LR)

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
            lid_getSurfaceDepth(j) * Subcatch[j].lidArea) /Subcatch[j].area;
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
            // --- see if snow-only buildup is in effect                       //(5.0.012 - LR)
            if (Pollut[p].snowOnly                                             //(5.0.012 - LR)
            && Subcatch[j].newSnowDepth < 0.001/12.0) continue;                //(5.0.012 - LR)

            // --- use land use's buildup function to update buildup amount
            oldBuildup = Subcatch[j].landFactor[i].buildup[p];        
            newBuildup = landuse_getBuildup(i, p, area, curb, oldBuildup,
                         tStep);
            newBuildup = MAX(newBuildup, oldBuildup);                          //(5.0.014 - LR)
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

    // --- no sweeping occurs if subcatch's surfaces are not dry
    if ( !sweptSurfacesDry(j) ) return;

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
//     runon, deposition, infil., & evap.
//  2. washoff of pollutant buildup as described by the project's land
//     use washoff functions.
//
{
    double v;                          // ponded depth (ft)
    double qUp;                        // runon inflow rate (ft/sec)
    double qPpt;                       // precipitation rate (ft/sec)
    double qInfil;                     // infiltration rate (ft/sec)
    double qEvap;                      // evaporation rate (ft/sec)
    double area;                       // subcatchment area (ft2)
    double *wUp;                       // runon inflow loads (mass/sec)
    double *pondedQual;                // quality of ponded water (mass/ft3)

    // --- return if there is no area or no pollutants
    area = Subcatch[j].area;
    if ( Nobjects[POLLUT] == 0 || area == 0.0 ) return;

    // --- get flow rates of the various inflows & outflows
    qUp    = Subcatch[j].runon;             // upstream runon
    qPpt   = Subcatch[j].rainfall;          // precipitation (rain + snow)
    qEvap  = Vevap / tStep;                 // evaporation
    qInfil = Vinfil / tStep;                // infiltration

    // --- assign upstream runon load (computed previously from call to 
    //     subcatch_getRunon) and ponded quality to local variables
    //     merely for notational convenience
    wUp = Subcatch[j].newQual;
    pondedQual = Subcatch[j].pondedQual;

    // --- average the ponded depth volumes over the time step
    v = 0.5 * (Vponded + subcatch_getDepth(j));

    // --- get quality in surface ponding at end of time step
    getPondedQual(wUp, qUp, qPpt, qEvap, qInfil, v, area, tStep, pondedQual);

    // --- get quality in washoff from pollutant buildup
    getWashoffQual(j, runoff, tStep, WashoffQual);

    // --- combine ponded & washoff quality in the subcatchment's outflow
    //     (updates Subcatch[j].newQual[])
    combineWashoffQual(j, pondedQual, WashoffQual, tStep);                     //(5.0.014 - LR)
}

//=============================================================================

void getPondedQual(double wUp[], double qUp, double qPpt, double qEvap, 
                   double qInfil, double v, double area, double tStep,
                   double pondedQual[])
//
//  Input:   wUp[]  = runon load from upstream subcatchments (mass/sec)
//           qUp    = runon inflow flow rate (ft/sec)
//           qPpt   = precip. rate (ft/sec)
//           qEvap  = evaporation rate (ft/sec)
//           qInfil = infiltration rate (ft/sec)
//           v      = ponded depth (ft)
//           area   = subcatchment area (ft2)
//           tStep  = time step (sec)
//  Output:  pondedQual[] = pollutant concentrations in ponded water (mass/ft3)
//  Purpose: computes new quality of ponded surface water in a subcatchment.
//
{
    int    p;
    double wIn, wPpt;
    double massLoad;
    double qIn = qUp + qPpt - qEvap;                                           //(5.0.014 - LR)

    // --- analyze each individual pollutant
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {

        // --- compute direct deposition from precip. in mass/sec
        wPpt = Pollut[p].pptConcen * LperFT3 * qPpt * area;

        // --- add direct deposition (in lbs or kg) to mass balance
        massLoad = wPpt * tStep * Pollut[p].mcf;
        massbal_updateLoadingTotals(DEPOSITION_LOAD, p, massLoad);

        // --- add infiltration loss to mass balance
        massLoad = pondedQual[p] * qInfil * area * tStep * Pollut[p].mcf;
        massbal_updateLoadingTotals(INFIL_LOAD, p, massLoad);

        // ---- add direct deposition to runon load from upstream subcatchments
        wIn = (wPpt + wUp[p]) / area;
    
        // --- update ponded concentration (in mass/ft3)
        pondedQual[p] = qualrout_getCstrQual(pondedQual[p], v, wIn, qIn, 0.0,  //(5.0.014 - LR)
                                             tStep);                           //(5.0.014 - LR)
    }
}

//=============================================================================

void  getWashoffQual(int j, double runoff, double tStep, double washoffQual[])
//
//  Input:   j       = subcatchment index
//           runoff  = runoff flow rate over entire subcatchment (ft/sec)
//           tStep   = time step (sec)
//  Output:  washoffQual[] = quality of surface washoff (mass/ft3)
//  Purpose: finds concentrations in washoff from pollutant buildup over a
//           subcatchment within a time step.
//
{
    int    p;                          // pollutant index
    int    i;                          // land use index
    double area;                       // subcatchment area (ft2)

    // --- initialize total washoff quality from subcatchment
    area = Subcatch[j].area;
    for (p = 0; p < Nobjects[POLLUT]; p++) washoffQual[p] = 0.0;

    // --- no washoff if no runoff                                             //(5.0.014 - LR)
    if ( runoff < MIN_RUNOFF ) return;                                         //(5.0.014 - LR)

    // --- get local washoff mass flow from each landuse and add to total        
    for (i = 0; i < Nobjects[LANDUSE]; i++)
    {
        if ( Subcatch[j].landFactor[i].fraction == 0.0 ) continue;
        landuse_getWashoff(i, area, Subcatch[j].landFactor, runoff, tStep,
            WashoffLoad);
        for (p = 0; p < Nobjects[POLLUT]; p++) {
            washoffQual[p] += WashoffLoad[p];
        }
    }

    // --- convert from mass/sec to mass/ft3
    runoff *= area;
    for (p = 0; p < Nobjects[POLLUT]; p++) washoffQual[p] /= runoff; 
}

//=============================================================================

void combineWashoffQual(int j, double pondedQual[], double washoffQual[],
                        double tStep)
//
//  Input:   j             = subcatchment index
//           pondedQual[]  = quality of ponded water (mass/ft3)
//           washoffQual[] = quality of washoff (mass/ft3)
//           tStep         = time step (sec)
//  Output:  updates Subcatch[j].newQual[]
//  Purpose: computes combined concentration of ponded water & washoff streams

////  ----  This function was re-written for Release 5.0.014.  ----  ////      //(5.0.014 - LR)
{
    int   p;
    double qOut, cOut, cPonded, bmpRemoval, massLoad;

    qOut = Subcatch[j].newRunoff;
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        // --- zero concen. if no runoff flow
        if ( qOut <= FUDGE ) cOut = 0.0;                                       //(5.0.017 - LR)
        else
        {
            // --- apply any BMP removal to ponded water
            cPonded = pondedQual[p];
            bmpRemoval = getBmpRemoval(j, p) * cPonded;
            if ( bmpRemoval > 0.0 )
            {
                massLoad = bmpRemoval * qOut *  tStep * Pollut[p].mcf; 
                massbal_updateLoadingTotals(BMP_REMOVAL_LOAD, p, massLoad);
                cPonded -= bmpRemoval;
            }

            // --- add concen. of ponded water to that of washoff
            cOut = cPonded + washoffQual[p];
        }
    
        // --- save new outflow runoff concentration (in mass/L)
        Subcatch[j].newQual[p] = MAX(cOut, 0.0) / LperFT3;


        // --- update total runoff pollutant load from subcatchment
        massLoad = 0.5 * (Subcatch[j].oldQual[p]*Subcatch[j].oldRunoff +
                          Subcatch[j].newQual[p]*Subcatch[j].newRunoff) *
                          LperFT3 * tStep * Pollut[p].mcf;
        Subcatch[j].totalLoad[p] += massLoad;

        // --- update mass balance if runoff goes to an outlet node
        if ( Subcatch[j].outNode >= 0 ) 
        {
            massbal_updateLoadingTotals(RUNOFF_LOAD, p, massLoad);
        }
    }
}

//=============================================================================

double getBmpRemoval(int j, int p)
//
//  Input:   j = subcatchment index
//           p = pollutant index
//  Output:  returns a BMP removal fraction for pollutant p
//  Purpose: finds the overall average BMP removal achieved for pollutant p
//           treated in subcatchment j.
{
    int    i;
    double r = 0.0;
    for (i = 0; i < Nobjects[LANDUSE]; i++)
    {
        r += Subcatch[j].landFactor[i].fraction *
             Landuse[i].washoffFunc[p].bmpEffic;
    }
    return r;
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
    return (1.0 - f) * Subcatch[j].oldQual[p] + f * Subcatch[j].newQual[p];
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
    x[SUBCATCH_LOSSES] = (float)(Subcatch[j].losses * UCF(RAINFALL));
    z = ( f1 * Subcatch[j].oldRunoff +
          f * Subcatch[j].newRunoff ) * UCF(FLOW);
    x[SUBCATCH_RUNOFF] = (float)z;

    // --- retrieve groundwater flow & water table if present
    gw = Subcatch[j].groundwater;
    if ( gw )
    {
        z = (f1 * gw->oldFlow + f * gw->newFlow) *
             Subcatch[j].area * UCF(FLOW);
        x[SUBCATCH_GW_FLOW] = (float)z;
        z = (Aquifer[gw->aquifer].bottomElev +
             gw->lowerDepth) * UCF(LENGTH);
        x[SUBCATCH_GW_ELEV] = (float)z;
    }
    else
    {
        x[SUBCATCH_GW_FLOW] = 0.0f;
        x[SUBCATCH_GW_ELEV] = 0.0f;
    }

    // --- retrieve pollutant washoff
    for (p = 0; p < Nobjects[POLLUT]; p++ )
    {
        z = f1 * Subcatch[j].oldQual[p] +
            f * Subcatch[j].newQual[p];
        x[SUBCATCH_WASHOFF+p] = (float)z;
    }
}

//=============================================================================

//// This function was modified for the SWMM5 LID edition  ////                //(5.0.019 - LR)

char  sweptSurfacesDry(int j)
//
//  Input:   j = subcatchment index
//  Output:  returns TRUE if subcatchment surfaces are dry
//  Purpose: checks if surfaces subject to street sweeping are dry.
//
{
    double     depth = 0.0;                      // depth of standing water (ft)
    TSnowpack* snowpack = Subcatch[j].snowpack;  // snowpack data

    // --- check snow depth on plowable impervious area
    if ( snowpack != NULL )
    {
        if ( snowpack->wsnow[IMPERV0] > MIN_TOTAL_DEPTH ) return FALSE;
    }

    // --- check water depth on impervious surfaces
    if ( Subcatch[j].fracImperv > 0.0 )
    {
       depth = (Subcatch[j].subArea[IMPERV0].depth *
                Subcatch[j].subArea[IMPERV0].fArea) +
               (Subcatch[j].subArea[IMPERV1].depth *
                Subcatch[j].subArea[IMPERV1].fArea);
       depth = depth / Subcatch[j].fracImperv;
    }

    // --- check depth on pervious pavement LID
    if ( Subcatch[j].lidArea > 0.0 ) depth = lid_getDepthOnPavement(j, depth);

    if ( depth > MIN_TOTAL_DEPTH ) return FALSE;
    return TRUE;
}


//=============================================================================
//                              SUB-AREA METHODS
//=============================================================================

////  This function was re-written for the SWMM5 LID edition  ////             //(5.0.019 - LR)

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
    double    surfMoisture;            // surface water available (ft/sec)     //(5.0.022 - LR)
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

////  The following code segment was modified for release 5.0.022  ////        //(5.0.022 - LR)

    // --- determine evaporation loss rate
    surfMoisture = subarea->depth / tStep;
    surfEvap = MIN(surfMoisture, evap);

    // --- compute infiltration loss rate
    if ( i == PERV ) infil = getSubareaInfil(j, subarea, tStep);

    // --- add precip to other subarea inflows
    subarea->inflow += precip;
    surfMoisture += subarea->inflow;

////  End of modified code segment  ////                                       //(5.0.022 - LR)

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

////  This function was added for the SWMM5 LID edition  ////                  //(5.0.019 - LR)
////  This function was re-written for release 5.0.021  ////                   //(5.0.021 - LR)
////  This function was re-written for release 5.0.022  ////                   //(5.0.022 - LR)

double getSubareaInfil(int j, TSubarea* subarea, double tStep)
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
    infil = infil_getInfil(j, InfilModel, tStep, Subcatch[j].rainfall,
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

////  This function was added for the SWMM5 LID edition  ////                  //(5.0.019 - LR)

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
