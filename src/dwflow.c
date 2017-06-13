//-----------------------------------------------------------------------------
//   dwflow.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//             03/19/15   (Build 5.1.008)
//             03/14/17   (Build 5.1.012)
//   Author:   L. Rossman (EPA)
//             M. Tryby (EPA)
//             R. Dickinson (CDM)
//
//   Solves the momentum equation for flow in a conduit under dynamic wave
//   flow routing.
//
//   Build 5.1.008:
//   - Bug in finding if conduit was upstrm/dnstrm full was fixed.
//
//   Build 5.1.012:
//   - Modified uniform loss rate term of conduit momentum equation.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include "headers.h"
#include <math.h>

static const  double MAXVELOCITY =  50.;     // max. allowable velocity (ft/sec)

static int    getFlowClass(int link, double q, double h1, double h2,
              double y1, double y2, double* criticalDepth, double* normalDepth,
              double* fasnh);
static void   findSurfArea(int link, double q, double length, double* h1,
              double* h2, double* y1, double* y2);
static double findLocalLosses(int link, double a1, double a2, double aMid,
              double q);

static double getWidth(TXsect* xsect, double y);
static double getArea(TXsect* xsect, double y);
static double getHydRad(TXsect* xsect, double y);

static double checkNormalFlow(int j, double q, double y1, double y2,
              double a1, double r1);

//=============================================================================

void  dwflow_findConduitFlow(int j, int steps, double omega, double dt)
//
//  Input:   j        = link index
//           steps    = number of iteration steps taken
//           omega    = under-relaxation parameter
//           dt       = time step (sec)
//  Output:  returns new flow value (cfs)
//  Purpose: updates flow in conduit link by solving finite difference
//           form of continuity and momentum equations.
//
{
    int    k;                          // index of conduit
    int    n1, n2;                     // indexes of end nodes
    double z1, z2;                     // upstream/downstream invert elev. (ft)
    double h1, h2;                     // upstream/dounstream flow heads (ft)
    double y1, y2;                     // upstream/downstream flow depths (ft)
    double a1, a2;                     // upstream/downstream flow areas (ft2)
    double r1;                         // upstream hyd. radius (ft)
    double yMid, rMid, aMid;           // mid-stream or avg. values of y, r, & a
    double aWtd, rWtd;                 // upstream weighted area & hyd. radius
    double qLast;                      // flow from previous iteration (cfs)
    double qOld;                       // flow from previous time step (cfs)
    double aOld;                       // area from previous time step (ft2)
    double v;                          // velocity (ft/sec)
    double rho;                        // upstream weighting factor
    double sigma;                      // inertial damping factor
    double length;                     // effective conduit length (ft)
    double dq1, dq2, dq3, dq4, dq5,    // terms in momentum eqn.
           dq6;                        // term for evap and infil losses
    double denom;                      // denominator of flow update formula
    double q;                          // new flow value (cfs)
    double barrels;                    // number of barrels in conduit
    TXsect* xsect = &Link[j].xsect;    // ptr. to conduit's cross section data
    char   isFull = FALSE;             // TRUE if conduit flowing full
    char   isClosed = FALSE;           // TRUE if conduit closed



    // --- adjust isClosed status by any control action
    if ( Link[j].setting == 0 ) isClosed = TRUE;

    // --- get flow from last time step & previous iteration
    k =  Link[j].subIndex;
    barrels = Conduit[k].barrels;
    qOld = Link[j].oldFlow / barrels;
    qLast = Conduit[k].q1;

    // --- get most current heads at upstream and downstream ends of conduit
    n1 = Link[j].node1;
    n2 = Link[j].node2;
    z1 = Node[n1].invertElev + Link[j].offset1;
    z2 = Node[n2].invertElev + Link[j].offset2;
    h1 = Node[n1].newDepth + Node[n1].invertElev;
    h2 = Node[n2].newDepth + Node[n2].invertElev;
    h1 = MAX(h1, z1);
    h2 = MAX(h2, z2);

    // --- get unadjusted upstream and downstream flow depths in conduit
    //    (flow depth = head in conduit - elev. of conduit invert)
    y1 = h1 - z1;
    y2 = h2 - z2;
    y1 = MAX(y1, FUDGE);
    y2 = MAX(y2, FUDGE);

    // --- flow depths can't exceed full depth of conduit
    y1 = MIN(y1, xsect->yFull);
    y2 = MIN(y2, xsect->yFull);

    // -- get area from solution at previous time step
    aOld = Conduit[k].a2;
    aOld = MAX(aOld, FUDGE);

    // --- use Courant-modified length instead of conduit's actual length
    length = Conduit[k].modLength;

    // --- find surface area contributions to upstream and downstream nodes
    //     based on previous iteration's flow estimate
    findSurfArea(j, qLast, length, &h1, &h2, &y1, &y2);

    // --- compute area at each end of conduit & hyd. radius at upstream end
    a1 = getArea(xsect, y1);
    a2 = getArea(xsect, y2);
    r1 = getHydRad(xsect, y1);

    // --- compute area & hyd. radius at midpoint
    yMid = 0.5 * (y1 + y2);
    aMid = getArea(xsect, yMid);
    rMid = getHydRad(xsect, yMid);

    // --- alternate approach not currently used, but might produce better
    //     Bernoulli energy balance for steady flows
    //aMid = (a1+a2)/2.0;
    //rMid = (r1+getHydRad(xsect,y2))/2.0;

    // --- check if conduit is flowing full
    if ( y1 >= xsect->yFull &&
         y2 >= xsect->yFull) isFull = TRUE;

    // --- set new flow to zero if conduit is dry or if flap gate is closed
    if ( Link[j].flowClass == DRY ||
         Link[j].flowClass == UP_DRY ||
         Link[j].flowClass == DN_DRY ||
         isClosed ||
         aMid <= FUDGE )
    {
        Conduit[k].a1 = 0.5 * (a1 + a2);
        Conduit[k].q1 = 0.0;;
        Conduit[k].q2 = 0.0;
        Link[j].dqdh  = GRAVITY * dt * aMid / length * barrels;
        Link[j].froude = 0.0;
        Link[j].newDepth = MIN(yMid, Link[j].xsect.yFull);
        Link[j].newVolume = Conduit[k].a1 * link_getLength(j) * barrels;
        Link[j].newFlow = 0.0;
        return;
    }

    // --- compute velocity from last flow estimate
    v = qLast / aMid;
    if ( fabs(v) > MAXVELOCITY )  v = MAXVELOCITY * SGN(qLast);

    // --- compute Froude No.
    Link[j].froude = link_getFroude(j, v, yMid);
    if ( Link[j].flowClass == SUBCRITICAL &&
         Link[j].froude > 1.0 ) Link[j].flowClass = SUPCRITICAL;

    // --- find inertial damping factor (sigma)
    if      ( Link[j].froude <= 0.5 ) sigma = 1.0;
    else if ( Link[j].froude >= 1.0 ) sigma = 0.0;
    else    sigma = 2.0 * (1.0 - Link[j].froude);

    // --- get upstream-weighted area & hyd. radius based on damping factor
    //     (modified version of R. Dickinson's slope weighting)
    rho = 1.0;
    if ( !isFull && qLast > 0.0 && h1 >= h2 ) rho = sigma;
    aWtd = a1 + (aMid - a1) * rho;
    rWtd = r1 + (rMid - r1) * rho;

    // --- determine how much inertial damping to apply
    if      ( InertDamping == NO_DAMPING )   sigma = 1.0;
    else if ( InertDamping == FULL_DAMPING ) sigma = 0.0;

    // --- use full inertial damping if closed conduit is surcharged
    if ( isFull && !xsect_isOpen(xsect->type) ) sigma = 0.0;

    // --- compute terms of momentum eqn.:
    // --- 1. friction slope term
    if ( xsect->type == FORCE_MAIN && isFull )
         dq1 = dt * forcemain_getFricSlope(j, fabs(v), rMid);
    else dq1 = dt * Conduit[k].roughFactor / pow(rWtd, 1.33333) * fabs(v);

    // --- 2. energy slope term
    dq2 = dt * GRAVITY * aWtd * (h2 - h1) / length;

    // --- 3 & 4. inertial terms
    dq3 = 0.0;
    dq4 = 0.0;
    if ( sigma > 0.0 )
    {
        dq3 = 2.0 * v * (aMid - aOld) * sigma;
        dq4 = dt * v * v * (a2 - a1) / length * sigma;
    }

    // --- 5. local losses term
    dq5 = 0.0;
    if ( Conduit[k].hasLosses )
    {
        dq5 = findLocalLosses(j, a1, a2, aMid, qLast) / 2.0 / length * dt;
    }

    // --- 6. term for evap and seepage losses per unit length
    dq6 = link_getLossRate(j, qOld, dt) * 2.5 * dt * v / link_getLength(j);    //(5.1.012)

    // --- combine terms to find new conduit flow
    denom = 1.0 + dq1 + dq5;
    q = (qOld - dq2 + dq3 + dq4 - dq6) / denom;

    // --- compute derivative of flow w.r.t. head
    Link[j].dqdh = 1.0 / denom  * GRAVITY * dt * aWtd / length * barrels;

    // --- check if any flow limitation applies
    Link[j].inletControl = FALSE;
    Link[j].normalFlow = FALSE;
    if ( q > 0.0 )
    {
        // --- check for inlet controlled culvert flow
        if ( xsect->culvertCode > 0 && !isFull )
            q = culvert_getInflow(j, q, h1);

        // --- check for normal flow limitation based on surface slope & Fr
        else
        if ( y1 < Link[j].xsect.yFull &&
               ( Link[j].flowClass == SUBCRITICAL ||
                 Link[j].flowClass == SUPCRITICAL )
           ) q = checkNormalFlow(j, q, y1, y2, a1, r1);
    }

    // --- apply under-relaxation weighting between new & old flows;
    // --- do not allow change in flow direction without first being zero
    if ( steps > 0 )
    {
        q = (1.0 - omega) * qLast + omega * q;
        if ( q * qLast < 0.0 ) q = 0.001 * SGN(q);
    }

    // --- check if user-supplied flow limit applies
    if ( Link[j].qLimit > 0.0 )
    {
         if ( fabs(q) > Link[j].qLimit ) q = SGN(q) * Link[j].qLimit;
    }

    // --- check for reverse flow with closed flap gate
    if ( link_setFlapGate(j, n1, n2, q) ) q = 0.0;

    // --- do not allow flow out of a dry node
    //     (as suggested by R. Dickinson)
    if( q >  FUDGE && Node[n1].newDepth <= FUDGE ) q =  FUDGE;
    if( q < -FUDGE && Node[n2].newDepth <= FUDGE ) q = -FUDGE;

    // --- save new values of area, flow, depth, & volume
    Conduit[k].a1 = aMid;
    Conduit[k].q1 = q;
    Conduit[k].q2 = q;
    Link[j].newDepth  = MIN(yMid, xsect->yFull);
    aMid = (a1 + a2) / 2.0;
    aMid = MIN(aMid, xsect->aFull);
    Conduit[k].fullState = link_getFullState(a1, a2, xsect->aFull);            //(5.1.008)
    Link[j].newVolume = aMid * link_getLength(j) * barrels;
    Link[j].newFlow = q * barrels;
}

//=============================================================================

int getFlowClass(int j, double q, double h1, double h2, double y1, double y2,
	double *yC, double *yN, double* fasnh)
//
//  Input:   j  = conduit link index
//           q  = current conduit flow (cfs)
//           h1 = head at upstream end of conduit (ft)
//           h2 = head at downstream end of conduit (ft)
//           y1 = upstream flow depth in conduit (ft)
//           y2 = downstream flow depth in conduit (ft)
//           yC = critical flow depth (ft)
//           yN = normal flow depth (ft)
//           fasnh = fraction between norm. & crit. depth
//  Output:  returns flow classification code
//  Purpose: determines flow class for a conduit based on depths at each end.
//
{
    int    n1, n2;                     // indexes of upstrm/downstrm nodes
    int    flowClass;                  // flow classification code
    double ycMin, ycMax;               // min/max critical depths (ft)
    double z1, z2;                     // offsets of conduit inverts (ft)

    // --- get upstream & downstream node indexes
    n1 = Link[j].node1;
    n2 = Link[j].node2;

    // --- get upstream & downstream conduit invert offsets
    z1 = Link[j].offset1;
    z2 = Link[j].offset2;

    // --- base offset of an outfall conduit on outfall's depth
    if ( Node[n1].type == OUTFALL ) z1 = MAX(0.0, (z1 - Node[n1].newDepth));
    if ( Node[n2].type == OUTFALL ) z2 = MAX(0.0, (z2 - Node[n2].newDepth));

    // --- default class is SUBCRITICAL
    flowClass = SUBCRITICAL;
    *fasnh = 1.0;

    // --- case where both ends of conduit are wet
    if ( y1 > FUDGE && y2 > FUDGE )
    {
        if ( q < 0.0 )
        {
            // --- upstream end at critical depth if flow depth is
            //     below conduit's critical depth and an upstream
            //     conduit offset exists
            if ( z1 > 0.0 )
            {
                *yN = link_getYnorm(j, fabs(q));
                *yC = link_getYcrit(j, fabs(q));
                ycMin = MIN(*yN, *yC);
                if ( y1 < ycMin ) flowClass = UP_CRITICAL;
            }
        }

        // --- case of normal direction flow
        else
        {
            // --- downstream end at smaller of critical and normal depth
            //     if downstream flow depth below this and a downstream
            //     conduit offset exists
            if ( z2 > 0.0 )
            {
                *yN = link_getYnorm(j, fabs(q));
                *yC = link_getYcrit(j, fabs(q));
                ycMin = MIN(*yN, *yC);
                ycMax = MAX(*yN, *yC);
                if ( y2 < ycMin ) flowClass = DN_CRITICAL;
                else if ( y2 < ycMax )
                {
                    if ( ycMax - ycMin < FUDGE ) *fasnh = 0.0;
                    else *fasnh = (ycMax - y2) / (ycMax - ycMin);
                }
            }
        }
    }

    // --- case where no flow at either end of conduit
    else if ( y1 <= FUDGE && y2 <= FUDGE ) flowClass = DRY;

    // --- case where downstream end of pipe is wet, upstream dry
    else if ( y2 > FUDGE )
    {
        // --- flow classification is UP_DRY if downstream head <
        //     invert of upstream end of conduit
        if ( h2 < Node[n1].invertElev + Link[j].offset1 ) flowClass = UP_DRY;

        // --- otherwise, the downstream head will be >= upstream
        //     conduit invert creating a flow reversal and upstream end
        //     should be at critical depth, providing that an upstream
        //     offset exists (otherwise subcritical condition is maintained)
        else if ( z1 > 0.0 )
        {
            *yN = link_getYnorm(j, fabs(q));
            *yC = link_getYcrit(j, fabs(q));
            flowClass = UP_CRITICAL;
        }
    }

    // --- case where upstream end of pipe is wet, downstream dry
    else
    {
        // --- flow classification is DN_DRY if upstream head <
        //     invert of downstream end of conduit
        if ( h1 < Node[n2].invertElev + Link[j].offset2 ) flowClass = DN_DRY;

        // --- otherwise flow at downstream end should be at critical depth
        //     providing that a downstream offset exists (otherwise
        //     subcritical condition is maintained)
        else if ( z2 > 0.0 )
        {
            *yN = link_getYnorm(j, fabs(q));
            *yC = link_getYcrit(j, fabs(q));
            flowClass = DN_CRITICAL;
        }
    }
    return flowClass;
}

//=============================================================================

void findSurfArea(int j, double q, double length, double* h1, double* h2,
                  double* y1, double* y2)
//
//  Input:   j  = conduit link index
//           q  = current conduit flow (cfs)
//           length = conduit length (ft)
//           h1 = head at upstream end of conduit (ft)
//           h2 = head at downstream end of conduit (ft)
//           y1 = upstream flow depth (ft)
//           y2 = downstream flow depth (ft)
//  Output:  updated values of h1, h2, y1, & y2;
//  Purpose: assigns surface area of conduit to its up and downstream nodes.
//
{
    int     n1, n2;                    // indexes of upstrm/downstrm nodes
    double  flowDepth1;                // flow depth at upstrm end (ft)
    double  flowDepth2;                // flow depth at downstrm end (ft)
    double  flowDepthMid;              // flow depth at midpt. (ft)
    double  width1;                    // top width at upstrm end (ft)
    double  width2;                    // top width at downstrm end (ft)
    double  widthMid;                  // top width at midpt. (ft)
    double  surfArea1 = 0.0;           // surface area at upstream node (ft2)
    double  surfArea2 = 0.0;           // surface area st downstrm node (ft2)
    double  criticalDepth;             // critical flow depth (ft)
    double  normalDepth;               // normal flow depth (ft)
    double  fasnh;                     // fraction between norm. & crit. depth
    TXsect* xsect = &Link[j].xsect;    // pointer to cross-section data

    // --- get node indexes & current flow depths
    n1 = Link[j].node1;
    n2 = Link[j].node2;
    flowDepth1 = *y1;
    flowDepth2 = *y2;

    normalDepth = (flowDepth1 + flowDepth2) / 2.0;
    criticalDepth = normalDepth;

    // --- find conduit's flow classification
    Link[j].flowClass = getFlowClass(j, q, *h1, *h2, *y1, *y2,
	                    &criticalDepth, &normalDepth, &fasnh);

    // --- add conduit's surface area to its end nodes depending on flow class
    switch ( Link[j].flowClass )
    {
      case SUBCRITICAL:
        flowDepthMid = 0.5 * (flowDepth1 + flowDepth2);
        if ( flowDepthMid < FUDGE ) flowDepthMid = FUDGE;
        width1 =   getWidth(xsect, flowDepth1);
        width2 =   getWidth(xsect, flowDepth2);
        widthMid = getWidth(xsect, flowDepthMid);
        surfArea1 = (width1 + widthMid) * length / 4.;
        surfArea2 = (widthMid + width2) * length / 4. * fasnh;
        break;

      case UP_CRITICAL:
        flowDepth1 = criticalDepth;
        if ( normalDepth < criticalDepth ) flowDepth1 = normalDepth;
        flowDepth1 = MAX(flowDepth1, FUDGE);
        *h1 = Node[n1].invertElev + Link[j].offset1 + flowDepth1;
        flowDepthMid = 0.5 * (flowDepth1 + flowDepth2);
        if ( flowDepthMid < FUDGE ) flowDepthMid = FUDGE;
        width2   = getWidth(xsect, flowDepth2);
        widthMid = getWidth(xsect, flowDepthMid);
        surfArea2 = (widthMid + width2) * length * 0.5;
        break;

      case DN_CRITICAL:
        flowDepth2 = criticalDepth;
        if ( normalDepth < criticalDepth ) flowDepth2 = normalDepth;
        flowDepth2 = MAX(flowDepth2, FUDGE);
        *h2 = Node[n2].invertElev + Link[j].offset2 + flowDepth2;
        width1 = getWidth(xsect, flowDepth1);
        flowDepthMid = 0.5 * (flowDepth1 + flowDepth2);
        if ( flowDepthMid < FUDGE ) flowDepthMid = FUDGE;
        widthMid = getWidth(xsect, flowDepthMid);
        surfArea1 = (width1 + widthMid) * length * 0.5;
        break;

      case UP_DRY:
        flowDepth1 = FUDGE;
        flowDepthMid = 0.5 * (flowDepth1 + flowDepth2);
        if ( flowDepthMid < FUDGE ) flowDepthMid = FUDGE;
        width1 = getWidth(xsect, flowDepth1);
        width2 = getWidth(xsect, flowDepth2);
        widthMid = getWidth(xsect, flowDepthMid);

        // --- assign avg. surface area of downstream half of conduit
        //     to the downstream node
        surfArea2 = (widthMid + width2) * length / 4.;

        // --- if there is no free-fall at upstream end, assign the
        //     upstream node the avg. surface area of the upstream half
        if ( Link[j].offset1 <= 0.0 )
        {
            surfArea1 = (width1 + widthMid) * length / 4.;
        }
        break;

      case DN_DRY:
        flowDepth2 = FUDGE;
        flowDepthMid = 0.5 * (flowDepth1 + flowDepth2);
        if ( flowDepthMid < FUDGE ) flowDepthMid = FUDGE;
        width1 = getWidth(xsect, flowDepth1);
        width2 = getWidth(xsect, flowDepth2);
        widthMid = getWidth(xsect, flowDepthMid);

        // --- assign avg. surface area of upstream half of conduit
        //     to the upstream node
        surfArea1 = (widthMid + width1) * length / 4.;

        // --- if there is no free-fall at downstream end, assign the
        //     downstream node the avg. surface area of the downstream half
        if ( Link[j].offset2 <= 0.0 )
        {
            surfArea2 = (width2 + widthMid) * length / 4.;
        }
        break;

      case DRY:
        surfArea1 = FUDGE * length / 2.0;
        surfArea2 = surfArea1;
        break;
    }
    Link[j].surfArea1 = surfArea1;
    Link[j].surfArea2 = surfArea2;
    *y1 = flowDepth1;
    *y2 = flowDepth2;
}

//=============================================================================

double findLocalLosses(int j, double a1, double a2, double aMid, double q)
//
//  Input:   j    = link index
//           a1   = upstream area (ft2)
//           a2   = downstream area (ft2)
//           aMid = midpoint area (ft2)
//           q    = flow rate (cfs)
//  Output:  returns local losses (ft/sec)
//  Purpose: computes local losses term of momentum equation.
//
{
    double losses = 0.0;
    q = fabs(q);
    if ( a1 > FUDGE ) losses += Link[j].cLossInlet  * (q/a1);
    if ( a2 > FUDGE ) losses += Link[j].cLossOutlet * (q/a2);
    if ( aMid  > FUDGE ) losses += Link[j].cLossAvg * (q/aMid);
    return losses;
}

//=============================================================================

double getWidth(TXsect* xsect, double y)
//
//  Input:   xsect = ptr. to conduit cross section
//           y     = flow depth (ft)
//  Output:  returns top width (ft)
//  Purpose: computes top width of flow surface in conduit.
//
{
    double yNorm = y/xsect->yFull;
    if ( yNorm > 0.96 &&
         !xsect_isOpen(xsect->type) ) y = 0.96*xsect->yFull;
    return xsect_getWofY(xsect, y);
}

//=============================================================================

double getArea(TXsect* xsect, double y)
//
//  Input:   xsect = ptr. to conduit cross section
//           y     = flow depth (ft)
//  Output:  returns flow area (ft2)
//  Purpose: computes area of flow cross-section in a conduit.
//
{
    double area;                        // flow area (ft2)
    y = MIN(y, xsect->yFull);
    area = xsect_getAofY(xsect, y);
    return area;
}

//=============================================================================

double getHydRad(TXsect* xsect, double y)
//
//  Input:   xsect = ptr. to conduit cross section
//           y     = flow depth (ft)
//  Output:  returns hydraulic radius (ft)
//  Purpose: computes hydraulic radius of flow cross-section in a conduit.
//
{
    double hRadius;                     // hyd. radius (ft)
    y = MIN(y, xsect->yFull);
    hRadius = xsect_getRofY(xsect, y);
    return hRadius;
}

//=============================================================================

double checkNormalFlow(int j, double q, double y1, double y2, double a1,
                       double r1)
//
//  Input:   j = link index
//           q = link flow found from dynamic wave equations (cfs)
//           y1 = flow depth at upstream end (ft)
//           y2 = flow depth at downstream end (ft)
//           a1 = flow area at upstream end (ft2)
//           r1 = hyd. radius at upstream end (ft)
//  Output:  returns modifed flow in link (cfs)
//  Purpose: checks if flow in link should be replaced by normal flow.
//
{
    int    check  = FALSE;
    int    k = Link[j].subIndex;
    int    n1 = Link[j].node1;
    int    n2 = Link[j].node2;
    int    hasOutfall = (Node[n1].type == OUTFALL || Node[n2].type == OUTFALL);
    double qNorm;
    double f1;

    // --- check if water surface slope < conduit slope
    if ( NormalFlowLtd == SLOPE || NormalFlowLtd == BOTH || hasOutfall )
    {
        if ( y1 < y2 ) check = TRUE;
    }

    // --- check if Fr >= 1.0 at upstream end of conduit
    if ( !check && (NormalFlowLtd == FROUDE || NormalFlowLtd == BOTH) &&
         !hasOutfall )
    {
        if ( y1 > FUDGE && y2 > FUDGE )
        {
            f1 = link_getFroude(j, q/a1, y1);
            if ( f1 >= 1.0 ) check = TRUE;
        }
    }

    // --- check if normal flow < dynamic flow
    if ( check )
    {
        qNorm = Conduit[k].beta * a1 * pow(r1, 2./3.);
        if ( qNorm < q )
        {
            Link[j].normalFlow = TRUE;
            return qNorm;
        }
    }
    return q;
}
