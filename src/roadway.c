//-----------------------------------------------------------------------------
//   roadway.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     08/05/15   (Build 5.1.010)
//             03/14/17   (Build 5.1.012)
//   Author:   L. Rossman
//
//   Roadway Weir module for SWMM5
//
//   Computes flow overtopping a roadway (with a ROADWAY_WEIR object) using
//   the FWHA HDS-5 methodology.
//   Typically used in conjuction with a culvert crossing where the culvert
//   conduit is placed at zero offset at the upstream node and the Roadway
//   weir has the same upstream node but with an offset equal to the height
//   of the roadway.
//
//   Build 5.1.012:
//   - Entries in discharge coeff. table for gravel roadways corrected.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include "headers.h"

enum RoadSurface {PAVED = 1, GRAVEL = 2};

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------

// The discharge coefficients and submergence factors listed below were
// derived from Figure 10 in "Bridge Waterways Analysis Model: Research
// Report", U.S. Dept. of Transportation Federal Highway Administration
// Report No. FHWA/RD-86/108, McLean, VA, July 1986.

// Discharge Coefficients for (head / road width) <= 0.15
static const int N_Cr_Low_Paved = 4;
static const double Cr_Low_Paved[4][2] = {
    {0.0, 2.85}, {0.2, 2.95}, {0.7, 3.03}, {4.0, 3.05}};
    
static const int N_Cr_Low_Gravel = 8;
static const double Cr_Low_Gravel[8][2] = {
    {0.0, 2.5}, {0.5, 2.7},  {1.0, 2.8}, {1.5, 2.9}, {2.0, 2.98},
    {2.5, 3.02}, {3.0, 3.03}, {4.0, 3.05} };

// Discharge Coefficients for (head / road width) > 0.15
static const int N_Cr_High_Paved = 2;
static const double Cr_High_Paved[2][2] = {{0.15,3.05}, {0.25,3.10}};

static const int N_Cr_High_Gravel = 2;
static const double Cr_High_Gravel[2][2] = {{0.15,2.95}, {0.30,3.10}};

// Submergence Factors
static const int N_Kt_Paved = 9;
static const double Kt_Paved[9][2] = {
    {0.8, 1.0}, {0.85, 0.98}, {0.90, 0.92}, {0.93, 0.85}, {0.95, 0.80},
    {0.97, 0.70}, {0.98, 0.60}, {0.99, 0.50}, {1.00, 0.40}};

static const int N_Kt_Gravel = 12;
static const double Kt_Gravel[12][2] = {
    {0.75, 1.00}, {0.80, 0.985}, {0.83, 0.97}, {0.86, 0.93}, {0.89, 0.90},
    {0.90, 0.87}, {0.92, 0.80},  {0.94, 0.70}, {0.96, 0.60}, {0.98, 0.50},
    {0.99, 0.40}, {1.00, 0.24}};

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  double roadway_getInflow   (called by weir_getInflow in link.c)

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static double getCd(double hWr, double ht, double roadWidth, int roadSurf);
static double getY(double x, const double table[][2], const int n);

//=============================================================================

double roadway_getInflow(int j,          // link index
                         double dir,     // flow direction (+1 or -1)
                         double hRoad,   // road elev. (ft)
                         double h1,      // upstream head (ft)
                         double h2)      // downstream head (ft)
{
    int    k;                // weir array index
    int    roadSurf;         // type of road surface
    int    useVariableCd;    // true if HDS-5 coeff. curves used
    double length,           // length of roadway segment transverse to flow (ft)
           roadWidth,        // width of roadway which receives flow (ft)
           hWr,              // water elevation on upstream side of roadway (ft)
           ht,               // water elevation on downstream side of roadway (ft)
           cD,               // discharge coefficient for cfs flow units
           q = 0.0,          // flow across roadway (cfs)
           dqdh = 0.0;       // derivative of flow w.r.t. head (ft2/sec)

    // --- get road width & surface type
    if ( Link[j].type != WEIR ) return 0.0;
    k = Link[j].subIndex;
    roadWidth = Weir[k].roadWidth;
    roadSurf = Weir[k].roadSurface;

    // --- user-supplied discharge coeff.
    cD = Weir[k].cDisch1;
    if ( UnitSystem == SI ) cD = cD / 0.552;

    // --- check if there's enough info to use a variable cD value
    useVariableCd = FALSE;
    if ( roadWidth > 0.0 && roadSurf >= 1 ) useVariableCd = TRUE;

    // --- upstream and downstream heads
    hWr = h1 - hRoad;
    ht = h2 - hRoad;
    if ( hWr > FUDGE )
    {
        // --- get discharge coeff. as function of heads
        if ( useVariableCd ) cD = getCd(hWr, ht, roadWidth, roadSurf);

        // --- use user-supplied weir length
        length = Link[j].xsect.wMax;

        // --- weir eqn. for discharge across roadway
        q = cD * length * pow(hWr, 1.5);
        dqdh = 1.5 * q / hWr;
    }

    // --- assign output values
    Link[j].dqdh = dqdh;
    Link[j].newDepth = MAX(h1 - hRoad, 0.0);
    Link[j].flowClass = SUBCRITICAL;
    if ( hRoad > h2 )
    {
        if ( dir == 1.0 ) Link[j].flowClass = DN_CRITICAL;
        else              Link[j].flowClass = UP_CRITICAL;
    }
    return dir * q;
}

//=============================================================================

double getCd(double hWr, double ht, double roadWidth, int roadSurf)
{
    double kT = 1.0;              // submergence factor
    double hL,                    // ratio of water elevation to road width
           htH,                   // ratio of downstream to upstream water depth
           cR;                    // roadway discharge coeff.

    if ( hWr <= 0.0 ) return 0.0;
    hL = hWr / roadWidth;
    if ( hL <= 0.15 )
    {
        if ( roadSurf == PAVED ) cR = getY(hWr, Cr_Low_Paved, N_Cr_Low_Paved);
        else                     cR = getY(hWr, Cr_Low_Gravel, N_Cr_Low_Gravel);
    }
    else
    {
        if ( roadSurf == PAVED ) cR = getY(hL, Cr_High_Paved, N_Cr_High_Paved);
        else                     cR = getY(hL, Cr_High_Gravel, N_Cr_High_Gravel);
    }
    if ( ht > 0.0 )
    {
        htH = ht / hWr;
        if ( roadSurf == PAVED ) kT = getY(htH, Kt_Paved, N_Kt_Paved);
        else                     kT = getY(htH, Kt_Gravel, N_Kt_Gravel);
    }
    return cR * kT;
}

//=============================================================================

double getY(double x, const double table[][2], const int n)
{
    int i;
    double x1, y1,dx, dy;
    if ( x <= table[0][0] ) return table[0][1];
    if ( x >= table[n-1][0] ) return table[n-1][1];
    for (i = 1; i < n; i++)
    {
        if ( x <= table[i][0] )
        {
            x1 = table[i-1][0];
            dx = table[i][0] - x1;
            y1 = table[i-1][1];
            dy = table[i][1] - y1;
            return y1 + (x - x1) * dy / dx;
        }
    }
    return table[n-1][1];
}

