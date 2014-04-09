//-----------------------------------------------------------------------------
//   culvert.c
//
//   Project:  EPA SWMM5
//   Version:  5.0
//   Date:     1/21/09    (Build 5.0.014)
//             07/30/10   (Build 5.0.019)
//   Author:   L. Rossman
//
//   Culvert equations for SWMM5
//
//   Computes flow reduction in a culvert-type conduit due to
//   inlet control using equations from the FHWA HEC-5 circular.
//
//   This module was substantially revised for release 5.0.019.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include "findroot.h"
#include "headers.h"

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
enum CulvertParam {FORM, K, M, C, Y};
static const int    MAX_CULVERT_CODE = 57;
static const double Params[58][5] = {

//   FORM   K       M     C        Y  
//------------------------------------
    {0.0, 0.0,    0.0,  0.0,    0.00},

    //Circular concrete 
    {1.0, 0.0098, 2.00, 0.0398, 0.67},  //Square edge w/headwall
    {1.0, 0.0018, 2.00, 0.0292, 0.74},  //Groove end w/headwall
    {1.0, 0.0045, 2.00, 0.0317, 0.69},  //Groove end projecting

    //Circular Corrugated Metal Pipe
    {1.0, 0.0078, 2.00, 0.0379, 0.69},  //Headwall
    {1.0, 0.0210, 1.33, 0.0463, 0.75},  //Mitered to slope
    {1.0, 0.0340, 1.50, 0.0553, 0.54},  //Projecting

    //Circular Pipe, Beveled Ring Entrance
    {1.0, 0.0018, 2.50, 0.0300, 0.74},  //Beveled ring, 45 deg bevels
    {1.0, 0.0018, 2.50, 0.0243, 0.83},  //Beveled ring, 33.7 deg bevels

    //Rectangular Box with Flared Wingwalls
    {1.0, 0.026, 1.0,   0.0347, 0.81},  //30-75 deg. wingwall flares
    {1.0, 0.061, 0.75,  0.0400, 0.80},  //90 or 15 deg. wingwall flares
    {1.0, 0.061, 0.75,  0.0423, 0.82},  //0 deg. wingwall flares (striaght sides)

    //Rectanglar Box with Flared Wingwalls & Top Edge Bevel 
    {2.0, 0.510, 0.667, 0.0309, 0.80},  //45 deg. flare; 0.43D top edge bevel
    {2.0, 0.486, 0.667, 0.0249, 0.83},  //18-33.7 deg flare; 0.083D top edge bevel

    //Rectangular Box; 90-deg Headwall; Chamfered or Beveled Inlet Edges
    {2.0, 0.515, 0.667, 0.0375, 0.79},  //chamfered 3/4-in
    {2.0, 0.495, 0.667, 0.0314, 0.82},  //beveled 1/2-in/ft at 45 deg (1:1)
    {2.0, 0.486, 0.667, 0.0252, 0.865}, //beveled 1-in/ft at 33.7 deg (1:1.5)

    //Rectangular Box; Skewed Headwall; Chamfered or Beveled Inlet Edges
    {2.0, 0.545, 0.667, 0.04505,0.73},  //3/4" chamfered edge, 45 deg skewed headwall
    {2.0, 0.533, 0.667, 0.0425, 0.705}, //3/4" chamfered edge, 30 deg skewed headwall
    {2.0, 0.522, 0.667, 0.0402, 0.68},  //3/4" chamfered edge, 15 deg skewed headwall
    {2.0, 0.498, 0.667, 0.0327, 0.75},  //45 deg beveled edge, 10-45 deg skewed headwall

    //Rectangular box, Non-offset Flared Wingwalls; 3/4" Chamfer at Top of Inlet
    {2.0, 0.497, 0.667, 0.0339, 0.803}, //45 deg (1:1) wingwall flare
    {2.0, 0.493, 0.667, 0.0361, 0.806}, //18.4 deg (3:1) wingwall flare
    {2.0, 0.495, 0.667, 0.0386, 0.71},  //18.4 deg (3:1) wingwall flare, 30 deg inlet skew

    //Rectangular box, Offset Flared Wingwalls, Beveled Edge at Inlet Top
    {2.0, 0.497, 0.667, 0.0302, 0.835},  //45 deg (1:1) flare, 0.042D top edge bevel
    {2.0, 0.495, 0.667, 0.0252, 0.881},  //33.7 deg (1.5:1) flare, 0.083D top edge bevel
    {2.0, 0.493, 0.667, 0.0227, 0.887},  //18.4 deg (3:1) flare, 0.083D top edge bevel

    // Corrugated Metal Box
    {1.0, 0.0083, 2.00, 0.0379, 0.69},  //90 deg headwall
    {1.0, 0.0145, 1.75, 0.0419, 0.64},  //Thick wall projecting
    {1.0, 0.0340, 1.50, 0.0496, 0.57},  //Thin wall projecting

    // Horizontal Ellipse Concrete
    {1.0, 0.0100, 2.00, 0.0398, 0.67},  //Square edge w/headwall
    {1.0, 0.0018, 2.50, 0.0292, 0.74},  //Grooved end w/headwall
    {1.0, 0.0045, 2.00, 0.0317, 0.69},  //Grooved end projecting

    // Vertical Ellipse Concrete
    {1.0, 0.0100, 2.00, 0.0398, 0.67},  //Square edge w/headwall
    {1.0, 0.0018, 2.50, 0.0292, 0.74},  //Grooved end w/headwall
    {1.0, 0.0095, 2.00, 0.0317, 0.69},  //Grooved end projecting

    // Pipe Arch, 18" Corner Radius, Corrugated Metal
    {1.0, 0.0083, 2.00, 0.0379, 0.69},  //90 deg headwall
    {1.0, 0.0300, 1.00, 0.0463, 0.75},  //Mitered to slope
    {1.0, 0.0340, 1.50, 0.0496, 0.57},  //Projecting

    // Pipe Arch, 18" Corner Radius, Corrugated Metal
    {1.0, 0.0300, 1.50, 0.0496, 0.57},  //Projecting
    {1.0, 0.0088, 2.00, 0.0368, 0.68},  //No bevels
    {1.0, 0.0030, 2.00, 0.0269, 0.77},  //33.7 deg bevels

    // Pipe Arch, 31" Corner Radius, Corrugated Metal
    {1.0, 0.0300, 1.50, 0.0496, 0.57},  //Projecting
    {1.0, 0.0088, 2.00, 0.0368, 0.68},  //No bevels
    {1.0, 0.0030, 2.00, 0.0269, 0.77},  //33.7 deg. bevels

    // Arch, Corrugated Metal
    {1.0, 0.0083, 2.00, 0.0379, 0.69},  //90 deg headwall
    {1.0, 0.0300, 1.00, 0.0463, 0.75},  //Mitered to slope
    {1.0, 0.0340, 1.50, 0.0496, 0.57},  //Thin wall projecting

    // Circular Culvert
    {2.0, 0.534, 0.555, 0.0196, 0.90},  //Smooth tapered inlet throat
    {2.0, 0.519, 0.640, 0.0210, 0.90},  //Rough tapered inlet throat

    // Elliptical Inlet Face
    {2.0, 0.536, 0.622, 0.0368, 0.83},  //Tapered inlet, beveled edges
    {2.0, 0.5035,0.719, 0.0478, 0.80},  //Tapered inlet, square edges
    {2.0, 0.547, 0.800, 0.0598, 0.75},  //Tapered inlet, thin edge projecting

    // Rectangular
    {2.0, 0.475, 0.667, 0.0179, 0.97},  //Tapered inlet throat

    // Rectangular Concrete
    {2.0, 0.560, 0.667, 0.0446, 0.85},  //Side tapered, less favorable edges
    {2.0, 0.560, 0.667, 0.0378, 0.87},  //Side tapered, more favorable edges

    // Rectangular Concrete
    {2.0, 0.500, 0.667, 0.0446, 0.65}, //Slope tapered, less favorable edges
    {2.0, 0.500, 0.667, 0.0378, 0.71}  //Slope tapered, more favorable edges

    };

//-----------------------------------------------------------------------------
//  Shared Variables
//-----------------------------------------------------------------------------
static double  Yfull;                  // full depth of culvert (ft)
static double  Scf;                    // slope correction factor
static double  Dqdh;                   // Derivative of flow w.r.t. head 
static double  Qc;                     // Unsubmerged critical flow
static double  KK, MM;                 // Coeffs. for unsubmerged flow
static double  AD, Hplus;              // Intermediate terms
static TXsect* Xsect;                  // Pointer to culvert cross section

static double Yratio;

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  double culvert_getInflow

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static double getUnsubmergedFlow(int code, double h);
static double getSubmergedFlow(int code, double h);
static double getTransitionFlow(int code, double h, double h1, double h2);
static double getForm1Flow(double h);
static double form1Eqn(double yc);

static void report_CulvertControl(int j, double q0, double q, int condition);  //for debugging only


//=============================================================================

double culvert_getInflow(int j, double q0, double h)
//
//  Input:   j  = link index
//           q0 = unmodified flow rate (cfs)
//           h  = upstream head (ft)
//  Output:  returns modified flow rate through culvert (cfs)
//  Purpose: uses FHWA HEC-5 equations to find flow through inlet
//           controlled culverts
//
{
    int     code,                      //culvert type code number
            k,                         //conduit index
            condition;                 //flow condition
    double  y,                         //current depth (ft)
            y1,                        //unsubmerged depth limit (ft)
            y2,                        //submerged depth limit (ft)
            q;                         //inlet-controlled flow (cfs)

    // --- check that we have a culvert conduit    
    if ( Link[j].type != CONDUIT ) return q0;
    Xsect = &Link[j].xsect;
    code = Xsect->culvertCode;
    if ( code <= 0 || code > MAX_CULVERT_CODE ) return q0;

    // --- compute often-used variables 
    k = Link[j].subIndex;
    Scf = 0.5 * Conduit[k].slope;
    Yfull = Xsect->yFull;
    AD = Xsect->aFull * sqrt(Yfull);

    // --- find head relative to culvert's upstream invert
    //     (can be greater than yFull when inlet is submerged) 
    y = h - (Node[Link[j].node1].invertElev + Link[j].offset1);

    Yratio = y / Yfull;

    // --- check for submerged flow (based on FHWA criteria of Q/AD > 4)
    y2 = Yfull * (16.0 * Params[code][C] + Params[code][Y] - Scf);
    if ( y >= y2 )
    {    
        q = getSubmergedFlow(code, y);
        condition = 2;
    }
    else
    {
        // --- check for unsubmerged flow (based on arbitrary limit of 0.95 full)
        y1 = 0.95 * Yfull;
        if ( y <= y1 )
        {
            q = getUnsubmergedFlow(code, y);
            condition = 1;
        }
        // --- flow is in transition zone
        else
        {
            q = getTransitionFlow(code, y, y1, y2);
            condition = 0;
        }
    }

    // --- check if inlet controls and replace conduit's value of dq/dh
    if ( q < q0 )
    {
        // --- for debugging only
        //if ( RptFlags.controls ) report_CulvertControl(j, q0, q, condition);

        Link[j].dqdh = Dqdh;
        return q;
    }
    else return q0;
}

//=============================================================================

double getUnsubmergedFlow(int code, double h)
//
//  Input:   code  = culvert type code number
//           h     = inlet water depth above culvert invert
//  Output:  returns flow rate;
//           computes value of module-level variable Dqdh
//  Purpose: computes flow rate and its derivative for unsubmerged
//           culvert inlet.
//
{
    double arg; 
    double q;

    // --- assign shared variables
    KK = Params[code][K];
    MM = Params[code][M];
    arg = h / Yfull / KK;

    // --- evaluate correct equation form
    if ( Params[code][FORM] == 1.0)
    {
        q = getForm1Flow(h);
    }
    else q = AD * pow(arg, 1.0/MM);
    Dqdh = q / h / MM;
    return q;
}

//=============================================================================

double getSubmergedFlow(int code, double h)
//
//  Input:   code  = culvert type code number
//           h     = inlet head (ft)
//  Output:  returns flow rate;
//           computes value of module-level variable Dqdh
//  Purpose: computes flow rate and its derivative for submerged
//           culvert inlet.
//
{
    double CC = Params[code][C];
    double YY = Params[code][Y];
    double arg = (h/Yfull - YY + Scf) / CC ;
    double q;

    if ( arg <= 0.0 )
    {
        Dqdh = 0.0;
        return BIG;
    }
    q = sqrt(arg) * AD;
    Dqdh = 0.5 * q / arg / Yfull / CC;
    return q;
}

//=============================================================================

double getTransitionFlow(int code, double h, double h1, double h2)
//
//  Input:   code  = culvert type code number
//           h     = inlet water depth above culvert invert (ft)
//           h1    = head limit for unsubmerged condition (ft)
//           h2    = head limit for submerged condition (ft)
//  Output:  returns flow rate )cfs);
//           computes value of module-level variable Dqdh (cfs/ft)
//  Purpose: computes flow rate and its derivative for inlet-controlled flow
//           when inlet water depth lies in the transition range between
//           submerged and unsubmerged conditions.
//
{
    double q1 = getUnsubmergedFlow(code, h1);
    double q2 = getSubmergedFlow(code, h2);
    double q = q1 + (q2 - q1) * (h - h1) / (h2 - h1);
    Dqdh = (q2 - q1) / (h2 - h1);
    return q;
}

//=============================================================================

double getForm1Flow(double h)
//
//  Input:   h     = inlet water depth above culvert invert
//  Output:  returns inlet controlled flow rate
//  Purpose: computes inlet-controlled flow rate for unsubmerged culvert
//           using FHWA Equation Form1.
//
//  See pages 195-196 of FHWA HEC-5 (2001) for details.
//
{
    double yc;

    // --- save re-used terms in module-level variables
    Hplus = h/Yfull + Scf;

    // --- use Ridder's method to solve Equation Form 1 for critical depth
    //     between a range of 0.01h and h
    yc = findroot_Ridder(0.01*h, h, 0.001, form1Eqn);

    // --- return the flow value used in evaluating Equation Form 1
    return Qc;
}

//=============================================================================

double form1Eqn(double yc)
//
//  Input:   yc = critical depth
//  Output:  returns residual error
//  Purpose: evaluates the error in satisfying FHWA culvert Equation Form1:
//
//  h/yFull + 0.5*s = yc/yFull + yh/2/yFull + K[ac/aFull*sqrt(g*yh/yFull)]^M 
//
//  for a given value of critical depth yc where:
//    h = inlet depth above culvert invert
//    s = culvert slope
//    yFull = full depth of culvert
//    yh = hydraulic depth at critical depth
//    ac = flow area at critical depth
//    g = accel. of gravity
//    K and M = coefficients
//
{
    double ac = xsect_getAofY(Xsect, yc);
    double wc = xsect_getWofY(Xsect, yc);
    double yh = ac/wc;
    
    Qc = ac * sqrt(GRAVITY * yh);
    return Hplus - yc/Yfull - yh/2.0/Yfull - KK * pow(Qc/AD, MM);
}

//=============================================================================

void report_CulvertControl(int j, double q0, double q, int condition)
//
//  Used for debugging only
//
{
    static   char* conditionTxt[] = {"transition", "unsubmerged", "submerged"};
    char     theDate[12];
    char     theTime[9];
	DateTime aDate = getDateTime(NewRoutingTime);
    datetime_dateToStr(aDate, theDate);
    datetime_timeToStr(aDate, theTime);
    fprintf(Frpt.file,
            "\n  %11s: %8s Culvert %s flow reduced from %.3f to %.3f cfs for %s flow (%.2f).",
            theDate, theTime, Link[j].ID, q0, q, conditionTxt[condition], Yratio);
}
