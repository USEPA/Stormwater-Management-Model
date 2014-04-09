//-----------------------------------------------------------------------------
//   landuse.c
//
//   Project:  EPA SWMM5
//   Version:  5.0
//   Date:     6/19/07   (Build 5.0.010)
//             2/4/08    (Build 5.0.012)
//             1/21/09   (Build 5.0.014)
//             10/7/09   (Build 5.0.017)
//             07/30/10  (Build 5.0.019)
//   Author:   L. Rossman
//
//   Pollutant buildup and washoff functions.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include <string.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  landuse_readParams        (called by parseLine in input.c)
//  landuse_readPollutParams  (called by parseLine in input.c)
//  landuse_readBuildupParams (called by parseLine in input.c)
//  landuse_readWashoffParams (called by parseLine in input.c)
//  landuse_getBuildup        (called by subcatch_getBuildup)
//  landuse_getWashoff        (called by getWashoffLoads in subcatch.c)

//-----------------------------------------------------------------------------
// Function declarations
//-----------------------------------------------------------------------------
static double landuse_getBuildupDays(int landuse, int pollut, double buildup);
static double landuse_getBuildupMass(int landuse, int pollut, double days);
static double landuse_getRunoffLoad(int landuse, int pollut, double area,
              TLandFactor landFactor[], double runoff, double tStep);
static double landuse_getWashoffMass(int landuse, int pollut, double buildup,
              double runoff, double area);
static double landuse_getCoPollutLoad(int pollut, double washoff[], double tStep);
static double landuse_getExternalBuildup(int i, int p, double buildup,
              double tStep);

//=============================================================================

int  landuse_readParams(int j, char* tok[], int ntoks)
//
//  Input:   j = land use index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads landuse parameters from a tokenized line of input.
//
//  Data format is:
//    landuseID  (sweepInterval sweepRemoval sweepDays0)
//
{
    char *id;
    if ( ntoks < 1 ) return error_setInpError(ERR_ITEMS, "");
    id = project_findID(LANDUSE, tok[0]);
    if ( id == NULL ) return error_setInpError(ERR_NAME, tok[0]);
    Landuse[j].ID = id;
    if ( ntoks > 1 )
    {
        if ( ntoks < 4 ) return error_setInpError(ERR_ITEMS, "");
        if ( ! getDouble(tok[1], &Landuse[j].sweepInterval) )
            return error_setInpError(ERR_NUMBER, tok[1]);
        if ( ! getDouble(tok[2], &Landuse[j].sweepRemoval) )
            return error_setInpError(ERR_NUMBER, tok[2]);
        if ( ! getDouble(tok[3], &Landuse[j].sweepDays0) )
            return error_setInpError(ERR_NUMBER, tok[3]);
    }
    else
    {
        Landuse[j].sweepInterval = 0.0;
        Landuse[j].sweepRemoval = 0.0;
        Landuse[j].sweepDays0 = 0.0;
    }
    if ( Landuse[j].sweepRemoval < 0.0
        || Landuse[j].sweepRemoval > 1.0 )
        return error_setInpError(ERR_NUMBER, tok[2]);
    return 0;
}

//=============================================================================

int  landuse_readPollutParams(int j, char* tok[], int ntoks)
//
//  Input:   j = pollutant index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads pollutant parameters from a tokenized line of input.
//
//  Data format is:
//    ID cUnits cRain cGW cRDII kDecay (snowOnly coPollut coFrac cDWF)         //(5.0.017 - LR)
//
{
    int    i, k, coPollut, snowFlag;
    double x[4], coFrac;
    double cDWF;                                                               //(5.0.017 - LR)
    char   *id;

    // --- extract pollutant name & units
    if ( ntoks < 6 ) return error_setInpError(ERR_ITEMS, "");
    id = project_findID(POLLUT, tok[0]);
    if ( id == NULL ) return error_setInpError(ERR_NAME, tok[0]);
    k = findmatch(tok[1], QualUnitsWords);
    if ( k < 0 ) return error_setInpError(ERR_KEYWORD, tok[1]);

    // --- extract concen. in rain, gwater, & I&I and decay coeff
    for ( i = 2; i <= 5; i++ )
    {
        if ( ! getDouble(tok[i], &x[i-2]) )
            return error_setInpError(ERR_NUMBER, tok[i]);
    }

    // --- set defaults for snow only flag & co-pollut. parameters
    snowFlag = 0;
    coPollut = -1;
    coFrac = 0.0;
    cDWF = 0.0;                                                                //(5.0.017 - LR)

    // --- check for snow only flag
    if ( ntoks >= 7 )
    {
        snowFlag = findmatch(tok[6], NoYesWords);             
        if ( snowFlag < 0 ) return error_setInpError(ERR_KEYWORD, tok[6]);
    }

    // --- check for co-pollutant
    if ( ntoks >= 9 )
    {
        if ( !strcomp(tok[7], "*") )
        {
            coPollut = project_findObject(POLLUT, tok[7]);
            if ( coPollut < 0 ) return error_setInpError(ERR_NAME, tok[7]);
            if ( ! getDouble(tok[8], &coFrac) )
                return error_setInpError(ERR_NUMBER, tok[8]);
        }
    }

    // --- check for DWF concen.                                               //(5.0.017 - LR)
    if ( ntoks >= 10 )                                                         //(5.0.017 - LR)
        if ( ! getDouble(tok[9], &cDWF) )                                      //(5.0.017 - LR)
            return error_setInpError(ERR_NUMBER, tok[9]);                      //(5.0.017 - LR)

    // --- save values for pollutant object   
    Pollut[j].ID = id;
    Pollut[j].units = k;
    if      ( Pollut[j].units == MG ) Pollut[j].mcf = UCF(MASS);
    else if ( Pollut[j].units == UG ) Pollut[j].mcf = UCF(MASS) / 1000.0;
    else                              Pollut[j].mcf = 1.0;
    Pollut[j].pptConcen = x[0];
    Pollut[j].gwConcen  = x[1];
    Pollut[j].rdiiConcen = x[2];
    Pollut[j].kDecay = x[3]/SECperDAY;
    Pollut[j].snowOnly = snowFlag;
    Pollut[j].coPollut = coPollut;
    Pollut[j].coFraction = coFrac;
    Pollut[j].dwfConcen = cDWF;                                                //(5.0.017 - LR)
    return 0;
}

//=============================================================================

int  landuse_readBuildupParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads pollutant buildup parameters from a tokenized line of input.
//
//  Data format is:
//    landuseID  pollutID  buildupType  c1  c2  c3  normalizerType
//
{
    int    i, j, k, n, p;
    double c[3], tmax;

    if ( ntoks < 3 ) return 0;
    j = project_findObject(LANDUSE, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);
    p = project_findObject(POLLUT, tok[1]);
    if ( p < 0 ) return error_setInpError(ERR_NAME, tok[1]);
    k = findmatch(tok[2], BuildupTypeWords);
    if ( k < 0 ) return error_setInpError(ERR_KEYWORD, tok[2]);
    Landuse[j].buildupFunc[p].funcType = k;
    if ( k > NO_BUILDUP )
    {
        if ( ntoks < 7 ) return error_setInpError(ERR_ITEMS, "");
        if ( k != EXTERNAL_BUILDUP ) for (i=0; i<3; i++)                       //(5.0.019 - LR)
        {
            if ( ! getDouble(tok[i+3], &c[i])  || c[i] < 0.0  )
            return error_setInpError(ERR_NUMBER, tok[i+3]);
        }
        n = findmatch(tok[6], NormalizerWords);
        if (n < 0 ) return error_setInpError(ERR_KEYWORD, tok[6]);
        Landuse[j].buildupFunc[p].normalizer = n;
    }

    // Find time until max. buildup (or time series for external buildup)      //(5.0.019 - LR)
    switch (Landuse[j].buildupFunc[p].funcType)
    {
      case POWER_BUILDUP:
        // --- check for too small or large an exponent
        if ( c[2] > 0.0 && (c[2] < 0.01 || c[2] > 10.0) )
            return error_setInpError(ERR_KEYWORD, tok[5]);

        // --- find time to reach max. buildup
        // --- use zero if coeffs. are 0        
        if ( c[1]*c[2] == 0.0 ) tmax = 0.0;

        // --- use 10 years if inverse power function tends to blow up
        else if ( log10(c[0]) / c[2] > 3.5 ) tmax = 3650.0;

        // --- otherwise use inverse power function
        else tmax = pow(c[0]/c[1], 1.0/c[2]);
        break;

      case EXPON_BUILDUP:
        if ( c[1] == 0.0 ) tmax = 0.0;
        else tmax = -log(0.001)/c[1];
        break;

      case SATUR_BUILDUP:
        tmax = 1000.0*c[2];
        break;

////  The following code segment was added to release 5.0.019.  ////           //(5.0.019 - LR)
      case EXTERNAL_BUILDUP:
        if ( !getDouble(tok[3], &c[0]) || c[0] < 0.0 )     //max. buildup
            return error_setInpError(ERR_NUMBER, tok[3]);
        if ( !getDouble(tok[4], &c[1]) || c[1] < 0.0 )     //scaling factor
            return error_setInpError(ERR_NUMBER, tok[3]);
        n = project_findObject(TSERIES, tok[5]);           //time series
        if ( n < 0 ) return error_setInpError(ERR_NAME, tok[4]);
        Tseries[n].refersTo = EXTERNAL_BUILDUP;
        c[2] = n;
        tmax = 0.0;
        break;
////  End of new code segment  ////                                            //(5.0.019 - LR)

      default:
        tmax = 0.0;
    }

    // Assign parameters to buildup object
    Landuse[j].buildupFunc[p].coeff[0]   = c[0];
    Landuse[j].buildupFunc[p].coeff[1]   = c[1];
    Landuse[j].buildupFunc[p].coeff[2]   = c[2];
    Landuse[j].buildupFunc[p].maxDays = tmax;
    return 0;
}

//=============================================================================

int  landuse_readWashoffParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads pollutant washoff parameters from a tokenized line of input.
//
//  Data format is:
//    landuseID  pollutID  washoffType  c1  c2  sweepEffic  bmpRemoval
{
    int    i, j, p;
    int    func;
    double x[4];

    if ( ntoks < 3 ) return 0;
    for (i=0; i<4; i++) x[i] = 0.0;
    func = NO_WASHOFF;
    j = project_findObject(LANDUSE, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);
    p = project_findObject(POLLUT, tok[1]);
    if ( p < 0 ) return error_setInpError(ERR_NAME, tok[1]);
    if ( ntoks > 2 )
    {
        func = findmatch(tok[2], WashoffTypeWords);
        if ( func < 0 ) return error_setInpError(ERR_KEYWORD, tok[2]);
        if ( func != NO_WASHOFF )
        {
            if ( ntoks < 5 ) return error_setInpError(ERR_ITEMS, "");
            if ( ! getDouble(tok[3], &x[0]) )
                    return error_setInpError(ERR_NUMBER, tok[3]);
            if ( ! getDouble(tok[4], &x[1]) )
                    return error_setInpError(ERR_NUMBER, tok[4]);
            if ( ntoks >= 6 )
            {
                if ( ! getDouble(tok[5], &x[2]) )
                        return error_setInpError(ERR_NUMBER, tok[5]);
            }
            if ( ntoks >= 7 )
            {
                if ( ! getDouble(tok[6], &x[3]) )
                        return error_setInpError(ERR_NUMBER, tok[6]);
            }
        }
    }

    // --- check for valid parameter values
    //     x[0] = washoff coeff.
    //     x[1] = washoff expon.
    //     x[2] = sweep effic.
    //     x[3] = BMP effic.
    if ( x[0] < 0.0 ) return error_setInpError(ERR_NUMBER, tok[3]);
    if ( x[1] < -10.0 || x[1] > 10.0 )
        return error_setInpError(ERR_NUMBER, tok[4]);;
    if ( x[2] < 0.0 || x[2] > 100.0 )
        return error_setInpError(ERR_NUMBER, tok[5]);
    if ( x[3] < 0.0 || x[3] > 100.0 )
        return error_setInpError(ERR_NUMBER, tok[6]);

    // --- convert units of washoff coeff.
    if ( func == EXPON_WASHOFF  ) x[0] /= 3600.0;
    if ( func == RATING_WASHOFF ) x[0] *= pow(UCF(FLOW), x[1]);
    if ( func == EMC_WASHOFF    ) x[0] *= LperFT3;

    // --- assign washoff parameters to washoff object
    Landuse[j].washoffFunc[p].funcType = func;
    Landuse[j].washoffFunc[p].coeff = x[0];
    Landuse[j].washoffFunc[p].expon = x[1];
    Landuse[j].washoffFunc[p].sweepEffic = x[2] / 100.0;
    Landuse[j].washoffFunc[p].bmpEffic = x[3] / 100.0;
    return 0;
}

//=============================================================================

double  landuse_getBuildup(int i, int p, double area, double curb, double buildup,
                           double tStep)
//
//  Input:   i = land use index
//           p = pollutant index
//           area = land use area (ac or ha)
//           curb = land use curb length (users units)
//           buildup = current pollutant buildup (lbs or kg)
//           tStep = time increment for buildup (sec)
//  Output:  returns new buildup mass (lbs or kg)
//  Purpose: computes new pollutant buildup on a landuse after a time increment.
//
{
    int     n;                         // normalizer code
    double  days;                      // accumulated days of buildup
    double  perUnit;                   // normalizer value (area or curb length)

    // --- return current buildup if no buildup function or time increment     //(5.0.014 - LR)
    if ( Landuse[i].buildupFunc[p].funcType == NO_BUILDUP || tStep == 0.0 )
    {
        return buildup;                                                        //(5.0.014 - LR)
    }

    // --- see what buildup is normalized to
    n = Landuse[i].buildupFunc[p].normalizer;
    perUnit = 1.0;
    if ( n == PER_AREA ) perUnit = area;
    if ( n == PER_CURB ) perUnit = curb;
    if ( perUnit == 0.0 ) return 0.0;

    // --- buildup determined by loading time series                           //(5.0.019 - LR)
    if ( Landuse[i].buildupFunc[p].funcType == EXTERNAL_BUILDUP )              //(5.0.019 - LR)
        return landuse_getExternalBuildup(i, p, buildup/perUnit, tStep) *      //(5.0.019 - LR)
               perUnit;                                                        //(5.0.019 - LR)

    // --- determine equivalent days of current buildup
    days = landuse_getBuildupDays(i, p, buildup/perUnit);

    // --- compute buildup after adding on time increment
    days += tStep / SECperDAY;
    return landuse_getBuildupMass(i, p, days) * perUnit;
}

//=============================================================================

double landuse_getBuildupDays(int i, int p, double buildup)
//
//  Input:   i = land use index
//           p = pollutant index
//           buildup = amount of pollutant buildup
//  Output:  returns number of days it takes for buildup to reach a given level
//  Purpose: finds the number of days corresponding to a pollutant buildup.
//
{
    double c0 = Landuse[i].buildupFunc[p].coeff[0];
    double c1 = Landuse[i].buildupFunc[p].coeff[1];
    double c2 = Landuse[i].buildupFunc[p].coeff[2];

    if ( buildup == 0.0 ) return 0.0;
    if ( buildup >= c0 ) return Landuse[i].buildupFunc[p].maxDays;   
    switch (Landuse[i].buildupFunc[p].funcType)
    {
      case POWER_BUILDUP:
        if ( c1*c2 == 0.0 ) return 0.0;
        else return pow( (buildup/c1), (1.0/c2) );

      case EXPON_BUILDUP:
        if ( c0*c1 == 0.0 ) return 0.0;
        else return -log(1. - buildup/c0) / c1;

      case SATUR_BUILDUP:
        if ( c0 == 0.0 ) return 0.0;
        else return buildup*c2 / (c0 - buildup);

      default:
        return 0.0;
    }
}

//=============================================================================

double landuse_getBuildupMass(int i, int p, double days)
//
//  Input:   i = land use index
//           p = pollutant index
//           days = time over which buildup has occurred (days)
//  Output:  returns mass of pollutant buildup (lbs or kg per area or curblength)
//  Purpose: finds amount of buildup of pollutant on a land use.
//
{
    double b;
    double c0 = Landuse[i].buildupFunc[p].coeff[0];
    double c1 = Landuse[i].buildupFunc[p].coeff[1];
    double c2 = Landuse[i].buildupFunc[p].coeff[2];

    if ( days == 0.0 ) return 0.0;
    if ( days >= Landuse[i].buildupFunc[p].maxDays ) return c0;
    switch (Landuse[i].buildupFunc[p].funcType)
    {
      case POWER_BUILDUP:
        b = c1 * pow(days, c2);
        if ( b > c0 ) b = c0;
        break;

      case EXPON_BUILDUP:
        b = c0*(1.0 - exp(-days*c1));
        break;

      case SATUR_BUILDUP:
        b = days*c0/(c2 + days);
        break;

      default: b = 0.0;
    }
    return b;
}

//=============================================================================

void  landuse_getWashoff(int i, double area, TLandFactor landFactor[],
                         double runoff, double tStep, double washoffLoad[])    //(5.0.012 - LR)
//
//  Input:   i            = land use index
//           area         = subcatchment area (ft2)
//           landFactor[] = array of land use data for subcatchment
//           runoff       = runoff flow rate (ft/sec) over subcatchment
//           tStep        = time step (sec)
//  Output:  washoffLoad[] = pollutant load in surface washoff (mass/sec)
//  Purpose: computes surface washoff load for all pollutants generated by a
//           land use within a subcatchment.
//
{
    int    p;                          //pollutant index
    double fArea;                      //area devoted to land use (ft2)

    // --- find area devoted to land use
    fArea = landFactor[i].fraction * area;

    // --- initialize washoff loads from land use
    for (p = 0; p < Nobjects[POLLUT]; p++) washoffLoad[p] = 0.0;

    // --- compute contribution from direct runoff load
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        washoffLoad[p] +=
            landuse_getRunoffLoad(i, p, fArea, landFactor, runoff, tStep);     //(5.0.012 - LR)
    }

    // --- compute contribution from co-pollutant
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        washoffLoad[p] += landuse_getCoPollutLoad(p, washoffLoad, tStep);
    }
}

//=============================================================================

double landuse_getRunoffLoad(int i, int p, double area, TLandFactor landFactor[], 
                             double runoff, double tStep)
//
//  Input:   i = land use index
//           p = pollut. index
//           area = area devoted to land use (ft2)
//           landFactor[] = array of land use data for subcatchment
//           runoff = runoff flow on subcatchment (ft/sec)
//           tStep = time step (sec)
//  Output:  returns runoff load for pollutant (mass/sec)
//  Purpose: computes pollutant load generated by a specific land use.
//
{
    double buildup;
    double washoff;
    double bmpRemoval;                                                         //(5.0.014 - LR)

    // --- compute washoff mass/sec for this pollutant
    buildup = landFactor[i].buildup[p];
    washoff = landuse_getWashoffMass(i, p, buildup, runoff, area);

    // --- convert washoff to lbs (or kg) over time step so that
    //     buildup and mass balances can be adjusted
    //     (Pollut[].mcf converts from concentration mass units
    //      to either lbs or kg)
    washoff *= tStep * Pollut[p].mcf;

    // --- if buildup modelled, reduce it by amount of washoff
    if ( Landuse[i].buildupFunc[p].funcType != NO_BUILDUP ||                   //(5.0.014 - LR)
         buildup > washoff )                                                   //(5.0.014 - LR)
    {
        washoff = MIN(washoff, buildup);
        buildup -= washoff;
        landFactor[i].buildup[p] = buildup;
    }

    // --- otherwise add washoff to buildup mass balance totals
    //     so that things will balance
    else                                                                       //(5.0.014 - LR)
    {                                                                          //(5.0.014 - LR)
        massbal_updateLoadingTotals(BUILDUP_LOAD, p, washoff-buildup);         //(5.0.014 - LR)
        landFactor[i].buildup[p] = 0.0;                                        //(5.0.014 - LR)
    }

    // --- apply any BMP removal to washoff                                    //(5.0.014 - LR)
    bmpRemoval = Landuse[i].washoffFunc[p].bmpEffic * washoff;                 //(5.0.014 - LR)
    if ( bmpRemoval > 0.0 )                                                    //(5.0.014 - LR)
    {                                                                          //(5.0.014 - LR)
        massbal_updateLoadingTotals(BMP_REMOVAL_LOAD, p, bmpRemoval);          //(5.0.014 - LR)
        washoff -= bmpRemoval;                                                 //(5.0.014 - LR)
    }                                                                          //(5.0.014 - LR)

    // --- return washoff converted back to mass/sec
    return washoff / tStep / Pollut[p].mcf;
}

//=============================================================================

double landuse_getWashoffMass(int i, int p, double buildup, double runoff,
                              double area)
//
//  Input:   i = land use index
//           p = pollutant index
//           buildup = current buildup over land use (lbs or kg)
//           runoff = current runoff on subcatchment (ft/sec)
//           area = area devoted to land use (ft2)
//  Output:  returns pollutant washoff rate (mass/sec)
//  Purpose: finds mass loading of pollutant washed off a land use.
//
//  Notes:   "coeff" for each washoff function was previously adjusted to
//           result in units of mass/sec
//
{
    double washoff;
    double coeff = Landuse[i].washoffFunc[p].coeff;
    double expon = Landuse[i].washoffFunc[p].expon;
    int    func  = Landuse[i].washoffFunc[p].funcType;

    // --- if no washoff function, return 0
    if ( func == NO_WASHOFF ) return 0.0;
    
    // --- if buildup function exists but no current buildup, return 0
    if ( Landuse[i].buildupFunc[p].funcType != NO_BUILDUP && buildup == 0.0 )
        return 0.0;

    if ( func == EXPON_WASHOFF )
    {
        // --- convert runoff to inches/hr (or mm/hr) and 
        //     convert buildup from lbs (or kg) to concen. mass units
        runoff = runoff * UCF(RAINFALL);
        buildup /= Pollut[p].mcf;

        // --- evaluate washoff eqn.
        washoff = coeff * pow(runoff, expon) * buildup;
    }

    else if ( func == RATING_WASHOFF )
    {
        runoff = runoff * area;             // runoff in cfs
        if ( runoff == 0.0 ) washoff = 0.0;
        else washoff = coeff * pow(runoff, expon);
    }

    else if ( func == EMC_WASHOFF )
    {
        runoff = runoff * area;             // runoff in cfs
        washoff = coeff * runoff;           // coeff includes LperFT3 factor
    }

    else washoff = 0.0;
    return washoff;
}

//=============================================================================

double landuse_getCoPollutLoad(int p, double washoff[], double tStep)
//
//  Input:   p = pollutant index
//           washoff = pollut. washoff rate (mass/sec)
//           tStep = time step (sec)
//  Output:  returns washoff mass added by co-pollutant relation (mass/sec)
//  Purpose: finds washoff mass added by a co-pollutant of a given pollutant.
//
{
    int    k;
    double w;
    double load;

    // --- check if pollutant p has a co-pollutant k
    k = Pollut[p].coPollut;
    if ( k >= 0 )
    {
        // --- compute addition to washoff from co-pollutant
        w = Pollut[p].coFraction * washoff[k];

        // --- add to mass balance totals
        load = w * tStep * Pollut[p].mcf;
        massbal_updateLoadingTotals(BUILDUP_LOAD, p, load);
        return w;
    }
    return 0.0;
}

//=============================================================================

////  New function added to release 5.0.019  ////                              //(5.0.019 - LR)

double landuse_getExternalBuildup(int i, int p, double buildup, double tStep)
//
//  Input:   i = landuse index
//           p = pollutant index
//           buildup = buildup at start of time step (mass/unit)
//           tStep = time step (sec)
//  Output:  returns pollutant buildup at end of time interval (mass/unit)
//  Purpose: finds pollutant buildup contributed by external loading over a
//           given time step.
//
{
    double maxBuildup = Landuse[i].buildupFunc[p].coeff[0];
    double sf = Landuse[i].buildupFunc[p].coeff[1];              // scaling factor
    int    ts = (int)floor(Landuse[i].buildupFunc[p].coeff[2]);  // time series index
    double rate = 0.0;

    // --- no buildup increment at start of simulation
    if (NewRunoffTime == 0.0) return 0.0;

    // --- get buildup rate (mass/unit/day) over the interval
    if ( ts >= 0 )
    {        
        rate = sf * table_tseriesLookup(&Tseries[ts],
               getDateTime(NewRunoffTime), FALSE);
    }

    // --- compute buildup at end of time interval
    buildup = buildup + rate * tStep / SECperDAY;
    buildup = MIN(buildup, maxBuildup);
    return buildup;
}
