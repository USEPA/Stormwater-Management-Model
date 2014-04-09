//-----------------------------------------------------------------------------
//   infil.c
//
//   Project:  EPA SWMM5
//   Version:  5.0
//   Date:     4/10/09   (Build 5.0.015)
//             10/7/09   (Build 5.0.017)
//             07/30/10  (Build 5.0.019)
//             09/30/10  (Build 5.0.021)
//             04/20/11  (Build 5.0.022)
//   Author:   L. Rossman
//
//   Infiltration functions.
//
//   Note: for release 5.0.015, the infiltration functions were modified to
//         accept a pointer to an infiltration object rather than an index 
//         into an array of infiltration objects.
//
//   Note: for release 5.0.019, the infiltration data arrays are created and
//         deleted locally in this module rather than in project.c. Also the
//         infiltration data structures are declared in infil.h rather than in
//         objects.h.
//         
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include "malloc.h"                                                            //(5.0.019 - LR)
#include "stdlib.h"                                                            //(5.0.019 - LR)
#include "headers.h"
#include "infil.h"                                                             //(5.0.019 - LR)

//-----------------------------------------------------------------------------
//  Local Variables
//-----------------------------------------------------------------------------
THorton*   HortInfil = NULL;                                                   //(5.0.019 - LR)
TGrnAmpt*  GAInfil   = NULL;                                                   //(5.0.019 - LR)
TCurveNum* CNInfil   = NULL;                                                   //(5.0.019 - LR)

//-----------------------------------------------------------------------------
//  External Functions (declared in infil.h)                                   //(5.0.019 - LR)
//-----------------------------------------------------------------------------
//  infil_create     (called by createObjects in project.c)                    //(5.0.019 - LR)
//  infil_delete     (called by deleteObjects in project.c)                    //(5.0.019 - LR)
//  infil_readParams (called by input_readLine)
//  infil_initState  (called by subcatch_initState)
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
static double horton_getInfil(THorton *infil, double tstep, double irate,
              double depth);

static double grnampt_getRate(TGrnAmpt *infil, double tstep, double F2,
              double F);
static double grnampt_getF2(double f1, double c1, double c2, double iv2);
static void   grnampt_setT(TGrnAmpt *infil);

static int    curvenum_setParams(TCurveNum *infil, double p[]);
static void   curvenum_initState(TCurveNum *infil);
static double curvenum_getInfil(TCurveNum *infil, double tstep, double irate,
              double depth);

//=============================================================================

////  New function added to SWMM5 LID edition.  ////                           //(5.0.019 - LR)

void infil_create(int subcatchCount, int model)
//
//  Purpose: creates an array of infiltration objects.
//  Input:   n = number of subcatchments
//           m = infiltration method code
//  Output:  none
//
{
    switch (model)
    {
    case HORTON:
        HortInfil = (THorton *) calloc(subcatchCount, sizeof(THorton));
        if ( HortInfil == NULL ) ErrorCode = ERR_MEMORY;
        break;
    case GREEN_AMPT:
        GAInfil = (TGrnAmpt *) calloc(subcatchCount, sizeof(TGrnAmpt));
        if ( GAInfil == NULL ) ErrorCode = ERR_MEMORY;
        break;
    case CURVE_NUMBER:
        CNInfil = (TCurveNum *) calloc(subcatchCount, sizeof(TCurveNum));
        if ( CNInfil == NULL ) ErrorCode = ERR_MEMORY;
        break;
    default: ErrorCode = ERR_MEMORY;
    }
}

//=============================================================================

////  New function added to SWMM5 LID edition.  ////                           //(5.0.019 - LR)

void infil_delete()
//
//  Purpose: deletes infiltration objects associated with subcatchments
//  Input:   none
//  Output:  none
//
{
    FREE(HortInfil);
    FREE(GAInfil);
    FREE(CNInfil);
}

//=============================================================================

int infil_readParams(int m, char* tok[], int ntoks)
//
//  Input:   m = infiltration method code
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: sets infiltration parameters from a line of input data.
//
//  Format of data line is:
//     subcatch  p1  p2 ...
{
    int   i, j, n, status;
    double x[5];

    // --- check that subcatchment exists
    j = project_findObject(SUBCATCH, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);

    // --- number of input tokens depends on infiltration model m
    if      ( m == HORTON )       n = 5;
    else if ( m == GREEN_AMPT )   n = 4;
    else if ( m == CURVE_NUMBER ) n = 4;
    else return 0;
    if ( ntoks < n ) return error_setInpError(ERR_ITEMS, "");

    // --- parse numerical values from tokens
    for (i = 0; i < 5; i++) x[i] = 0.0;
    for (i = 1; i < n; i++)
    {
        if ( ! getDouble(tok[i], &x[i-1]) )
            return error_setInpError(ERR_NUMBER, tok[i]);
    }

    // --- special case for Horton infil. - last parameter is optional
    if ( m == HORTON && ntoks > n )
    {
        if ( ! getDouble(tok[n], &x[n-1]) )
            return error_setInpError(ERR_NUMBER, tok[n]);
    }

    // --- assign parameter values to infil. object
    Subcatch[j].infil = j;
    switch (m)
    {
      case HORTON:       status = horton_setParams(&HortInfil[j], x);
                         break;
      case GREEN_AMPT:   status = grnampt_setParams(&GAInfil[j], x);
                         break;
      case CURVE_NUMBER: status = curvenum_setParams(&CNInfil[j], x);
                         break;
      default:           status = TRUE;
    }
    if ( !status ) return error_setInpError(ERR_NUMBER, "");
    return 0;
}

//=============================================================================

void infil_initState(int j, int m)
//
//  Input:   j = subcatchment index
//           m = infiltration method code
//  Output:  none
//  Purpose: initializes state of infiltration for a subcatchment.
//
{
    switch (m)
    {
      case HORTON:       horton_initState(&HortInfil[j]);   break;
      case GREEN_AMPT:   grnampt_initState(&GAInfil[j]);    break;
      case CURVE_NUMBER: curvenum_initState(&CNInfil[j]);   break;
    }
}

//=============================================================================

double infil_getInfil(int j, int m, double tstep, double rainfall,
                      double runon, double depth)
//
//  Input:   j = subcatchment index
//           m = infiltration method code
//           tstep = runoff time step (sec)
//           rainfall = rainfall rate (ft/sec)
//           runon = runon rate from other sub-areas or subcatchments (ft/sec)
//           depth = depth of surface water on subcatchment (ft)
//  Output:  returns infiltration rate (ft/sec)
//  Purpose: computes infiltration rate depending on infiltration method.
//
{
    switch (m)
    {
      case HORTON:
        return horton_getInfil(&HortInfil[j], tstep, rainfall+runon, depth);

      case GREEN_AMPT:
        return grnampt_getInfil(&GAInfil[j], tstep, rainfall+runon, depth);

      case CURVE_NUMBER:
        depth += runon / tstep;
        return curvenum_getInfil(&CNInfil[j], tstep, rainfall, depth);

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
    for (k=0; k<5; k++) if ( p[k] < 0.0 ) return FALSE;

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
    double fmin = infil->fmin;
    double Fmax = infil->Fmax;
    double tp   = infil->tp;
    double df   = infil->f0 - fmin;
    double kd   = infil->decay;
    double kr   = infil->regen * Evap.recoveryFactor;

    // --- special cases of no infil. or constant infil
    if ( df < 0.0 || kd < 0.0 || kr < 0.0 ) return 0.0;
    if ( df == 0.0 || kd == 0.0 )
    {
        fp = infil->f0;
        fa = irate + depth / tstep;
        if ( fp > fa ) fp = fa;
        return MAX(0.0, fp);
    }

    // --- compute water available for infiltration
    fa = irate + depth / tstep;

    // --- case where there is water to infiltrate
    if ( fa > ZERO )                                                           //(5.0.022 - LR)
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
        if (Fmax > 0.0)
        {
             if ( Fmax < Fp ) Fp = Fmax;
             if ( Fmax < F1 ) F1 = Fmax;
        }
        fp = (F1 - Fp) / tstep;

        // --- limit infil rate to available infil
        if ( fp > fa ) fp = fa;

        // --- if fp on flat portion of curve then increase tp by tstep
        if ( t1 > tlim ) tp = t1;

        // --- if infil < available capacity then increase tp by tstep
        else if ( fp < fa ) tp = t1;

        // --- if infil limited by available capcity then
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

    }

    // --- case where infil. capacity is regenerating; update tp.
    else if (kr > 0.0)
    {
        r = exp(-kr * tstep);                                                  //(5.0.022 - LR)
        tp = 1.0 - exp(-kd * tp);                                              //(5.0.022 - LR)
        tp = -log(1.0 - r*tp) / kd;                                            //(5.0.022 - LR)
    }
    infil->tp = tp;
    return fp;
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

    if ( p[0] <= 0.0 || p[1] <= 0.0 || p[2] < 0.0 ) return FALSE;
    infil->S      = p[0] / UCF(RAINDEPTH);   // Capillary suction head (ft)
    infil->Ks     = p[1] / UCF(RAINFALL);    // Sat. hyd. conductivity (ft/sec)
    infil->IMDmax = p[2];                    // Max. init. moisture deficit

    // --- find depth of upper soil zone (ft) using Mein's eqn.
    ksat = infil->Ks * 12. * 3600.;
    infil->L = 4.0 * sqrt(ksat) / 12.;

    // --- set max. water volume of upper layer
    infil->FUmax = infil->L * infil->IMDmax;

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
    infil->F = 0.0;
    infil->FU = infil->L * infil->IMD;
    infil->Sat = FALSE;
    infil->T = MISSING;
}

//=============================================================================

////  Comments were revised for release 5.0.019  ////                          //(5.0.019 - LR)

double grnampt_getInfil(TGrnAmpt *infil, double tstep, double irate,
    double depth)
//
//  Input:   infil = ptr. to Green-Ampt infiltration object
//           tstep =  runoff time step (sec),
//           irate = net "rainfall" rate to upper zone (ft/sec);
//                 = rainfall + snowmelt + runon,                              //(5.0.022 - LR)
//                   does not include ponded water (added on below)
//           depth = depth of ponded water (ft).
//  Output:  returns infiltration rate (ft/sec)
//  Purpose: computes Green-Ampt infiltration for a subcatchment.
//
//  Definition of variables:
//   IMD    = initial soil moisture deficit at start of current rain event
//            (void volume / total volume)
//   IMDmax = max. IMD available (ft/ft)
//   Ks     = saturated hyd. conductivity (ft/sec)
//   S      = capillary suction head (ft)
//   F      = cumulative event infiltration at start of time interval (ft)
//   F2     = cumulative infiltration at end of time interval (ft)
//   Fs     = infiltration volume needed to saturate surface (ft)
//   T      = cumulative event duration (sec)
//   Tmax   = max. discrete event duration (sec)
//   L      = depth of upper soil zone (ft)
//   FU     = current moisture content of upper zone (ft)
//   FUmax  = saturated moisture content of upper zone (ft)
//   DF     = upper zone moisture depeletion factor (1/sec)
//   DV     = moisture depletion in upper zone (ft)
//
//   f      = infiltration rate (ft/sec)
//   ivol   = total volume of water on surface (ft)
//   ts     = remainder of time step after surface becomes saturated (sec)
//   iv2    = available surface water volume over time step (ft)               //(5.0.021 - LR)
//   c1, c2 = terms of the implicit Green-Ampt equation
{
    double F = infil->F;
    double F2;
    double DF;
    double DV;
    double Fs;
    double f;
    double ivol, iv2;
    double ts, c1, c2;

    // --- add ponded water onto potential infiltration
    irate += depth / tstep;
    if ( irate < ZERO ) irate = 0.0;                                           //(5.0.021 - LR)
    ivol = irate * tstep;

    // --- add ponded water head to suction head                               //(5.0.019 - LR)
    c1 = (infil->S + depth) * infil->IMD;                                      //(5.0.019 - LR)

    // --- initialize time to drain upper zone
    if ( infil->T == MISSING )
    {
        if ( irate > 0.0 ) grnampt_setT(infil);
        else return 0.0;
    }

    // --- upper soil zone is unsaturated
    if ( !infil->Sat )
    {
        // --- update time remaining until upper zone is completely drained
        infil->T -= tstep;

        // --- no rainfall; deplete soil moisture
        if ( irate <= 0.0 )
        {
           // --- return if no upper zone moisture
            if ( infil->FU <= 0.0 ) return 0.0;
            DF = infil->L / 300. * (12. / 3600.) * Evap.recoveryFactor;
            DV = DF * infil->FUmax * tstep;
            infil->F -= DV;
            infil->FU -= DV;
            if ( infil->FU <= 0.0 )
            {
                infil->FU = 0.0;
                infil->F = 0.0;
                infil->IMD = infil->IMDmax;
                return 0.0;
            }

            // --- if upper zone drained, then redistribute moisture content
            if ( infil->T <= 0.0 )
            {
                infil->IMD = (infil->FUmax - infil->FU) / infil->L;
                infil->F = 0.0;
            }
            return 0.0;
        }

        // --- low rainfall; everything infiltrates
        if ( irate <= infil->Ks )
        {
            F2 = F + ivol;
            f = grnampt_getRate(infil, tstep, F2, F);

            // --- if sufficient time to drain upper zone, then redistribute
            if ( infil->T <= 0.0 )
            {
                infil->IMD = (infil->FUmax - infil->FU) / infil->L;
                infil->F = 0.0;
            }
            return f;
        }

        // --- rainfall > hyd. conductivity; renew time to drain upper zone
        grnampt_setT(infil);

        // --- check if surface already saturated
        Fs = c1 * infil->Ks / (irate - infil->Ks);
        if ( F - Fs >= 0.0 )
        {
            infil->Sat = TRUE;
        }

        // --- check if all water infiltrates
        else if ( Fs - F >= ivol )
        {
            F2 = F + ivol;
            f = grnampt_getRate(infil, tstep, F2, F);
            return f;
        }

        // --- otherwise surface saturates during time interval
        else
        {
            ts  = tstep - (Fs - F) / irate;
            if ( ts <= 0.0 ) ts = 0.0;
            c2  = c1 * log(Fs + c1) - infil->Ks * ts;
            iv2 = ts * irate;                                                  //(5.0.021 - LR)
            iv2 = MIN(iv2, infil->Ks * ts);                                    //(5.0.021 - LR)

            F2  = grnampt_getF2(Fs, c1, c2, iv2);
            f   = grnampt_getRate(infil, tstep, F2, Fs);
            infil->Sat = TRUE;
            return f;
        }
    }

    // --- upper soil zone saturated:

    // --- renew time to drain upper zone
    grnampt_setT(infil);

    // --- compute volume of potential infiltration
    if ( c1 <= 0.0 ) F2 = infil->Ks * tstep + F;
    else
    {
        c2 = c1 * log(F + c1) - infil->Ks * tstep;
        iv2 = tstep * irate;                                                   //(5.0.021 - LR)
        iv2 = MIN(iv2, infil->Ks * tstep);                                     //(5.0.021 - LR)
        F2 = grnampt_getF2(F, c1, c2, iv2);
    }

    // --- excess water will remain on surface
    if ( F2 - F <= ivol )
    {
        f = grnampt_getRate(infil, tstep, F2, F);
        return f;
    }

    // --- all rain + ponded water infiltrates
    F2 = F + ivol;
    f = grnampt_getRate(infil, tstep, F2, F);
    infil->Sat = FALSE;
    return f;
}

//=============================================================================

double grnampt_getRate(TGrnAmpt *infil, double tstep, double F2, double F)
//
//  Input:   infil = ptr. to Green-Ampt infiltration object
//           tstep =  runoff time step (sec),
//           F2 = new cumulative event infiltration volume (ft)
//           F = old cumulative event infiltration volume (ft)
//  Output:  returns infiltration rate (ft/sec)
//  Purpose: computes infiltration rate from change in infiltration volume.
//
{
    double f = (F2 - infil->F) / tstep;
    double dF = F2 - F;
    if ( f < 0.0 ) f = 0.0;
    if ( dF < 0.0 ) dF = 0.0;
    infil->FU += dF;
    if ( infil->FU > infil->FUmax ) infil->FU = infil->FUmax;
    infil->F = F2;
    return f;
}

//=============================================================================

double grnampt_getF2(double f1, double c1, double c2, double iv2)
//
//  Input:   f1 = old infiltration volume (ft)
//           c1, c2 =  equation terms
//           iv2 = minimum possible infiltration over time step (ft)           //(5.0.021 - LR)
//  Output:  returns infiltration volume at end of time step (ft)
//  Purpose: computes new infiltration volume over a time step
//           using Green-Ampt formula for saturated upper soil zone
//
{
    int    i;
    double f2 = f1;
    double df2;


    // --- use Newton-Raphson method to solve governing nonlinear equation
    for ( i = 1; i <= 20; i++ )
    {
        df2 = (f2 - f1 - c1 * log(f2 + c1) + c2) / (1.0 - c1 / (f2 + c1) );
        if ( fabs(df2) < 0.0001 )
        {
            //if ( f2 - f1 < 0.0001 ) return f1 + MIN(0.0001, iv2);            //(5.0.021 - LR)
            //return f2;                                                       //(5.0.021 - LR)
            return MAX(f2, f1+iv2);                                            //(5.0.021 - LR)
        }
        f2 -= df2;
    }
    return f1 + iv2;
}

//=============================================================================

void grnampt_setT(TGrnAmpt *infil)
//
//  Input:   infil = ptr. to Green-Ampt infiltration object
//  Output:  none
//  Purpose: resets maximum time to drain upper soil zone for Green-Ampt
//           infiltration.
//
{
    double DF = infil->L / 300.0 * (12. / 3600.) * Evap.recoveryFactor;
    infil->T = 6.0 / (100.0 * DF);
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

//// ---- linear regeneration constant and inter-event --- //// 
////      time now computed directly from drying time;     ////
////      hydraulic conductivity no longer used.           ////

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

//=============================================================================

////  This function was re-written for release 5.0.022  ////                   //(5.0.022 - LR)

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

//=============================================================================
