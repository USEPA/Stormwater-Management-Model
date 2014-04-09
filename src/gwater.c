//-----------------------------------------------------------------------------
//   gwater.c
//
//   Project:  EPA SWMM5
//   Version:  5.0
//   Date:     6/19/07   (Build 5.0.010)
//             1/21/09   (Build 5.0.014)
//             07/30/10  (Build 5.0.019)
//             09/30/10  (Build 5.0.021)
//             04/20/11  (Build 5.0.022)
//   Author:   L. Rossman
//
//   Groundwater functions.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <math.h>
#include "headers.h"
#include "odesolve.h"

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
static const double GWTOL = 0.0001;    // ODE solver tolerance
static const double XTOL  = 0.001;     // tolerance on moisture & depth
enum   GWstates {THETA,                // moisture content of upper GW zone
                 LOWERDEPTH};          // depth of lower sat. GW zone

//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
//  NOTE: all flux rates are in ft/sec, all depths are in ft.
static double    Infil;           // infiltration rate from surface
static double    MaxEvap;         // max. evaporation rate
static double    AvailEvap;       // available evaporation rate
static double    UpperEvap;       // evaporation rate from upper GW zone
static double    LowerEvap;       // evaporation rate from lower GW zone
static double    UpperPerc;       // percolation rate from upper to lower zone
static double    LowerLoss;       // loss rate from lower GW zone
static double    GWFlow;          // flow rate from lower zone to conveyance node
static double    MaxUpperPerc;    // upper limit on UpperPerc
static double    MaxGWFlowPos;    // upper limit on GWFlow when its positve
static double    MaxGWFlowNeg;    // upper limit on GWFlow when its negative
static double    FracPerv;        // fraction of surface that is pervious
static double    TotalDepth;      // total depth of GW aquifer

////  ---- these variables are deprecated ----                                 //(5.0.014 - LR)
//static double    NodeInvert;      // elev. of conveyance node invert
//static double    NodeDepth;       // current water depth at conveyance node

////  ---- these variables replace those listed above ----                     //(5.0.014 - LR)
static double    Hstar;           // ht. from aquifer bottom to node invert
static double    Hsw;             // ht. from aquifer bottom to water surface

static TAquifer A;                // aquifer being analyzed
static TGroundwater* GW;          // groundwater object being analyzed

static double Tstep;

//-----------------------------------------------------------------------------
//  External Functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  gwater_readAquiferParams     (called by input_readLine)
//  gwater_readGroundwaterParams (called by input_readLine)
//  gwater_validateAquifer       (called by swmm_open)
//  gwater_initState             (called by subcatch_initState)
//  gwater_getVolume             (called by massbal_open & massbal_getGwaterError)
//  gwater_getGroundwater        (called by getSubareaRunoff in subcatch.c)

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void   getDxDt(double t, double* x, double* dxdt);
//static double getExcessInfil(double* x, double tStep);                       //(5.0.019 - LR)
static void   getFluxes(double upperVolume, double lowerDepth);
static void   getEvapRates(double theta, double upperDepth);
static double getUpperPerc(double theta, double upperDepth);
static double getGWFlow(double lowerDepth);
static void   updateMassBal(double area,  double tStep);


//=============================================================================

int gwater_readAquiferParams(int j, char* tok[], int ntoks)
//
//  Input:   j = aquifer index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error message
//  Purpose: reads aquifer parameter values from line of input data
//
//  Data line contains following parameters:
//    ID, porosity, wiltingPoint, fieldCapacity,     conductivity,
//    conductSlope, tensionSlope, upperEvapFraction, lowerEvapDepth,
//    gwRecession,  bottomElev,   waterTableElev,    upperMoisture
//
{
    int   i;
    double x[12];
    char *id;

    // --- check that aquifer exists
    if ( ntoks < 12 ) return error_setInpError(ERR_ITEMS, "");
    id = project_findID(AQUIFER, tok[0]);
    if ( id == NULL ) return error_setInpError(ERR_NAME, tok[0]);

    // --- read remaining tokens as floats
    for (i = 0; i < 11; i++) x[i] = 0.0;
    for (i = 1; i < 13; i++)
    {
        if ( ! getDouble(tok[i], &x[i-1]) )
            return error_setInpError(ERR_NUMBER, tok[i]);
    }

    // --- assign parameters to aquifer object
    Aquifer[j].ID = id;
    Aquifer[j].porosity       = x[0];
    Aquifer[j].wiltingPoint   = x[1];
    Aquifer[j].fieldCapacity  = x[2];
    Aquifer[j].conductivity   = x[3] / UCF(RAINFALL);
    Aquifer[j].conductSlope   = x[4];
    Aquifer[j].tensionSlope   = x[5] / UCF(LENGTH);
    Aquifer[j].upperEvapFrac  = x[6];
    Aquifer[j].lowerEvapDepth = x[7] / UCF(LENGTH);
    Aquifer[j].lowerLossCoeff = x[8] / UCF(RAINFALL);
    Aquifer[j].bottomElev     = x[9] / UCF(LENGTH);
    Aquifer[j].waterTableElev = x[10] / UCF(LENGTH);
    Aquifer[j].upperMoisture  = x[11];
    return 0;
}

//=============================================================================

int gwater_readGroundwaterParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//  Purpose: reads groundwater inflow parameters for a subcatchment from
//           a line of input data.
//
//  Data format is:
//    subcatch  aquifer  node  surfElev  x0 ... x7 (flow parameters)
//
{
    int    i, j, k, n;
    double x[8];

    TGroundwater* gw;

    // --- check that specified subcatchment, aquifer & node exist
    if ( ntoks < 10 ) return error_setInpError(ERR_ITEMS, "");
    j = project_findObject(SUBCATCH, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);
    k = project_findObject(AQUIFER, tok[1]);
    if ( k < 0 ) return error_setInpError(ERR_NAME, tok[1]);
    n = project_findObject(NODE, tok[2]);
    if ( n < 0 ) return error_setInpError(ERR_NAME, tok[2]);

    // --- read in the groundwater flow parameters as floats
    for ( i = 0; i < 7; i++ )
    {
        if ( ! getDouble(tok[i+3], &x[i]) ) 
            return error_setInpError(ERR_NUMBER, tok[i+3]);
    }

    // --- read in overridden node invert elev.
    x[7] = MISSING;
    if ( ntoks > 10 )
    {
        if ( ! getDouble(tok[10], &x[7]) ) 
            return error_setInpError(ERR_NUMBER, tok[10]);
        x[7] /= UCF(LENGTH);
    }

    // --- create a groundwater flow object
    if ( !Subcatch[j].groundwater )
    {
        gw = (TGroundwater *) malloc(sizeof(TGroundwater));
        if ( !gw ) return error_setInpError(ERR_MEMORY, "");
        Subcatch[j].groundwater = gw;
    }
    else gw = Subcatch[j].groundwater;

    // --- populate the groundwater flow object with its parameters
    gw->aquifer    = k;
    gw->node       = n;
    gw->surfElev   = x[0] / UCF(LENGTH);
    gw->a1         = x[1];
    gw->b1         = x[2];
    gw->a2         = x[3];
    gw->b2         = x[4];
    gw->a3         = x[5];
    gw->fixedDepth = x[6] / UCF(LENGTH);
    gw->nodeElev   = x[7];                       //already converted to ft.
    return 0;
}

//=============================================================================

void  gwater_validateAquifer(int j)
//
//  Input:   j = aquifer index
//  Output:  none
//  Purpose: validates groundwater aquifer properties .
//
{
    if ( Aquifer[j].porosity          <= 0.0 
    ||   Aquifer[j].fieldCapacity     >= Aquifer[j].porosity
    ||   Aquifer[j].wiltingPoint      >= Aquifer[j].fieldCapacity
    ||   Aquifer[j].conductivity      <= 0.0
    ||   Aquifer[j].conductSlope      <  0.0
    ||   Aquifer[j].tensionSlope      <  0.0
    ||   Aquifer[j].upperEvapFrac     <  0.0
    ||   Aquifer[j].lowerEvapDepth    <  0.0
    ||   Aquifer[j].waterTableElev    <  Aquifer[j].bottomElev
    ||   Aquifer[j].upperMoisture     >  Aquifer[j].porosity 
    ||   Aquifer[j].upperMoisture     <  Aquifer[j].wiltingPoint )
        report_writeErrorMsg(ERR_AQUIFER_PARAMS, Aquifer[j].ID);
}

//=============================================================================

void  gwater_initState(int j)
//
//  Input:   j = subcatchment index
//  Output:  none
//  Purpose: initializes state of subcatchment's groundwater.
//
{
    TAquifer a;
    TGroundwater* gw;
    
    gw = Subcatch[j].groundwater;
    if ( gw )
    {
        a = Aquifer[gw->aquifer];
        gw->theta = a.upperMoisture;
        if ( gw->theta >= a.porosity )
        {
            gw->theta = a.porosity - XTOL;
        }
        gw->lowerDepth = a.waterTableElev - a.bottomElev;
        if ( gw->lowerDepth >= gw->surfElev - a.bottomElev )
        {
            gw->lowerDepth = gw->surfElev - a.bottomElev - XTOL;
        }
        gw->oldFlow = 0.0;
        gw->newFlow = 0.0;
        gw->maxInfilVol = (gw->surfElev - a.waterTableElev) *                  //(5.0.019 - LR)
                          (a.porosity - gw->theta) /                           //(5.0.019 - LR)
                          subcatch_getFracPerv(j);                             //(5.0.019 - LR)
    }
}

//=============================================================================

double gwater_getVolume(int j)
//
//  Input:   j = subcatchment index
//  Output:  returns total volume of groundwater in ft/ft2
//  Purpose: finds volume of groundwater stored in upper & lower zones
//
{
    TAquifer a;
    TGroundwater* gw;
    double upperDepth;
    gw = Subcatch[j].groundwater;
    if ( gw == NULL ) return 0.0;
    a = Aquifer[gw->aquifer];
    upperDepth = gw->surfElev - a.bottomElev - gw->lowerDepth;
    return (upperDepth * gw->theta) + (gw->lowerDepth * a.porosity);
}

//=============================================================================

////  This function was re-written for the SWMM5 LID edition  ////             //(5.0.019 - LR)

void gwater_getGroundwater(int j, double evap, double infil, double tStep)
//
//  Purpose: computes groundwater flow from subcatchment during current time step.
//  Input:   j     = subcatchment index
//           evap  = surface evaporation volume consumed (ft3)
//           infil = surface infiltration volume (ft3)
//           tStep = time step (sec)
//  Output:  none
//
{
    int    n;                          // node exchanging groundwater
    double x[2];                       // upper moisture content & lower depth 
    double vUpper;                     // upper vol. available for percolation
    double nodeFlow;                   // max. possible GW flow from node
    double area;                       // total subcatchment area

    // --- save subcatchment's groundwater and aquifer objects to 
    //     shared variables
    GW = Subcatch[j].groundwater;
    if ( GW == NULL ) return;
    A = Aquifer[GW->aquifer];

    // --- get fraction of total area that is pervious
    FracPerv = subcatch_getFracPerv(j);
    if ( FracPerv <= 0.0 ) return;
    area = Subcatch[j].area;

    // --- convert evap & infil volumes to rates
    evap = evap / (FracPerv*area) / tStep;                                     //(5.0.021 - LR)
    infil = infil / (FracPerv*area) / tStep;                                   //(5.0.021 - LR)
    Infil = infil;
    Tstep = tStep;

    // --- save max. and available evap rates to shared variables
    MaxEvap = Evap.rate;
    AvailEvap = MAX((MaxEvap - evap), 0.0);

    // --- save total depth & outlet node properties to shared variables
    TotalDepth = GW->surfElev - A.bottomElev;
    if ( TotalDepth <= 0.0 ) return;
    n = GW->node;

    // --- override node's invert if value was provided in the GW object
    if ( GW->nodeElev != MISSING )
        Hstar = GW->nodeElev - A.bottomElev;
    else Hstar = Node[n].invertElev - A.bottomElev;
    
    if ( GW->fixedDepth > 0.0 )
        Hsw = GW->fixedDepth + Node[n].invertElev - A.bottomElev;
    else Hsw = Node[n].newDepth + Node[n].invertElev - A.bottomElev;

    // --- store state variables in work vector x
    x[THETA] = GW->theta;
    x[LOWERDEPTH] = GW->lowerDepth;

    // --- set limits on upper perc
    vUpper = (TotalDepth - x[LOWERDEPTH]) * (x[THETA] - A.fieldCapacity);
    vUpper = MAX(0.0, vUpper); 
    MaxUpperPerc = vUpper / tStep;

    // --- set limit on GW flow out of aquifer based on volume of lower zone
    MaxGWFlowPos = x[LOWERDEPTH]*A.porosity / tStep;

    // --- set limit on GW flow into aquifer from drainage system node
    //     based on min. of capacity of upper zone and drainage system
    //     inflow to the node
    MaxGWFlowNeg = (TotalDepth - x[LOWERDEPTH]) * (A.porosity - x[THETA])
                   / tStep;
    nodeFlow = (Node[n].inflow + Node[n].newVolume/tStep) / area;
    MaxGWFlowNeg = -MIN(MaxGWFlowNeg, nodeFlow);
    
    // --- integrate eqns. for d(Theta)/dt and d(LowerDepth)/dt
    //     NOTE: ODE solver must have been initialized previously
    odesolve_integrate(x, 2, 0, tStep, GWTOL, tStep, getDxDt);
    
    // --- keep state variables within allowable bounds
    x[THETA] = MAX(x[THETA], A.wiltingPoint);
    if ( x[THETA] >= A.porosity )
    {
        x[THETA] = A.porosity - XTOL;
    }
    x[LOWERDEPTH] = MAX(x[LOWERDEPTH],  0.0);
    if ( x[LOWERDEPTH] >= TotalDepth )
    {
        x[LOWERDEPTH] = TotalDepth - XTOL;
    }

    // --- save new state values
    GW->theta = x[THETA];
    GW->lowerDepth  = x[LOWERDEPTH];
    getFluxes(GW->theta, GW->lowerDepth);
    GW->oldFlow = GW->newFlow;
    GW->newFlow = GWFlow;

    //--- get limit on infiltration into upper zone
    GW->maxInfilVol = (TotalDepth - x[LOWERDEPTH])*
                      (A.porosity - x[THETA])/ FracPerv;

    // --- update mass balance
    updateMassBal(area, tStep);
}

//=============================================================================

void updateMassBal(double area, double tStep)
//
//  Input:   area  = subcatchment area (ft2)
//           tStep = time step (sec)
//  Output:  none
//  Purpose: updates GW mass balance with volumes of water fluxes.
//
{
    double vInfil;                     // infiltration volume
    double vUpperEvap;                 // upper zone evap. volume
    double vLowerEvap;                 // lower zone evap. volume
    double vLowerPerc;                 // lower zone deep perc. volume
    double vGwater;                    // volume of exchanged groundwater
    double ft2sec = area * tStep;

    vInfil     = Infil * FracPerv * ft2sec;
    vUpperEvap = UpperEvap * FracPerv * ft2sec;
    vLowerEvap = LowerEvap * FracPerv * ft2sec;
    vLowerPerc = LowerLoss * ft2sec;
    vGwater    = 0.5 * (GW->oldFlow + GW->newFlow) * ft2sec;
    massbal_updateGwaterTotals(vInfil, vUpperEvap, vLowerEvap, vLowerPerc,
                               vGwater);
}

//=============================================================================

void  getFluxes(double theta, double lowerDepth)
//
//  Input:   upperVolume = vol. depth of upper zone (ft)
//           upperDepth  = depth of upper zone (ft)
//  Output:  none
//  Purpose: computes water fluxes into/out of upper/lower GW zones.
//
{
    double upperDepth;

    // --- find upper zone depth
    lowerDepth = MAX(lowerDepth, 0.0);
    lowerDepth = MIN(lowerDepth, TotalDepth);
    upperDepth = TotalDepth - lowerDepth;

    // --- find evaporation from both zones
    getEvapRates(theta, upperDepth);

    // --- find percolation rate at upper & lower zone boundaries
    UpperPerc = getUpperPerc(theta, upperDepth);
    UpperPerc = MIN(UpperPerc, MaxUpperPerc);

    // --- find losses to deep GW
    LowerLoss = A.lowerLossCoeff * lowerDepth / TotalDepth;

    // --- find GW flow from lower zone to conveyance system node
    GWFlow = getGWFlow(lowerDepth);
    if ( GWFlow >= 0.0 ) GWFlow = MIN(GWFlow, MaxGWFlowPos);
    else GWFlow = MAX(GWFlow, MaxGWFlowNeg);
}

//=============================================================================

void  getDxDt(double t, double* x, double* dxdt)
//
//  Input:   t    = current time (not used)
//           x    = array of state variables
//  Output:  dxdt = array of time derivatives of state variables
//  Purpose: computes time derivatives of upper moisture content 
//           and lower depth.
//
{
    double qUpper, qLower;
    double denom;                                                              //(5.0.022 - LR)

    getFluxes(x[THETA], x[LOWERDEPTH]);
    qUpper = (Infil - UpperEvap)*FracPerv - UpperPerc;
    qLower = UpperPerc - LowerLoss - (LowerEvap*FracPerv) - GWFlow;

////  Deprecated (as of release 5.0.014)  ////
////  Restored (as of release 5.0.019)  ////
////  Modified to prevent illegal values (release 5.0.022)  ////               //(5.0.022 - LR)
    denom = TotalDepth - x[LOWERDEPTH];
    if (denom > 0.0)
        dxdt[THETA] = qUpper / denom;
    else
        dxdt[THETA] = 0.0;
    denom = A.porosity - x[THETA];
    if (denom > 0.0)
        dxdt[LOWERDEPTH] = qLower / denom;
    else
        dxdt[LOWERDEPTH] = 0.0;
}

//=============================================================================

void getEvapRates(double theta, double upperDepth)
//
//  Input:   theta      = moisture content of upper zone
//           upperDepth = depth of upper zone (ft)
//  Output:  none
//  Purpose: computes evapotranspiration out of upper & lower zones.
//
{
    double lowerFrac;
    UpperEvap = A.upperEvapFrac * MaxEvap;
    if ( theta <= A.wiltingPoint || Infil > 0.0 ) UpperEvap = 0.0;
    else 
    {
        UpperEvap = MIN(UpperEvap, AvailEvap);
    }
    if ( A.lowerEvapDepth == 0.0 ) LowerEvap = 0.0;
    else
    {
        lowerFrac = (A.lowerEvapDepth - upperDepth) / A.lowerEvapDepth;
        lowerFrac = MAX(0.0, lowerFrac);
        
        lowerFrac = MIN(lowerFrac, 1.0);
        LowerEvap = lowerFrac * (AvailEvap - UpperEvap);

        //LowerEvap = (1.0 - A.upperEvapFrac) * MaxEvap * lowerFrac;
        //LowerEvap = MIN(LowerEvap, (AvailEvap - UpperEvap));
        //LowerEvap = MAX(0.0, LowerEvap);
    }
}

//=============================================================================

double getUpperPerc(double theta, double upperDepth)
//
//  Input:   theta      = moisture content of upper zone
//           upperDepth = depth of upper zone (ft)
//  Output:  returns percolation rate (ft/sec)
//  Purpose: finds percolation rate from upper to lower zone.
//
{
    double delta;                       // unfilled water content of upper zone
    double dhdz;                        // avg. change in head with depth
    double hydcon;                      // unsaturated hydraulic conductivity

    // --- no perc. from upper zone if no depth or moisture content too low    
    if ( upperDepth <= 0.0 || theta <= A.fieldCapacity ) return 0.0;

    // --- compute hyd. conductivity as function of moisture content
    delta = theta - A.porosity;
    hydcon = A.conductivity * exp(delta * A.conductSlope);

    // --- compute integral of dh/dz term
    delta = theta - A.fieldCapacity;
    dhdz = 1.0 + A.tensionSlope * 2.0 * delta / upperDepth;

    // --- compute upper zone percolation rate
    return hydcon * dhdz;
}

//=============================================================================

double getGWFlow(double lowerDepth)

////   ---- this function was entirely re-written ----                         //(5.0.014 - LR)

//
//  Input:   lowerDepth = depth of lower zone (ft)
//  Output:  returns groundwater flow rate (ft/sec)
//  Purpose: finds groundwater outflow from lower saturated zone.
//
{
    double q, t1, t2, t3;

    // --- water table must be above Hstar for flow to occur
    if ( lowerDepth <= Hstar ) return 0.0;

    // --- compute groundwater component of flow
    if ( GW->b1 == 0.0 ) t1 = GW->a1;
    else t1 = GW->a1 * pow( (lowerDepth - Hstar)*UCF(LENGTH), GW->b1);

    // --- compute surface water component of flow
    if ( GW->b2 == 0.0 ) t2 = GW->a2;
    else if (Hsw > Hstar)                                                      //(5.0.022 - LR) 
        t2 = GW->a2 * pow( (Hsw - Hstar)*UCF(LENGTH), GW->b2);                 //(5.0.022 - LR)
    else t2 = 0.0;                                                             //(5.0.022 - LR)

    // --- compute groundwater/surface water interaction term
    t3 = GW->a3 * lowerDepth * Hsw * UCF(LENGTH) * UCF(LENGTH);

    // --- compute total groundwater flow
    q = (t1 - t2 + t3) / UCF(GWFLOW); 
    if ( q < 0.0 && GW->a3 != 0.0 ) q = 0.0;
    return q;
}

//=============================================================================
