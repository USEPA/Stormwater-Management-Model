//-----------------------------------------------------------------------------
//   link.c
//
//   Project:  EPA SWMM5
//   Version:  5.0
//   Date:     6/19/07   (Build 5.0.010)
//             7/16/07   (Build 5.0.011)
//             2/4/08    (Build 5.0.012)
//             3/11/08   (Build 5.0.013)
//             1/21/09   (Build 5.0.014)
//             4/10/09   (Build 5.0.015)
//             6/22/09   (Build 5.0.016)
//             10/7/09   (Build 5.0.017)
//             11/18/09  (Build 5.0.018)
//             07/30/10  (Build 5.0.019)
//             04/20/11  (Build 5.0.022)
//   Author:   L. Rossman
//
//   Conveyance system link functions
//
//   Note: The Link offset properties z1 and z2 have been renamed to
//         offset1 and offset2 for build 5.0.012.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
static const double MIN_DELTA_Z = 0.001; // minimum elevation change for conduit
                                         // slopes (ft)

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  link_readParams        (called by parseLine in input.c)
//  link_readXsectParams   (called by parseLine in input.c)
//  link_readLossParams    (called by parseLine in input.c)
//  link_validate          (called by project_validate in project.c)
//  link_initState         (called by initObjects in swmm5.c)
//  link_setOldHydState    (called by routing_execute in routing.c)
//  link_setOldQualState   (called by routing_execute in routing.c)            
//  link_setTargetSetting  (called by routing_execute in routing.c)            //(5.0.010 - LR)
//  link_setSetting        (called by routing_execute in routing.c)            //(5.0.010 - LR)
//  link_getResults        (called by output_saveLinkResults)
//  link_getLength                                                             //(5.0.015 - LR)
//  link_getFroude
//  link_getInflow
//  link_setOutfallDepth
//  link_getYcrit
//  link_getYnorm
//  link_getVelocity
//  link_getPower          (called by stats_updateLinkStats in stats.c)        //(5.0.012 - LR)

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void   link_setParams(int j, int type, int n1, int n2, int k, double x[]);
static void   link_convertOffsets(int j);                                      //(5.0.012 - LR)
static double link_getOffsetHeight(int j, double offset, double elev);         //(5.0.016 - LR)

static int    conduit_readParams(int j, int k, char* tok[], int ntoks);
static void   conduit_validate(int j, int k);
static void   conduit_initState(int j, int k);
static void   conduit_reverse(int j, int k);
static double conduit_getLength(int j);                                        //(5.0.015 - LR)
static double conduit_getLengthFactor(int j, int k, double roughness);         //(5.0.010 - LR)
static double conduit_getSlope(int j);                                         //(5.0.015 - LR)
static double conduit_getInflow(int j);
static void   conduit_updateStats(int j, double dt, DateTime aDate);

static int    pump_readParams(int j, int k, char* tok[], int ntoks);
static void   pump_validate(int j, int k);
static void   pump_initState(int j, int k);
static double pump_getInflow(int j);

static int    orifice_readParams(int j, int k, char* tok[], int ntoks);
static void   orifice_validate(int j, int k);
static void   orifice_setSetting(int j, double tstep);                         //(5.0.010 - LR)
static double orifice_getWeirCoeff(int j, int k, double h);                    //(5.0.012 - LR)
static double orifice_getInflow(int j);
static double orifice_getFlow(int j, int k, double head, double f,             //(5.0.012 - LR)
              int hasFlapGate);                                                //(5.0.012 - LR)

static int    weir_readParams(int j, int k, char* tok[], int ntoks);
static void   weir_validate(int j, int k);
static void   weir_setSetting(int j);                                          //(5.0.011 - LR)
static double weir_getInflow(int j);
static double weir_getOpenArea(int j, double y);
static void   weir_getFlow(int j, int k, double head, double dir,
              int hasFlapGate, double* q1, double* q2);
static double weir_getOrificeFlow(int j, double head, double y, double cOrif); //(5.0.012 - LR)
static double weir_getdqdh(int k, double dir, double h, double q1, double q2);

static int    outlet_readParams(int j, int k, char* tok[], int ntoks);
static double outlet_getFlow(int k, double head);
static double outlet_getInflow(int j);


//=============================================================================

int link_readParams(int j, int type, int k, char* tok[], int ntoks)
//
//  Input:   j     = link index
//           type  = link type code
//           k     = link type index
//           tok[] = array of string tokens
//           ntoks = number of tokens   
//  Output:  returns an error code
//  Purpose: reads parameters for a specific type of link from a 
//           tokenized line of input data.
//
{
    switch ( type )
    {
      case CONDUIT: return conduit_readParams(j, k, tok, ntoks);
      case PUMP:    return pump_readParams(j, k, tok, ntoks);
      case ORIFICE: return orifice_readParams(j, k, tok, ntoks);
      case WEIR:    return weir_readParams(j, k, tok, ntoks);
      case OUTLET:  return outlet_readParams(j, k, tok, ntoks);
      default: return 0;
    }
}

//=============================================================================

int link_readXsectParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens   
//  Output:  returns an error code
//  Purpose: reads a link's cross section parameters from a tokenized
//           line of input data.
//
{
    int    i, j, k;
    double x[4];

    // --- get index of link
    if ( ntoks < 6 ) return error_setInpError(ERR_ITEMS, "");
    j = project_findObject(LINK, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);

    // --- get code of xsection shape
    k = findmatch(tok[1], XsectTypeWords);
    if ( k < 0 ) return error_setInpError(ERR_KEYWORD, tok[1]);

    // --- assign default number of barrels to conduit
    if ( Link[j].type == CONDUIT ) Conduit[Link[j].subIndex].barrels = 1;
    
    // --- assume link is not a culvert                                        //(5.0.014 - LR)
    Link[j].xsect.culvertCode = 0;                                             //(5.0.014 - LR)

    // --- for irregular shape, find index of transect object
    if ( k == IRREGULAR )
    {
        i = project_findObject(TRANSECT, tok[2]);
        if ( i < 0 ) return error_setInpError(ERR_NAME, tok[2]);
        Link[j].xsect.type = k;
        Link[j].xsect.transect = i;
    }
    else
    {
        // --- parse max. depth & shape curve for a custom shape               //(5.0.010 - LR)
        if ( k == CUSTOM )                                                     //(5.0.010 - LR)
        {                                                                      //(5.0.010 - LR)
            if ( !getDouble(tok[2], &x[0]) || x[0] <= 0.0 )                    //(5.0.010 - LR) 
               return error_setInpError(ERR_NUMBER, tok[2]);                   //(5.0.010 - LR)
            i = project_findObject(CURVE, tok[3]);                             //(5.0.010 - LR)
            if ( i < 0 ) return error_setInpError(ERR_NAME, tok[3]);           //(5.0.010 - LR)
            Link[j].xsect.type = k;                                            //(5.0.010 - LR)
            Link[j].xsect.transect = i;                                        //(5.0.010 - LR)
            Link[j].xsect.yFull = x[0] / UCF(LENGTH);                          //(5.0.010 - LR) 
        }                                                                      //(5.0.010 - LR)

        // --- parse and save geometric parameters
        else for (i = 2; i <= 5; i++)                                          //(5.0.010 - LR)
        {
            if ( !getDouble(tok[i], &x[i-2]) )
                return error_setInpError(ERR_NUMBER, tok[i]);
        }
        if ( !xsect_setParams(&Link[j].xsect, k, x, UCF(LENGTH)) )
        {
            return error_setInpError(ERR_NUMBER, "");
        }

        // --- parse number of barrels if present
        if ( Link[j].type == CONDUIT && ntoks >= 7 )
        {
            i = atoi(tok[6]);
            if ( i <= 0 ) return error_setInpError(ERR_NUMBER, tok[6]);
            else Conduit[Link[j].subIndex].barrels = (char)i;
        }

        // --- parse culvert code if present                                   //(5.0.014 - LR)
        if ( Link[j].type == CONDUIT && ntoks >= 8 )
        {
            i = atoi(tok[7]);
            if ( i < 0 ) return error_setInpError(ERR_NUMBER, tok[7]);
            else Link[j].xsect.culvertCode = i;
        }

    }
    return 0;
}

//=============================================================================

int link_readLossParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens   
//  Output:  returns an error code
//  Purpose: reads local loss parameters for a link from a tokenized
//           line of input data.
//
{
    int    i, j, k;
    double x[3];

    if ( ntoks < 4 ) return error_setInpError(ERR_ITEMS, "");
    j = project_findObject(LINK, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);
    for (i=1; i<=3; i++)
    {
        if ( ! getDouble(tok[i], &x[i-1]) || x[i-1] < 0.0 )                    //(5.0.019 - LR)
        return error_setInpError(ERR_NUMBER, tok[i]);
    }
    k = 0;
    if ( ntoks >= 5 )
    {
        k = findmatch(tok[4], NoYesWords);             
        if ( k < 0 ) return error_setInpError(ERR_KEYWORD, tok[4]);
    }
    Link[j].cLossInlet   = x[0];
    Link[j].cLossOutlet  = x[1];
    Link[j].cLossAvg     = x[2];
    Link[j].hasFlapGate  = k;
    return 0;
}

//=============================================================================

void  link_setParams(int j, int type, int n1, int n2, int k, double x[])
//
//  Input:   j   = link index
//           type = link type code
//           n1   = index of upstream node
//           n2   = index of downstream node
//           k    = index of link's sub-type
//           x    = array of parameter values
//  Output:  none
//  Purpose: sets parameters for a link.
//
{
    Link[j].node1       = n1;
    Link[j].node2       = n2;
    Link[j].type        = type;
    Link[j].subIndex    = k;
    Link[j].offset1     = 0.0;
    Link[j].offset2     = 0.0;
    Link[j].q0          = 0.0;
    Link[j].qFull       = 0.0;
    Link[j].setting     = 1.0;
    Link[j].targetSetting = 1.0;                                               //(5.0.010 - LR)
    Link[j].hasFlapGate = 0;
    Link[j].qLimit      = 0.0;         // 0 means that no limit is defined
    Link[j].direction   = 1;

    switch (type)
    {
      case CONDUIT:
        Conduit[k].length    = x[0] / UCF(LENGTH);
        Conduit[k].modLength = Conduit[k].length;
        Conduit[k].roughness = x[1];
        Link[j].offset1      = x[2] / UCF(LENGTH);
        Link[j].offset2      = x[3] / UCF(LENGTH);
        Link[j].q0           = x[4] / UCF(FLOW);
        Link[j].qLimit       = x[5] / UCF(FLOW);
        break;

      case PUMP:
        Pump[k].pumpCurve    = (int)x[0];
        Link[j].hasFlapGate  = FALSE;
        Pump[k].initSetting  = x[1];                                           //(5.0.010 - LR)
        Pump[k].yOn          = x[2] / UCF(LENGTH);                             //(5.0.012 - LR)
        Pump[k].yOff         = x[3] / UCF(LENGTH);                             //(5.0.012 - LR)
        Pump[k].xMin         = 0.0;                                            //(5.0.014 - LR)
        Pump[k].xMax         = 0.0;                                            //(5.0.014 - LR)
        break;

      case ORIFICE:
        Orifice[k].type      = (int)x[0];
        Link[j].offset1      = x[1] / UCF(LENGTH);
        Link[j].offset2      = Link[j].offset1;
        Orifice[k].cDisch    = x[2];
        Link[j].hasFlapGate  = (x[3] > 0.0) ? 1 : 0;
        Orifice[k].orate     = x[4] * 3600.0;                                  //(5.0.010 - LR) 
        break;

      case WEIR:
        Weir[k].type         = (int)x[0];
        Link[j].offset1      = x[1] / UCF(LENGTH);
        Link[j].offset2      = Link[j].offset1;
        Weir[k].cDisch1      = x[2];
        Link[j].hasFlapGate  = (x[3] > 0.0) ? 1 : 0;
        Weir[k].endCon       = x[4];
        Weir[k].cDisch2      = x[5];
        break;

      case OUTLET:
        Link[j].offset1      = x[0] / UCF(LENGTH);
        Link[j].offset2      = Link[j].offset1;
        Outlet[k].qCoeff     = x[1];
        Outlet[k].qExpon     = x[2];
        Outlet[k].qCurve     = (int)x[3];
        Link[j].hasFlapGate  = (x[4] > 0.0) ? 1 : 0;
        Outlet[k].curveType  = (int)x[5];                                      //(5.0.014 - LR)

        xsect_setParams(&Link[j].xsect, DUMMY, NULL, 0.0);
        break;

    }
}

//=============================================================================

void  link_validate(int j)
//
//  Input:   j = link index
//  Output:  none
//  Purpose: validates a link's properties.
//
{
    int   n;

    if ( LinkOffsets == ELEV_OFFSET ) link_convertOffsets(j);                  //(5.0.012 - LR)
    switch ( Link[j].type )
    {
      case CONDUIT: conduit_validate(j, Link[j].subIndex); break;
      case PUMP:    pump_validate(j, Link[j].subIndex);    break;
      case ORIFICE: orifice_validate(j, Link[j].subIndex); break;
      case WEIR:    weir_validate(j, Link[j].subIndex);    break;
    }

////  The following code segment was revised in release 5.0.019  ////          //(5.0.019 - LR)

    // --- force max. depth of end nodes to be >= link crown height
    //     at non-storage nodes (except for bottom orifices)
    if ( Link[j].type != ORIFICE ||
         Orifice[Link[j].subIndex].type != BOTTOM_ORIFICE )
    {
        n = Link[j].node1;
        if ( Node[n].type != STORAGE )
        {
            Node[n].fullDepth = MAX(Node[n].fullDepth,
                                Link[j].offset1 + Link[j].xsect.yFull);
        }
        n = Link[j].node2;
        if ( Node[n].type != STORAGE )
        {
            Node[n].fullDepth = MAX(Node[n].fullDepth,
                                Link[j].offset2 + Link[j].xsect.yFull);
        }
    }
////  End of revisions  ////
}

//=============================================================================

////  New function added  ////                                                 //(5.0.012 - LR)
////  Function modified   ////                                                 //(5.0.016 - LR)

void link_convertOffsets(int j)
//
//  Input:   j = link index
//  Output:  none
//  Purpose: converts offset elevations to offset heights for a link.
//
{
    double elev;
    
    elev = Node[Link[j].node1].invertElev;
    Link[j].offset1 = link_getOffsetHeight(j, Link[j].offset1, elev);
    if ( Link[j].type == CONDUIT )
    {
        elev = Node[Link[j].node2].invertElev;
        Link[j].offset2 = link_getOffsetHeight(j, Link[j].offset2, elev);
    }
    else Link[j].offset2 = Link[j].offset1;
}

//=============================================================================

////  New function added  ////                                                 //(5.0.012 - LR)

double link_getOffsetHeight(int j, double offset, double elev)
//
//  Input:   j = link index
//           offset = link elevation offset (ft)
//           elev   = node invert elevation (ft)
//  Output:  returns offset distance above node invert (ft)
//  Purpose: finds offset height for one end of a link.
//
{
    if ( offset == MISSING ) return 0.0;
    offset -= elev;
    if ( offset >= 0.0 ) return offset;
    if ( offset >= -MIN_DELTA_Z ) return 0.0;
    report_writeWarningMsg(WARN03, Link[j].ID);
    return 0.0;
}

//=============================================================================

void link_initState(int j)
//
//  Input:   j = link index
//  Output:  none
//  Purpose: initializes a link's state variables at start of simulation.
//
{
    int   p;

    // --- initialize hydraulic state
    Link[j].oldFlow   = Link[j].q0;
    Link[j].newFlow   = Link[j].q0;
    Link[j].oldDepth  = 0.0;
    Link[j].newDepth  = 0.0;
    Link[j].oldVolume = 0.0;
    Link[j].newVolume = 0.0;
    Link[j].isClosed  = FALSE;
    Link[j].setting   = 1.0;                                                   //(5.0.010 - LR)
    Link[j].targetSetting = 1.0;                                               //(5.0.010 - LR)
    if ( Link[j].type == CONDUIT ) conduit_initState(j, Link[j].subIndex);
    if ( Link[j].type == PUMP    ) pump_initState(j, Link[j].subIndex);
    
    // --- initialize water quality state
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        Link[j].oldQual[p] = 0.0;
        Link[j].newQual[p] = 0.0;
    }
}

//=============================================================================

double  link_getInflow(int j)
//
//  Input:   j = link index
//  Output:  returns link flow rate (cfs)
//  Purpose: finds total flow entering a link during current time step.
//
{
    if ( Link[j].setting == 0 ||
         Link[j].isClosed ) return 0.0;
    switch ( Link[j].type )
    {
      case CONDUIT: return conduit_getInflow(j);
      case PUMP:    return pump_getInflow(j);
      case ORIFICE: return orifice_getInflow(j);
      case WEIR:    return weir_getInflow(j);
      case OUTLET:  return outlet_getInflow(j);
      default:      return node_getOutflow(Link[j].node1, j);
    }
}

//=============================================================================

void link_setOldHydState(int j)
//
//  Input:   j = link index
//  Output:  none
//  Purpose: replaces link's old hydraulic state values with current ones.
//
{
    int k;
    Link[j].oldDepth  = Link[j].newDepth;
    Link[j].oldFlow   = Link[j].newFlow;
    Link[j].oldVolume = Link[j].newVolume;
    if ( Link[j].type == CONDUIT )
    {
        k = Link[j].subIndex;
        Conduit[k].q1Old = Conduit[k].q1;
        Conduit[k].q2Old = Conduit[k].q2;
    }
}

//=============================================================================

void link_setOldQualState(int j)
//
//  Input:   j = link index
//  Output:  none
//  Purpose: replaces link's old water quality state values with current ones.
//
{
    int p;
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        Link[j].oldQual[p] = Link[j].newQual[p];
        Link[j].newQual[p] = 0.0;
    }
}

//=============================================================================

void link_setTargetSetting(int j)                                              //(5.0.010 - LR)
//                                                                             //(5.0.010 - LR)
//  Input:   j = link index                                                    //(5.0.010 - LR)
//  Output:  none                                                              //(5.0.010 - LR)
//  Purpose: updates a link's target setting.                                  //(5.0.010 - LR)
//                                                                             //(5.0.010 - LR)
{                                                                              //(5.0.010 - LR)
    int k, n1;                                                                 //(5.0.010 - LR)
    if ( Link[j].type == PUMP )                                                //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR)
        k = Link[j].subIndex;                                                  //(5.0.010 - LR)
        n1 = Link[j].node1;                                                    //(5.0.010 - LR)
        Link[j].targetSetting = Link[j].setting;                               //(5.0.010 - LR)
        if ( Pump[k].yOff > 0.0 &&                                             //(5.0.010 - LR)
             Link[j].setting > 0.0 &&                                          //(5.0.010 - LR)
             Node[n1].newDepth < Pump[k].yOff ) Link[j].targetSetting = 0.0;   //(5.0.010 - LR)
        if ( Pump[k].yOn > 0.0 &&                                              //(5.0.010 - LR)
             Link[j].setting == 0.0 &&                                         //(5.0.010 - LR)
             Node[n1].newDepth > Pump[k].yOn )  Link[j].targetSetting = 1.0;   //(5.0.010 - LR)
    }                                                                          //(5.0.010 - LR)
}                                                                              //(5.0.010 - LR)

//=============================================================================

void link_setSetting(int j, double tstep)                                      //(5.0.010 - LR)
//                                                                             //(5.0.010 - LR)
//  Input:   j = link index                                                    //(5.0.010 - LR)
//           tstep = time step over which setting is adjusted                  //(5.0.010 - LR)
//  Output:  none                                                              //(5.0.010 - LR)
//  Purpose: updates a link's setting as a result of a control action.         //(5.0.010 - LR)
//                                                                             //(5.0.010 - LR)
{                                                                              //(5.0.010 - LR) 
    switch ( Link[j].type )                                                    //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR)
      case PUMP:    Link[j].setting = Link[j].targetSetting; break;            //(5.0.010 - LR)
      case ORIFICE: orifice_setSetting(j, tstep); break;                       //(5.0.011 - LR)
      case WEIR:    weir_setSetting(j); break;                                 //(5.0.011 - LR)
      default:      Link[j].setting = Link[j].targetSetting;                   //(5.0.011 - LR)
    }                                                                          //(5.0.010 - LR)
}                                                                              //(5.0.010 - LR)

//=============================================================================

////  This function was re-named and re-written for release 5.0.014.  ////     //(5.0.014 - LR)

int link_setFlapGate(int j, int n1, int n2, double q)
//
//  Input:   j = link index
//           n1 = index of node on upstream end of link
//           n2 = index of node on downstream end of link 
//           q = signed flow value (value and units don't matter) 
//  Output:  returns TRUE if there is reverse flow through a flap gate
//           associated with the link. 
//  Purpose: based on the sign of the flow, determines if a flap gate
//           associated with the link should close or not.
//
{
    int    n = -1;

    // --- check for reverse flow through link's flap gate
    if ( Link[j].hasFlapGate )
    {
        if ( q * (double)Link[j].direction < 0.0 ) return TRUE;
    }

    // --- check for Outfall with flap gate node on inflow end of link
    if ( q < 0.0 ) n = n2;
    if ( q > 0.0 ) n = n1;
    if ( n >= 0 &&
         Node[n].type == OUTFALL &&
         Outfall[Node[n].subIndex].hasFlapGate ) return TRUE;
    return FALSE;
}

//=============================================================================

void link_getResults(int j, double f, float x[])
//
//  Input:   j = link index
//           f = time weighting factor
//  Output:  x = array of weighted results
//  Purpose: retrieves time-weighted average of old and new results for a link.
//
{
    int    p;                     // pollutant index
    double y,                     // depth
           q,                     // flow
           v,                     // velocity
           fr,                    // Froude no.
           c;                     // capacity or concentration
    double f1 = 1.0 - f;

    y = f1*Link[j].oldDepth + f*Link[j].newDepth;
    q = f1*Link[j].oldFlow + f*Link[j].newFlow;
    v = link_getVelocity(j, q, y);
    fr = link_getFroude(j, v, y);
    c = 0.0;
    if ( Link[j].type != PUMP && Link[j].xsect.type != DUMMY)                  //(5.0.022 - LR)
    {                                                                          //(5.0.022 - LR)
         if (Link[j].setting == 0.0 ) y = 0.0;                                 //(5.0.022 - LR)
         else c = y / (Link[j].xsect.yFull * Link[j].setting);                 //(5.0.022 - LR)
    }                                                                          //(5.0.022 - LR)

    y *= UCF(LENGTH);
    q *= UCF(FLOW) * (double)Link[j].direction;
    v *= UCF(LENGTH) * (double)Link[j].direction;
    x[LINK_DEPTH]    = (float)y;
    x[LINK_FLOW]     = (float)q;
    x[LINK_VELOCITY] = (float)v;
    x[LINK_FROUDE]   = (float)fr;
    x[LINK_CAPACITY] = (float)c;
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        c = f1*Link[j].oldQual[p] + f*Link[j].newQual[p];
        x[LINK_QUAL+p] = (float)c;
    }
}

//=============================================================================

void link_setOutfallDepth(int j)
//
//  Input:   j = link index
//  Output:  none
//  Purpose: sets depth at outfall node connected to link j.
//
{
    int     k;                         // conduit index
    int     n;                         // outfall node index
    double  z;                         // invert offset height (ft)
    double  q;                         // flow rate (cfs)
    double  yCrit = 0.0;               // critical flow depth (ft)
    double  yNorm = 0.0;               // normal flow depth (ft)

    // --- find which end node of link is an outfall
    if ( Node[Link[j].node2].type == OUTFALL )
    {
        n = Link[j].node2;
        z = Link[j].offset2;
    }
    else if ( Node[Link[j].node1].type == OUTFALL )
    {
        n = Link[j].node1;
        z = Link[j].offset1;
    }
    else return;
    
    // --- find both normal & critical depth for current flow
    if ( Link[j].type == CONDUIT )
    {
        k = Link[j].subIndex;
        q = fabs(Link[j].newFlow / Conduit[k].barrels);
        yNorm = link_getYnorm(j, q);
        yCrit = link_getYcrit(j, q);
    }

    // --- set new depth at node
    node_setOutletDepth(n, yNorm, yCrit, z);
}

//=============================================================================

double link_getYcrit(int j, double q)
//
//  Input:   j = link index
//           q = link flow rate (cfs)
//  Output:  returns critical depth (ft)
//  Purpose: computes critical depth for given flow rate.
//
{
    return xsect_getYcrit(&Link[j].xsect, q);
}

//=============================================================================

double  link_getYnorm(int j, double q)
//
//  Input:   j = link index
//           q = link flow rate (cfs)
//  Output:  returns normal depth (ft)
//  Purpose: computes normal depth for given flow rate.
//
{
    int    k;
    double s, a, y;

    if ( Link[j].type != CONDUIT ) return 0.0;
    if ( Link[j].xsect.type == DUMMY ) return 0.0;
    q = fabs(q);
    if ( q <= 0.0 ) return 0.0;
    k = Link[j].subIndex;
    if ( q > Conduit[k].qMax ) return Link[j].xsect.yFull;
    s = q / Conduit[k].beta;
    a = xsect_getAofS(&Link[j].xsect, s);
    y = xsect_getYofA(&Link[j].xsect, a);
    return y;
}

//=============================================================================

////  New function added to release 5.0.015  ////                              //(5.0.015 - LR)

double link_getLength(int j)
//
//  Input:   j = link index
//  Output:  returns length (ft)
//  Purpose: finds true length of a link.
//
{
    if ( Link[j].type == CONDUIT ) return conduit_getLength(j);
    return 0.0;
}

//=============================================================================

double link_getVelocity(int j, double flow, double depth)
//
//  Input:   j     = link index
//           flow  = link flow rate (cfs)
//           depth = link flow depth (ft)
//  Output:  returns flow velocity (fps)
//  Purpose: finds flow velocity given flow and depth.
//
{
    double area;
    double veloc = 0.0;
    int    k;

    if ( depth <= 0.01 ) return 0.0;
    if ( Link[j].type == CONDUIT )
    {
        k = Link[j].subIndex;
        flow /= Conduit[k].barrels;
        area = xsect_getAofY(&Link[j].xsect, depth);
        if (area > FUDGE ) veloc = flow / area;
    }
    return veloc;
}

//=============================================================================

////  ------  This function was completely re-written.  ------  ////           //(5.0.014 - LR)

double link_getFroude(int j, double v, double y)
//
//  Input:   j = link index
//           v = flow velocity (fps)
//           y = flow depth (ft)
//  Output:  returns Froude Number
//  Purpose: computes Froude Number for given velocity and flow depth
//
{
    TXsect*  xsect = &Link[j].xsect;

    // --- return 0 if link is not a conduit
    if ( Link[j].type != CONDUIT ) return 0.0;
    
    // --- return 0 if link empty or closed conduit is full
    if ( y <= FUDGE ) return 0.0;
    if ( !xsect_isOpen(xsect->type) &&
         xsect->yFull - y <= FUDGE ) return 0.0;

    // --- compute hydraulic depth
    y = xsect_getAofY(xsect, y) / xsect_getWofY(xsect, y);

    // --- compute Froude No.
    return fabs(v) / sqrt(GRAVITY * y);
}

//=============================================================================

////  New function added to compute power consumption ////                     //(5.0.012 - LR)
double link_getPower(int j)
//
//  Input:   j = link index
//  Output:  returns power consumed by link in kwatts
//  Purpose: computes power consumed by head loss (or head gain) of
//           water flowing through a link
//
{
    int    n1 = Link[j].node1;
    int    n2 = Link[j].node2;
    double dh = (Node[n1].invertElev + Node[n1].newDepth) -
                (Node[n2].invertElev + Node[n2].newDepth);
    double q =  fabs(Link[j].newFlow);
    return fabs(dh) * q / 8.814 * KWperHP;
}


//=============================================================================
//                    C O N D U I T   M E T H O D S
//=============================================================================

int  conduit_readParams(int j, int k, char* tok[], int ntoks)
//
//  Input:   j = link index
//           k = conduit index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads conduit parameters from a tokenzed line of input.
//
{
    //int    i;                                                                //(5.0.012 - LR)
    int    n1, n2;
    double x[6];
    char*  id;

    // --- check for valid ID and end node IDs
    if ( ntoks < 7 ) return error_setInpError(ERR_ITEMS, "");
    id = project_findID(LINK, tok[0]);                // link ID
    if ( id == NULL ) return error_setInpError(ERR_NAME, tok[0]);
    n1 = project_findObject(NODE, tok[1]);            // upstrm. node
    if ( n1 < 0 ) return error_setInpError(ERR_NAME, tok[1]);
    n2 = project_findObject(NODE, tok[2]);            // dwnstrm. node
    if ( n2 < 0 ) return error_setInpError(ERR_NAME, tok[2]);

////   Modified for possibility of elevation offsets   ////                    //(5.0.012 - LR)
    // --- parse length & Mannings N
    if ( !getDouble(tok[3], &x[0]) )
        return error_setInpError(ERR_NUMBER, tok[3]);
    if ( !getDouble(tok[4], &x[1]) )
        return error_setInpError(ERR_NUMBER, tok[4]);

    // --- parse offsets
    if ( LinkOffsets == ELEV_OFFSET && *tok[5] == '*' ) x[2] = MISSING;
    else if ( !getDouble(tok[5], &x[2]) )
        return error_setInpError(ERR_NUMBER, tok[5]);
    if ( LinkOffsets == ELEV_OFFSET && *tok[6] == '*' ) x[3] = MISSING;
    else if ( !getDouble(tok[6], &x[3]) )
        return error_setInpError(ERR_NUMBER, tok[6]);
////////////////////////////////////////////////////////////

   // --- parse optional parameters
    x[4] = 0.0;                                       // init. flow
    if ( ntoks >= 8 )
    {
        if ( !getDouble(tok[7], &x[4]) )
        return error_setInpError(ERR_NUMBER, tok[7]);
    }
    x[5] = 0.0;
    if ( ntoks >= 9 )
    {
        if ( !getDouble(tok[8], &x[5]) )
        return error_setInpError(ERR_NUMBER, tok[8]);
    }

    // --- add parameters to data base
    Link[j].ID = id;
    link_setParams(j, CONDUIT, n1, n2, k, x);
    return 0;
}

//=============================================================================

void  conduit_validate(int j, int k)
//
//  Input:   j = link index
//           k = conduit index
//  Output:  none
//  Purpose: validates a conduit's properties.
//
{
    double aa;
    double lengthFactor, roughness, slope;                                     //(5.0.018 - LR)

    // --- if custom xsection, then set its parameters                         //(5.0.010 - LR)
    if ( Link[j].xsect.type == CUSTOM )                                        //(5.0.010 - LR)
        xsect_setCustomXsectParams(&Link[j].xsect);                            //(5.0.010 - LR)

    // --- if irreg. xsection, assign transect roughness to conduit
    if ( Link[j].xsect.type == IRREGULAR )
    {
        xsect_setIrregXsectParams(&Link[j].xsect);
        Conduit[k].roughness = Transect[Link[j].xsect.transect].roughness;
    }

    // --- if force main xsection, adjust units on D-W roughness height        //(5.0.010 - LR)
    if ( Link[j].xsect.type == FORCE_MAIN )                                    //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR)
        if ( ForceMainEqn == D_W ) Link[j].xsect.rBot /= UCF(RAINDEPTH);       //(5.0.010 - LR)
        if ( Link[j].xsect.rBot <= 0.0 )                                       //(5.0.010 - LR)
            report_writeErrorMsg(ERR_XSECT, Link[j].ID);                       //(5.0.010 - LR)
    }                                                                          //(5.0.010 - LR)

    // --- check for valid length & roughness
    if ( Conduit[k].length <= 0.0 )
        report_writeErrorMsg(ERR_LENGTH, Link[j].ID);
    if ( Conduit[k].roughness <= 0.0 )
        report_writeErrorMsg(ERR_ROUGHNESS, Link[j].ID);
    if ( Conduit[k].barrels <= 0 )
        report_writeErrorMsg(ERR_BARRELS, Link[j].ID);

    // --- check for valid xsection
    if ( Link[j].xsect.type != DUMMY )
    {
        if ( Link[j].xsect.type < 0 )
            report_writeErrorMsg(ERR_NO_XSECT, Link[j].ID);
        else if ( Link[j].xsect.aFull <= 0.0 )
            report_writeErrorMsg(ERR_XSECT, Link[j].ID);
    }
    if ( ErrorCode ) return;

    // --- check for negative offsets                                          //(5.0.012 - LR)
    if ( Link[j].offset1 < 0.0 )
    {
        report_writeWarningMsg(WARN03, Link[j].ID);                            //(5.0.015 - LR)
        Link[j].offset1 = 0.0;                                                 //(5.0.012 - LR)
    }
	if ( Link[j].offset2 < 0.0 )
    {
        report_writeWarningMsg(WARN03, Link[j].ID);                            //(5.0.015 - LR)
        Link[j].offset2 = 0.0;                                                 //(5.0.012 - LR)
    }

    // --- adjust conduit offsets for partly filled circular xsection
    if ( Link[j].xsect.type == FILLED_CIRCULAR )
    {
        Link[j].offset1 += Link[j].xsect.yBot;
        Link[j].offset2 += Link[j].xsect.yBot;
    }

    // --- compute conduit slope 
    slope = conduit_getSlope(j);                                               //(5.0.018 - LR)
    Conduit[k].slope = slope;                                                  //(5.0.018 - LR)

    // --- reverse orientation of conduit if using dynamic wave routing 
    //     and slope is negative
    if ( RouteModel == DW &&
         slope < 0.0 &&                                                        //(5.0.018 - LR)
         Link[j].xsect.type != DUMMY )
    {
        conduit_reverse(j, k);
    }

    // --- get equivalent Manning roughness for Force Mains                    //(5.0.010 - LR)
    //     for use when pipe is partly full                                    //(5.0.010 - LR)
    roughness = Conduit[k].roughness;                                          //(5.0.010 - LR)
    if ( RouteModel == DW && Link[j].xsect.type == FORCE_MAIN )                //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR)
        roughness = forcemain_getEquivN(j, k);                                 //(5.0.010 - LR)
    }                                                                          //(5.0.010 - LR)

    // --- adjust roughness for meandering natural channels                    //(5.0.015 - LR)
    if ( Link[j].xsect.type == IRREGULAR )
    {
        lengthFactor = Transect[Link[j].xsect.transect].lengthFactor;
        roughness *= sqrt(lengthFactor);
    }

    // --- lengthen conduit if lengthening option is in effect
    lengthFactor = 1.0;                                                        //(5.0.015 - LR)
    if ( RouteModel == DW &&
         LengtheningStep > 0.0 &&
         Link[j].xsect.type != DUMMY )
    {
        lengthFactor = conduit_getLengthFactor(j, k, roughness);               //(5.0.010 - LR)
    }
        
    if ( lengthFactor != 1.0 )                                                 //(5.0.015 - LR)
    {                                                                          //(5.0.015 - LR)
        //Conduit[k].modLength = lengthFactor * Conduit[k].length;             //(5.0.015 - LR)
        Conduit[k].modLength = lengthFactor * conduit_getLength(j);            //(5.0.015 - LR)
        slope /= lengthFactor;                                                 //(5.0.018 - LR)
        roughness = roughness / sqrt(lengthFactor);                            //(5.0.015 - LR)
    }                                                                          //(5.0.015 - LR)

    // --- compute roughness factor used when computing friction               //(5.0.010 - LR)
    //     slope term in Dynamic Wave flow routing                             //(5.0.010 - LR)

    // --- special case for non-Manning Force Mains                            //(5.0.010 - LR)
    //     (roughness factor for full flow is saved in xsect.sBot)             //(5.0.010 - LR)     
    if ( RouteModel == DW && Link[j].xsect.type == FORCE_MAIN )                //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR)
        Link[j].xsect.sBot =                                                   //(5.0.010 - LR)
            forcemain_getRoughFactor(j, lengthFactor);                         //(5.0.010 - LR)
    }                                                                          //(5.0.010 - LR)
    Conduit[k].roughFactor = GRAVITY * SQR(roughness/PHI);                     //(5.0.010 - LR)

    // --- compute full flow through cross section
    if ( Link[j].xsect.type == DUMMY ) Conduit[k].beta = 0.0;
    else Conduit[k].beta = PHI * sqrt(fabs(slope)) / roughness;                //(5.0.018 - LR)
    Link[j].qFull = Link[j].xsect.sFull * Conduit[k].beta;
    Conduit[k].qMax = Link[j].xsect.sMax * Conduit[k].beta;

    // --- see if flow is supercritical most of time
    //     by comparing normal & critical velocities.
    //     (factor of 0.3 is for circular pipe 95% full)
    // NOTE: this factor was used in the past for a modified version of        //(5.0.014 - LR)
    //       Kinematic Wave routing but is now deprecated.                     //(5.0.014 - LR)
    aa = Conduit[k].beta / sqrt(32.2) *
         pow(Link[j].xsect.yFull, 0.1666667) * 0.3;
    if ( aa >= 1.0 ) Conduit[k].superCritical = TRUE;
    else             Conduit[k].superCritical = FALSE;

    // --- set value of hasLosses flag
    if ( Link[j].cLossInlet  == 0.0 &&
         Link[j].cLossOutlet == 0.0 &&
         Link[j].cLossAvg    == 0.0
       ) Conduit[k].hasLosses = FALSE;
    else Conduit[k].hasLosses = TRUE;
}

//=============================================================================

void conduit_reverse(int j, int k)
//
//  Input:   j = link index
//           k = conduit index
//  Output:  none
//  Purpose: reverses direction of a conduit
//
{
    int    i;
    double z;
    double cLoss;

    // --- reverse end nodes
    i = Link[j].node1;
    Link[j].node1 = Link[j].node2;
    Link[j].node2 = i;

    // --- reverse node offsets
    z = Link[j].offset1;
    Link[j].offset1 = Link[j].offset2;
    Link[j].offset2 = z;

    // --- reverse loss coeffs.
    cLoss = Link[j].cLossInlet;
    Link[j].cLossInlet = Link[j].cLossOutlet;
    Link[j].cLossOutlet = cLoss;

    // --- reverse direction & slope
    Conduit[k].slope = -Conduit[k].slope;
    Link[j].direction *= (signed char)-1;

    // --- reverse initial flow value
    Link[j].q0 = -Link[j].q0;
}

//=============================================================================

////  New function added to release 5.0.015  ////                              //(5.0.015 - LR)

double conduit_getLength(int j)
//
//  Input:   j = link index
//  Output:  returns conduit's length (ft)
//  Purpose: finds true length of a conduit.
//
//  Note: for irregular natural channels, user inputs length of main
//        channel (for FEMA purposes) but program should use length
//        associated with entire flood plain. Transect.lengthFactor
//        is the ratio of these two lengths.
//
{
    int k = Link[j].subIndex;
    int t;
    if ( Link[j].xsect.type != IRREGULAR ) return Conduit[k].length;
    t = Link[j].xsect.transect;
    if ( t < 0 || t >= Nobjects[TRANSECT] ) return Conduit[k].length;
    return Conduit[k].length / Transect[t].lengthFactor;
}

//=============================================================================

double conduit_getLengthFactor(int j, int k, double roughness)                 //(5.0.010 - LR)
//
//  Input:   j = link index
//           k = conduit index
//           roughness = conduit Manning's n
//  Output:  returns factor by which a conduit should be lengthened
//  Purpose: computes amount of conduit lengthing to improve numerical stability.
//
//  The following form of the Courant criterion is used:
//      L = t * v * (1 + Fr) / Fr
//  where L = conduit length, t = time step, v = velocity, & Fr = Froude No.
//  After substituting Fr = v / sqrt(gy), where y = flow depth, we get:
//    L = t * ( sqrt(gy) + v )
//
{
    double ratio;
    double yFull;
    double vFull;
    double tStep;

    // --- evaluate flow depth and velocity at full normal flow condition
    yFull = Link[j].xsect.yFull;
    if ( xsect_isOpen(Link[j].xsect.type) )
    {
        yFull = Link[j].xsect.aFull / xsect_getWofY(&Link[j].xsect, yFull);
    }
    vFull = PHI / roughness * Link[j].xsect.sFull *                            //(5.0.010 - LR)
            sqrt(fabs(Conduit[k].slope)) / Link[j].xsect.aFull;                //(5.0.010 - LR)

    // --- determine ratio of Courant length to actual length
    if ( LengtheningStep == 0.0 ) tStep = RouteStep;
    else                          tStep = MIN(RouteStep, LengtheningStep);
    ratio = (sqrt(GRAVITY*yFull) + vFull) * tStep / conduit_getLength(j);      //(5.0.015 - LR)

    // --- return max. of 1.0 and ratio
    if ( ratio > 1.0 ) return ratio;
    else return 1.0;
}

//=============================================================================

//// --- New function added for release 5.0.014 --- ////                       //(5.0.014 - LR)

double conduit_getSlope(int j)                                                 //(5.0.015 - LR)
//
//  Input:   j = link index
//  Output:  returns conduit slope
//  Purpose: computes conduit slope.
//
{
    double elev1, elev2, delta, slope;
    double length = conduit_getLength(j);                                      //(5.0.015 - LR)

    // --- check that elevation drop > minimum allowable drop
    elev1 = Link[j].offset1 + Node[Link[j].node1].invertElev;
    elev2 = Link[j].offset2 + Node[Link[j].node2].invertElev;
    delta = fabs(elev1 - elev2);
    if ( delta < MIN_DELTA_Z )
    {
        report_writeWarningMsg(WARN04, Link[j].ID);                            //(5.0.015 - LR)
        delta = MIN_DELTA_Z;
    }

    // --- elevation drop cannot exceed conduit length
    if ( delta >= length )                                                     //(5.0.015 - LR)
    {
        report_writeWarningMsg(WARN08, Link[j].ID);                            //(5.0.017 - LR)
        slope = delta / length;                                                //(5.0.017 - LR)
    }

    // --- slope = elev. drop / horizontal distance
    else slope = delta / sqrt(SQR(length) - SQR(delta));                       //(5.0.015 - LR)

    // -- check that slope exceeds minimum allowable slope
    if ( MinSlope > 0.0 && slope < MinSlope )
    {
        report_writeWarningMsg(WARN05, Link[j].ID);                            //(5.0.015 - LR)
        slope = MinSlope;
    }

    // --- change sign for adverse slope
    if ( elev1 < elev2 ) slope = -slope;
    return slope;
}

//=============================================================================

void  conduit_initState(int j, int k)
//
//  Input:   j = link index
//           k = conduit index
//  Output:  none
//  Purpose: sets initial conduit depth to normal depth of initial flow
//
{
    Link[j].newDepth = link_getYnorm(j, Link[j].q0 / Conduit[k].barrels);
    Link[j].oldDepth = Link[j].newDepth;
}

//=============================================================================

double conduit_getInflow(int j)
//
//  Input:   j = link index
//  Output:  returns flow in link (cfs)
//  Purpose: finds inflow to conduit from upstream node.
//
{
    double qIn = node_getOutflow(Link[j].node1, j);
    if ( Link[j].qLimit > 0.0 ) qIn = MIN(qIn, Link[j].qLimit);                //(5.0.012 - LR)
    return qIn;
}


//=============================================================================
//                        P U M P   M E T H O D S
//=============================================================================

int  pump_readParams(int j, int k, char* tok[], int ntoks)
//
//  Input:   j = link index
//           k = pump index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads pump parameters from a tokenized line of input.
//
{
    int    m;
    int    n1, n2;
    double x[4];                                                               //(5.0.010 - LR)
    char*  id;

    // --- check for valid ID and end node IDs
    if ( ntoks < 3 ) return error_setInpError(ERR_ITEMS, "");                  //(5.0.010 - LR)
    id = project_findID(LINK, tok[0]);
    if ( id == NULL ) return error_setInpError(ERR_NAME, tok[0]);
    n1 = project_findObject(NODE, tok[1]);
    if ( n1 < 0 ) return error_setInpError(ERR_NAME, tok[1]);
    n2 = project_findObject(NODE, tok[2]);
    if ( n2 < 0 ) return error_setInpError(ERR_NAME, tok[2]);

    // --- parse curve name
    x[0] = -1.;                                                                //(5.0.010 - LR)
    if ( ntoks >= 4 )                                                          //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR) 
        if ( !strcomp(tok[3],"*") )                                            //(5.0.010 - LR)
        {                                                                      //(5.0.010 - LR)
            m = project_findObject(CURVE, tok[3]);
            if ( m < 0 ) return error_setInpError(ERR_NAME, tok[3]);
            x[0] = m;
        }                                                                      //(5.0.010 - LR)
    }                                                                          //(5.0.010 - LR)

    // --- parse init. status if present
    x[1] = 1.0;
    if ( ntoks >= 5 )
    {
        m = findmatch(tok[4], OffOnWords);
        if ( m < 0 ) return error_setInpError(ERR_KEYWORD, tok[4]);
        x[1] = m;
    }

    // --- parse startup/shutoff depths if present                             //(5.0.010 - LR)
    x[2] = 0.0;                                                                //(5.0.010 - LR)
    if ( ntoks >= 6 )                                                          //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR)
        if ( !getDouble(tok[5], &x[2]) )                                       //(5.0.010 - LR)
        return error_setInpError(ERR_NUMBER, tok[5]);                          //(5.0.010 - LR)
    }                                                                          //(5.0.010 - LR)
    x[3] = 0.0;                                                                //(5.0.010 - LR)
    if ( ntoks >= 7 )                                                          //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR)
        if ( !getDouble(tok[6], &x[3]) )                                       //(5.0.010 - LR)
        return error_setInpError(ERR_NUMBER, tok[6]);                          //(5.0.010 - LR)
    }                                                                          //(5.0.010 - LR)

    // --- add parameters to pump object
    Link[j].ID = id;
    link_setParams(j, PUMP, n1, n2, k, x);
    return 0;
}

//=============================================================================

void  pump_validate(int j, int k)
//
//  Input:   j = link index
//           k = pump index
//  Output:  none
//  Purpose: validates a pump's properties
//
{
    int    m, n1;                                                              //(5.0.014 - LR)
    double x, y;

    Link[j].xsect.yFull = 0.0;

    // --- check for valid curve type
    m = Pump[k].pumpCurve;
    //if ( m < 0 ) report_writeErrorMsg(ERR_NO_CURVE, Link[j].ID);             //(5.0.010 - LR)
    if ( m < 0 )                                                               //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR)
        Pump[k].type = IDEAL_PUMP;                                             //(5.0.010 - LR)
    }                                                                          //(5.0.010 - LR) 
    else                                                                       //(5.0.010 - LR)
    {
        if ( Curve[m].curveType < PUMP1_CURVE ||
             Curve[m].curveType > PUMP4_CURVE )
            report_writeErrorMsg(ERR_NO_CURVE, Link[j].ID);

        // --- store pump curve type with pump's parameters
        else 
        {
            Pump[k].type = Curve[m].curveType - PUMP1_CURVE;                   //(5.0.010 - LR)
            if ( table_getFirstEntry(&Curve[m], &x, &y) )
            {
                Link[j].qFull = y;
                Pump[k].xMin = x;                                              //(5.0.012 - LR)
                Pump[k].xMax = x;                                              //(5.0.012 - LR)       
                while ( table_getNextEntry(&Curve[m], &x, &y) )
                {
                    Link[j].qFull = MAX(y, Link[j].qFull);
                    Pump[k].xMax = x;                                          //(5.0.012 - LR)
                }
            }
            Link[j].qFull /= UCF(FLOW);
       }
    }

    // --- assign wet well volume to inlet node of Type 1 pump                 //(5.0.014 - LR)
    if ( Pump[k].type == TYPE1_PUMP )                                          //(5.0.014 - LR)
    {                                                                          //(5.0.014 - LR)
        n1 = Link[j].node1;                                                    //(5.0.014 - LR)
        if ( Node[n1].type != STORAGE )                                        //(5.0.014 - LR)
            Node[n1].fullVolume = MAX(Node[n1].fullVolume,                     //(5.0.014 - LR)
                                      Pump[k].xMax / UCF(VOLUME));             //(5.0.014 - LR)
    }                                                                          //(5.0.014 - LR)

}

//=============================================================================

////  --- This function was completely re-written. ---  ////                   //(5.0.014 - LR)

void  pump_initState(int j, int k)
//
//  Input:   j = link index
//           k = pump index
//  Output:  none
//  Purpose: initializes pump conditions at start of a simulation
//
{
    Link[j].setting = Pump[k].initSetting;
    Link[j].targetSetting = Pump[k].initSetting;
}

//=============================================================================

double pump_getInflow(int j)
//
//  Input:   j = link index
//  Output:  returns pump flow (cfs)
//  Purpose: finds flow produced by a pump.
//
{
    int     k, m;
    int     n1, n2;
    double  vol, depth, head;
    double  qIn, qIn1, dh = 0.001;

    k = Link[j].subIndex;
    m = Pump[k].pumpCurve;
    n1 = Link[j].node1;
    n2 = Link[j].node2;

    // --- no flow if setting is closed                                        //(5.0.010 - LR)
    Link[j].flowClass = NO;                                                    //(5.0.012 - LR)
    Link[j].setting = Link[j].targetSetting;                                   //(5.0.010 - LR)
    if ( Link[j].setting == 0.0 ) return 0.0;                                  //(5.0.010 - LR)

    // --- no flow if inlet node < startup depth or > 

    // --- pump flow = node inflow for IDEAL_PUMP                              //(5.0.010 - LR)
    if ( Pump[k].type == IDEAL_PUMP )                                          //(5.0.010 - LR)
        qIn = Node[n1].inflow + Node[n1].overflow;                             //(5.0.010 - LR)

    // --- pumping rate depends on pump curve type
    else switch(Curve[m].curveType)                                            //(5.0.010 - LR)
    {
      case PUMP1_CURVE:
        vol = Node[n1].newVolume * UCF(VOLUME);
        qIn = table_intervalLookup(&Curve[m], vol) / UCF(FLOW);

        // --- check if off of pump curve                                      //(5.0.012 - LR)
        if ( vol < Pump[k].xMin || vol > Pump[k].xMax )                        //(5.0.012 - LR)     
            Link[j].flowClass = YES;                                           //(5.0.012 - LR)
        break;

      case PUMP2_CURVE:
        depth = Node[n1].newDepth * UCF(LENGTH);
        qIn = table_intervalLookup(&Curve[m], depth) / UCF(FLOW);

        // --- check if off of pump curve                                      //(5.0.012 - LR)
        if ( depth < Pump[k].xMin || depth > Pump[k].xMax )                    //(5.0.012 - LR)     
            Link[j].flowClass = YES;                                           //(5.0.012 - LR)
        break;

      case PUMP3_CURVE:
        head = ( (Node[n2].newDepth + Node[n2].invertElev) -
                 (Node[n1].newDepth + Node[n1].invertElev) );                  //(5.0.012 - LR)
        qIn = table_lookup(&Curve[m], head*UCF(LENGTH)) / UCF(FLOW);           //(5.0.012 - LR)

        // --- compute dQ/dh (slope of pump curve) and
        //     reverse sign since flow decreases with increasing head
        qIn1 = table_lookup(&Curve[m], (head+dh)*UCF(LENGTH)) / UCF(FLOW);
        Link[j].dqdh = -(qIn1 - qIn) / dh;

        // --- check if off of pump curve                                      //(5.0.012 - LR)
        head *= UCF(LENGTH);                                                   //(5.0.012 - LR)
        if ( head < Pump[k].xMin || head > Pump[k].xMax )                      //(5.0.012 - LR)     
            Link[j].flowClass = YES;                                           //(5.0.012 - LR)
        break;

      case PUMP4_CURVE:
        depth = Node[n1].newDepth;                                             //(5.0.012 - LR)
        qIn = table_lookup(&Curve[m], depth*UCF(LENGTH)) / UCF(FLOW);          //(5.0.012 - LR)

        // --- compute dQ/dh (slope of pump curve)
        qIn1 = table_lookup(&Curve[m], (depth+dh)*UCF(LENGTH)) / UCF(FLOW);
        Link[j].dqdh = (qIn1 - qIn) / dh;

        // --- check if off of pump curve                                      //(5.0.012 - LR)
        depth *= UCF(LENGTH);                                                  //(5.0.012 - LR)
        if ( depth < Pump[k].xMin ) Link[j].flowClass = DN_DRY;                //(5.0.022 - LR)
        if ( depth > Pump[k].xMax ) Link[j].flowClass = UP_DRY;                //(5.0.022 - LR)
        break;

      default: qIn = 0.0;
    }

    // --- do not allow reverse flow through pump
    if ( qIn < 0.0 )  qIn = 0.0;
    return qIn * Link[j].setting; 
}


//=============================================================================
//                    O R I F I C E   M E T H O D S
//=============================================================================

int  orifice_readParams(int j, int k, char* tok[], int ntoks)
//
//  Input:   j = link index
//           k = orifice index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads orifice parameters from a tokenized line of input.
//
{
    int    m;
    int    n1, n2;
    double x[5];                                                               //(5.0.010 - LR)
    char*  id;

    // --- check for valid ID and end node IDs
    if ( ntoks < 6 ) return error_setInpError(ERR_ITEMS, "");
    id = project_findID(LINK, tok[0]);
    if ( id == NULL ) return error_setInpError(ERR_NAME, tok[0]);
    n1 = project_findObject(NODE, tok[1]);
    if ( n1 < 0 ) return error_setInpError(ERR_NAME, tok[1]);
    n2 = project_findObject(NODE, tok[2]);
    if ( n2 < 0 ) return error_setInpError(ERR_NAME, tok[2]);

    // --- parse orifice parameters
    m = findmatch(tok[3], OrificeTypeWords);
    if ( m < 0 ) return error_setInpError(ERR_KEYWORD, tok[3]);
    x[0] = m;                                              // type
    if ( LinkOffsets == ELEV_OFFSET && *tok[4] == '*' ) x[1] = MISSING;        //(5.0.012 - LR)
    else if ( ! getDouble(tok[4], &x[1]) )                 // crest height     //(5.0.012 - LR)
        return error_setInpError(ERR_NUMBER, tok[4]);
    if ( ! getDouble(tok[5], &x[2]) || x[2] < 0.0 )        // cDisch
        return error_setInpError(ERR_NUMBER, tok[5]);
    x[3] = 0.0;
    if ( ntoks >= 7 )
    {
        m = findmatch(tok[6], NoYesWords);               
        if ( m < 0 ) return error_setInpError(ERR_KEYWORD, tok[6]);
        x[3] = m;                                          // flap gate
    }
    x[4] = 0.0;                                                                //(5.0.010 - LR)
    if ( ntoks >= 8 )                                                          //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR)
        if ( ! getDouble(tok[7], &x[4]) || x[4] < 0.0 )    // orate            //(5.0.010 - LR)
            return error_setInpError(ERR_NUMBER, tok[7]);                      //(5.0.010 - LR)
    }                                                                          //(5.0.010 - LR)

    // --- add parameters to orifice object
    Link[j].ID = id;
    link_setParams(j, ORIFICE, n1, n2, k, x);
    return 0;
}

//=============================================================================

void  orifice_validate(int j, int k)
//
//  Input:   j = link index
//           k = orifice index
//  Output:  none
//  Purpose: validates an orifice's properties
//
{
    int    err = 0;

    // --- check for valid xsection
    if ( Link[j].xsect.type != RECT_CLOSED
    &&   Link[j].xsect.type != CIRCULAR ) err = ERR_REGULATOR_SHAPE;
    if ( err > 0 )
    {
        report_writeErrorMsg(err, Link[j].ID);
        return;
    }

    // --- check for negative offset                                           //(5.0.012 - LR)
    if ( Link[j].offset1 < 0.0 ) Link[j].offset1 = 0.0;                        //(5.0.012 - LR)

    // --- compute partial flow adjustment
    orifice_setSetting(j, 0.0);                                                //(5.0.010 - LR)

    // --- compute an equivalent length
    Orifice[k].length = 2.0 * RouteStep * sqrt(GRAVITY * Link[j].xsect.yFull);
    Orifice[k].length = MAX(200.0, Orifice[k].length);
    Orifice[k].surfArea = 0.0;
}

//=============================================================================

void  orifice_setSetting(int j, double tstep)                                  //(5.0.010 - LR)
//                                                                             //(5.0.010 - LR)
//  Input:   j = link index                                                    //(5.0.010 - LR)
//           tstep = time step over which setting is adjusted (sec)            //(5.0.010 - LR)
//  Output:  none                                                              //(5.0.010 - LR)
//  Purpose: updates an orifice's setting as a result of a control action.     //(5.0.010 - LR)
//                                                                             //(5.0.010 - LR)
{                                                                              //(5.0.010 - LR)
    int    k = Link[j].subIndex;                                               //(5.0.010 - LR)
    double delta, step;                                                        //(5.0.010 - LR)
    double h, f;                                                               //(5.0.012 - LR)

    // --- case where adjustment rate is instantaneous                         //(5.0.010 - LR)
    if ( Orifice[k].orate == 0.0 || tstep == 0.0)                              //(5.0.013 - LR)
        Link[j].setting = Link[j].targetSetting;                               //(5.0.013 - LR)

    // --- case where orifice setting depends on time step                     //(5.0.010 - LR)
    else                                                                       //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR) 
        delta = Link[j].targetSetting - Link[j].setting;                       //(5.0.010 - LR)  
        step = tstep / Orifice[k].orate;                                       //(5.0.010 - LR)
        if ( step + 0.001 >= fabs(delta) )                                     //(5.0.010 - LR)
            Link[j].setting = Link[j].targetSetting;                           //(5.0.010 - LR)
        else Link[j].setting += SGN(delta) * step;                             //(5.0.010 - LR)
    }

    // --- find effective orifice discharge coeff.                             //(5.0.012 - LR)
    h = Link[j].setting * Link[j].xsect.yFull;                                 //(5.0.012 - LR)
    f = xsect_getAofY(&Link[j].xsect, h) * sqrt(2.0 * GRAVITY);                //(5.0.012 - LR)
    Orifice[k].cOrif = Orifice[k].cDisch * f;                                  //(5.0.012 - LR)

    // --- find equiv. discharge coeff. for when weir flow occurs              //(5.0.012 - LR)
    Orifice[k].cWeir = orifice_getWeirCoeff(j, k, h) * f;                      //(5.0.012 - LR)
}                                                                              //(5.0.010 - LR)

//=============================================================================

////  New function added for orifice acting as weir.  ////                     //(5.0.012 - LR)

double orifice_getWeirCoeff(int j, int k, double h)
//
//  Input:   j = link index
//           k = orifice index
//           h = height of orifice opening (ft)
//  Output:  returns a discharge coefficient (ft^1/2)
//  Purpose: computes the discharge coefficient for an orifice
//           at the critical depth where weir flow begins.
//
{
    double w, aOverL;

    // --- this is for bottom orifices
    if ( Orifice[k].type == BOTTOM_ORIFICE )
    {
        // --- find critical height above opening where orifice flow
        //     turns into weir flow. It equals (Co/Cw)*(Area/Length)
        //     where Co is the orifice coeff., Cw is the weir coeff/sqrt(2g),
        //     Area is the area of the opening, and Length = circumference
        //     of the opening. For a basic sharp crested weir, Cw = 0.414.
        if (Link[j].xsect.type == CIRCULAR) aOverL = h / 4.0;
        else
        {
            w = Link[j].xsect.wMax;
            aOverL = (h*w) / (2.0*(h+w));
        }
        h = Orifice[k].cDisch / 0.414 * aOverL;
        Orifice[k].hCrit = h;
    }

    // --- this is for side orifices
    else
    {
        // --- critical height is simply height of opening
        Orifice[k].hCrit = h;

        // --- head on orifice is distance to center line
        h = h / 2.0;
    }

    // --- return a coefficient for the critical depth
    return Orifice[k].cDisch * sqrt(h);
}

//=============================================================================

////  Function re-written to better handle bottom orifices.  ////              //(5.0.012 - LR)

double orifice_getInflow(int j)
//
//  Input:   j = link index
//  Output:  returns orifice flow rate (cfs)
//  Purpose: finds the flow through an orifice.
//
{
    int    k, n1, n2;
    double head, h1, h2, y1, dir;
    double f;
	double hcrest = 0.0;
	double hcrown = 0.0;
    double hmidpt;
    double q, ratio;                                                           //(5.0.019 - LR)

    // --- get indexes of end nodes and link's orifice
    n1 = Link[j].node1;
    n2 = Link[j].node2;
    k  = Link[j].subIndex;

    // --- find heads at upstream & downstream nodes
    if ( RouteModel == DW )
    {
        h1 = Node[n1].newDepth + Node[n1].invertElev;
        h2 = Node[n2].newDepth + Node[n2].invertElev;
    }
    else
    {
        h1 = Node[n1].newDepth + Node[n1].invertElev;
        h2 = Node[n1].invertElev;
    }
    dir = (h1 >= h2) ? +1.0 : -1.0; 
           
    // --- exchange h1 and h2 for reverse flow
    y1 = Node[n1].newDepth;
    if ( dir < 0.0 )
    {
        head = h1;
        h1 = h2;
        h2 = head;
        y1 = Node[n2].newDepth;
    }

    // --- orifice is a bottom orifice (oriented in horizontal plane)
    if ( Orifice[k].type == BOTTOM_ORIFICE )
    {
        // --- compute crest elevation
        hcrest = Node[n1].invertElev + Link[j].offset1;

        // --- compute head on orifice
        if (h1 < hcrest) head = 0.0;
        else if (h2 > hcrest) head = h1 - h2;
        else head = h1 - hcrest;

        // --- find fraction of critical height for which weir flow occurs
        f = head / Orifice[k].hCrit;
        f = MIN(f, 1.0);
    }

    // --- otherwise orifice is a side orifice (oriented in vertical plane)
    else
    {
        // --- compute elevations of orifice crest and crown
        hcrest = Node[n1].invertElev + Link[j].offset1;
        hcrown = hcrest + Link[j].xsect.yFull * Link[j].setting;
        hmidpt = (hcrest + hcrown) / 2.0;
    
        // --- compute degree of inlet submergence
        if ( h1 < hcrown && hcrown > hcrest )
            f = (h1 - hcrest) / (hcrown - hcrest);
        else f = 1.0;

        // --- compute head on orifice
        if ( f < 1.0 )          head = h1 - hcrest;
        else if ( h2 < hmidpt ) head = h1 - hmidpt;
        else                    head = h1 - h2;
    }

    // --- return if head is negligible or flap gate closed
    if ( head <= FUDGE || y1 <= FUDGE || 
         link_setFlapGate(j, n1, n2, dir) )                                    //(5.0.014 - LR)
    {
        Link[j].newDepth = 0.0;
        Link[j].flowClass = DRY;
        Orifice[k].surfArea = FUDGE * Orifice[k].length;
        Link[j].dqdh = 0.0;
        return 0.0;
    }

    // --- determine flow class
    Link[j].flowClass = SUBCRITICAL;
    if ( hcrest > h2 )
    {
        if ( dir == 1.0 ) Link[j].flowClass = DN_CRITICAL;
        else              Link[j].flowClass = UP_CRITICAL;
    }

    // --- compute flow depth and surface area
    y1 = Link[j].xsect.yFull * Link[j].setting;
    if ( Orifice[k].type == SIDE_ORIFICE )
    {
        Link[j].newDepth = y1 * f;
        Orifice[k].surfArea =
            xsect_getWofY(&Link[j].xsect, Link[j].newDepth) *
            Orifice[k].length;
    }
    else
    {
        Link[j].newDepth = y1;
        Orifice[k].surfArea = xsect_getAofY(&Link[j].xsect, y1);
    }

////  Following segment modified for release 5.0.019.  ////                    //(5.0.019 - LR)
    // --- find flow through the orifice
    q = dir * orifice_getFlow(j, k, head, f, Link[j].hasFlapGate);

    // --- apply Villemonte eqn. to correct for submergence
    if ( f < 1.0 && h2 > hcrest )
    {
        ratio = (h2 - hcrest) / (h1 - hcrest);
        q *= pow( (1.0 - pow(ratio, 1.5)), 0.385);
    }
    return q;
}

//=============================================================================

////  Function re-written to incorporate flap gate head loss.  ////            //(5.0.012 - LR)

double orifice_getFlow(int j, int k,  double head, double f, int hasFlapGate)
//
//  Input:   j = link index
//           k = orifice index
//           head = head across orifice
//           f = fraction of critical depth filled
//           hasFlapGate = flap gate indicator
//  Output:  returns flow through an orifice
//  Purpose: computes flow through an orifice as a function of head.
//
{
    double area, q;
    double veloc, hLoss;

    // --- case where orifice is closed
    if ( head == 0.0 || f <= 0.0  )                                            //(5.0.013 - LR)
    {
        Link[j].dqdh = 0.0;
        return 0.0;
    }

    // --- case where inlet depth is below critical depth;
    //     orifice behaves as a weir
    else if ( f < 1.0 )
    {
        q = Orifice[k].cWeir * pow(f, 1.5);
        Link[j].dqdh = 1.5 * q / (f * Orifice[k].hCrit);
    }

    // --- case where normal orifice flow applies
    else
    {
        q = Orifice[k].cOrif * sqrt(head);
        Link[j].dqdh = q / (2.0 * head);
    }

    // --- apply ARMCO adjustment for headloss from flap gate
    if ( hasFlapGate )
    {
        // --- compute velocity for current orifice flow
        area = xsect_getAofY(&Link[j].xsect,
                             Link[j].setting * Link[j].xsect.yFull);
        veloc = q / area;

        // --- compute head loss from gate
        hLoss = (4.0 / GRAVITY) * veloc * veloc *
                 exp(-1.15 * veloc / sqrt(head) );

        // --- update head (for orifice flow) 
        //     or critical depth fraction (for weir flow)
        if ( f < 1.0 )
        {
            f = f - hLoss/Orifice[k].hCrit;
            if ( f < 0.0 ) f = 0.0;
        }
        else
        {
            head = head - hLoss;
            if ( head < 0.0 ) head = 0.0;
        }

        // --- make recursive call to this function, with hasFlapGate
        //     set to false, to find flow values at adjusted head value
        q = orifice_getFlow(j, k, head, f, FALSE);
    }
    return q;
}

//=============================================================================
//                           W E I R   M E T H O D S
//=============================================================================

int   weir_readParams(int j, int k, char* tok[], int ntoks)
//
//  Input:   j = link index
//           k = weir index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads weir parameters from a tokenized line of input.
//
{
    int    m;
    int    n1, n2;
    double x[6];
    char*  id;

    // --- check for valid ID and end node IDs
    if ( ntoks < 6 ) return error_setInpError(ERR_ITEMS, "");
    id = project_findID(LINK, tok[0]);
    if ( id == NULL ) return error_setInpError(ERR_NAME, tok[0]);
    n1 = project_findObject(NODE, tok[1]);
    if ( n1 < 0 ) return error_setInpError(ERR_NAME, tok[1]);
    n2 = project_findObject(NODE, tok[2]);
    if ( n2 < 0 ) return error_setInpError(ERR_NAME, tok[2]);

    // --- parse weir parameters
    m = findmatch(tok[3], WeirTypeWords);
    if ( m < 0 ) return error_setInpError(ERR_KEYWORD, tok[3]);
    x[0] = m;                                              // type
    if ( LinkOffsets == ELEV_OFFSET && *tok[4] == '*' ) x[1] = MISSING;        //(5.0.012 - LR)
    else if ( ! getDouble(tok[4], &x[1]) )                 // height           //(5.0.012 - LR)
        return error_setInpError(ERR_NUMBER, tok[4]);
    if ( ! getDouble(tok[5], &x[2]) || x[2] < 0.0 )        // cDisch1
        return error_setInpError(ERR_NUMBER, tok[5]);
    x[3] = 0.0;
    x[4] = 0.0;
    x[5] = 0.0;
    if ( ntoks >= 7 )
    {
        m = findmatch(tok[6], NoYesWords);             
        if ( m < 0 ) return error_setInpError(ERR_KEYWORD, tok[6]);
        x[3] = m;                                          // flap gate
    }
    if ( ntoks >= 8 )
    {
        if ( ! getDouble(tok[7], &x[4]) || x[4] < 0.0 )     // endCon
            return error_setInpError(ERR_NUMBER, tok[7]);
    }
    if ( ntoks >= 9 )
    {
        if ( ! getDouble(tok[8], &x[5]) || x[5] < 0.0 )     // cDisch2
            return error_setInpError(ERR_NUMBER, tok[8]);
    }

    // --- add parameters to weir object
    Link[j].ID = id;
    link_setParams(j, WEIR, n1, n2, k, x);
    return 0;
}

//=============================================================================

void  weir_validate(int j, int k)
//
//  Input:   j = link index
//           k = weir index
//  Output:  none
//  Purpose: validates a weir's properties
//
{
    int    err = 0;
    double q, q1, q2, head;
 
    // --- check for valid cross section
    switch ( Weir[k].type)
    {
      case TRANSVERSE_WEIR:
      case SIDEFLOW_WEIR:
        if ( Link[j].xsect.type != RECT_OPEN ) err = ERR_REGULATOR_SHAPE;
        Weir[k].slope = 0.0;
        break;
        
      case VNOTCH_WEIR:
        if ( Link[j].xsect.type != TRIANGULAR ) err = ERR_REGULATOR_SHAPE;
        else
        {
            Weir[k].slope = Link[j].xsect.sBot;                                //(5.0.010 - LR)
        }
        break;

      case TRAPEZOIDAL_WEIR:
        if ( Link[j].xsect.type != TRAPEZOIDAL ) err = ERR_REGULATOR_SHAPE;
        else
        {
            Weir[k].slope = Link[j].xsect.sBot;                                //(5.0.010 - LR)
        }
        break;
    }
    if ( err > 0 )
    {
        report_writeErrorMsg(err, Link[j].ID);
        return;
    }

    // --- check for negative offset                                           //(5.0.012 - LR)
    if ( Link[j].offset1 < 0.0 ) Link[j].offset1 = 0.0;                        //(5.0.012 - LR)

    // --- compute an equivalent length
    Weir[k].length = 2.0 * RouteStep * sqrt(GRAVITY * Link[j].xsect.yFull);
    Weir[k].length = MAX(200.0, Weir[k].length);
    Weir[k].surfArea = 0.0;

    // --- find flow through weir when water level equals weir height
    head = Link[j].xsect.yFull;
    weir_getFlow(j, k, head, 1.0, FALSE, &q1, &q2);                            //(5.0.012 - LR)
    q = q1 + q2;

    // --- compute equivalent orifice coeff. (for CFS flow units)
    head = head / 2.0;  // head seen by equivalent orifice
    Weir[k].cSurcharge = q / sqrt(head);                                       //(5.0.012 - LR)
}

//=============================================================================

static void weir_setSetting(int j)                                             //(5.0.011 - LR)
//                                                                             //(5.0.011 - LR)
//  Input:   j = link index                                                    //(5.0.011 - LR)
//  Output:  none                                                              //(5.0.011 - LR)
//  Purpose: updates a weir's setting as a result of a control action.         //(5.0.011 - LR)
//                                                                             //(5.0.011 - LR)
{                                                                              //(5.0.011 - LR)
    int    k = Link[j].subIndex;                                               //(5.0.011 - LR)
    double h, q, q1, q2;                                                       //(5.0.012 - LR)

    // --- adjust weir setting                                                 //(5.0.011 - LR)
    Link[j].setting = Link[j].targetSetting;                                   //(5.0.011 - LR)
    if ( Link[j].setting == 0.0 ) Weir[k].cSurcharge = 0.0;                    //(5.0.011 - LR)
    else                                                                       //(5.0.011 - LR)
    {                                                                          //(5.0.011 - LR)
        // --- find flow through weir when water level equals weir height      //(5.0.011 - LR)
        h = Link[j].setting * Link[j].xsect.yFull;                             //(5.0.011 - LR)
        weir_getFlow(j, k, h, 1.0, FALSE, &q1, &q2);                           //(5.0.011 - LR)
        q = q1 + q2;                                                           //(5.0.011 - LR)

        // --- compute equivalent orifice coeff. (for CFS flow units)          //(5.0.011 - LR)
        h = h / 2.0;  // head seen by equivalent orifice                       //(5.0.011 - LR)
        Weir[k].cSurcharge = q / sqrt(h);                                      //(5.0.012 - LR)
    }                                                                          //(5.0.011 - LR)
}                                                                              //(5.0.011 - LR)

//=============================================================================

double weir_getInflow(int j)
//
//  Input:   j = link index
//  Output:  returns weir flow rate (cfs)
//  Purpose: finds the flow over a weir.
//
{
    int    n1;          // index of upstream node
    int    n2;          // index of downstream node
    int    k;           // index of weir
    double q1;          // flow through central part of weir (cfs)
    double q2;          // flow through end sections of weir (cfs)
    double head;        // head on weir (ft)
    double h1;          // upstrm nodal head (ft)
    double h2;          // downstrm nodal head (ft)
    double hcrest;      // head at weir crest (ft)
    double hcrown;      // head at weir crown (ft)
    double y;           // water depth in weir (ft)
    double dir;         // direction multiplier
    double ratio;
    double weirPower[] = {1.5,       // transverse weir
                          5./3.,     // side flow weir
                          2.5,       // v-notch weir
                          1.5};      // trapezoidal weir

    n1 = Link[j].node1;
    n2 = Link[j].node2;
    k  = Link[j].subIndex;
    if ( RouteModel == DW )
    {
        h1 = Node[n1].newDepth + Node[n1].invertElev;
        h2 = Node[n2].newDepth + Node[n2].invertElev;
    }
    else
    {
        h1 = Node[n1].newDepth + Node[n1].invertElev;
        h2 = Node[n1].invertElev;
    }
    dir = (h1 > h2) ? +1.0 : -1.0;            

    // --- exchange h1 and h2 for reverse flow
    if ( dir < 0.0 )
    {
        head = h1;
        h1 = h2;
        h2 = head;
    }

    // --- find head of weir's crest and crown
    hcrest = Node[n1].invertElev + Link[j].offset1;                            //(5.0.012 - LR)
    hcrown = hcrest + Link[j].xsect.yFull;

    // --- adjust crest ht. for partially open weir
    hcrest += (1.0 - Link[j].setting) * Link[j].xsect.yFull;

    // --- compute head relative to weir crest
    head = h1 - hcrest;

    // --- return if head is negligible or flap gate closed
    Link[j].dqdh = 0.0;
    if ( head <= FUDGE || hcrest >= hcrown ||
         link_setFlapGate(j, n1, n2, dir) )                                    //(5.0.014 - LR)
    {
        Link[j].newDepth = 0.0;
        Link[j].flowClass = DRY;
        return 0.0;
    }

    // --- determine flow class
    Link[j].flowClass = SUBCRITICAL;
    if ( hcrest > h2 )
    {
        if ( dir == 1.0 ) Link[j].flowClass = DN_CRITICAL;
        else              Link[j].flowClass = UP_CRITICAL;
    }

    // --- compute new equivalent surface area
    y = Link[j].xsect.yFull - (hcrown - MIN(h1, hcrown));
    Weir[k].surfArea = xsect_getWofY(&Link[j].xsect, y) * Weir[k].length;

    // --- if under surcharge condition then use equiv. orifice eqn.
    if ( h1 >= hcrown )
    {
        y = (hcrest + hcrown) / 2.0;                                           //(5.0.010 - LR)
        if ( h2 < y ) head = h1 - y;                                           //(5.0.010 - LR)
        else          head = h1 - h2;                                          //(5.0.010 - LR)
        y = hcrown - hcrest;                                                   //(5.0.012 - LR)
        q1 = weir_getOrificeFlow(j, head, y, Weir[k].cSurcharge);              //(5.0.012 - LR)
        Link[j].newDepth = y;                                                  //(5.0.012 - LR)
        return dir * q1;
    }

    // --- otherwise use weir eqn. to find flows through central (q1)
    //     and end sections (q2) of weir
    weir_getFlow(j, k, head, dir, Link[j].hasFlapGate, &q1, &q2);              //(5.0.012 - LR)

    // --- apply Villemonte eqn. to correct for submergence
    if ( h2 > hcrest )
    {
        ratio = (h2 - hcrest) / (h1 - hcrest);
        q1 *= pow( (1.0 - pow(ratio, weirPower[Weir[k].type])), 0.385);
        if ( q2 > 0.0 )
            q2 *= pow( (1.0 - pow(ratio, weirPower[VNOTCH_WEIR])), 0.385);
    }

    // --- return total flow through weir
    Link[j].newDepth = h1 - hcrest;                                             //(5.0.011 - LR)
    return dir * (q1 + q2);
}

//=============================================================================

void weir_getFlow(int j, int k,  double head, double dir, int hasFlapGate,
                  double* q1, double* q2)
//
//  Input:   j    = link index
//           k    = weir index
//           head = head across weir (ft)
//           dir  = flow direction indicator
//           hasFlapGate = flap gate indicator
//  Output:  q1 = flow through central portion of weir (cfs)
//           q2 = flow through end sections of weir (cfs)
//  Purpose: computes flow over weir given head.
//
{
    double length;
    double h;
    double y;
    double hLoss;
    double area;
    double veloc;
    int    wType;                                                              //(5.0.011 - LR)

    // --- q1 = flow through central portion of weir,
    //     q2 = flow through end sections of trapezoidal weir
    *q1 = 0.0;
    *q2 = 0.0;
    Link[j].dqdh = 0.0;                                                        //(5.0.012 - LR)
    if ( head <= 0.0 ) return;                                                 //(5.0.012 - LR)

    // --- convert weir length & head to original units
    length = Link[j].xsect.wMax * UCF(LENGTH);
    h = head * UCF(LENGTH);

    // --- reduce length when end contractions present
    length -= 0.1 * Weir[k].endCon * h;
    length = MAX(length, 0.0);

    // --- use appropriate formula for weir flow
    wType = Weir[k].type;                                                      //(5.0.011 - LR)
    if ( wType == VNOTCH_WEIR &&                                               //(5.0.011 - LR)
         Link[j].setting < 1.0 ) wType = TRAPEZOIDAL_WEIR;                     //(5.0.011 - LR)
    switch (wType)                                                             //(5.0.011 - LR)
    {
      case TRANSVERSE_WEIR:
        *q1 = Weir[k].cDisch1 * length * pow(h, 1.5);
        break;

      case SIDEFLOW_WEIR:
        // --- weir behaves as a transverse weir under reverse flow
        if ( dir < 0.0 )
            *q1 = Weir[k].cDisch1 * length * pow(h, 1.5);
        else
            *q1 = Weir[k].cDisch1 * length * pow(h, 5./3.);
        break;

      case VNOTCH_WEIR:
        *q1 = Weir[k].cDisch1 * Weir[k].slope * pow(h, 2.5);
        break;

      case TRAPEZOIDAL_WEIR:
        y = (1.0 - Link[j].setting) * Link[j].xsect.yFull;
        length = xsect_getWofY(&Link[j].xsect, y) * UCF(LENGTH);
        length -= 0.1 * Weir[k].endCon * h;                                    //(5.0.010 - LR)
        length = MAX(length, 0.0);                                             //(5.0.010 - LR)
        *q1 = Weir[k].cDisch1 * length * pow(h, 1.5);
        *q2 = Weir[k].cDisch2 * Weir[k].slope * pow(h, 2.5);
    }

    // --- convert CMS flows to CFS
    if ( UnitSystem == SI )
    {
        *q1 /= M3perFT3;
        *q2 /= M3perFT3;
    }

    // --- apply ARMCO adjustment for headloss from flap gate
    if ( hasFlapGate )
    {
        // --- compute flow area & velocity for current weir flow
        area = weir_getOpenArea(j, head);                                      //(5.0.012 - LR)
        veloc = (*q1 + *q2) / area;

        // --- compute headloss and subtract from original head
        hLoss = (4.0 / GRAVITY) * veloc * veloc *
                 exp(-1.15 * veloc / sqrt(head) );
        head = head - hLoss;
        if ( head < 0.0 ) head = 0.0;

        // --- make recursive call to this function, with hasFlapGate
        //     set to false, to find flow values at adjusted head value
        weir_getFlow(j, k, head, dir, FALSE, q1, q2);
    }
    Link[j].dqdh = weir_getdqdh(k, dir, head, *q1, *q2);                       //(5.0.012 - LR)
}

//=============================================================================

////  New function added to compute flow thru surcharged weir. ////            //(5.0.012 - LR)

double weir_getOrificeFlow(int j, double head, double y, double cOrif)
//
//  Input:   j = link index
//           head = head across weir (ft)
//           y = height of upstream water level above weir crest (ft)
//           cOrif = orifice flow coefficient
//  Output:  returns flow through weir
//  Purpose: finds flow through a surcharged weir using the orifice equation.
//
{
    double a, q, v, hloss;

    // --- evaluate the orifice flow equation
    q = cOrif * sqrt(head);

    // --- apply Armco adjustment if weir has a flap gate
    if ( Link[j].hasFlapGate )
    {
        a = weir_getOpenArea(j, y);
        if ( a > 0.0 )
        {
            v = q / a;
            hloss = (4.0 / GRAVITY) * v * v * exp(-1.15 * v / sqrt(y) );
            head -= hloss;
            head = MAX(head, 0.0);
            q = cOrif * sqrt(head);
        }
    }
    if ( head > 0.0 ) Link[j].dqdh = q / (2.0 * head);
    else Link[j].dqdh = 0.0;
    return q;
}


//=============================================================================

////  Function generalized to apply to all water depths.  ////                 //(5.0.012 - LR)

double weir_getOpenArea(int j, double y)
//
//  Input:   j = link index
//           y = depth of water above weir crest (ft)
//  Output:  returns area between weir crest and y (ft2)
//  Purpose: finds flow area through a weir.
//
{
    double z;

    // --- find offset of weir crest due to control setting
    z = (1.0 - Link[j].setting) * Link[j].xsect.yFull;

    // --- return difference between area of offset + water depth
    //     and area of just the offset
    return xsect_getAofY(&Link[j].xsect, z+y) -
           xsect_getAofY(&Link[j].xsect, z);
}

//=============================================================================

double  weir_getdqdh(int k, double dir, double h, double q1, double q2)
{
    double q1h;
    double q2h;

    if ( fabs(h) < FUDGE ) return 0.0;
    q1h = fabs(q1/h);
    q2h = fabs(q2/h);

    switch (Weir[k].type)
    {
      case TRANSVERSE_WEIR: return 1.5 * q1h;

      case SIDEFLOW_WEIR:
        // --- weir behaves as a transverse weir under reverse flow
        if ( dir < 0.0 ) return 1.5 * q1h;
        else return 5./3. * q1h;

      case VNOTCH_WEIR: 
        if ( q2h == 0.0 ) return 2.5 * q1h;  // Fully open                     //(5.0.011 - LR)
        else return 1.5 * q1h + 2.5 * q2h;   // Partly open                    //(5.0.011 - LR)

      case TRAPEZOIDAL_WEIR: return 1.5 * q1h + 2.5 * q2h;
    }
    return 0.0;
}
 

//=============================================================================
//               O U T L E T    D E V I C E    M E T H O D S
//=============================================================================

int outlet_readParams(int j, int k, char* tok[], int ntoks)
//
//  Input:   j = link index
//           k = outlet index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads outlet parameters from a tokenized  line of input.
//
{
    int    i, m, n;
    int    n1, n2;
    double x[6];                                                               //(5.0.014 - LR)
    char*  id;
    char*  s;                                                                  //(5.0.014 - LR)

    // --- check for valid ID and end node IDs
    if ( ntoks < 6 ) return error_setInpError(ERR_ITEMS, "");
    id = project_findID(LINK, tok[0]);
    if ( id == NULL ) return error_setInpError(ERR_NAME, tok[0]);
    n1 = project_findObject(NODE, tok[1]);
    if ( n1 < 0 ) return error_setInpError(ERR_NAME, tok[1]);
    n2 = project_findObject(NODE, tok[2]);
    if ( n2 < 0 ) return error_setInpError(ERR_NAME, tok[2]);

////   Modified for possibility of elevation offsets   ////                    //(5.0.012 - LR)
    // --- get height above invert
    if ( LinkOffsets == ELEV_OFFSET && *tok[3] == '*' ) x[0] = MISSING;
    else
	{
		if ( ! getDouble(tok[3], &x[0]) )
            return error_setInpError(ERR_NUMBER, tok[3]);
		if ( LinkOffsets == DEPTH_OFFSET && x[0] < 0.0 ) x[0] = 0.0;
	}
///////////////////////////////////////////////////////////

    // --- see if outlet flow relation is tabular or functional
    m = findmatch(tok[4], RelationWords);
    if ( m < 0 ) return error_setInpError(ERR_KEYWORD, tok[4]);
    x[1] = 0.0;
    x[2] = 0.0;
    x[3] = -1.0;
    x[4] = 0.0;

////  This code segment was added for release 5.0.014  ////                    //(5.0.014 - LR)
    // --- see if rating curve is head or depth based
    x[5] = NODE_DEPTH;                                //default is depth-based
    s = strtok(tok[4], "/");                          //parse token for
    s = strtok(NULL, "/");                            //  qualifier term
    if ( strcomp(s, w_HEAD) ) x[5] = NODE_HEAD;       //check if its "HEAD"

    // --- get params. for functional outlet device
    if ( m == FUNCTIONAL )
    {
        if ( ntoks < 7 ) return error_setInpError(ERR_ITEMS, "");
        if ( ! getDouble(tok[5], &x[1]) )
            return error_setInpError(ERR_NUMBER, tok[5]);
        if ( ! getDouble(tok[6], &x[2]) )
            return error_setInpError(ERR_NUMBER, tok[6]);
        n = 7;
    }

    // --- get name of outlet rating curve
    else
    {
        i = project_findObject(CURVE, tok[5]);
        if ( i < 0 ) return error_setInpError(ERR_NAME, tok[5]);
        x[3] = i;
        n = 6;
    }

    // --- check if flap gate specified
    if ( ntoks > n)
    {
        i = findmatch(tok[n], NoYesWords);               
        if ( i < 0 ) return error_setInpError(ERR_KEYWORD, tok[n]);
        x[4] = i;
    }

    // --- add parameters to outlet object
    Link[j].ID = id;
    link_setParams(j, OUTLET, n1, n2, k, x);
    return 0;
}

//=============================================================================

double outlet_getInflow(int j)
//
//  Input:   j = link index
//  Output:  outlet flow rate (cfs)
//  Purpose: finds the flow through an outlet.
//
{
    int    k, n1, n2;
    double head, hcrest, h1, h2, y1, dir;

    // --- get indexes of end nodes
    n1 = Link[j].node1;
    n2 = Link[j].node2;
    k  = Link[j].subIndex;

    // --- find heads at upstream & downstream nodes
    if ( RouteModel == DW )
    {
        h1 = Node[n1].newDepth + Node[n1].invertElev;
        h2 = Node[n2].newDepth + Node[n2].invertElev;
    }
    else
    {
        h1 = Node[n1].newDepth + Node[n1].invertElev;
        h2 = Node[n1].invertElev;
    }
    dir = (h1 >= h2) ? +1.0 : -1.0; 

    // --- exchange h1 and h2 for reverse flow
    y1 = Node[n1].newDepth;
    if ( dir < 0.0 )
    {
        h1 = h2;
        y1 = Node[n2].newDepth;
    }

    // --- for a NODE_DEPTH rating curve the effective head across the
    //     outlet is the depth above the crest elev. while for a NODE_HEAD
    //     curve it is the difference between upstream & downstream heads
    hcrest = Node[n1].invertElev + Link[j].offset1;                            //(5.0.012 - LR)
    if ( Outlet[k].curveType == NODE_HEAD && RouteModel == DW )                //(5.0.014 - LR)
        head = h1 - MAX(h2, hcrest);                                           //(5.0.014 - LR)
    else head = h1 - hcrest;                                                   //(5.0.014 - LR)

    // --- no flow if either no effective head difference,
    //     no upstream water available, or closed flap gate
    if ( head <= FUDGE || y1 <= FUDGE ||
         link_setFlapGate(j, n1, n2, dir) )                                    //(5.0.014 - LR)
    {
        Link[j].newDepth = 0.0;
        Link[j].flowClass = DRY;
        return 0.0;
    }

    // --- otherwise use rating curve to compute flow
    Link[j].newDepth = head;
    Link[j].flowClass = SUBCRITICAL;
    return dir * Link[j].setting * outlet_getFlow(k, head);
}

//=============================================================================

double outlet_getFlow(int k, double head)
//
//  Input:   k    = outlet index
//           head = head across outlet (ft)
//  Output:  returns outlet flow rate (cfs)
//  Purpose: computes flow rate through an outlet given head.
//
{
    int    m;
    double h;

    // --- convert head to original units
    h = head * UCF(LENGTH);

    // --- look-up flow in rating curve table if provided
    m = Outlet[k].qCurve;
    if ( m >= 0 ) return table_lookup(&Curve[m], h) / UCF(FLOW);
    
    // --- otherwise use function to find flow
    else return Outlet[k].qCoeff * pow(h, Outlet[k].qExpon) / UCF(FLOW);
}
