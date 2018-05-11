//-----------------------------------------------------------------------------
//   landuse.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14  (Build 5.1.001)
//             03/19/15  (Build 5.1.008)
//   Author:   L. Rossman
//
//   Pollutant buildup and washoff functions.
//
//   Build 5.1.008:
//   - landuse_getWashoffMass() re-named to landuse_getWashoffQual() and
//     modified to return concentration instead of mass load.
//   - landuse_getRunoffLoad() re-named to landuse_getWashoffLoad() and
//     modified to work with landuse_getWashoffQual().
//
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

//  landuse_getInitBuildup    (called by subcatch_initState)
//  landuse_getBuildup        (called by surfqual_getBuildup)
//  landuse_getWashoffLoad    (called by surfqual_getWashoff)
//  landuse_getCoPollutLoad   (called by surfqual_getwashoff));
//  landuse_getAvgBMPEffic    (called by updatePondedQual in surfqual.c)

//-----------------------------------------------------------------------------
// Function declarations
//-----------------------------------------------------------------------------
static double landuse_getBuildupDays(int landuse, int pollut, double buildup);
static double landuse_getBuildupMass(int landuse, int pollut, double days);
static double landuse_getRunoffLoad(int landuse, int pollut, double area,
              TLandFactor landFactor[], double runoff, double tStep);
static double landuse_getWashoffQual(int landuse, int pollut, double buildup,
              double runoff, double area);
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
//    ID Units cRain cGW cRDII kDecay (snowOnly coPollut coFrac cDWF cInit)
//
{
    int    i, k, coPollut, snowFlag;
    double x[4], coFrac, cDWF, cInit;
    char   *id;

    // --- extract pollutant name & units
    if ( ntoks < 6 ) return error_setInpError(ERR_ITEMS, "");
    id = project_findID(POLLUT, tok[0]);
    if ( id == NULL ) return error_setInpError(ERR_NAME, tok[0]);
    k = findmatch(tok[1], QualUnitsWords);
    if ( k < 0 ) return error_setInpError(ERR_KEYWORD, tok[1]);

    // --- extract concen. in rain, gwater, & I&I
    for ( i = 2; i <= 4; i++ )
    {
        if ( ! getDouble(tok[i], &x[i-2]) || x[i-2] < 0.0 )
        {
            return error_setInpError(ERR_NUMBER, tok[i]);
        }
    }

    // --- extract decay coeff. (which can be negative for growth)
    if ( ! getDouble(tok[5], &x[3]) )
    {
        return error_setInpError(ERR_NUMBER, tok[5]);
    }

    // --- set defaults for snow only flag & co-pollut. parameters
    snowFlag = 0;
    coPollut = -1;
    coFrac = 0.0;
    cDWF = 0.0;
    cInit = 0.0;

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
            if ( ! getDouble(tok[8], &coFrac) || coFrac < 0.0 )
                return error_setInpError(ERR_NUMBER, tok[8]);
        }
    }

    // --- check for DWF concen.
    if ( ntoks >= 10 )
    {
        if ( ! getDouble(tok[9], &cDWF) || cDWF < 0.0)
            return error_setInpError(ERR_NUMBER, tok[9]);
    }

    // --- check for initial concen.
    if ( ntoks >= 11 ) 
    {
        if ( ! getDouble(tok[10], &cInit) || cInit < 0.0 )
            return error_setInpError(ERR_NUMBER, tok[9]);
    }

    // --- save values for pollutant object   
    Pollut[j].ID = id;
    Pollut[j].units = k;
    if      ( Pollut[j].units == MG ) Pollut[j].mcf = UCF(MASS);
    else if ( Pollut[j].units == UG ) Pollut[j].mcf = UCF(MASS) / 1000.0;
    else                              Pollut[j].mcf = 1.0;
    Pollut[j].pptConcen  = x[0];
    Pollut[j].gwConcen   = x[1];
    Pollut[j].rdiiConcen = x[2];
    Pollut[j].kDecay     = x[3]/SECperDAY;
    Pollut[j].snowOnly   = snowFlag;
    Pollut[j].coPollut   = coPollut;
    Pollut[j].coFraction = coFrac;
    Pollut[j].dwfConcen  = cDWF;
    Pollut[j].initConcen = cInit;
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
        if ( k != EXTERNAL_BUILDUP ) for (i=0; i<3; i++)
        {
            if ( ! getDouble(tok[i+3], &c[i])  || c[i] < 0.0  )
            {
                return error_setInpError(ERR_NUMBER, tok[i+3]);
            }
        }
        n = findmatch(tok[6], NormalizerWords);
        if (n < 0 ) return error_setInpError(ERR_KEYWORD, tok[6]);
        Landuse[j].buildupFunc[p].normalizer = n;
    }

    // Find time until max. buildup (or time series for external buildup)
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

void  landuse_getInitBuildup(TLandFactor* landFactor,  double* initBuildup,
		double area, double curb)
//
//  Input:   landFactor = array of land use factors
//           initBuildup = total initial buildup of each pollutant
//           area = subcatchment's area (ft2)
//           curb = subcatchment's curb length (users units)
//  Output:  modifies each land use factor's initial pollutant buildup 
//  Purpose: determines the initial buildup of each pollutant on
//           each land use for a given subcatchment.
//
//  Notes:   Contributions from co-pollutants to initial buildup are not
//           included since the co-pollutant mechanism only applies to
//           washoff.
//
{
	int i, p;
	double startDrySeconds;       // antecedent dry period (sec)
	double f;                     // faction of total land area
    double fArea;                 // area of land use (ft2)
    double fCurb;                 // curb length of land use
	double buildup;               // pollutant mass buildup

    // --- convert antecedent dry days into seconds
    startDrySeconds = StartDryDays*SECperDAY;

    // --- examine each land use
    for (i = 0; i < Nobjects[LANDUSE]; i++)
    {
        // --- initialize date when last swept
        landFactor[i].lastSwept = StartDateTime - Landuse[i].sweepDays0;

        // --- determine area and curb length covered by land use
        f = landFactor[i].fraction;
        fArea = f * area * UCF(LANDAREA);
        fCurb = f * curb;

        // --- determine buildup of each pollutant
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            // --- if an initial loading was supplied, then use it to
            //     find the starting buildup over the land use
            buildup = 0.0;
            if ( initBuildup[p] > 0.0 ) buildup = initBuildup[p] * fArea;

            // --- otherwise use the land use's buildup function to 
            //     compute a buildup over the antecedent dry period
            else buildup = landuse_getBuildup(i, p, fArea, fCurb, buildup,
                           startDrySeconds);
            landFactor[i].buildup[p] = buildup;
        }
    }
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

    // --- return current buildup if no buildup function or time increment
    if ( Landuse[i].buildupFunc[p].funcType == NO_BUILDUP || tStep == 0.0 )
    {
        return buildup;
    }

    // --- see what buildup is normalized to
    n = Landuse[i].buildupFunc[p].normalizer;
    perUnit = 1.0;
    if ( n == PER_AREA ) perUnit = area;
    if ( n == PER_CURB ) perUnit = curb;
    if ( perUnit == 0.0 ) return 0.0;

    // --- buildup determined by loading time series
    if ( Landuse[i].buildupFunc[p].funcType == EXTERNAL_BUILDUP )
    {
        return landuse_getExternalBuildup(i, p, buildup/perUnit, tStep) *
               perUnit;
    }

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

double landuse_getAvgBmpEffic(int j, int p)
//
//  Input:   j = subcatchment index
//           p = pollutant index
//  Output:  returns a BMP removal fraction for pollutant p
//  Purpose: finds the overall average BMP removal achieved for pollutant p
//           treated in subcatchment j.
//
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

double landuse_getWashoffLoad(int i, int p, double area,
    TLandFactor landFactor[], double runoff, double vOutflow)
//
//  Input:   i = land use index
//           p = pollut. index
//           area = sucatchment area (ft2)
//           landFactor[] = array of land use data for subcatchment
//           runoff = runoff flow generated by subcatchment (ft/sec)
//           vOutflow = runoff volume leaving the subcatchment (ft3)
//  Output:  returns pollutant runoff load (mass)
//  Purpose: computes pollutant load generated by a land use over a time step.
//
{
    double landuseArea;      // area of current land use (ft2)
    double buildup;          // current pollutant buildup (lb or kg)
    double washoffQual;      // pollutant concentration in washoff (mass/ft3)
    double washoffLoad;      // pollutant washoff load over time step (lb or kg)
    double bmpRemoval;       // pollutant load removed by BMP treatment (lb or kg)

    // --- compute concen. of pollutant in washoff (mass/ft3)
    buildup = landFactor[i].buildup[p];
    landuseArea = landFactor[i].fraction * area;
    washoffQual = landuse_getWashoffQual(i, p, buildup, runoff, landuseArea);

    // --- compute washoff load exported (lbs or kg) from landuse
    //     (Pollut[].mcf converts from mg (or ug) mass units to lbs (or kg)
    washoffLoad = washoffQual * vOutflow * landuseArea / area * Pollut[p].mcf;

    // --- if buildup modelled, reduce it by amount of washoff
    if ( Landuse[i].buildupFunc[p].funcType != NO_BUILDUP ||
         buildup > washoffLoad )
    {
        washoffLoad = MIN(washoffLoad, buildup);
        buildup -= washoffLoad;
        landFactor[i].buildup[p] = buildup;
    }

    // --- otherwise add washoff to buildup mass balance totals
    //     so that things will balance
    else
    {
        massbal_updateLoadingTotals(BUILDUP_LOAD, p, washoffLoad);
        landFactor[i].buildup[p] = 0.0;
    }
	
    // --- apply any BMP removal to washoff
    bmpRemoval = Landuse[i].washoffFunc[p].bmpEffic * washoffLoad;
    if ( bmpRemoval > 0.0 )
    {
        massbal_updateLoadingTotals(BMP_REMOVAL_LOAD, p, bmpRemoval);
        washoffLoad -= bmpRemoval;
    }

    // --- return washoff load converted back to mass (mg or ug)
    return washoffLoad / Pollut[p].mcf;
}

//=============================================================================

double landuse_getWashoffQual(int i, int p, double buildup, double runoff,
                              double area)
//
//  Input:   i = land use index
//           p = pollutant index
//           buildup = current buildup over land use (lbs or kg)
//           runoff = current runoff on subcatchment (ft/sec)
//           area = area devoted to land use (ft2)
//  Output:  returns pollutant concentration in washoff (mass/ft3)
//  Purpose: finds concentration of pollutant washed off a land use.
//
//  Notes:   "coeff" for each washoff function was previously adjusted to
//           result in units of mass/sec
//
{
    double cWashoff = 0.0;
    double coeff = Landuse[i].washoffFunc[p].coeff;
    double expon = Landuse[i].washoffFunc[p].expon;
    int    func  = Landuse[i].washoffFunc[p].funcType;

    // --- if no washoff function or no runoff, return 0
    if ( func == NO_WASHOFF || runoff == 0.0 ) return 0.0;
    
    // --- if buildup function exists but no current buildup, return 0
    if ( Landuse[i].buildupFunc[p].funcType != NO_BUILDUP && buildup == 0.0 )
        return 0.0;

    // --- Exponential Washoff function
    if ( func == EXPON_WASHOFF )
    {
        // --- evaluate washoff eqn. with runoff in in/hr (or mm/hr)
        //     and buildup converted from lbs (or kg) to concen. mass units
        cWashoff = coeff * pow(runoff * UCF(RAINFALL), expon) *
                  buildup / Pollut[p].mcf;
        cWashoff /= runoff * area;
    }

    // --- Rating Curve Washoff function
    else if ( func == RATING_WASHOFF )
    {
        cWashoff = coeff * pow(runoff * area, expon-1.0);
    }

    // --- Event Mean Concentration Washoff
    else if ( func == EMC_WASHOFF )
    {
        cWashoff = coeff;     // coeff includes LperFT3 factor
    }
    return cWashoff;
}

//=============================================================================

double landuse_getCoPollutLoad(int p, double washoff[])
//
//  Input:   p = pollutant index
//           washoff = pollut. washoff rate (mass/sec)
//  Output:  returns washoff mass added by co-pollutant relation (mass)
//  Purpose: finds washoff mass added by a co-pollutant of a given pollutant.
//
{
    int    k;
    double w;

    // --- check if pollutant p has a co-pollutant k
    k = Pollut[p].coPollut;
    if ( k >= 0 )
    {
        // --- compute addition to washoff from co-pollutant
        w = Pollut[p].coFraction * washoff[k];

        // --- add washoff to buildup mass balance totals
        //     so that things will balance
        massbal_updateLoadingTotals(BUILDUP_LOAD, p, w * Pollut[p].mcf);
        return w;
    }
    return 0.0;
}

//=============================================================================

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
