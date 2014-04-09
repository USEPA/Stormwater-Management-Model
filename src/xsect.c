//-----------------------------------------------------------------------------
//   xsect.c
//
//   Project:  EPA SWMM5
//   Version:  5.0
//   Date:     6/19/07   (Build 5.0.010)
//             2/4/08    (Build 5.0.012)
//             1/21/09   (Build 5.0.014)
//             04/20/11  (Build 5.0.022)
//   Author:   L. Rossman
//
//   Cross section geometry functions.
//
//   The primary functions are:
//      getAofY   -- returns area given depth
//      getWofY   -- returns top width given depth
//      getRofY   -- returns hyd. radius given depth
//      getYofA   -- returns flow depth given area
//      getRofA   -- returns hyd. radius given area
//      getSofA   -- returns section factor given area
//      getAofS   -- returns area given section factor
//      getdSdA   -- returns derivative of section factor w.r.t. area
//   where
//      Y = flow depth
//      A = flow area
//      R = hyd. radius
//      S = section factor = A*R^(2/3)
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include "headers.h"
#include "findroot.h"

#define  RECT_ALFMAX        0.97
#define  RECT_TRIANG_ALFMAX 0.98
#define  RECT_ROUND_ALFMAX  0.98

#include "xsect.dat"    // File containing geometry tables for rounded shapes

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
// Ratio of area at max. flow to full area
// (= 1.0 for open shapes, < 1.0 for closed shapes)
double  Amax[] = {
                     1.0,     //  DUMMY
                     0.9756,  //  CIRCULAR
                     0.9756,  //  FILLED_CIRCULAR
                     0.97,    //  RECT_CLOSED
                     1.0,     //  RECT_OPEN
                     1.0,     //  TRAPEZOIDAL
                     1.0,     //  TRIANGULAR
                     1.0,     //  PARABOLIC
                     1.0,     //  POWERFUNC
                     0.98,    //  RECT_TRIANG
                     0.98,    //  RECT_ROUND
                     0.96,    //  MOD_BASKET
                     0.96,    //  HORIZ_ELLIPSE
                     0.96,    //  VERT_ELLIPSE
                     0.92,    //  ARCH
                     0.96,    //  EGGSHAPED
                     0.96,    //  HORSESHOE
                     0.96,    //  GOTHIC
                     0.98,    //  CATENARY
                     0.98,    //  SEMIELLIPTICAL
                     0.96,    //  BASKETHANDLE
                     0.96,    //  SEMICIRCULAR
                     1.0,     //  IRREGULAR
                     0.96,    //  CUSTOM                                       //(5.0.010 - LR)
                     0.9756}; //  FORCE_MAIN                                   //(5.0.010 - LR)

//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
static double  Sstar;                // section factor 
static TXsect* Xstar;                // pointer to a cross section object        
static double  Qcritical;            // critical flow

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  xsect_isOpen
//  xsect_setParams
//  xsect_setIrregXsectParams
//  xsect_setCustomXsectParams                                                 //(5.0.010 - LR)
//  xsect_getAmax
//  xsect_getSofA
//  xsect_getYofA
//  xsect_getRofA
//  xsect_getAofS
//  xsect_getdSdA
//  xsect_getAofY
//  xsect_getRofY
//  xsect_getWofY
//  xsect_getYcrit

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static double generic_getAofS(TXsect* xsect, double s);
static void   evalSofA(double a, double* f, double* df);
static double tabular_getdSdA(TXsect* xsect, double a, double *table, int nItems);
static double generic_getdSdA(TXsect* xsect, double a);
static double lookup(double x, double *table, int nItems);
static double invLookup(double y, double *table, int nItems);
static int    locate(double y, double *table, int nItems);

static double rect_closed_getSofA(TXsect* xsect, double a);
static double rect_closed_getdSdA(TXsect* xsect, double a);
static double rect_closed_getRofA(TXsect* xsect, double a);

static double rect_open_getSofA(TXsect* xsect, double a);
static double rect_open_getdSdA(TXsect* xsect, double a);

static double rect_triang_getYofA(TXsect* xsect, double a);
static double rect_triang_getRofA(TXsect* xsect, double a);
static double rect_triang_getSofA(TXsect* xsect, double a);
static double rect_triang_getdSdA(TXsect* xsect, double a);
static double rect_triang_getAofY(TXsect* xsect, double y);
static double rect_triang_getRofY(TXsect* xsect, double y);
static double rect_triang_getWofY(TXsect* xsect, double y);

static double rect_round_getYofA(TXsect* xsect, double a);
static double rect_round_getRofA(TXsect* xsect, double a);
static double rect_round_getSofA(TXsect* xsect, double a);
static double rect_round_getdSdA(TXsect* xsect, double a);
static double rect_round_getAofY(TXsect* xsect, double y);
static double rect_round_getRofY(TXsect* xsect, double y);
static double rect_round_getWofY(TXsect* xsect, double y);

static double mod_basket_getYofA(TXsect* xsect, double a);
static double mod_basket_getRofA(TXsect* xsect, double a);
static double mod_basket_getdSdA(TXsect* xsect, double a);
static double mod_basket_getAofY(TXsect* xsect, double y);
static double mod_basket_getWofY(TXsect* xsect, double y);

static double trapez_getYofA(TXsect* xsect, double a);
static double trapez_getRofA(TXsect* xsect, double a);
static double trapez_getdSdA(TXsect* xsect, double a);
static double trapez_getAofY(TXsect* xsect, double y);
static double trapez_getRofY(TXsect* xsect, double y);
static double trapez_getWofY(TXsect* xsect, double y);

static double triang_getYofA(TXsect* xsect, double a);
static double triang_getRofA(TXsect* xsect, double a);
static double triang_getdSdA(TXsect* xsect, double a);
static double triang_getAofY(TXsect* xsect, double y);
static double triang_getRofY(TXsect* xsect, double y);
static double triang_getWofY(TXsect* xsect, double y);

static double parab_getYofA(TXsect* xsect, double a);
static double parab_getRofA(TXsect* xsect, double a);
static double parab_getPofY(TXsect* xsect, double y);
static double parab_getAofY(TXsect* xsect, double y);
static double parab_getRofY(TXsect* xsect, double y);
static double parab_getWofY(TXsect* xsect, double y);

static double powerfunc_getYofA(TXsect* xsect, double a);
static double powerfunc_getRofA(TXsect* xsect, double a);
static double powerfunc_getPofY(TXsect* xsect, double y);
static double powerfunc_getAofY(TXsect* xsect, double y);
static double powerfunc_getRofY(TXsect* xsect, double y);
static double powerfunc_getWofY(TXsect* xsect, double y);

static double circ_getYofA(TXsect* xsect, double a);
static double circ_getSofA(TXsect* xsect, double a);
static double circ_getdSdA(TXsect* xsect, double a);
static double circ_getAofS(TXsect* xsect, double s);
static double circ_getAofY(TXsect* xsect, double y);

static double filled_circ_getYofA(TXsect* xsect, double a);
static double filled_circ_getAofY(TXsect* xsect, double y);
static double filled_circ_getRofY(TXsect* xsect, double y);

static double getYcircular(double alpha);
static double getScircular(double alpha);
static double getAcircular(double psi);
static double getThetaOfAlpha(double alpha);
static double getThetaOfPsi(double psi);

static double getQcritical(double yc);
static double getYcritEnum(TXsect* xsect, double q, double y0);
static double getYcritRidder(TXsect* xsect, double q, double y0);

//=============================================================================

int xsect_isOpen(int type)
//
//  Input:   type = type of xsection shape
//  Output:  returns 1 if xsection is open, 0 if not
//  Purpose: determines if a xsection type is open or closed.
//
{
    return ((Amax[type] >= 1.0) ? 1 : 0);
}

//=============================================================================

int xsect_setParams(TXsect *xsect, int type, double p[], double ucf)
//
//  Input:   xsect = ptr. to a cross section data structure
//           type = xsection shape type
//           p[] = vector or xsection parameters
//           ucf = units correction factor
//  Output:  returns TRUE if successful, FALSE if not
//  Purpose: assigns parameters to a cross section's data structure.
//
{
    int    index;
    double aMax, theta;

    if ( type != DUMMY && p[0] <= 0.0 ) return FALSE;
    xsect->type  = type;
    switch ( xsect->type )
    {
    case DUMMY:
        xsect->yFull = TINY;
        xsect->wMax  = TINY;
        xsect->aFull = TINY;
        xsect->rFull = TINY;
        xsect->sFull = TINY;
        xsect->sMax  = TINY;
        break;

    case CIRCULAR:
        xsect->yFull = p[0]/ucf;
        xsect->wMax  = xsect->yFull;
        xsect->aFull = PI / 4.0 * xsect->yFull * xsect->yFull;
        xsect->rFull = 0.2500 * xsect->yFull;
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = 1.08 * xsect->sFull;
        break;

    case FORCE_MAIN:                                                           //(5.0.010 - LR)
        xsect->yFull = p[0]/ucf;                                               //(5.0.010 - LR)
        xsect->wMax  = xsect->yFull;                                           //(5.0.010 - LR)
        xsect->aFull = PI / 4.0 * xsect->yFull * xsect->yFull;                 //(5.0.010 - LR)
        xsect->rFull = 0.2500 * xsect->yFull;                                  //(5.0.010 - LR)
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 0.63);                 //(5.0.010 - LR)
        xsect->sMax  = 1.06949 * xsect->sFull;                                 //(5.0.010 - LR)

        // --- save C-factor or roughness in rBot position                     //(5.0.010 - LR)
        xsect->rBot  = p[1];                                                   //(5.0.010 - LR)
        break;                                                                 //(5.0.010 - LR)

    case FILLED_CIRCULAR:
        if ( p[1] >= p[0] ) return FALSE;

        // --- initially compute full values for unfilled pipe
        xsect->yFull = p[0]/ucf;
        xsect->wMax  = xsect->yFull;
        xsect->aFull = PI / 4.0 * xsect->yFull * xsect->yFull;
        xsect->rFull = 0.2500 * xsect->yFull;
        
        // --- find:
        //     yBot = depth of filled bottom
        //     aBot = area of filled bottom
        //     sBot = width of filled bottom
        //     rBot = wetted perimeter of filled bottom
        xsect->yBot  = p[1]/ucf;
        xsect->aBot  = circ_getAofY(xsect, xsect->yBot);
        xsect->sBot  = xsect_getWofY(xsect, xsect->yBot);
        xsect->rBot  = xsect->aBot / (xsect->rFull *
                       lookup(xsect->yBot/xsect->yFull, R_Circ, N_R_Circ));

        // --- revise full values for filled bottom
        xsect->aFull -= xsect->aBot;
        xsect->rFull = xsect->aFull /
                       (PI*xsect->yFull - xsect->rBot + xsect->sBot);
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = 1.08 * xsect->sFull;
        xsect->yFull -= xsect->yBot;
        break;

    case EGGSHAPED:
        xsect->yFull = p[0]/ucf;
        xsect->aFull = 0.5105 * xsect->yFull * xsect->yFull;
        xsect->rFull = 0.1931 * xsect->yFull;
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = 1.065 * xsect->sFull;
        xsect->wMax  = 2./3. * xsect->yFull;
        break;

    case HORSESHOE:
        xsect->yFull = p[0]/ucf;
        xsect->aFull = 0.8293 * xsect->yFull * xsect->yFull;
        xsect->rFull = 0.2538 * xsect->yFull;
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = 1.077 * xsect->sFull;
        xsect->wMax  = 1.0 * xsect->yFull;
        break;

    case GOTHIC:
        xsect->yFull = p[0]/ucf;
        xsect->aFull = 0.6554 * xsect->yFull * xsect->yFull;
        xsect->rFull = 0.2269 * xsect->yFull;
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = 1.065 * xsect->sFull;
        xsect->wMax  = 0.84 * xsect->yFull;
        break;

    case CATENARY:
        xsect->yFull = p[0]/ucf;
        xsect->aFull = 0.70277 * xsect->yFull * xsect->yFull;
        xsect->rFull = 0.23172 * xsect->yFull;
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = 1.05 * xsect->sFull;
        xsect->wMax  = 0.9 * xsect->yFull;
        break;

    case SEMIELLIPTICAL:
        xsect->yFull = p[0]/ucf;
        xsect->aFull = 0.785 * xsect->yFull * xsect->yFull;
        xsect->rFull = 0.242 * xsect->yFull;
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = 1.045 * xsect->sFull;
        xsect->wMax  = 1.0 * xsect->yFull;
        break;

    case BASKETHANDLE:
        xsect->yFull = p[0]/ucf;
        xsect->aFull = 0.7862 * xsect->yFull * xsect->yFull;
        xsect->rFull = 0.2464 * xsect->yFull;
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = 1.06078 * xsect->sFull;
        xsect->wMax  = 0.944 * xsect->yFull;
        break;

    case SEMICIRCULAR:
        xsect->yFull = p[0]/ucf;
        xsect->aFull = 1.2697 * xsect->yFull * xsect->yFull;
        xsect->rFull = 0.2946 * xsect->yFull;
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = 1.06637 * xsect->sFull;
        xsect->wMax  = 1.64 * xsect->yFull;
        break;

    case RECT_CLOSED:
        if ( p[1] <= 0.0 ) return FALSE;
        xsect->yFull = p[0]/ucf;
        xsect->wMax  = p[1]/ucf;
        xsect->aFull = xsect->yFull * xsect->wMax;
        xsect->rFull = xsect->aFull / (2.0 * (xsect->yFull + xsect->wMax));
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        aMax = RECT_ALFMAX * xsect->aFull;
        xsect->sMax = aMax * pow(rect_closed_getRofA(xsect, aMax), 2./3.);
        break;

    case RECT_OPEN:
        if ( p[1] <= 0.0 ) return FALSE;
        xsect->yFull = p[0]/ucf;
        xsect->wMax  = p[1]/ucf;
        xsect->aFull = xsect->yFull * xsect->wMax;
        xsect->rFull = xsect->aFull / (2.0 * xsect->yFull + xsect->wMax);
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = xsect->sFull;
        break;

    case RECT_TRIANG:
        if ( p[1] <= 0.0 || p[2] <= 0.0 ) return FALSE;
        xsect->yFull = p[0]/ucf;
        xsect->wMax  = p[1]/ucf;
        xsect->yBot  = p[2]/ucf;

        // --- area of bottom triangle
        xsect->aBot  = xsect->yBot * xsect->wMax / 2.0;

        // --- slope of bottom side wall
        xsect->sBot  = xsect->wMax / xsect->yBot / 2.0;

        // --- length of side wall per unit of depth
        xsect->rBot  = sqrt( 1. + xsect->sBot * xsect->sBot );   

        xsect->aFull = xsect->wMax * (xsect->yFull - xsect->yBot / 2.0);
        xsect->rFull = xsect->aFull / (2.0 * xsect->yBot * xsect->rBot + 2.0 *
                        (xsect->yFull - xsect->yBot) + xsect->wMax);
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        aMax = RECT_TRIANG_ALFMAX * xsect->aFull;
        xsect->sMax  = aMax * pow(rect_triang_getRofA(xsect, aMax), 2./3.);
        break;

    case RECT_ROUND:
        if ( p[1] <= 0.0 ) return FALSE;                                       //(5.0.014 - LR)
        if ( p[2] < p[1]/2.0 ) p[2] = p[1]/2.0;                                //(5.0.014 - LR)
        xsect->yFull = p[0]/ucf;
        xsect->wMax  = p[1]/ucf;
        xsect->rBot  = p[2]/ucf;

        // --- angle of circular arc
        theta = 2.0 * asin(xsect->wMax / 2.0 / xsect->rBot);

        // --- area of circular bottom
        xsect->aBot  = xsect->rBot * xsect->rBot /
                       2.0 * (theta - sin(theta)); 

        // --- section factor for circular bottom
        xsect->sBot  = PI * xsect->rBot * xsect->rBot *
                       pow(xsect->rBot/2.0, 2./3.);

        // --- depth of circular bottom
        xsect->yBot  = xsect->rBot * (1.0 - cos(theta/2.0));

        xsect->aFull = xsect->wMax * (xsect->yFull - xsect->yBot) + xsect->aBot;
        xsect->rFull = xsect->aFull / (xsect->rBot * theta + 2.0 *
                        (xsect->yFull - xsect->yBot) + xsect->wMax);
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        aMax = RECT_ROUND_ALFMAX * xsect->aFull;
        xsect->sMax = aMax * pow(rect_round_getRofA(xsect, aMax), 2./3.);
        break;

    case MOD_BASKET:

//// --- The code below was modified to accommodate a more                     //(5.0.014 - LR)
////     general type of modified baskethandle cross-section. 

        if ( p[1] <= 0.0 ) return FALSE;
        if ( p[2] < p[1]/2.0 ) p[2] = p[1]/2.0;
        xsect->yFull = p[0]/ucf;
        xsect->wMax  = p[1]/ucf;

        // --- radius of circular arc
        xsect->rBot = p[2]/ucf;

        // --- angle of circular arc
        theta = 2.0 * asin(xsect->wMax / 2.0 / xsect->rBot);
        xsect->sBot = theta;

        // --- height of circular arc
        xsect->yBot = xsect->rBot * (1.0 - cos(theta/2.0));

        // --- area of circular arc
        xsect->aBot = xsect->rBot * xsect->rBot /
                      2.0 * (theta - sin(theta)); 

        // --- full area
        xsect->aFull = (xsect->yFull - xsect->yBot) * xsect->wMax +
                       xsect->aBot;

        // --- full hydraulic radius & section factor
        xsect->rFull = xsect->aFull / (xsect->rBot * theta + 2.0 *
                        (xsect->yFull - xsect->yBot) + xsect->wMax);
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);

        // --- area corresponding to max. section factor
        xsect->sMax = xsect_getSofA(xsect, Amax[MOD_BASKET]*xsect->aFull);
        break;
       
    case TRAPEZOIDAL:
        if ( p[1] < 0.0 || p[2] < 0.0 || p[3] < 0.0 ) return FALSE;
        xsect->yFull = p[0]/ucf;

        // --- bottom width
        xsect->yBot = p[1]/ucf;

        // --- avg. slope of side walls
        xsect->sBot  = ( p[2] + p[3] )/2.0;

        // --- length of side walls per unit of depth
        xsect->rBot  = sqrt( 1.0 + p[2]*p[2] ) + sqrt( 1.0 + p[3]*p[3] );

        // --- top width
        xsect->wMax = xsect->yBot + xsect->yFull * (p[2] + p[3]);

        xsect->aFull = ( xsect->yBot + xsect->sBot * xsect->yFull ) * xsect->yFull;
        xsect->rFull = xsect->aFull / (xsect->yBot + xsect->yFull * xsect->rBot);
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = xsect->sFull;
        break;

    case TRIANGULAR:
        if ( p[1] <= 0.0 ) return FALSE;
        xsect->yFull = p[0]/ucf;
        xsect->wMax  = p[1]/ucf;

        // --- slope of side walls
        xsect->sBot  = xsect->wMax / xsect->yFull / 2.;

        // --- length of side wall per unit of depth
        xsect->rBot  = sqrt( 1. + xsect->sBot * xsect->sBot );  

        xsect->aFull = xsect->yFull * xsect->yFull * xsect->sBot;
        xsect->rFull = xsect->aFull / (2.0 * xsect->yFull * xsect->rBot);
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = xsect->sFull;
        break;

    case PARABOLIC:
        if ( p[1] <= 0.0 ) return FALSE;
        xsect->yFull = p[0]/ucf;
        xsect->wMax  = p[1]/ucf;

        // --- rBot :: 1/c^.5, where y = c*x^2 is eqn. of parabolic shape
        xsect->rBot  = xsect->wMax / 2.0 / sqrt(xsect->yFull);

        xsect->aFull = (2./3.) * xsect->yFull * xsect->wMax;
        xsect->rFull = xsect_getRofY(xsect, xsect->yFull);
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = xsect->sFull;
        break;

    case POWERFUNC:
        if ( p[1] <= 0.0 || p[2] <= 0.0 ) return FALSE;
        xsect->yFull = p[0]/ucf;
        xsect->wMax  = p[1]/ucf;
        xsect->sBot  = 1.0 / p[2];
        xsect->rBot  = xsect->wMax / (xsect->sBot + 1) /
                       pow(xsect->yFull, xsect->sBot);
        xsect->aFull = xsect->yFull * xsect->wMax / (xsect->sBot+1);
        xsect->rFull = xsect_getRofY(xsect, xsect->yFull);
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = xsect->sFull;
        break;

    case HORIZ_ELLIPSE:
        if ( p[1] == 0.0 )                       // std. ellipse pipe
        {
            index = (int)floor(p[0]) - 1;        // size code
            if ( index < 0 ||
                 index >= NumCodesEllipse ) return FALSE;
            xsect->yFull = MinorAxis_Ellipse[index]/12.;
            xsect->wMax  = MajorAxis_Ellipse[index]/12.;
            xsect->aFull = Afull_Ellipse[index];
            xsect->rFull = Rfull_Ellipse[index];
        }
        else
        {
            // --- length of minor axis
            xsect->yFull = p[0]/ucf;

            // --- length of major axis
            if ( p[1] < 0.0 ) return FALSE;
            xsect->wMax = p[1]/ucf;
            xsect->aFull = 1.2692 * xsect->yFull * xsect->yFull;
            xsect->rFull = 0.3061 * xsect->yFull;
        }
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = xsect->sFull;
        break;

    case VERT_ELLIPSE:
        if ( p[1] == 0.0 )                       // std. ellipse pipe
        {
            index = (int)floor(p[0]) - 1;        // size code
            if ( index < 0 ||
                 index >= NumCodesEllipse ) return FALSE;
            xsect->yFull = MajorAxis_Ellipse[index]/12.;
            xsect->wMax  = MinorAxis_Ellipse[index]/12.;
            xsect->aFull = Afull_Ellipse[index];
            xsect->rFull = Rfull_Ellipse[index];
        }
        else
        {
            // --- length of major axis
            if ( p[1] < 0.0 ) return FALSE;

            // --- length of minor axis
            xsect->yFull = p[0]/ucf;
            xsect->wMax = p[1]/ucf;
            xsect->aFull = 1.2692 * xsect->wMax * xsect->wMax;
            xsect->rFull = 0.3061 * xsect->wMax;
        }
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = xsect->sFull;
        break;

    case ARCH:
        if ( p[1] == 0.0 )                       // std. arch pipe
        {
            index = (int)floor(p[0]) - 1;        // size code
            if ( index < 0 ||
                 index >= NumCodesArch ) return FALSE;
            xsect->yFull = Yfull_Arch[index]/12.;     // Yfull units are inches
            xsect->wMax  = Wmax_Arch[index]/12.;      // Wmax units are inches
            xsect->aFull = Afull_Arch[index];
            xsect->rFull = Rfull_Arch[index];
        }
        else                                     // non-std. arch pipe
        {
            if ( p[1] < 0.0 ) return FALSE;
            xsect->yFull = p[0]/ucf;
            xsect->wMax  = p[1]/ucf;
            xsect->aFull = 0.7879 * xsect->yFull * xsect->wMax;
            xsect->rFull = 0.2991 * xsect->yFull;
        }
        xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
        xsect->sMax  = xsect->sFull;
        break;
    }
    return TRUE;
}

//=============================================================================

void xsect_setIrregXsectParams(TXsect *xsect)
//
//  Input:   xsect = ptr. to a cross section data structure
//  Output:  none
//  Purpose: assigns transect parameters to an irregular shaped cross section.
//
{
    int index = xsect->transect;
    xsect->yFull = Transect[index].yFull;
    xsect->wMax  = Transect[index].wMax;
    xsect->aFull = Transect[index].aFull;
    xsect->rFull = Transect[index].rFull;
    xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);
    xsect->sMax = Transect[index].sMax;
    xsect->aBot = Transect[index].aMax;
}
//=============================================================================

void xsect_setCustomXsectParams(TXsect *xsect)                                 //(5.0.010 - LR)
//                                                                             //(5.0.010 - LR)
//  Input:   xsect = ptr. to a cross section data structure                    //(5.0.010 - LR)
//  Output:  none                                                              //(5.0.010 - LR)
//  Purpose: assigns parameters to a custom-shaped cross section.              //(5.0.010 - LR)
//                                                                             //(5.0.010 - LR)
{                                                                              //(5.0.010 - LR)
    int    index = Curve[xsect->transect].refersTo;                            //(5.0.010 - LR)
    double yFull = xsect->yFull;                                               //(5.0.010 - LR)
    xsect->wMax  = Shape[index].wMax * yFull;                                  //(5.0.010 - LR)
    xsect->aFull = Shape[index].aFull * yFull * yFull;                         //(5.0.010 - LR)
    xsect->rFull = Shape[index].rFull * yFull;                                 //(5.0.010 - LR)
    xsect->sFull = xsect->aFull * pow(xsect->rFull, 2./3.);                    //(5.0.010 - LR)
    xsect->sMax  = Shape[index].sMax * yFull * yFull * pow(yFull, 2./3.);      //(5.0.010 - LR)
    xsect->aBot  = Shape[index].aMax * yFull * yFull;                          //(5.0.010 - LR)
}                                                                              //(5.0.010 - LR)

//=============================================================================

double xsect_getAmax(TXsect* xsect)
//
//  Input:   xsect = ptr. to a cross section data structure
//  Output:  returns area (ft2)
//  Purpose: finds xsection area at maximum flow depth.
//
{
/*
    if ( xsect->type == MOD_BASKET )
    {
        return xsect->yBot*xsect->wMax +
            PI / 4.0 * xsect->wMax * xsect->wMax * (0.96-0.5);
    }
    else
*/
    if ( xsect->type == IRREGULAR ) return xsect->aBot;
    else if ( xsect->type == CUSTOM ) return xsect->aBot;                      //(5.0.010 - LR)
    else return Amax[xsect->type] * xsect->aFull;
}

//=============================================================================

double xsect_getSofA(TXsect *xsect, double a)
//
//  Input:   xsect = ptr. to a cross section data structure
//           a = area (ft2)
//  Output:  returns section factor (ft^(8/3))
//  Purpose: computes xsection's section factor at a given area.
//
{
    double alpha = a / xsect->aFull;
    double r;
    switch ( xsect->type )
    {
      case FORCE_MAIN:                                                         //(5.0.01 - LR)
      case CIRCULAR:
        return circ_getSofA(xsect, a);

      case EGGSHAPED:
        return xsect->sFull * lookup(alpha, S_Egg, N_S_Egg);
  
      case HORSESHOE:
        return xsect->sFull * lookup(alpha, S_Horseshoe, N_S_Horseshoe);

      case GOTHIC:
        return xsect->sFull * lookup(alpha, S_Gothic, N_S_Gothic);

      case CATENARY:
        return xsect->sFull * lookup(alpha, S_Catenary, N_S_Catenary);

      case SEMIELLIPTICAL:
        return xsect->sFull * lookup(alpha, S_SemiEllip, N_S_SemiEllip);

      case BASKETHANDLE:
        return xsect->sFull * lookup(alpha, S_BasketHandle, N_S_BasketHandle);

      case SEMICIRCULAR:
        return xsect->sFull * lookup(alpha, S_SemiCirc, N_S_SemiCirc);

      case RECT_CLOSED:
        return rect_closed_getSofA(xsect, a);

      case RECT_OPEN:
        return rect_open_getSofA(xsect, a);

      case RECT_TRIANG:
        return rect_triang_getSofA(xsect, a);

      case RECT_ROUND:
        return rect_round_getSofA(xsect, a);

      default:
        if (a == 0.0) return 0.0;
        r = xsect_getRofA(xsect, a);
        if ( r < TINY ) return 0.0;
        return a * pow(r, 2./3.);
    }
}

//=============================================================================

double xsect_getYofA(TXsect *xsect, double a)
//
//  Input:   xsect = ptr. to a cross section data structure
//           a = area (ft2)
//  Output:  returns depth (ft)
//  Purpose: computes xsection's depth at a given area.
//
{
    double alpha = a / xsect->aFull;
    switch ( xsect->type )
    {
      case FORCE_MAIN:                                                         //(5.0.010 - LR)
      case CIRCULAR: return circ_getYofA(xsect, a);

      case FILLED_CIRCULAR:
        return filled_circ_getYofA(xsect, a);
  
      case EGGSHAPED:
        return xsect->yFull * lookup(alpha, Y_Egg, N_Y_Egg);
  
      case HORSESHOE:
        return xsect->yFull * lookup(alpha, Y_Horseshoe, N_Y_Horseshoe);

      case GOTHIC:
        return xsect->yFull * lookup(alpha, Y_Gothic, N_Y_Gothic);

      case CATENARY:
        return xsect->yFull * lookup(alpha, Y_Catenary, N_Y_Catenary);

      case SEMIELLIPTICAL:
        return xsect->yFull * lookup(alpha, Y_SemiEllip, N_Y_SemiEllip);

      case BASKETHANDLE:
        return xsect->yFull * lookup(alpha, Y_BasketHandle, N_Y_BasketHandle);

      case SEMICIRCULAR:
        return xsect->yFull * lookup(alpha, Y_SemiCirc, N_Y_SemiCirc);

      case HORIZ_ELLIPSE:
        return xsect->yFull * invLookup(alpha, A_HorizEllipse, N_A_HorizEllipse);

      case VERT_ELLIPSE:
        return xsect->yFull * invLookup(alpha, A_VertEllipse, N_A_VertEllipse);

      case IRREGULAR:
        return xsect->yFull * invLookup(alpha,
            Transect[xsect->transect].areaTbl, N_TRANSECT_TBL);

      case CUSTOM:                                                             //(5.0.010 - LR)
        return xsect->yFull * invLookup(alpha,                                 //(5.0.010 - LR)
            Shape[Curve[xsect->transect].refersTo].areaTbl, N_SHAPE_TBL);      //(5.0.010 - LR)

      case ARCH:
        return xsect->yFull * invLookup(alpha, A_Arch, N_A_Arch);

      case RECT_CLOSED: return a / xsect->wMax;

      case RECT_TRIANG: return rect_triang_getYofA(xsect, a);

      case RECT_ROUND:  return rect_round_getYofA(xsect, a);

      case RECT_OPEN:   return a / xsect->wMax;

      case MOD_BASKET:  return mod_basket_getYofA(xsect, a);

      case TRAPEZOIDAL: return trapez_getYofA(xsect, a);

      case TRIANGULAR:  return triang_getYofA(xsect, a);

      case PARABOLIC:   return parab_getYofA(xsect, a);

      case POWERFUNC:   return powerfunc_getYofA(xsect, a);

      default:          return 0.0;
    }
}

//=============================================================================

double xsect_getAofY(TXsect *xsect, double y)
//
//  Input:   xsect = ptr. to a cross section data structure
//           y = depth (ft)
//  Output:  returns area (ft2)
//  Purpose: computes xsection's area at a given depth.
//
{
    double yNorm = y / xsect->yFull;
    if ( y <= 0.0 ) return 0.0;
    switch ( xsect->type )
    {
      case FORCE_MAIN:                                                         //(5.0.010 - LR)
      case CIRCULAR:
        return xsect->aFull * lookup(yNorm, A_Circ, N_A_Circ);

      case FILLED_CIRCULAR:
        return filled_circ_getAofY(xsect, y);
  
      case EGGSHAPED:
        return xsect->aFull * lookup(yNorm, A_Egg, N_A_Egg);
  
      case HORSESHOE:
        return xsect->aFull * lookup(yNorm, A_Horseshoe, N_A_Horseshoe);

      case GOTHIC:
        return xsect->aFull * invLookup(yNorm, Y_Gothic, N_Y_Gothic);

      case CATENARY:
        return xsect->aFull * invLookup(yNorm, Y_Catenary, N_Y_Catenary);

      case SEMIELLIPTICAL:
        return xsect->aFull * invLookup(yNorm, Y_SemiEllip, N_Y_SemiEllip);

      case BASKETHANDLE:
        return xsect->aFull * lookup(yNorm, A_Baskethandle, N_A_Baskethandle);

      case SEMICIRCULAR:
        return xsect->aFull * invLookup(yNorm, Y_SemiCirc, N_Y_SemiCirc);

      case HORIZ_ELLIPSE:
        return xsect->aFull * lookup(yNorm, A_HorizEllipse, N_A_HorizEllipse);

      case VERT_ELLIPSE:
        return xsect->aFull * lookup(yNorm, A_VertEllipse, N_A_VertEllipse);

      case ARCH:
        return xsect->aFull * lookup(yNorm, A_Arch, N_A_Arch);
 
      case IRREGULAR:
        return xsect->aFull * lookup(yNorm,
            Transect[xsect->transect].areaTbl, N_TRANSECT_TBL);
 
      case CUSTOM:                                                             //(5.0.010 - LR)
        return xsect->aFull * lookup(yNorm,                                    //(5.0.010 - LR)
            Shape[Curve[xsect->transect].refersTo].areaTbl, N_SHAPE_TBL);      //(5.0.010 - LR)

     case RECT_CLOSED: return y * xsect->wMax;

      case RECT_TRIANG: return rect_triang_getAofY(xsect, y);

      case RECT_ROUND:  return rect_round_getAofY(xsect, y);

      case RECT_OPEN:   return y * xsect->wMax;

      case MOD_BASKET:  return mod_basket_getAofY(xsect, y);

      case TRAPEZOIDAL: return trapez_getAofY(xsect, y);

      case TRIANGULAR:  return triang_getAofY(xsect, y);

      case PARABOLIC:   return parab_getAofY(xsect, y);

      case POWERFUNC:   return powerfunc_getAofY(xsect, y);

      default:          return 0.0;
    }
}

//=============================================================================

double xsect_getWofY(TXsect *xsect, double y)
//
//  Input:   xsect = ptr. to a cross section data structure
//           y = depth ft)
//  Output:  returns top width (ft)
//  Purpose: computes xsection's top width at a given depth.
//
{
    double yNorm = y / xsect->yFull;
    switch ( xsect->type )
    {
      case FORCE_MAIN:                                                         //(5.0.010 - LR)
      case CIRCULAR:
        return xsect->wMax * lookup(yNorm, W_Circ, N_W_Circ);

      case FILLED_CIRCULAR:
        yNorm = (y + xsect->yBot) / (xsect->yFull + xsect->yBot);
        return xsect->wMax * lookup(yNorm, W_Circ, N_W_Circ);

      case EGGSHAPED:
        return xsect->wMax * lookup(yNorm, W_Egg, N_W_Egg);
  
      case HORSESHOE:
        return xsect->wMax * lookup(yNorm, W_Horseshoe, N_W_Horseshoe);

      case GOTHIC:
        return xsect->wMax * lookup(yNorm, W_Gothic, N_W_Gothic);

      case CATENARY:
        return xsect->wMax * lookup(yNorm, W_Catenary, N_W_Catenary);

      case SEMIELLIPTICAL:
        return xsect->wMax * lookup(yNorm, W_SemiEllip, N_W_SemiEllip);

      case BASKETHANDLE:
        return xsect->wMax * lookup(yNorm, W_BasketHandle, N_W_BasketHandle);

      case SEMICIRCULAR:
        return xsect->wMax * lookup(yNorm, W_SemiCirc, N_W_SemiCirc);

      case HORIZ_ELLIPSE:
        return xsect->wMax * lookup(yNorm, W_HorizEllipse, N_W_HorizEllipse);

      case VERT_ELLIPSE:
        return xsect->wMax * lookup(yNorm, W_VertEllipse, N_W_VertEllipse);

      case ARCH:
        return xsect->wMax * lookup(yNorm, W_Arch, N_W_Arch);

      case IRREGULAR:
        return xsect->wMax * lookup(yNorm,
            Transect[xsect->transect].widthTbl, N_TRANSECT_TBL);

      case CUSTOM:                                                             //(5.0.010 - LR)
        return xsect->wMax * lookup(yNorm,                                     //(5.0.010 - LR)
            Shape[Curve[xsect->transect].refersTo].widthTbl, N_SHAPE_TBL);     //(5.0.010 - LR) 

      case RECT_CLOSED: return xsect->wMax;

      case RECT_TRIANG: return rect_triang_getWofY(xsect, y);

      case RECT_ROUND:  return rect_round_getWofY(xsect, y);

      case RECT_OPEN:   return xsect->wMax;

      case MOD_BASKET:  return mod_basket_getWofY(xsect, y);

      case TRAPEZOIDAL: return trapez_getWofY(xsect, y);

      case TRIANGULAR:  return triang_getWofY(xsect, y);

      case PARABOLIC:   return parab_getWofY(xsect, y);

      case POWERFUNC:   return powerfunc_getWofY(xsect, y);

      default:          return 0.0;
    }
}

//=============================================================================

double xsect_getRofY(TXsect *xsect, double y)
//
//  Input:   xsect = ptr. to a cross section data structure
//           y = depth (ft)
//  Output:  returns hydraulic radius (ft)
//  Purpose: computes xsection's hydraulic radius at a given depth.
//
{
    double yNorm = y / xsect->yFull;
    switch ( xsect->type )
    {
      case FORCE_MAIN:                                                         //(5.0.010 - LR)
      case CIRCULAR:
        return xsect->rFull * lookup(yNorm, R_Circ, N_R_Circ);

      case FILLED_CIRCULAR:
        if ( xsect->yBot == 0.0 )                                              //(5.0.022 - LR)
            return xsect->rFull * lookup(yNorm, R_Circ, N_R_Circ);             //(5.0.022 - LR)
        return filled_circ_getRofY(xsect, y);

      case EGGSHAPED:
        return xsect->rFull * lookup(yNorm, R_Egg, N_R_Egg);

      case HORSESHOE:
        return xsect->rFull * lookup(yNorm, R_Horseshoe, N_R_Horseshoe);

      case BASKETHANDLE:
        return xsect->rFull * lookup(yNorm, R_Baskethandle, N_R_Baskethandle);

      case HORIZ_ELLIPSE:
        return xsect->rFull * lookup(yNorm, R_HorizEllipse, N_R_HorizEllipse);

      case VERT_ELLIPSE:
        return xsect->rFull * lookup(yNorm, R_VertEllipse, N_R_VertEllipse);

      case ARCH:
        return xsect->rFull * lookup(yNorm, R_Arch, N_R_Arch);

      case IRREGULAR:
        return xsect->rFull * lookup(yNorm,
            Transect[xsect->transect].hradTbl, N_TRANSECT_TBL);

      case CUSTOM:                                                             //(5.0.010 - LR)
        return xsect->rFull * lookup(yNorm,                                    //(5.0.010 - LR)
            Shape[Curve[xsect->transect].refersTo].hradTbl, N_SHAPE_TBL);      //(5.0.010 - LR)

      case RECT_TRIANG:  return rect_triang_getRofY(xsect, y);

      case RECT_ROUND:   return rect_round_getRofY(xsect, y);

      case TRAPEZOIDAL:  return trapez_getRofY(xsect, y);

      case TRIANGULAR:   return triang_getRofY(xsect, y);

      case PARABOLIC:    return parab_getRofY(xsect, y);

      case POWERFUNC:    return powerfunc_getRofY(xsect, y);

      default:           return xsect_getRofA( xsect, xsect_getAofY(xsect, y) );
    }
}

//=============================================================================

double xsect_getRofA(TXsect *xsect, double a)
//
//  Input:   xsect = ptr. to a cross section data structure
//           a = area (ft2)
//  Output:  returns hydraulic radius (ft)
//  Purpose: computes xsection's hydraulic radius at a given area.
//
{
    double cathy;
    if ( a <= 0.0 ) return 0.0;
    switch ( xsect->type )
    {
      case HORIZ_ELLIPSE:
      case VERT_ELLIPSE:
      case ARCH:
      case IRREGULAR:
      case FILLED_CIRCULAR:
      case CUSTOM:                                                             //(5.0.010 - LR)
        return xsect_getRofY( xsect, xsect_getYofA(xsect, a) );

      case RECT_CLOSED:  return rect_closed_getRofA(xsect, a);

      case RECT_OPEN:    return a / (xsect->wMax + 2. * a / xsect->wMax);

      case RECT_TRIANG:  return rect_triang_getRofA(xsect, a);

      case RECT_ROUND:   return rect_round_getRofA(xsect, a);

      case MOD_BASKET:   return mod_basket_getRofA(xsect, a);

      case TRAPEZOIDAL:  return trapez_getRofA(xsect, a);

      case TRIANGULAR:   return triang_getRofA(xsect, a);

      case PARABOLIC:    return parab_getRofA(xsect, a);

      case POWERFUNC:    return powerfunc_getRofA(xsect, a);

      default:
        cathy = xsect_getSofA(xsect, a);
        if ( cathy < TINY || a < TINY ) return 0.0;
        return pow(cathy/a, 3./2.);
    }
}

//=============================================================================

double xsect_getAofS(TXsect* xsect, double s)
//
//  Input:   xsect = ptr. to a cross section data structure
//           s = section factor (ft^(8/3))
//  Output:  returns area (ft2)
//  Purpose: computes xsection's area at a given section factor.
//
{
    double psi = s / xsect->sFull;
    if ( s <= 0.0 ) return 0.0;
    switch ( xsect->type )
    {
      case DUMMY:     return 0.0;
      
      case FORCE_MAIN:                                                         //(5.0.010 - LR)
      case CIRCULAR:  return circ_getAofS(xsect, s);
  
      case EGGSHAPED:
        return xsect->aFull * invLookup(psi, S_Egg, N_S_Egg);
  
      case HORSESHOE:
        return xsect->aFull * invLookup(psi, S_Horseshoe, N_S_Horseshoe);

      case GOTHIC:
        return xsect->aFull * invLookup(psi, S_Gothic, N_S_Gothic);

      case CATENARY:
        return xsect->aFull * invLookup(psi, S_Catenary, N_S_Catenary);

      case SEMIELLIPTICAL:
        return xsect->aFull * invLookup(psi, S_SemiEllip, N_S_SemiEllip);

      case BASKETHANDLE:
        return xsect->aFull * invLookup(psi, S_BasketHandle, N_S_BasketHandle);

      case SEMICIRCULAR:
        return xsect->aFull * invLookup(psi, S_SemiCirc, N_S_SemiCirc);

      default: return generic_getAofS(xsect, s);
    }
}

//=============================================================================

double xsect_getdSdA(TXsect* xsect, double a)
//
//  Input:   xsect = ptr. to a cross section data structure
//           a = area (ft2)
//  Output:  returns derivative of section factor w.r.t. area (ft^2/3)
//  Purpose: computes xsection's derivative of its section factor with 
//           respect to area at a given area.
//
{
    switch ( xsect->type )
    {
      case FORCE_MAIN:                                                         //(5.0.010 - LR)
      case CIRCULAR:
        return circ_getdSdA(xsect, a);

      case EGGSHAPED:
        return tabular_getdSdA(xsect, a, S_Egg, N_S_Egg);
  
      case HORSESHOE:
        return tabular_getdSdA(xsect, a, S_Horseshoe, N_S_Horseshoe);

      case GOTHIC:
        return tabular_getdSdA(xsect, a, S_Gothic, N_S_Gothic);

      case CATENARY:
        return tabular_getdSdA(xsect, a, S_Catenary, N_S_Catenary);

      case SEMIELLIPTICAL:
        return  tabular_getdSdA(xsect, a, S_SemiEllip, N_S_SemiEllip);

      case BASKETHANDLE:
        return  tabular_getdSdA(xsect, a, S_BasketHandle, N_S_BasketHandle);

      case SEMICIRCULAR:
        return  tabular_getdSdA(xsect, a, S_SemiCirc, N_S_SemiCirc);

      case RECT_CLOSED:
        return rect_closed_getdSdA(xsect, a);

      case RECT_OPEN:
        return rect_open_getdSdA(xsect, a);

	  case RECT_TRIANG:
		return rect_triang_getdSdA(xsect, a);

	  case RECT_ROUND:
		return rect_round_getdSdA(xsect, a);

	  case MOD_BASKET:
		return mod_basket_getdSdA(xsect, a);

	  case TRAPEZOIDAL:
		return trapez_getdSdA(xsect, a);

	  case TRIANGULAR:
		return triang_getdSdA(xsect, a);

      default: return generic_getdSdA(xsect, a);
    }
}

//=============================================================================

double xsect_getYcrit(TXsect* xsect, double q)
//
//  Input:   xsect = ptr. to a cross section data structure
//           q = flow rate (cfs)
//  Output:  returns critical depth (ft)
//  Purpose: computes critical depth at a specific flow rate.
//
{
    double q2g = SQR(q) / GRAVITY;
    double y, r;

    if ( q2g == 0.0 ) return 0.0;
    switch ( xsect->type )
    {
      case DUMMY:
        return 0.0;

      case RECT_OPEN:
      case RECT_CLOSED:
        // --- analytical expression for yCritical is
        //     y = (q2g / w^2)^(1/3) where w = width
        y = pow(q2g / SQR(xsect->wMax), 1./3.);
        break;

      case TRIANGULAR:
        // --- analytical expression for yCritical is
        //     y = (2 * q2g / s^2)^(1/5) where s = side slope
        y = pow(2.0 * q2g / SQR(xsect->sBot), 1./5.);
        break;

      case PARABOLIC:
        // --- analytical expression for yCritical is
        //     y = (27/32 * q2g * c)^(1/4) where y = c*x^2
        //     is eqn. for parabola and 1/sqrt(c) = rBot
        y = pow(27./32. * q2g / SQR(xsect->rBot), 1./4.);
        break;

      case POWERFUNC:
        y = 1. / (2.0 * xsect->sBot + 3.0);
        y = pow( q2g * (xsect->sBot + 1.0) / SQR(xsect->rBot), y);
        break;

      default:
        // --- first estimate yCritical for an equivalent circular conduit
        //     using 1.01 * (q2g / yFull)^(1/4)
        y = 1.01 * pow(q2g / xsect->yFull, 1./4.);
        if (y >= xsect->yFull) y = 0.97 * xsect->yFull;

        // --- then find ratio of conduit area to equiv. circular area
        r = xsect->aFull / (PI / 4.0 * SQR(xsect->yFull));

        // --- use interval enumeration method to find yCritical if 
        //     area ratio not too far from 1.0
        if ( r >= 0.5 && r <= 2.0 )
            y = getYcritEnum(xsect, q, y);

        // --- otherwise use Ridder's root finding method
        else y = getYcritRidder(xsect, q, y);
    }

    // --- do not allow yCritical to be > yFull
    return MIN(y, xsect->yFull);
}

//=============================================================================

double generic_getAofS(TXsect* xsect, double s)
//
//  Input:   xsect = ptr. to a cross section data structure
//           s = section factor (ft^8/3)
//  Output:  returns area (ft2)
//  Purpose: finds area given section factor by
//           solving S = A*(A/P(A))^(2/3) using Newton-Raphson iterations.
//
{
    double a, a1, a2, tol;

    if (s <= 0.0) return 0.0;

    // --- if S is between sMax and sFull then
    //     bracket A between aFull and aMax
    if ( (s <= xsect->sMax && s >= xsect->sFull)
    &&   xsect->sMax != xsect->sFull )
    {
        a1 = xsect->aFull;          // do this because sFull < sMax
        a2 = xsect_getAmax(xsect);
    }

    // --- otherwise bracket A between 0 and aMax
    else
    {
        a1 = 0.0;
        a2 = xsect_getAmax(xsect);
    }

    // --- save S & xsect in global variables for access by evalSofA function
    Xstar = xsect;
    Sstar = s;

    // --- compute starting guess for A
    a = 0.5 * (a1 + a2);

    // use the Newton-Raphson root finder function to find A
    tol = 0.0001 * xsect->aFull;
    findroot_Newton(a1, a2, &a, tol, evalSofA);
    return a;
}

//=============================================================================

void evalSofA(double a, double* f, double* df)
//
//  Input:   a = area
//  Output:  f = root finding function
//           df = derivative of root finding function
//  Purpose: function used in conjunction with getAofS() that evaluates 
//           f = S(a) - s and df = dS(a)/dA.
//
{
    double s = xsect_getSofA(Xstar, a);
    *f = s - Sstar;
    *df = xsect_getdSdA(Xstar, a);
}

//=============================================================================

double tabular_getdSdA(TXsect* xsect, double a, double *table, int nItems)
//
//  Input:   xsect = ptr. to cross section data structure
//           a = area (ft2)
//           table = ptr. to table of section factor v. normalized area
//           nItems = number of equally spaced items in table
//  Output:  returns derivative of section factor w.r.t. area (ft^2/3)
//  Purpose: computes derivative of section factor w.r.t area 
//           using geometry tables.
//
{
    int    i;
    double alpha = a / xsect->aFull;
    double delta = 1.0 / (nItems-1);
    double dSdA;

    // --- find which segment of table contains alpha
    i = (int)(alpha / delta);
    if ( i >= nItems - 1 ) i = nItems - 2;

    // --- compute slope from this interval of table
    dSdA = (table[i+1] - table[i]) / delta;

    // --- convert slope to un-normalized value
    return dSdA * xsect->sFull / xsect->aFull;
}

//=============================================================================

double generic_getdSdA(TXsect* xsect, double a)
//
//  Input:   xsect = ptr. to cross section data structure
//           a = area (ft2)
//  Output:  returns derivative of section factor w.r.t. area (ft^2/3)
//  Purpose: computes derivative of section factor w.r.t area 
//           using central difference approximation.
//
{
    double a1, a2;
    double alpha = a / xsect->aFull;
    double alpha1 = alpha - 0.001;
    double alpha2 = alpha + 0.001;
    if ( alpha1 < 0.0 ) alpha1 = 0.0;
    a1 = alpha1 * xsect->aFull;
    a2 = alpha2 * xsect->aFull;
    return (xsect_getSofA(xsect, a2) - xsect_getSofA(xsect, a1)) / (a2 - a1);
}

//=============================================================================

double lookup(double x, double *table, int nItems)
//
//  Input:   x = value of independent variable in a geometry table
//           table = ptr. to geometry table
//           nItems = number of equally spaced items in table
//  Output:  returns value of dependent table variable 
//  Purpose: looks up a value in a geometry table (i.e., finds y given x).
//
{
    double  delta, x0, x1, y, y2;
    int     i;

    // --- find which segment of table contains x
    delta = 1.0 / (nItems-1);
    i = (int)(x / delta);
    if ( i >= nItems - 1 ) return table[nItems-1];

    // --- compute x at start and end of segment
    x0 = i * delta;
    x1 = (i+1) * delta;

    // --- linearly interpolate a y-value
    y = table[i] + (x - x0) * (table[i+1] - table[i]) / delta;

    // --- use quadratic interpolation for low x value
    if ( i < 2 )
    {
        y2 = y + (x - x0) * (x - x1) / (delta*delta) *
             (table[i]/2.0 - table[i+1] + table[i+2]/2.0) ;
        if ( y2 > 0.0 ) y = y2;
    }
    if ( y < 0.0 ) y = 0.0;
    return y;
}

//=============================================================================

double invLookup(double y, double *table, int nItems)
//
//  Input:   y = value of dependent variable in a geometry table
//           table = ptr. to geometry table
//           nItems = number of equally spaced items in table
//  Output:  returns value of independent table variable 
//  Purpose: performs inverse lookup in a geometry table (i.e., finds
//           x given y).
//
{
    double delta, x, x0, x1;
    int    i;

    // --- locate table segment that contains y
    i = locate(y, table, nItems);
    if ( i >= nItems - 1 ) return 1.0;

    // --- compute x at start and end of segment
    delta = 1.0 / (nItems-1);
    x0 = i * delta;
    x1 = x0 + delta;

    // --- linearly interpolate an x value
    x = x0 + (y - table[i]) * (x1 - x0) / (table[i+1] - table[i]) ;
    if ( x < 0.0 ) x = 0.0;
    return x;
}

//=============================================================================

int locate(double y, double *table, int nItems)
//
//  Input:   y = value of dependent variable in a geometry table
//           table = ptr. to geometry table
//           nItems = number of equally spaced items in table
//  Output:  returns index j of table such that table[j-1] <= y <= table[j]
//  Purpose: uses bisection method to locate the lowest table index whose
//           value does not exceed a given value.
//
{
    int j = 1;
    int j1 = 0;
    int j2 = nItems;
    while ( j2 - j1 > 1)
    {
        j = (j1 + j2) >> 1;
        if ( y > table[j] ) j1 = j;
        else                j2 = j;
    }
    return j - 1;
}

//=============================================================================

double getQcritical(double yc)
//
//  Input:   yc = critical depth (ft)
//  Output:  returns flow difference value (cfs)
//  Purpose: finds difference between critical flow at depth yc and 
//           target value Qcritical
//
{
    double a = xsect_getAofY(Xstar, yc);
    double w = xsect_getWofY(Xstar, yc);
    double qc = -Qcritical;
    if ( w > 0.0 )  qc = a * sqrt(GRAVITY * a / w) - Qcritical;
    return qc;
}

//=============================================================================
 
double getYcritEnum(TXsect* xsect, double q, double y0)
//
//  Input:   xsect = ptr. to cross section data structure
//           q = critical flow rate (cfs)
//           y0 = estimate of critical depth (ft)
//  Output:  returns true critical depth (ft)
//  Purpose: solves a * sqrt(a(y)*g / w(y)) - q for y using interval
//           enumeration with starting guess of y0.
//
{
    double  q0, dy, qc, yc;
    int     i1, i;

    // --- store reference to cross section in global pointer
    Xstar = xsect;

    // --- divide cross section depth into 25 increments and
    //     locate increment corresponding to initial guess y0
    dy = xsect->yFull / 25.;
    i1 = (int)(y0 / dy);

    // --- evaluate critical flow at this increment
    Qcritical = 0.0;
    q0 = getQcritical(i1*dy);

    // --- initial flow lies below target flow 
    if ( q0 < q )
    {
        // --- search each successive higher depth increment
        yc = xsect->yFull;
        for ( i=i1+1; i<=25; i++)
        {
            // --- if critical flow at current depth is above target
            //     then use linear interpolation to compute critical depth
            qc = getQcritical(i*dy);
            if ( qc >= q )
            {
                yc = ( (q-q0) / (qc - q0) + (double)(i-1) ) * dy;
                break;
            }
            q0 = qc;
        }
    }

    // --- initial flow lies above target flow
    else
    {
        // --- search each successively lower depth increment
        yc = 0.0;
        for ( i=i1-1; i>=0; i--)
        {
            // --- if critical flow at current depth is below target
            //     then use linear interpolation to compute critical depth
            qc = getQcritical(i*dy);
            if ( qc < q )
            {
                yc = ( (q-qc) / (q0-qc) + (double)i ) * dy;
                break;
            }
            q0 = qc;
        }
    }
    return yc;
}

//=============================================================================

double getYcritRidder(TXsect* xsect, double q, double y0)
//
//  Input:   xsect = ptr. to cross section data structure
//           q = critical flow rate (cfs)
//           y0 = estimate of critical depth (ft)
//  Output:  returns true critical depth (ft)
//  Purpose: solves a * sqrt(a(y)*g / w(y)) - q for y using Ridder's
//           root finding method with starting guess of y0.
//
{
    double  y1 = 0.0;
    double  y2 = 0.99 * xsect->yFull;
    double  yc;
    double q0, q1, q2;

    // --- store reference to cross section in global pointer
    Xstar = xsect;

    // --- check if critical flow at (nearly) full depth < target flow
    Qcritical = 0.0;
    q2 = getQcritical(y2);
    if (q2 < q ) return xsect->yFull;

    // --- evaluate critical flow at initial depth guess y0
    //     and at 1/2 of full depth
    q0 = getQcritical(y0);
    q1 = getQcritical(0.5*xsect->yFull);

    // --- adjust search interval on depth so it contains flow q
    if ( q0 > q )
    {
        y2 = y0;
        if ( q1 < q ) y1 = 0.5*xsect->yFull;
    }
    else
    {
        y1 = y0;
        if ( q1 > q ) y2 = 0.5*xsect->yFull;
    }

    // --- save value of target critical flow in global variable
    Qcritical = q;

    // --- call Ridder root finding procedure with error tolerance
    //     of 0.001 ft. to find critical depth yc
    yc = findroot_Ridder(y1, y2, 0.001, getQcritical);
    return yc;
}


//=============================================================================
//  RECT_CLOSED fuctions
//=============================================================================

double rect_closed_getSofA(TXsect* xsect, double a)
{
    // --- if a > area corresponding to Smax then
    //     interpolate between sMax and Sfull
    double alfMax = RECT_ALFMAX;
    if ( a / xsect->aFull > alfMax )
    {
        return xsect->sMax + (xsect->sFull - xsect->sMax) *
               (a/xsect->aFull - alfMax) / (1.0 - alfMax);
    }

    // --- otherwise use regular formula
    return a * pow(xsect_getRofA(xsect, a), 2./3.);
}

double rect_closed_getdSdA(TXsect* xsect, double a)
{
    double alpha, alfMax, r;

    // --- if above level corresponding to sMax, then
    //     use slope between sFull & sMax
    alfMax = RECT_ALFMAX;
    alpha = a / xsect->aFull;
    if ( alpha > alfMax )
    {
        return (xsect->sFull - xsect->sMax) /
               ((1.0 - alfMax) * xsect->aFull);
    }

    // --- for small a/aFull use generic central difference formula
    if ( alpha <= 1.0e-30 ) return generic_getdSdA(xsect, a);

    // --- otherwise evaluate dSdA = [5/3 - (2/3)(dP/dA)R]R^(2/3)
    //     (where P = wetted perimeter & dPdA = 2/width)
    r = xsect_getRofA(xsect, a);
    return  (5./3. - (2./3.) * (2.0/xsect->wMax) * r) * pow(r, 2./3.);
}

double rect_closed_getRofA(TXsect* xsect, double a)
{
    double p;
    if ( a <= 0.0 )   return 0.0;
    p = xsect->wMax + 2.*a/xsect->wMax; // Wetted Perim = width + 2*area/width
    if ( a/xsect->aFull > RECT_ALFMAX )
    {
        p += (a/xsect->aFull - RECT_ALFMAX) / (1.0 - RECT_ALFMAX) * xsect->wMax;
    }

    return a / p;
}


//=============================================================================
//  RECT_OPEN fuctions
//=============================================================================

double rect_open_getSofA(TXsect* xsect, double a)
{
    double y = a / xsect->wMax;
    double r = a / (2.0*y + xsect->wMax);
    return a * pow(r, 2./3.);
}


double rect_open_getdSdA(TXsect* xsect, double a)
{
    double r, dPdA;

    // --- for small a/aFull use generic central difference formula
    if ( a / xsect->aFull <= 1.0e-30 ) return generic_getdSdA(xsect, a);

    // --- otherwise evaluate dSdA = [5/3 - (2/3)(dP/dA)R]R^(2/3)
    //     (where P = wetted perimeter)
    r = xsect_getRofA(xsect, a);
    dPdA = 2.0 / xsect->wMax;      // since P = geom2 + 2a/geom2
    return  (5./3. - (2./3.) * dPdA * r) * pow(r, 2./3.);
}


//=============================================================================
//  RECT_TRIANG fuctions
//=============================================================================

double rect_triang_getYofA(TXsect* xsect, double a)
{
    // below upper section
    if ( a <= xsect->aBot ) return sqrt(a / xsect->sBot);

    // above bottom section
    else return xsect->yBot + (a - xsect->aBot) / xsect->wMax;       
}

double rect_triang_getRofA(TXsect* xsect, double a)
{
    double y;
    double p, alf;

    if ( a <= 0.0 )   return 0.0;
    y = rect_triang_getYofA(xsect, a);
    
    // below upper section
    if ( y <= xsect->yBot ) return a / (2. * y * xsect->rBot); 

    // wetted perimeter without contribution of top surface
    p = 2. * xsect->yBot * xsect->rBot + 2. * (y - xsect->yBot);

    // top-surface contribution
    alf = (a / xsect->aFull) - RECT_TRIANG_ALFMAX;
    if ( alf > 0.0 ) p += alf / (1.0 - RECT_TRIANG_ALFMAX) * xsect->wMax;
    return a / p;
}

double rect_triang_getSofA(TXsect* xsect, double a)
{
    // --- if a > area corresponding to sMax, then
    //     interpolate between sMax and Sfull
    double alfMax = RECT_TRIANG_ALFMAX;
    if ( a / xsect->aFull > alfMax )
        return xsect->sMax + (xsect->sFull - xsect->sMax) *
               (a/xsect->aFull - alfMax) / (1.0 - alfMax);

    // --- otherwise use regular formula
    else return a * pow(rect_triang_getRofA(xsect, a), 2./3.);
}

double rect_triang_getdSdA(TXsect* xsect, double a)
{
    double alpha, alfMax, dPdA, r;

    // --- if a > area corresponding to sMax, then
    //     use slope between sFull & sMax
    alfMax = RECT_TRIANG_ALFMAX;
    alpha = a / xsect->aFull;
    if ( alpha > alfMax )
        return (xsect->sFull - xsect->sMax) / ((1.0 - alfMax) * xsect->aFull);

    // --- use generic central difference method for very small a
    if ( alpha <= 1.0e-30 ) return generic_getdSdA(xsect, a);

    // --- find deriv. of wetted perimeter
    if ( a > xsect->aBot ) dPdA = 2.0 / xsect->wMax;  // for upper rectangle
    else dPdA = xsect->rBot / sqrt(a * xsect->sBot);  // for triang. bottom

    // --- get hyd. radius & evaluate section factor derivative formula
    r = rect_triang_getRofA(xsect, a);
    return  (5./3. - (2./3.) * dPdA * r) * pow(r, 2./3.);
}

double rect_triang_getAofY(TXsect* xsect, double y)
{
    if ( y <= xsect->yBot ) return y * y * xsect->sBot;         // below upper section
    else return xsect->aBot + (y - xsect->yBot) * xsect->wMax;  // above bottom section
}

double rect_triang_getRofY(TXsect* xsect, double y)
{
    double y1, p;
    y1 = y - xsect->yBot;
    if ( y1 <= 0.0 )                             // below upper section
        return xsect->sBot / (2. * xsect->rBot);
    else                                         // above bottom section
    {
        p = (2. * xsect->yBot * xsect->rBot) + (2. * y1);
        if ( y >= xsect->yFull ) p += xsect->wMax;
        return (xsect->aBot + y1 * xsect->wMax) / p;
    }
}

double rect_triang_getWofY(TXsect* xsect, double y)
{
    if ( y <= xsect->yBot ) return 2.0 * xsect->sBot * y;  // below upper section
    else return xsect->wMax;                               // above bottom section
}


//=============================================================================
//  RECT_ROUND fuctions
//=============================================================================

//// --- The functions below were re-written to correctly account              //(5.0.014 - LR)
////     for the case where the bottom curvature is less than that
////     of a half circle.

double rect_round_getYofA(TXsect* xsect, double a)
{
    double alpha;

    // --- if above circular bottom:
    if ( a > xsect->aBot )
        return xsect->yBot + (a - xsect->aBot) / xsect->wMax;

    // --- otherwise use circular xsection method to find height
    alpha = a / (PI * xsect->rBot * xsect->rBot);
    if ( alpha < 0.04 ) return (2.0 * xsect->rBot) * getYcircular(alpha);      //5.0.014 - LR)
    return (2.0 * xsect->rBot) * lookup(alpha, Y_Circ, N_Y_Circ);
}

double rect_round_getRofA(TXsect* xsect, double a)
{
    double y1, theta1, p, arg;

    // --- if above circular invert ...
    if ( a <= 0.0 ) return 0.0;
    if ( a > xsect->aBot )
    {
        // wetted perimeter without contribution of top surface
        y1 = (a - xsect->aBot) / xsect->wMax;
        theta1 = 2.0 * asin(xsect->wMax/2.0/xsect->rBot);
        p = xsect->rBot*theta1 + 2.0*y1;

        // top-surface contribution
        arg = (a / xsect->aFull) - RECT_ROUND_ALFMAX;
        if ( arg > 0.0 ) p += arg / (1.0 - RECT_ROUND_ALFMAX) * xsect->wMax;
        return a / p;
    }

    // --- if within circular invert ...
    y1 = rect_round_getYofA(xsect, a);
    theta1 = 2.0*acos(1.0 - y1/xsect->rBot);
    p = xsect->rBot * theta1;
    return a / p;
}

double rect_round_getSofA(TXsect* xsect, double a)
{
    double alpha, aFull, sFull;

    // --- if a > area corresponding to sMax,
    //     interpolate between sMax and sFull
    double alfMax = RECT_ROUND_ALFMAX;
    if ( a / xsect->aFull > alfMax )
    {
        return xsect->sMax + (xsect->sFull - xsect->sMax) *
               (a / xsect->aFull - alfMax) / (1.0 - alfMax);
    }

    // --- if above circular invert, use generic function
    else if ( a > xsect->aBot )
    {
        return a * pow(xsect_getRofA(xsect, a), 2./3.);
    }

    // --- otherwise use circular xsection function applied
    //     to full circular shape of bottom section
    else
    {
        aFull = PI * xsect->rBot * xsect->rBot;
        alpha = a / aFull;
        sFull = xsect->sBot;

        // --- use special function for small a/aFull
        if ( alpha < 0.04 ) return sFull * getScircular(alpha);

        // --- otherwise use table
        else return sFull * lookup(alpha, S_Circ, N_S_Circ);
    }
}

double rect_round_getdSdA(TXsect* xsect, double a)
{
    double alfMax, r, dPdA;

    // --- if a > area corresponding to sMax, then
    //     use slope between sFull & sMax
    alfMax = RECT_ROUND_ALFMAX;
    if ( a / xsect->aFull > alfMax )
    {
        return (xsect->sFull - xsect->sMax) /
               ((1.0 - alfMax) * xsect->aFull);
    }

    // --- if above circular invert, use analytical function for dS/dA
    else if ( a > xsect->aBot )
    {
        r = rect_round_getRofA(xsect, a);
        dPdA = 2.0 / xsect->wMax;       // d(wet perim)/dA for rect.
        return  (5./3. - (2./3.) * dPdA * r) * pow(r, 2./3.);
    }

    // --- otherwise use generic finite difference function
    else return generic_getdSdA(xsect, a);
}

double rect_round_getAofY(TXsect* xsect, double y)
{
    double theta1;

    // --- if above circular invert...
    if ( y > xsect->yBot )
        return xsect->aBot + (y - xsect->yBot) * xsect->wMax;

    // --- find area of circular section
    theta1 = 2.0*acos(1.0 - y/xsect->rBot);
    return 0.5 * xsect->rBot * xsect->rBot * (theta1 - sin(theta1));
}

double rect_round_getRofY(TXsect* xsect, double y)
{
    double theta1;

    // --- if above top of circular bottom, use RofA formula
    if ( y <= 0.0 ) return 0.0;
    if ( y > xsect->yBot )
        return rect_round_getRofA( xsect, rect_round_getAofY(xsect, y) );

    // --- find hyd. radius of circular section
    theta1 = 2.0*acos(1.0 - y/xsect->rBot);
    return 0.5 * xsect->rBot * (1.0 - sin(theta1)) / theta1;
}

double rect_round_getWofY(TXsect* xsect, double y)
{
    // --- return width if depth above circular bottom section
    if ( y > xsect->yBot ) return xsect->wMax;

    // --- find width of circular section
    return 2.0 * sqrt( y * (2.0*xsect->rBot - y) ); 
}


//=============================================================================
//  MOD_BASKETHANDLE fuctions
//=============================================================================

//// --- The functions below were modified to accommodate a more               //(5.0.014 - LR)
////     general type of modified baskethandle cross section
////     that is the same as an upside down round rectangular shape.


// Note: the variables rBot, yBot, and aBot refer to properties of the
//       circular top portion of the cross-section (not the bottom)

double mod_basket_getYofA(TXsect* xsect, double a)
{
    double alpha, y1;

    // --- water level below top of rectangular bottom
    if ( a <= xsect->aFull - xsect->aBot ) return a / xsect->wMax;

    // --- find unfilled top area / area of full circular top
    alpha = (xsect->aFull - a) / (PI * xsect->rBot * xsect->rBot);

    // --- find unfilled height
    if ( alpha < 0.04 ) y1 = getYcircular(alpha);
    else                y1 = lookup(alpha, Y_Circ, N_Y_Circ);
    y1 = 2.0 * xsect->rBot * y1;

    // --- return difference between full height & unfilled height
    return xsect->yFull - y1;
}

double mod_basket_getRofA(TXsect* xsect, double a)
{
    double y1, p, theta1;

    // --- water level is below top of rectangular bottom;
    //     return hyd. radius of rectangle
    if ( a <= xsect->aFull - xsect->aBot )
        return a / (xsect->wMax + 2.0 * a / xsect->wMax);

    // --- find height of empty area
    y1 = xsect->yFull - mod_basket_getYofA(xsect, a);

    // --- find angle of circular arc corresponding to this height
    theta1 = 2.0 * acos(1.0 - y1 / xsect->rBot);

    // --- find perimeter of wetted portion of circular arc
    //     (angle of full circular opening was stored in sBot)
    p = (xsect->sBot - theta1) * xsect->rBot;

    // --- add on wetted perimeter of bottom rectangular area
    y1 = xsect->yFull - xsect->yBot;
    p =  p + 2.0*y1 + xsect->wMax;

    // --- return area / wetted perimeter
    return a / p;
}

double mod_basket_getdSdA(TXsect* xsect, double a)
{
    double r, dPdA;

    // --- if water level below top of rectangular bottom but not
    //     empty then use same code as for rectangular xsection
    if ( a <= xsect->aFull - xsect->aBot && a/xsect->aFull > 1.0e-30 )
    {
        r = a / (xsect->wMax + 2.0 * a / xsect->wMax);
        dPdA = 2.0 / xsect->wMax;
        return  (5./3. - (2./3.) * dPdA * r) * pow(r, 2./3.);
    }

    // --- otherwise use generic function
    else return generic_getdSdA(xsect, a);
}

double mod_basket_getAofY(TXsect* xsect, double y)
{
    double a1, theta1, y1;

    // --- if water level is below top of rectangular bottom
    //     return depth * width
    if ( y <= xsect->yFull - xsect->yBot ) return y * xsect->wMax;

    // --- find empty top circular area
    y1 = xsect->yFull - y;
    theta1 = 2.0*acos(1.0 - y1/xsect->rBot);
    a1 = 0.5 * xsect->rBot * xsect->rBot * (theta1 - sin(theta1));

    // --- return difference between full and empty areas
    return xsect->aFull - a1;
}

double mod_basket_getWofY(TXsect* xsect, double y)
{
    double y1;

    // --- if water level below top of rectangular bottom then return width
    if ( y <= 0.0 ) return 0.0;
    if ( y <= xsect->yFull - xsect->yBot ) return xsect->wMax;

    // --- find width of empty top circular section
    y1 = xsect->yFull - y;
    return 2.0 * sqrt( y1 * (2.0*xsect->rBot - y1) ); 
}


//=============================================================================
//  TRAPEZOIDAL fuctions
//
//  Note: yBot = width of bottom
//        sBot = avg. of side slopes
//        rBot = length of sides per unit of depth
//=============================================================================

double trapez_getYofA(TXsect* xsect, double a)
{
    if ( xsect->sBot == 0.0 ) return a / xsect->yBot;                          //(5.0.012 - LR)
    return ( sqrt( xsect->yBot*xsect->yBot + 4.*xsect->sBot*a )
             - xsect->yBot )/(2. * xsect->sBot);
}

double trapez_getRofA(TXsect* xsect, double a)
{
    return a / (xsect->yBot + trapez_getYofA(xsect, a) * xsect->rBot);
}

double trapez_getdSdA(TXsect* xsect, double a)
{
    double r, dPdA;
    // --- use generic central difference method for very small a
    if ( a/xsect->aFull <= 1.0e-30 ) return generic_getdSdA(xsect, a);

    // --- otherwise use analytical formula:
    //     dSdA = [5/3 - (2/3)(dP/dA)R]R^(2/3)
    r = trapez_getRofA(xsect, a);
    dPdA = xsect->rBot /
           sqrt( xsect->yBot * xsect->yBot + 4. * xsect->sBot * a );
    return  (5./3. - (2./3.) * dPdA * r) * pow(r, 2./3.);
}

double trapez_getAofY(TXsect* xsect, double y)
{
    return ( xsect->yBot + xsect->sBot * y ) * y;
}

double trapez_getRofY(TXsect* xsect, double y)
{
    if ( y == 0.0 ) return 0.0;                                                //(5.0.022 - LR)
    return trapez_getAofY(xsect, y) / (xsect->yBot + y * xsect->rBot);
}

double trapez_getWofY(TXsect* xsect, double y)
{
    return xsect->yBot + 2.0 * y * xsect->sBot;
}


//=============================================================================
//  TRIANGULAR fuctions
//=============================================================================

double triang_getYofA(TXsect* xsect, double a)
{
    return sqrt(a / xsect->sBot);
}

double triang_getRofA(TXsect* xsect, double a)
{
    return a / (2. * triang_getYofA(xsect, a) * xsect->rBot);
}

double triang_getdSdA(TXsect* xsect, double a)
{
    double r, dPdA;
    // --- use generic finite difference method for very small a
    if ( a/xsect->aFull <= 1.0e-30 ) return generic_getdSdA(xsect, a);

    // --- evaluate dSdA = [5/3 - (2/3)(dP/dA)R]R^(2/3)
    r = triang_getRofA(xsect, a);
    dPdA = xsect->rBot / sqrt(a * xsect->sBot);
    return  (5./3. - (2./3.) * dPdA * r) * pow(r, 2./3.);
}

double triang_getAofY(TXsect* xsect, double y)
{
    return y * y * xsect->sBot;
}

double triang_getRofY(TXsect* xsect, double y)
{
    return (y * xsect->sBot) / (2. * xsect->rBot);
}

double triang_getWofY(TXsect* xsect, double y)
{
    return 2.0 * xsect->sBot * y;
}


//=============================================================================
//  PARABOLIC fuctions
//=============================================================================

double parab_getYofA(TXsect* xsect, double a)
{
    return pow( (3./4.) * a / xsect->rBot, 2./3. );
}

double parab_getRofA(TXsect* xsect, double a)
{
    if ( a <= 0.0 ) return 0.0;
    return a / parab_getPofY( xsect, parab_getYofA(xsect, a) );
}

double parab_getPofY(TXsect* xsect, double y)
{
    double x = 2. * sqrt(y) / xsect->rBot;
    double t = sqrt(1.0 + x * x);
    return 0.5 * xsect->rBot * xsect->rBot * ( x * t + log(x + t) );
}

double parab_getAofY(TXsect* xsect, double y)
{
    return (4./3. * xsect->rBot * y * sqrt(y));
}

double parab_getRofY(TXsect* xsect, double y)
{
    if ( y <= 0.0 ) return 0.0;
    return parab_getAofY(xsect, y) / parab_getPofY(xsect, y);
}

double parab_getWofY(TXsect* xsect, double y)
{
    return 2.0 * xsect->rBot * sqrt(y);
}


//=============================================================================
//  POWERFUNC fuctions
//=============================================================================

double powerfunc_getYofA(TXsect* xsect, double a)
{
    return pow(a / xsect->rBot, 1.0 / (xsect->sBot + 1.0));
}

double powerfunc_getRofA(TXsect* xsect, double a)
{
    if ( a <= 0.0 ) return 0.0;
    return a / powerfunc_getPofY(xsect, powerfunc_getYofA(xsect, a));
}

double powerfunc_getPofY(TXsect* xsect, double y)
{
    double dy1 = 0.02 * xsect->yFull;
    double h = (xsect->sBot + 1.0) * xsect->rBot / 2.0;
    double m = xsect->sBot;
    double p = 0.0;
    double y1 = 0.0;
    double x1 = 0.0;
    double x2, y2, dx, dy;
    do
    {
        y2 = y1 + dy1;
        if ( y2 > y ) y2 = y;
        x2 = h * pow(y2, m);
        dx = x2 - x1;
        dy = y2 - y1;
        p += sqrt(dx*dx + dy*dy);
        x1 = x2;
        y1 = y2;
    } while ( y2 < y );
    return 2.0 * p;
}

double powerfunc_getAofY(TXsect* xsect, double y)
{
    return xsect->rBot * pow(y, xsect->sBot + 1.0);
}

double powerfunc_getRofY(TXsect* xsect, double y)
{
    if ( y <= 0.0 ) return 0.0;
    return powerfunc_getAofY(xsect, y) / powerfunc_getPofY(xsect, y);
}

double powerfunc_getWofY(TXsect* xsect, double y)
{
    return (xsect->sBot + 1.0) * xsect->rBot * pow(y, xsect->sBot);
}


//=============================================================================
//  CIRCULAR functions 
//=============================================================================

double circ_getYofA(TXsect* xsect, double a)
{
    double alpha = a / xsect->aFull;

    // --- use special function for small a/aFull
    if ( alpha < 0.04 )  return xsect->yFull * getYcircular(alpha);

    // --- otherwise use table 
    else return xsect->yFull * lookup(alpha, Y_Circ, N_Y_Circ);
}

double circ_getAofS(TXsect* xsect, double s)
{
    double psi = s / xsect->sFull;
    if (psi == 0.0) return 0.0;
    if (psi >= 1.0) return xsect->aFull;

    // --- use special function for small s/sFull
    if (psi <= 0.015) return xsect->aFull * getAcircular(psi);

    // --- otherwise use table
    else return xsect->aFull * invLookup(psi, S_Circ, N_S_Circ);
}

double circ_getSofA(TXsect* xsect, double a)
{
    double alpha = a / xsect->aFull;

    // --- use special function for small a/aFull
    if ( alpha < 0.04 ) return xsect->sFull * getScircular(alpha);

    // --- otherwise use table
    else
    return xsect->sFull * lookup(alpha, S_Circ, N_S_Circ);
}

double circ_getdSdA(TXsect* xsect, double a)
{
    double alpha, theta, p, r, dPdA;

    // --- for near-zero area, use generic central difference formula
    alpha = a / xsect->aFull;
    if ( alpha <= 1.0e-30 ) return 1.0e-30;  //generic_getdSdA(xsect, a);

	// --- for small a/aFull use analytical derivative
    else if ( alpha < 0.04 )
    {
        theta = getThetaOfAlpha(alpha);
        p = theta * xsect->yFull / 2.0;
        r = a / p;
        dPdA = 4.0 / xsect->yFull / (1. - cos(theta));
        return  (5./3. - (2./3.) * dPdA * r) * pow(r, 2./3.);
    }

    // --- otherwise use generic tabular getdSdA
    else return tabular_getdSdA(xsect, a, S_Circ, N_S_Circ);
}

////////////////////////////////////////////////
// This is an alternate method used in SWMM 4.4.
////////////////////////////////////////////////
/*
double circ_getdSdA(TXsect* xsect, double a)
{
    double alpha, a1, a2, da, s1, s2, ds;
    alpha = a / xsect->aFull;
    if ( alpha <= 1.0e-30 ) return 1.0e-30;
    da = 0.002;
    a1 = alpha - 0.001;
    a2 = alpha + 0.001;
    if ( a1 < 0.0 )
    {
	    a1 = 0.0;
    	da = alpha + 0.001;
    }
    s1 = getScircular(a1);
    s2 = getScircular(a2);
    ds = (s2 - s1) / da;
    if ( ds <= 1.0e-30 ) ds = 1.0e-30;
    return xsect->sFull * ds / xsect->aFull;
}
*/

double circ_getAofY(TXsect* xsect, double y)
{
    double yNorm;
    yNorm = y / xsect->yFull;
    return xsect->aFull * lookup(yNorm, A_Circ, N_A_Circ);
}


//=============================================================================
//  FILLED_CIRCULAR functions 
//=============================================================================

double filled_circ_getYofA(TXsect* xsect, double a)
{
    double y;

    // --- temporarily remove filled portion of circle
    xsect->yFull += xsect->yBot;
    xsect->aFull += xsect->aBot;
    a += xsect->aBot;

    // --- find depth in unfilled circle
    y = circ_getYofA(xsect, a);

    // --- restore original values
    y -= xsect->yBot;
    xsect->yFull -= xsect->yBot;
    xsect->aFull -= xsect->aBot;
    return y;
}

double filled_circ_getAofY(TXsect* xsect, double y)
{
    double a;

    // --- temporarily remove filled portion of circle
    xsect->yFull += xsect->yBot;
    xsect->aFull += xsect->aBot;
    y += xsect->yBot;

    // --- find area of unfilled circle
    a = circ_getAofY(xsect, y);

    // --- restore original values
    a -= xsect->aBot;
    xsect->yFull -= xsect->yBot;
    xsect->aFull -= xsect->aBot;
    return a;
}

double filled_circ_getRofY(TXsect* xsect, double y)
{
    double a, r, p;

    // --- temporarily remove filled portion of circle
    xsect->yFull += xsect->yBot;
    xsect->aFull += xsect->aBot;
    y += xsect->yBot;

    // --- get area,  hyd. radius & wetted perimeter of unfilled circle
    a = circ_getAofY(xsect, y);
    r = 0.25 * xsect->yFull * lookup(y/xsect->yFull, R_Circ, N_R_Circ);
    p = (a/r);

    // --- reduce area and wetted perimeter by amount of filled circle
    //     (rBot = filled perimeter, sBot = filled width)
    a = a - xsect->aBot;
    p = p - xsect->rBot + xsect->sBot;

    // --- compute actual hyd. radius & restore xsect parameters
    r = a / p;
    xsect->yFull -= xsect->yBot;
    xsect->aFull -= xsect->aBot;
    return r;
}


//=============================================================================
//  Special functions for circular cross sections
//=============================================================================

double getYcircular(double alpha)
{
    double theta;
    if ( alpha >= 1.0 ) return 1.0;
    if ( alpha <= 0.0 ) return 0.0;
    if ( alpha <= 1.0e-5 )
    {
        theta = pow(37.6911*alpha, 1./3.);
        return theta * theta / 16.0;
    }
    theta = getThetaOfAlpha(alpha);
    return (1.0 - cos(theta/2.)) / 2.0;
}

double getScircular(double alpha)
{
    double theta;
    if ( alpha >= 1.0 ) return 1.0;
    if ( alpha <= 0.0 ) return 0.0;
    if ( alpha <= 1.0e-5 )
    {
        theta = pow(37.6911*alpha, 1./3.);
        return pow(theta, 13./3.) / 124.4797;
    }
    theta = getThetaOfAlpha(alpha);
    return pow((theta - sin(theta)), 5./3.) / (2.0 * PI) / pow(theta, 2./3.);
}

double getAcircular(double psi)
{
    double theta;
    if ( psi >= 1.0 ) return 1.0;
    if ( psi <= 0.0 ) return 0.0;
    if ( psi <= 1.0e-6 )
    {
        theta = pow(124.4797*psi, 3./13.);
        return theta*theta*theta / 37.6911;
    }
    theta = getThetaOfPsi(psi);
    return (theta - sin(theta)) / (2.0 * PI);
}

double getThetaOfAlpha(double alpha)
{
    int    k;
    double theta, theta1, ap, d;

    if ( alpha > 0.04 ) theta = 1.2 + 5.08 * (alpha - 0.04) / 0.96;
    else theta = 0.031715 - 12.79384 * alpha + 8.28479 * sqrt(alpha);
    theta1 = theta;
    ap  = (2.0*PI) * alpha;
    for (k = 1; k <= 40; k++ )
    {
        d = - (ap - theta + sin(theta)) / (1.0 - cos(theta));
        // --- modification to improve convergence for large theta
        if ( d > 1.0 ) d = SIGN( 1.0, d );
        theta = theta - d;
        if ( fabs(d) <= 0.0001 ) return theta;
    }
    return theta1;
}

double getThetaOfPsi(double psi)
{
    int    k;
    double theta, theta1, ap, tt, tt23, t3, d;

    if      (psi > 0.90)  theta = 4.17 + 1.12 * (psi - 0.90) / 0.176;
    else if (psi > 0.5)   theta = 3.14 + 1.03 * (psi - 0.5) / 0.4;
    else if (psi > 0.015) theta = 1.2 + 1.94 * (psi - 0.015) / 0.485;
    else                  theta = 0.12103 - 55.5075 * psi +
                                  15.62254 * sqrt(psi);
    theta1 = theta;
    ap     = (2.0*PI) * psi;

    for (k = 1; k <= 40; k++)
    {
        theta    = fabs(theta);
        tt       = theta - sin(theta);
        tt23     = pow(tt, 2./3.);
        t3       = pow(theta, 1./3.);
        d        = ap * theta / t3 - tt * tt23;
        d        = d / ( ap*(2./3.)/t3 - (5./3.)*tt23*(1.0-cos(theta)) );
        theta    = theta - d;
        if ( fabs(d) <= 0.0001 ) return theta;
    }
    return theta1;
}

//=============================================================================
