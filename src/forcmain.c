//-----------------------------------------------------------------------------
//   forcemain.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//   Author:   L. Rossman
//
//   Special Non-Manning Force Main functions
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
static const double VISCOS = 1.1E-5;   // Kinematic viscosity of water
                                       // @ 20 deg C (sq ft/sec)

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
// forcemain_getEquivN
// forcemain_getRoughFactor
// forcemain_getFricSlope

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static double forcemain_getFricFactor(double e, double hrad, double re);
static double forcemain_getReynolds(double v, double hrad);

//=============================================================================

double forcemain_getEquivN(int j, int k)
//
//  Input:   j = link index
//           k = conduit index
//  Output:  returns an equivalent Manning's n for a force main
//  Purpose: computes a Mannng's n that results in the same normal flow
//           value for a force main flowing full under fully turbulent
//           conditions using either the Hazen-Williams or Dary-Weisbach
//           flow equations.
//
{
    TXsect xsect = Link[j].xsect;
    double f;
    double d = xsect.yFull;
    switch ( ForceMainEqn )
    {
      case H_W:
        return 1.067 / xsect.rBot * pow(d/Conduit[k].slope, 0.04);
      case D_W:
        f = forcemain_getFricFactor(xsect.rBot, d/4.0, 1.0e12);
        return sqrt(f/185.0) * pow(d, (1./6.));
    }
    return Conduit[k].roughness;
}

//=============================================================================

double forcemain_getRoughFactor(int j, double lengthFactor)
//
//  Input:   j = link index
//           lengthFactor = factor by which a pipe will be artifically lengthened
//  Output:  returns a roughness adjustment factor for a force main
//  Purpose: computes an adjustment factor for a force main that compensates for
//           any artificial lengthening the pipe may have received.
//
{
    TXsect xsect = Link[j].xsect;
    double r;
    switch ( ForceMainEqn )
    {
      case H_W:
        r = 1.318*xsect.rBot*pow(lengthFactor, 0.54);
        return GRAVITY / pow(r, 1.852);
      case D_W:
        return 1.0/8.0/lengthFactor;
    }
    return 0.0;
}

//=============================================================================

double forcemain_getFricSlope(int j, double v, double hrad)
//
//  Input:   j = link index
//           v = flow velocity (ft/sec)
//           hrad = hydraulic radius (ft)
//  Output:  returns a force main pipe's friction slope 
//  Purpose: computes the headloss per unit length used in dynamic wave
//           flow routing for a pressurized force main using either the
//           Hazen-Williams or Darcy-Weisbach flow equations.
//  Note:    the pipe's roughness factor was saved in xsect.sBot in
//           conduit_validate() in LINK.C.
//
{
    double re, f;
    TXsect xsect = Link[j].xsect;
    switch ( ForceMainEqn )
    {
      case H_W:
        return xsect.sBot * pow(v, 0.852) / pow(hrad, 1.1667);
      case D_W:
        re = forcemain_getReynolds(v, hrad);
        f = forcemain_getFricFactor(xsect.rBot, hrad, re);
        return f * xsect.sBot * v / hrad;
    }
    return 0.0;
}

//=============================================================================

double forcemain_getReynolds(double v, double hrad)
//
//  Input:   v = flow velocity (ft/sec)
//           hrad = hydraulic radius (ft)
//  Output:  returns a flow's Reynolds Number
//  Purpose: computes a flow's Reynolds Number
//
{
    return 4.0 * hrad * v / VISCOS;
}   
    
//=============================================================================

double forcemain_getFricFactor(double e, double hrad, double re)
//
//  Input:   e = roughness height (ft)
//           hrad = hydraulic radius (ft)
//           re = Reynolds number
//  Output:  returns a Darcy-Weisbach friction factor
//  Purpose: computes the Darcy-Weisbach friction factor for a force main
//           using the Swamee and Jain approximation to the Colebrook-White
//           equation.
//
{
    double f;
    if ( re < 10.0 ) re = 10.0;
    if ( re <= 2000.0 ) f = 64.0 / re;
    else if ( re < 4000.0 )
    {
        f = forcemain_getFricFactor(e, hrad, 4000.0);
        f = 0.032 + (f - 0.032) * ( re - 2000.0) / 2000.0;
    }
    else
    {
        f = e/3.7/(4.0*hrad);
        if ( re < 1.0e10 ) f += 5.74/pow(re, 0.9);
        f = log10(f);
        f = 0.25 / f / f;
    }
    return f;
}
