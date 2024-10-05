//-----------------------------------------------------------------------------
//   infil.c
//
//   Project:  EPA SWMM5
//   Version:  5.2
//   Date:     11/01/21  (Build 5.2.0)
//   Author:   L. Rossman
//
//   Infiltration functions.
//
//   Update History
//   ==============
//   Build 5.1.007:
//   - Revised formula for infiltration capacity recovery for the Modified
//     Horton method.
//   - The Green-Ampt functions were re-written.
//   Build 5.1.008:
//   - Monthly adjustment factors applied to hydraulic conductivity.
//   Build 5.1.010:
//   - Support for Modified Green Ampt model added.
//   - Green-Ampt initial recovery time set to 0.
//   Build 5.1.011:
//   - Monthly hydraulic conductivity factor also applied to Fu parameter
//     for Green-Ampt infiltration.
//   - Prevented computed Horton infiltration from dropping below 0.
//   Build 5.1.013:
//   - Support added for subcatchment-specific time patterns that adjust
//     hydraulic conductivity.
//   Build 5.1.015:
//   - Support added for multiple infiltration methods within a project.
//   Build 5.2.0:
//   - Additional validity check for G-A initial deficit added.
//   - New error message 235 added for invalid infiltration parameters.
//   - Conversion of runon to ponded depth fixed for Curve Number infiltration.
//   Build 5.3.0:
//   - Bug fix for Modified Horton max. infiltration.Cummulative infiltration
//     limited to max limit in line with page 103 of the hydrology reference.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include <stdlib.h>
#include "headers.h"
#include "infil.h"

//-----------------------------------------------------------------------------
//  Local Variables
//-----------------------------------------------------------------------------
typedef union TInfil {
    THorton   horton;
    TGrnAmpt  grnAmpt;
    TCurveNum curveNum;
} TInfil;
TInfil *Infil;

static double Fumax;   // saturated water volume in upper soil zone (ft)
static double InfilFactor;

//-----------------------------------------------------------------------------
//  External Functions (declared in infil.h)
//-----------------------------------------------------------------------------
//  infil_create     (called by createObjects in project.c)
//  infil_delete     (called by deleteObjects in project.c)
//  infil_readParams (called by input_readLine)
//  infil_initState  (called by subcatch_initState)
//  infil_getState   (called by writeRunoffFile in hotstart.c)
//  infil_setState   (called by readRunoffFile in hotstart.c)
//  infil_getInfil   (called by getSubareaRunoff in subcatch.c)

//  Called locally and by storage node methods in node.c
//  grnampt_setParams
//  grnampt_initState
//  grnampt_getInfil

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static int    horton_setParams(THorton *infil, double p[]);
static void   horton_initState(THorton *infil);
static void   horton_getState(THorton *infil, double x[]);
static void   horton_setState(THorton *infil, double x[]);
static double horton_getInfil(THorton *infil, double tstep, double irate,
              double depth);
static double modHorton_getInfil(THorton *infil, double tstep, double irate,
              double depth);

static void   grnampt_getState(TGrnAmpt *infil, double x[]);
static void   grnampt_setState(TGrnAmpt *infil, double x[]);
static double grnampt_getUnsatInfil(TGrnAmpt *infil, double tstep,
              double irate, double depth, int modelType);
static double grnampt_getSatInfil(TGrnAmpt *infil, double tstep,
              double irate, double depth);
static double grnampt_getF2(double f1, double c1, double ks, double ts);

static int    curvenum_setParams(TCurveNum *infil, double p[]);
static void   curvenum_initState(TCurveNum *infil);
static void   curvenum_getState(TCurveNum *infil, double x[]);
static void   curvenum_setState(TCurveNum *infil, double x[]);
static double curvenum_getInfil(TCurveNum *infil, double tstep, double irate,
              double depth);

//=============================================================================

void infil_create(int n)
//
//  Purpose: creates an array of infiltration objects.
//  Input:   n = number of subcatchments
//  Output:  none
//
{
    Infil = (TInfil *) calloc(n, sizeof(TInfil));
    if (Infil == NULL) ErrorCode = ERR_MEMORY;
    InfilFactor = 1.0;
    return;
}

//=============================================================================

void infil_delete()
//
//  Purpose: deletes infiltration objects associated with subcatchments
//  Input:   none
//  Output:  none
//
{
    FREE(Infil);
}

//=============================================================================

int infil_readParams(int m, char* tok[], int ntoks)
//
//  Input:   m = default infiltration model
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: sets infiltration parameters from a line of input data.
//
//  Format of data line is:
//     subcatch  p1  p2 ... (infilMethod)
{
    int   i, j, n, status;
    double x[5];

    // --- check that subcatchment exists
    j = project_findObject(SUBCATCH, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);

    // --- check for infiltration method keyword is last token 
    i = findmatch(tok[ntoks-1], InfilModelWords);
    if ( i >= 0 )
    {
        m = i; 
        --ntoks;
    }

    // --- number of input tokens depends on infiltration model m
    if      ( m == HORTON )         n = 5; 
    else if ( m == MOD_HORTON )     n = 5;
    else if ( m == GREEN_AMPT )     n = 4;
    else if ( m == MOD_GREEN_AMPT ) n = 4;
    else if ( m == CURVE_NUMBER )   n = 4;
    else return 0; 

    if ( ntoks < n ) return error_setInpError(ERR_ITEMS, "");
   
    // --- parse numerical values from tokens
    for (i = 0; i < 5; i++) x[i] = 0.0;
    for (i = 1; i < n; i++)
    {
        if (!getDouble(tok[i], &x[i - 1]))
            return error_setInpError(ERR_NUMBER, tok[i]);
    }

    // --- special case for Horton infil. - last parameter is optional
    if ( (m == HORTON || m == MOD_HORTON) && ntoks > n )
    {
        if ( ! getDouble(tok[n], &x[n-1]) )
            return error_setInpError(ERR_NUMBER, tok[n]);
    }

    // --- assign parameter values to infil, infilModel object
    Subcatch[j].infil = j;
    Subcatch[j].infilModel = m;
    switch (m)
    {
      case HORTON:
      case MOD_HORTON:   status = horton_setParams(&Infil[j].horton, x);
                         break;
      case GREEN_AMPT:
      case MOD_GREEN_AMPT:
                         status = grnampt_setParams(&Infil[j].grnAmpt, x);
                         break;
      case CURVE_NUMBER: status = curvenum_setParams(&Infil[j].curveNum, x);
                         break;
      default:           status = TRUE;
    }
    if ( !status ) return error_setInpError(ERR_INFIL_PARAMS, "");
    return 0;
}

//=============================================================================

void infil_initState(int j)
//
//  Input:   j = subcatchment index
//  Output:  none
//  Purpose: initializes state of infiltration for a subcatchment.
//
{
    switch (Subcatch[j].infilModel)
    {
      case HORTON:
      case MOD_HORTON:   horton_initState(&Infil[j].horton);   break;
      case GREEN_AMPT:
      case MOD_GREEN_AMPT:
                         grnampt_initState(&Infil[j].grnAmpt);    break;
      case CURVE_NUMBER: curvenum_initState(&Infil[j].curveNum);   break;
    }
}

//=============================================================================

void infil_getState(int j, double x[])
//
//  Input:   j = subcatchment index
//  Output:  x = subcatchment's infiltration state
//  Purpose: retrieves the current infiltration state for a subcatchment.
//
{
    switch (Subcatch[j].infilModel)
    {
      case HORTON:
      case MOD_HORTON:   horton_getState(&Infil[j].horton, x); break;
      case GREEN_AMPT:
      case MOD_GREEN_AMPT:
                         grnampt_getState(&Infil[j].grnAmpt, x);   break;
      case CURVE_NUMBER: curvenum_getState(&Infil[j].curveNum, x); break;
    }
}

//=============================================================================

void infil_setState(int j, double x[])
//
//  Input:   j = subcatchment index
//           m = infiltration method code
//  Output:  none
//  Purpose: sets the current infiltration state for a subcatchment.
//
{
    switch (Subcatch[j].infilModel)
    {
      case HORTON:
      case MOD_HORTON:   horton_setState(&Infil[j].horton, x); break;
      case GREEN_AMPT:
      case MOD_GREEN_AMPT:
                         grnampt_setState(&Infil[j].grnAmpt, x);   break;
      case CURVE_NUMBER: curvenum_setState(&Infil[j].curveNum, x); break;
    }
}

//=============================================================================

void infil_setInfilFactor(int j)
//
//  Input:   j = subcatchment index
//  Output:  none
//  Purpose: assigns a value to the infiltration adjustment factor.
{
    int m;
    int p;

    // ... set factor to the global conductivity adjustment factor
    InfilFactor = Adjust.hydconFactor;

    // ... override global factor with subcatchment's adjustment if assigned 
    if (j >= 0)
    {
        p = Subcatch[j].infilPattern;
        if (p >= 0 && Pattern[p].type == MONTHLY_PATTERN)
        {
            m = datetime_monthOfYear(getDateTime(OldRunoffTime)) - 1;
            InfilFactor = Pattern[p].factor[m];
        }
    }
}

//=============================================================================

double infil_getInfil(int j, double tstep, double rainfall,
                      double runon, double depth)
//
//  Input:   j = subcatchment index
//           tstep = runoff time step (sec)
//           rainfall = rainfall rate (ft/sec)
//           runon = runon rate from other sub-areas or subcatchments (ft/sec)
//           depth = depth of surface water on subcatchment (ft)
//  Output:  returns infiltration rate (ft/sec)
//  Purpose: computes infiltration rate depending on infiltration method.
//
{
    switch (Subcatch[j].infilModel)
    {
      case HORTON:
          return horton_getInfil(&Infil[j].horton, tstep, rainfall+runon, depth);

      case MOD_HORTON:
          return modHorton_getInfil(&Infil[j].horton, tstep, rainfall+runon,
                                    depth);

      case GREEN_AMPT:
      case MOD_GREEN_AMPT:
        return grnampt_getInfil(&Infil[j].grnAmpt, tstep, rainfall+runon, depth,
            Subcatch[j].infilModel);

      case CURVE_NUMBER:
        depth += runon * tstep;
        return curvenum_getInfil(&Infil[j].curveNum, tstep, rainfall, depth);

      default:
        return 0.0;
    }
}

//=============================================================================

int horton_setParams(THorton *infil, double p[])
//
//  Input:   infil = ptr. to Horton infiltration object
//           p[] = array of parameter values
//  Output:  returns TRUE if parameters are valid, FALSE otherwise
//  Purpose: assigns Horton infiltration parameters to a subcatchment.
//
{
    int k;
    for (k = 0; k < 5; k++) if ( p[k] < 0.0 ) return FALSE;

    // --- max. & min. infil rates (ft/sec)
    infil->f0   = p[0] / UCF(RAINFALL);
    infil->fmin = p[1] / UCF(RAINFALL);

    // --- convert decay const. to 1/sec
    infil->decay = p[2] / 3600.;

    // --- convert drying time (days) to a regeneration const. (1/sec)
    //     assuming that former is time to reach 98% dry along an
    //     exponential drying curve
    if (p[3] == 0.0 ) p[3] = TINY;
    infil->regen = -log(1.0-0.98) / p[3] / SECperDAY;

    // --- optional max. infil. capacity (ft) (p[4] = 0 if no value supplied)
    infil->Fmax = p[4] / UCF(RAINDEPTH);
    if ( infil->f0 < infil->fmin ) return FALSE;
    return TRUE;
}

//=============================================================================

void horton_initState(THorton *infil)
//
//  Input:   infil = ptr. to Horton infiltration object
//  Output:  none
//  Purpose: initializes time on Horton infiltration curve for a subcatchment.
//
{
    infil->tp = 0.0;
    infil->Fe = 0.0;
    infil->Fmh = 0.0;

}

//=============================================================================

void horton_getState(THorton *infil, double x[])
{
    x[0] = infil->tp;
    x[1] = infil->Fe;
}

void horton_setState(THorton *infil, double x[])
{
    infil->tp = x[0];
    infil->Fe = x[1];
}

//=============================================================================

double horton_getInfil(THorton *infil, double tstep, double irate, double depth)
//
//  Input:   infil = ptr. to Horton infiltration object
//           tstep =  runoff time step (sec),
//           irate = net "rainfall" rate (ft/sec),
//                 = rainfall + snowmelt + runon - evaporation
//           depth = depth of ponded water (ft).
//  Output:  returns infiltration rate (ft/sec)
//  Purpose: computes Horton infiltration for a subcatchment.
//
{
    // --- assign local variables
    int    iter;
    double fa, fp = 0.0;
    double Fp, F1, t1, tlim, ex, kt;
    double FF, FF1, r;
    double f0   = infil->f0 * InfilFactor;
    double fmin = infil->fmin * InfilFactor;
    double Fmax = infil->Fmax;
    double tp   = infil->tp;
    double df   = f0 - fmin;
    double kd   = infil->decay;
    double kr   = infil->regen * Evap.recoveryFactor;

    // --- special cases of no infil. or constant infil
    if ( df < 0.0 || kd < 0.0 || kr < 0.0 ) return 0.0;
    if ( df == 0.0 || kd == 0.0 )
    {
        fp = f0;
        fa = irate + depth / tstep;
        if ( fp > fa ) fp = fa;
        return MAX(0.0, fp);
    }

    // --- compute water available for infiltration
    fa = irate + depth / tstep;

    // --- case where there is water to infiltrate
    if ( fa > ZERO )
    {
        // --- compute average infil. rate over time step
        t1 = tp + tstep;         // future cumul. time
        tlim = 16.0 / kd;        // for tp >= tlim, f = fmin
        if ( tp >= tlim )
        {
            Fp = fmin * tp + df / kd;
            F1 = Fp + fmin * tstep;
        }
        else
        {
            Fp = fmin * tp + df / kd * (1.0 - exp(-kd * tp));
            F1 = fmin * t1 + df / kd * (1.0 - exp(-kd * t1));
        }
        fp = (F1 - Fp) / tstep;
        fp = MAX(fp, fmin);

        // --- limit infil rate to available infil
        if ( fp > fa ) fp = fa;

        // --- if fp on flat portion of curve then increase tp by tstep
        if ( t1 > tlim ) tp = t1;

        // --- if infil < available capacity then increase tp by tstep
        else if ( fp < fa ) tp = t1;

        // --- if infil limited by available capacity then
        //     solve F(tp) - F1 = 0 using Newton-Raphson method
        else
        {
            F1 = Fp + fp * tstep;
            tp = tp + tstep / 2.0;
            for ( iter=1; iter<=20; iter++ )
            {
                kt = MIN( 60.0, kd*tp );
                ex = exp(-kt);
                FF = fmin * tp + df / kd * (1.0 - ex) - F1;
                FF1 = fmin + df * ex;
                r = FF / FF1;
                tp = tp - r;
                if ( fabs(r) <= 0.001 * tstep ) break;
            }
        }

        // --- limit cumulative infiltration to Fmax
        if ( Fmax > 0.0 )
        {
            if ( infil->Fe + fp * tstep > Fmax )
                fp = (Fmax - infil->Fe) / tstep;
            fp = MAX(fp, 0.0);
            infil->Fe += fp * tstep;
        }
    }

    // --- case where infil. capacity is regenerating; update tp.
    else if (kr > 0.0)
    {
        r = exp(-kr * tstep);
        tp = 1.0 - exp(-kd * tp);
        tp = -log(1.0 - r*tp) / kd;

        // reduction in cumulative infiltration
        if ( Fmax > 0.0 )
        {
            infil->Fe = fmin*tp + (df/kd)*(1.0 - exp(-kd*tp));
        }
    }
    infil->tp = tp;
    return fp;
}

//=============================================================================

double modHorton_getInfil(THorton *infil, double tstep, double irate,
                          double depth)
//
//  Input:   infil = ptr. to Horton infiltration object
//           tstep =  runoff time step (sec),
//           irate = net "rainfall" rate (ft/sec),
//                 = rainfall + snowmelt + runon
//           depth = depth of ponded water (ft).
//  Output:  returns infiltration rate (ft/sec)
//  Purpose: computes modified Horton infiltration for a subcatchment.
//
{
    // --- assign local variables
    double f  = 0.0;
    double fp, fa;
    double f0 = infil->f0 * InfilFactor;
    double fmin = infil->fmin * InfilFactor;
    double df = f0 - fmin;
    double kd = infil->decay;
    double kr = infil->regen * Evap.recoveryFactor;

    // --- special cases of no or constant infiltration
    if ( df < 0.0 || kd < 0.0 || kr < 0.0 ) return 0.0;
    if ( df == 0.0 || kd == 0.0 )
    {
        fp = f0;
        fa = irate + depth / tstep;
        if ( fp > fa ) fp = fa;
        return MAX(0.0, fp);
    }

    // --- compute water available for infiltration
    fa = irate + depth / tstep;

    // --- case where there is water to infiltrate
    if ( fa > ZERO )
    {
        // --- saturated condition
        if ( infil->Fmax > 0.0 && infil->Fe >= infil->Fmax ) return 0.0;

        // --- potential infiltration
        fp = f0 - kd * infil->Fe;
        fp = MAX(fp, fmin);

        // --- actual infiltration
        f = MIN(fa, fp);

        // --- limit cumulative infiltration to Fmax
        if (infil->Fmax > 0.0)
        {
            if (infil->Fmh + f * tstep > infil->Fmax)
                f = (infil->Fmax - infil->Fmh) / tstep;

            f = MAX(f, 0.0);
            
            infil->Fmh += f * tstep;
        }

        // --- new cumulative infiltration minus seepage
        infil->Fe += MAX((f - fmin), 0.0) * tstep;

        if (infil->Fmax > 0.0)
            infil->Fe = MIN(infil->Fe, infil->Fmax);
    }

    // --- reduce cumulative infiltration for dry condition
    else if (kr > 0.0)
    {
        df = exp(-kr * tstep);
        infil->Fe *= df;
        infil->Fe = MAX(infil->Fe, 0.0);
        infil->Fmh *= df;
        infil->Fmh = MAX(infil->Fmh, 0.0);
    }
    return f;
}

//=============================================================================

void grnampt_getParams(int j, double p[])
//
//  Input:   j = subcatchment index
//           p[] = array of parameter values
//  Output:  none
//  Purpose: retrieves Green-Ampt infiltration parameters for a subcatchment.
//
{
    p[0] = Infil[j].grnAmpt.S * UCF(RAINDEPTH);   // Capillary suction head (ft)
    p[1] = Infil[j].grnAmpt.Ks * UCF(RAINFALL);   // Sat. hyd. conductivity (ft/sec)
    p[2] = Infil[j].grnAmpt.IMDmax;               // Max. init. moisture deficit
}

//=============================================================================

int grnampt_setParams(TGrnAmpt *infil, double p[])
//
//  Input:   infil = ptr. to Green-Ampt infiltration object
//           p[] = array of parameter values
//  Output:  returns TRUE if parameters are valid, FALSE otherwise
//  Purpose: assigns Green-Ampt infiltration parameters to a subcatchment.
//
{
    double ksat;                       // sat. hyd. conductivity in in/hr

    if ( p[0] < 0.0 || p[1] <= 0.0 || p[2] < 0.0 || p[2] > 1.0) return FALSE;
    infil->S      = p[0] / UCF(RAINDEPTH);   // Capillary suction head (ft)
    infil->Ks     = p[1] / UCF(RAINFALL);    // Sat. hyd. conductivity (ft/sec)
    infil->IMDmax = p[2];                    // Max. init. moisture deficit

    // --- find depth of upper soil zone (ft) using Mein's eqn.
    ksat = infil->Ks * 12. * 3600.;
    infil->Lu = 4.0 * sqrt(ksat) / 12.;
    return TRUE;
}

//=============================================================================

void grnampt_initState(TGrnAmpt *infil)
//
//  Input:   infil = ptr. to Green-Ampt infiltration object
//  Output:  none
//  Purpose: initializes state of Green-Ampt infiltration for a subcatchment.
//
{
    if (infil == NULL) return;
    infil->IMD = infil->IMDmax;
    infil->Fu = 0.0;
    infil->F = 0.0;
    infil->Sat = FALSE;
    infil->T = 0.0;
}

void grnampt_getState(TGrnAmpt *infil, double x[])
{
    x[0] = infil->IMD;
    x[1] = infil->F;
    x[2] = infil->Fu;
    x[3] = infil->Sat;
    x[4] = infil->T;
}

void grnampt_setState(TGrnAmpt *infil, double x[])
{
    infil->IMD = x[0];
    infil->F   = x[1];
    infil->Fu  = x[2];
    infil->Sat = (char)x[3];
    infil->T   = x[4];
}

//=============================================================================

double grnampt_getInfil(TGrnAmpt *infil, double tstep, double irate,
    double depth, int modelType) 
//
//  Input:   infil = ptr. to Green-Ampt infiltration object
//           tstep =  time step (sec),
//           irate = net "rainfall" rate to upper zone (ft/sec);
//                 = rainfall + snowmelt + runon,
//                   does not include ponded water (added on below)
//           depth = depth of ponded water (ft)
//           modelType = either GREEN_AMPT or MOD_GREEN_AMPT 
//  Output:  returns infiltration rate (ft/sec)
//  Purpose: computes Green-Ampt infiltration for a subcatchment
//           or a storage node.
//
{
    // --- find saturated upper soil zone water volume
    Fumax = infil->IMDmax * infil->Lu * sqrt(InfilFactor);

    // --- reduce time until next event
    infil->T -= tstep;

    // --- use different procedures depending on upper soil zone saturation
    if ( infil->Sat ) return grnampt_getSatInfil(infil, tstep, irate, depth);
    else return grnampt_getUnsatInfil(infil, tstep, irate, depth, modelType);
}

//=============================================================================

double grnampt_getUnsatInfil(TGrnAmpt *infil, double tstep, double irate,
    double depth, int modelType)
//
//  Input:   infil = ptr. to Green-Ampt infiltration object
//           tstep =  runoff time step (sec),
//           irate = net "rainfall" rate to upper zone (ft/sec);
//                 = rainfall + snowmelt + runon,
//                   does not include ponded water (added on below)
//           depth = depth of ponded water (ft)
//           modelType = either GREEN_AMPT or MOD_GREEN_AMPT
//  Output:  returns infiltration rate (ft/sec)
//  Purpose: computes Green-Ampt infiltration when upper soil zone is
//           unsaturated.
//
{
    double ia, c1, F2, dF, Fs, kr, ts;
    double ks = infil->Ks * InfilFactor;
    double lu = infil->Lu * sqrt(InfilFactor);

    // --- get available infiltration rate (rainfall + ponded water)
    ia = irate + depth / tstep;
    if ( ia < ZERO ) ia = 0.0;

    // --- no rainfall so recover upper zone moisture
    if ( ia == 0.0 )
    {
        if ( infil->Fu <= 0.0 ) return 0.0;
        kr = lu / 90000.0 * Evap.recoveryFactor; 
        dF = kr * Fumax * tstep;
        infil->F -= dF;
        infil->Fu -= dF;
        if ( infil->Fu <= 0.0 )
        {
            infil->Fu = 0.0;
            infil->F = 0.0;
            infil->IMD = infil->IMDmax;
            return 0.0;
        }

        // --- if new wet event begins then reset IMD & F
        if ( infil->T <= 0.0 )
        {
            infil->IMD = (Fumax - infil->Fu) / lu; 
            infil->F = 0.0;
        }
        return 0.0;
    }

    // --- rainfall does not exceed Ksat
    if ( ia <= ks )
    {
        dF = ia * tstep;
        infil->F += dF;
        infil->Fu += dF;
        infil->Fu = MIN(infil->Fu, Fumax);
        if ( modelType == GREEN_AMPT &&  infil->T <= 0.0 )
        {
            infil->IMD = (Fumax - infil->Fu) / lu;
            infil->F = 0.0;
        }
        return ia;
    }

    // --- rainfall exceeds Ksat; renew time to drain upper zone
    infil->T = 5400.0 / lu / Evap.recoveryFactor; 

    // --- find volume needed to saturate surface layer
    Fs = ks * (infil->S + depth) * infil->IMD / (ia - ks);

    // --- surface layer already saturated
    if ( infil->F > Fs )
    {
        infil->Sat = TRUE;
        return grnampt_getSatInfil(infil, tstep, irate, depth);
    }

    // --- surface layer remains unsaturated
    if ( infil->F + ia*tstep < Fs )
    {
        dF = ia * tstep;
        infil->F += dF;
        infil->Fu += dF;
        infil->Fu = MIN(infil->Fu, Fumax);
        return ia;
    }

    // --- surface layer becomes saturated during time step;
    // --- compute portion of tstep when saturated
    ts  = tstep - (Fs - infil->F) / ia;
    if ( ts <= 0.0 ) ts = 0.0;

    // --- compute new total volume infiltrated
    c1 = (infil->S + depth) * infil->IMD;
    F2 = grnampt_getF2(Fs, c1, ks, ts);
    if ( F2 > Fs + ia*ts ) F2 = Fs + ia*ts;

    // --- compute infiltration rate
    dF = F2 - infil->F;
    infil->F = F2;
    infil->Fu += dF;
    infil->Fu = MIN(infil->Fu, Fumax);
    infil->Sat = TRUE;
    return dF / tstep;
}

//=============================================================================

double grnampt_getSatInfil(TGrnAmpt *infil, double tstep, double irate,
    double depth)
//
//  Input:   infil = ptr. to Green-Ampt infiltration object
//           tstep =  runoff time step (sec),
//           irate = net "rainfall" rate to upper zone (ft/sec);
//                 = rainfall + snowmelt + runon,
//                   does not include ponded water (added on below)
//           depth = depth of ponded water (ft).
//  Output:  returns infiltration rate (ft/sec)
//  Purpose: computes Green-Ampt infiltration when upper soil zone is
//           saturated.
//
{
    double ia, c1, dF, F2;
    double ks = infil->Ks * InfilFactor;
    double lu = infil->Lu * sqrt(InfilFactor);

    // --- get available infiltration rate (rainfall + ponded water)
    ia = irate + depth / tstep;
    if ( ia < ZERO ) return 0.0;

    // --- re-set new event recovery time
    infil->T = 5400.0 / lu / Evap.recoveryFactor;

    // --- solve G-A equation for new cumulative infiltration volume (F2)
    c1 = (infil->S + depth) * infil->IMD;
    F2 = grnampt_getF2(infil->F, c1, ks, tstep);
    dF = F2 - infil->F;

    // --- all available water infiltrates -- set saturated state to false
    if ( dF > ia * tstep )
    {
        dF = ia * tstep;
        infil->Sat = FALSE;
    }

    // --- update total infiltration and upper zone moisture deficit
    infil->F += dF;
    infil->Fu += dF;
    infil->Fu = MIN(infil->Fu, Fumax);
    return dF / tstep;
}

//=============================================================================

double grnampt_getF2(double f1, double c1, double ks, double ts)
//
//  Input:   f1 = old infiltration volume (ft)
//           c1 = head * moisture deficit (ft)
//           ks = sat. hyd. conductivity (ft/sec)
//           ts = time step (sec)
//  Output:  returns infiltration volume at end of time step (ft)
//  Purpose: computes new infiltration volume over a time step
//           using Green-Ampt formula for saturated upper soil zone.
//
{
    int    i;
    double f2 = f1;
    double f2min;
    double df2;
    double c2;

    // --- find min. infil. volume
    f2min = f1 + ks * ts;

    // --- use min. infil. volume for 0 moisture deficit
    if ( c1 == 0.0 ) return f2min;

    // --- use direct form of G-A equation for small time steps
    //     and c1/f1 < 100
    if ( ts < 10.0 && f1 > 0.01 * c1 )
    {
        f2 = f1 + ks * (1.0 + c1/f1) * ts;
        return MAX(f2, f2min);
    }

    // --- use Newton-Raphson method to solve integrated G-A equation
    //     (convergence limit reduced from that used in previous releases)
    c2 = c1 * log(f1 + c1) - ks * ts;
    for ( i = 1; i <= 20; i++ )
    {
        df2 = (f2 - f1 - c1 * log(f2 + c1) + c2) / (1.0 - c1 / (f2 + c1) );
        if ( fabs(df2) < 0.00001 )
        {
            return MAX(f2, f2min);
        }
        f2 -= df2;
    }
    return f2min;
}

//=============================================================================

int curvenum_setParams(TCurveNum *infil, double p[])
//
//  Input:   infil = ptr. to Curve Number infiltration object
//           p[] = array of parameter values
//  Output:  returns TRUE if parameters are valid, FALSE otherwise
//  Purpose: assigns Curve Number infiltration parameters to a subcatchment.
//
{

    // --- convert Curve Number to max. infil. capacity
    if ( p[0] < 10.0 ) p[0] = 10.0;
    if ( p[0] > 99.0 ) p[0] = 99.0;
    infil->Smax    = (1000.0 / p[0] - 10.0) / 12.0;
    if ( infil->Smax < 0.0 ) return FALSE;

    // --- convert drying time (days) to a regeneration const. (1/sec)
    if ( p[2] > 0.0 )  infil->regen =  1.0 / (p[2] * SECperDAY);
    else return FALSE;

    // --- compute inter-event time from regeneration const. as in Green-Ampt
    infil->Tmax = 0.06 / infil->regen;

    return TRUE;
}

//=============================================================================

void curvenum_initState(TCurveNum *infil)
//
//  Input:   infil = ptr. to Curve Number infiltration object
//  Output:  none
//  Purpose: initializes state of Curve Number infiltration for a subcatchment.
//
{
    infil->S  = infil->Smax;
    infil->P  = 0.0;
    infil->F  = 0.0;
    infil->T  = 0.0;
    infil->Se = infil->Smax;
    infil->f  = 0.0;
}

void curvenum_getState(TCurveNum *infil, double x[])
{
    x[0] = infil->S;
    x[1] = infil->P;
    x[2] = infil->F;
    x[3] = infil->T;
    x[4] = infil->Se;
    x[5] = infil->f;
}

void curvenum_setState(TCurveNum *infil, double x[])
{
    infil->S  = x[0];
    infil->P  = x[1];
    infil->F  = x[2];
    infil->T  = x[3];
    infil->Se = x[4];
    infil->f  = x[5];
}

//=============================================================================

double curvenum_getInfil(TCurveNum *infil, double tstep, double irate,
    double depth)
//
//  Input:   infil = ptr. to Curve Number infiltration object
//           tstep = runoff time step (sec),
//           irate = rainfall rate (ft/sec);
//           depth = depth of runon + ponded water (ft)
//  Output:  returns infiltration rate (ft/sec)
//  Purpose: computes infiltration rate using the Curve Number method.
//  Note:    this function treats runon from other subcatchments as part
//           of the ponded depth and not as an effective rainfall rate.
{
    double F1;                         // new cumulative infiltration (ft)
    double f1 = 0.0;                   // new infiltration rate (ft/sec)
    double fa = irate + depth/tstep;   // max. available infil. rate (ft/sec)

    // --- case where there is rainfall
    if ( irate > ZERO )
    {
        // --- check if new rain event
        if ( infil->T >= infil->Tmax )
        {
            infil->P = 0.0;
            infil->F = 0.0;
            infil->f = 0.0;
            infil->Se = infil->S;
        }
        infil->T = 0.0;

        // --- update cumulative precip.
        infil->P += irate * tstep;

        // --- find potential new cumulative infiltration
        F1 = infil->P * (1.0 - infil->P / (infil->P + infil->Se));

        // --- compute potential infiltration rate
        f1 = (F1 - infil->F) / tstep;
        if ( f1 < 0.0 || infil->S <= 0.0 ) f1 = 0.0;

    }

    // --- case of no rainfall
    else
    {
        // --- if there is ponded water then use previous infil. rate
        if ( depth > MIN_TOTAL_DEPTH && infil->S > 0.0 )
        {
            f1 = infil->f;
            if ( f1*tstep > infil->S ) f1 = infil->S / tstep;
        }

        // --- otherwise update inter-event time
        else infil->T += tstep;
    }

    // --- if there is some infiltration
    if ( f1 > 0.0 )
    {
        // --- limit infil. rate to max. available rate
        f1 = MIN(f1, fa);
        f1 = MAX(f1, 0.0);

        // --- update actual cumulative infiltration
        infil->F += f1 * tstep;

        // --- reduce infil. capacity if a regen. constant was supplied
        if ( infil->regen > 0.0 )
        {
            infil->S -= f1 * tstep;
            if ( infil->S < 0.0 ) infil->S = 0.0;
        }
    }

    // --- otherwise regenerate infil. capacity
    else
    {
        infil->S += infil->regen * infil->Smax * tstep * Evap.recoveryFactor;
        if ( infil->S > infil->Smax ) infil->S = infil->Smax;
    }
    infil->f = f1;
    return f1;
}
