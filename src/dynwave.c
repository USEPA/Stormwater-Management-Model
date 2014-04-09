//-----------------------------------------------------------------------------
//   dynwave.c
//
//   Project:  EPA SWMM5
//   Version:  5.0
//   Date:     3/11/08  (5.0.013)
//             1/21/09  (5.0.014)
//             4/10/09  (5.0.015)
//             6/22/09  (5.0.016)
//             10/7/09  (5.0.017)
//             11/18/09 (5.0.018)
//             07/30/10 (5.0.019)
//             04/20/11 (5.0.022)
//   Author:   L. Rossman
//             R. Dickinson
//
//   Dynamic wave flow routing functions.
//
//   This module solves the dynamic wave flow routing equations using
//   Picard Iterations (i.e., a method of successive approximations)
//   to solve the explicit form of the continuity and momentum equations
//   for conduits.
//
//   All previous change comments prior to release 5.0.013 were removed
//   to improve readability.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <malloc.h>
#include <math.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//     Constants 
//-----------------------------------------------------------------------------
static const double MINSURFAREA =  12.566;  // min. nodal surface area (~4 ft diam.)
static const double MAXVELOCITY =  50.;     // max. allowable velocity (ft/sec)
static const double MINTIMESTEP =  0.5;     // min. time step (sec)
static const double OMEGA       =  0.5;     // under-relaxation parameter
static const double STOP_TOL    =  0.005;   // Picard iteration stop criterion
static const int    MAXSTEPS    =  8;       // max. number of Picard iterations (5.0.019 - LR)

//-----------------------------------------------------------------------------
//  Data Structures
//-----------------------------------------------------------------------------
typedef struct 
{
    char    converged;                 // TRUE if iterations for a node done
    double  newSurfArea;               // current surface area (ft2)
    double  oldSurfArea;               // previous surface area (ft2)
    double  sumdqdh;                   // sum of dqdh from adjoining links
    double  dYdT;                      // change in depth w.r.t. time (ft/sec)
} TXnode;

typedef struct
{
    char    bypassed;                  // TRUE if can bypass calcs. for a link
    double  surfArea1;                 // surf. area at upstrm end of link (ft2)
    double  surfArea2;                 // surf. area at dnstrm end of link (ft2)
} TXlink;

//-----------------------------------------------------------------------------
//  Shared Variables
//-----------------------------------------------------------------------------
static double  MinSurfAreaFt2;         // actual min. nodal surface area (ft2)
static double  VariableStep;           // size of variable time step (sec)
static double  Omega;                  // actual under-relaxation parameter
static double  CriticalDepth;          // critical flow depth (ft)
static double  NormalDepth;            // normal flow depth (ft)
static double  Fasnh;                  // fraction between norm. & crit. depth
static int     Converged;              // TRUE if Picard iterations converged
static int     Steps;                  // number of Picard iterations
static TXnode* Xnode;
static TXlink* Xlink;

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  dynwave_init            (called by flowrout_init)
//  dynwave_getRoutingStep  (called by flowrout_getRoutingStep)
//  dynwave_execute         (called by flowrout_execute)

//-----------------------------------------------------------------------------
//  Function declarations
//-----------------------------------------------------------------------------
static void   execRoutingStep(int links[], double dt);
static void   initNodeState(int i);
static void   findConduitFlow(int i, double dt);
static void   findNonConduitFlow(int i, double dt);
static double getModPumpFlow(int i, double q, double dt);
static void   updateNodeFlows(int i, double q);

static double getConduitFlow(int link, double qin, double dt);
static int    getFlowClass(int link, double q, double h1, double h2,
              double y1, double y2);
static void   findSurfArea(int link, double length, double* h1, double* h2,
              double* y1, double* y2);
static double findLocalLosses(int link, double a1, double a2, double aMid,
              double q);
static void   findNonConduitSurfArea(int link);

static double getWidth(TXsect* xsect, double y);
static double getArea(TXsect* xsect, double y);
static double getHydRad(TXsect* xsect, double y);
static double checkNormalFlow(int j, double q, double y1, double y2,           //(5.0.019 - LR)
              double a1, double r1);                                           //(5.0.019 - LR)

static void   setNodeDepth(int node, double dt);
static double getFloodedDepth(int i, int canPond, double dV, double yNew,      //(5.0.014 - LR)
              double yMax, double dt);                                         //(5.0.014 - LR)

static double getVariableStep(double maxStep);
static double getLinkStep(double tMin, int *minLink);
static double getNodeStep(double tMin, int *minNode);

static void   checkCapacity(int j);


//=============================================================================

void dynwave_init()
//
//  Input:   none
//  Output:  none
//  Purpose: initializes dynamic wave routing method.
//
{
    int i;

    VariableStep = 0.0;
    if ( MinSurfArea == 0.0 ) MinSurfAreaFt2 = MINSURFAREA;
    else MinSurfAreaFt2 = MinSurfArea / UCF(LENGTH) / UCF(LENGTH);
    Xnode = (TXnode *) calloc(Nobjects[NODE], sizeof(TXnode));
    Xlink = (TXlink *) calloc(Nobjects[LINK], sizeof(TXlink));

    // --- initialize node surface areas
    for (i = 0; i < Nobjects[NODE]; i++ )
    {
        Xnode[i].newSurfArea = 0.0;
        Xnode[i].oldSurfArea = 0.0;
    }
    for (i = 0; i < Nobjects[LINK]; i++)
    {
        Link[i].flowClass = DRY;
        Link[i].dqdh = 0.0;
    }
}

//=============================================================================

void  dynwave_close()
//
//  Input:   none
//  Output:  none
//  Purpose: frees memory allocated for dynamic wave routing method.
//
{
    FREE(Xnode);
    FREE(Xlink);
}

//=============================================================================

double dynwave_getRoutingStep(double fixedStep)
//
//  Input:   fixedStep = user-supplied fixed time step (sec)
//  Output:  returns routing time step (sec)
//  Purpose: computes variable routing time step if applicable.
//
{
    // --- use user-supplied fixed step if variable step option turned off
    //     or if its smaller than the min. allowable variable time step
    if ( CourantFactor == 0.0 ) return fixedStep;
    if ( fixedStep < MINTIMESTEP ) return fixedStep;

    // --- at start of simulation (when current variable step is zero)
    //     use the minimum allowable time step
    if ( VariableStep == 0.0 )
    {
        VariableStep = MINTIMESTEP;
    }

    // --- otherwise compute variable step based on current flow solution
    else VariableStep = getVariableStep(fixedStep);

    // --- adjust step to be a multiple of a millisecond
    VariableStep = floor(1000.0 * VariableStep) / 1000.0;
    return VariableStep;
}

//=============================================================================

int dynwave_execute(int links[], double tStep)
//
//  Input:   links = array of topo sorted links indexes
//           tStep = time step (sec)
//  Output:  returns number of iterations used
//  Purpose: routes flows through drainage network over current time step.
//
{
    int i;

    // --- initialize
    if ( ErrorCode ) return 0;
    Steps = 0;
    Converged = FALSE;
    Omega = OMEGA;
    for (i=0; i<Nobjects[NODE]; i++)
    {
        Xnode[i].converged = FALSE;
        Xnode[i].dYdT = 0.0;
    }
    for (i=0; i<Nobjects[LINK]; i++)
    {
        Xlink[i].bypassed = FALSE;
        Xlink[i].surfArea1 = 0.0;
        Xlink[i].surfArea2 = 0.0;
    }

    // --- a2 preserves conduit area from solution at last time step
    for ( i=0; i<Nlinks[CONDUIT]; i++) Conduit[i].a2 = Conduit[i].a1;

    // --- keep iterating until convergence 
    while ( Steps < MAXSTEPS )
    {
        // --- execute a routing step & check for nodal convergence
        execRoutingStep(links, tStep);
        Steps++;
        if ( Steps > 1 )
        {
            if ( Converged ) break;

            // --- check if link calculations can be skipped in next step
            for (i=0; i<Nobjects[LINK]; i++)
            {
                if ( Xnode[Link[i].node1].converged &&
                     Xnode[Link[i].node2].converged )
                     Xlink[i].bypassed = TRUE;
                else Xlink[i].bypassed = FALSE;
            }
        }
    }

    //  --- identify any capacity-limited conduits
    for (i=0; i<Nobjects[LINK]; i++) checkCapacity(i);
    return Steps;
}

//=============================================================================

void execRoutingStep(int links[], double dt)
//
//  Input:   links = array of link indexes
//           dt    = time step (sec)
//  Output:  none
//  Purpose: solves momentum eq. in links and continuity eq. at nodes
//           over specified time step.
//
{
    int    i;                          // node or link index
    double yOld;                       // old node depth (ft)

    // --- re-initialize state of each node
    for ( i = 0; i < Nobjects[NODE]; i++ ) initNodeState(i);
    Converged = TRUE;

    // --- find new flows in conduit links and non-conduit links
    for ( i=0; i<Nobjects[LINK]; i++) findConduitFlow(links[i], dt);
    for ( i=0; i<Nobjects[LINK]; i++) findNonConduitFlow(links[i], dt);

    // --- compute outfall depths based on flow in connecting link
    for ( i = 0; i < Nobjects[LINK]; i++ ) link_setOutfallDepth(i);

    // --- compute new depth for all non-outfall nodes and determine if
    //     depth change from previous iteration is below tolerance
    for ( i = 0; i < Nobjects[NODE]; i++ )
    {
        if ( Node[i].type == OUTFALL ) continue;
        yOld = Node[i].newDepth;
        setNodeDepth(i, dt);
        Xnode[i].converged = TRUE;
        if ( fabs(yOld - Node[i].newDepth) > STOP_TOL )
        {
            Converged = FALSE;
            Xnode[i].converged = FALSE;
        }
    }
}

//=============================================================================

void initNodeState(int i)
//
//  Input:   i = node index
//  Output:  none
//  Purpose: initializes node's surface area, inflow & outflow
//
{
    // --- initialize nodal surface area
    if ( AllowPonding )
    {
        Xnode[i].newSurfArea = node_getPondedArea(i, Node[i].newDepth);
    }
    else
    {
        Xnode[i].newSurfArea = node_getSurfArea(i, Node[i].newDepth);
    }
    if ( Xnode[i].newSurfArea < MinSurfAreaFt2 )
    {
        Xnode[i].newSurfArea = MinSurfAreaFt2;
    }

    // --- initialize nodal inflow & outflow
    Node[i].inflow = Node[i].newLatFlow;
    Node[i].outflow = 0.0;
    Xnode[i].sumdqdh = 0.0;
}

//=============================================================================

void findConduitFlow(int i, double dt)
//
//  Input:   i = link index
//           dt = time step (sec)
//  Output:  none
//  Purpose: finds new flow in a conduit-type link
//
{
    double qOld;                       // old link flow (cfs)
    double barrels;                    // number of barrels in conduit

    // --- do nothing if link not a conduit
    if ( Link[i].type != CONDUIT || Link[i].xsect.type == DUMMY) return;

    // --- get link flow from last "full" time step
    qOld = Link[i].oldFlow;

    // --- solve momentum eqn. to update conduit flow
    if ( !Xlink[i].bypassed )
    {
        Link[i].dqdh = 0.0;
        Link[i].newFlow = getConduitFlow(i, qOld, dt);
    }
    // NOTE: if link was bypassed, then its flow and surface area values
    //       from the previous iteration will still be valid.

    // --- add surf. area contributions to upstream/downstream nodes
    barrels = Conduit[Link[i].subIndex].barrels;
    Xnode[Link[i].node1].newSurfArea += Xlink[i].surfArea1 * barrels;
    Xnode[Link[i].node2].newSurfArea += Xlink[i].surfArea2 * barrels;

    // --- update summed value of dqdh at each end node
    Xnode[Link[i].node1].sumdqdh += Link[i].dqdh;
    Xnode[Link[i].node2].sumdqdh += Link[i].dqdh;

    // --- update outflow/inflow at upstream/downstream nodes
    updateNodeFlows(i, Link[i].newFlow);
}

//=============================================================================

void findNonConduitFlow(int i, double dt)
//
//  Input:   i = link index
//           dt = time step (sec)
//  Output:  none
//  Purpose: finds new flow in a non-conduit-type link
//
{
    double qLast;                      // previous link flow (cfs)
    double qNew;                       // new link flow (cfs)
    int    k, m;

    // --- ignore non-dummy conduit links
    if ( Link[i].type == CONDUIT && Link[i].xsect.type != DUMMY ) return;

    // --- update flow in link if not bypassed
    if ( !Xlink[i].bypassed )
    {
        // --- get link flow from last iteration
        qLast = Link[i].newFlow;
        Link[i].dqdh = 0.0;

        // --- get new inflow to link from its upstream node
        //     (link_getInflow returns 0 if flap gate closed or pump is offline)
        qNew = link_getInflow(i);
        if ( Link[i].type == PUMP ) qNew = getModPumpFlow(i, qNew, dt);

        // --- find surface area at each end of link
        findNonConduitSurfArea(i);

        // --- apply under-relaxation with flow from previous iteration;
        // --- do not allow flow to change direction without first being 0
        if ( Steps > 0 && Link[i].type != PUMP )                               //(5.0.014 - LR)
        {
            qNew = (1.0 - Omega) * qLast + Omega * qNew;
            if ( qNew * qLast < 0.0 ) qNew = 0.001 * SGN(qNew);
        }
        Link[i].newFlow = qNew;
    }

    // --- add surf. area contributions to upstream/downstream nodes
    Xnode[Link[i].node1].newSurfArea += Xlink[i].surfArea1;
    Xnode[Link[i].node2].newSurfArea += Xlink[i].surfArea2;

    // --- update summed value of dqdh at each end node
    //     (but not for discharge node of Type 4 pumps)
    Xnode[Link[i].node1].sumdqdh += Link[i].dqdh;
    if ( Link[i].type == PUMP )
    {
        k = Link[i].subIndex;
        m = Pump[k].pumpCurve;
        if ( Curve[m].curveType != PUMP4_CURVE )
            Xnode[Link[i].node2].sumdqdh += Link[i].dqdh;
    }
    else Xnode[Link[i].node2].sumdqdh += Link[i].dqdh;

    // --- update outflow/inflow at upstream/downstream nodes
    updateNodeFlows(i, Link[i].newFlow);
}

//=============================================================================

double getModPumpFlow(int i, double q, double dt)
//
//  Input:   i = link index
//           q = pump flow from pump curve (cfs)
//           dt = time step (sec)
//  Output:  returns modified pump flow rate (cfs)
//  Purpose: modifies pump curve pumping rate depending on amount of water
//           available at pump's inlet node.
//
{
    int    j = Link[i].node1;          // pump's inlet node index
    int    k = Link[i].subIndex;       // pump's index
    double newNetInflow;               // inflow - outflow rate (cfs)
    double netFlowVolume;              // inflow - outflow volume (ft3)
    double y;                          // node depth (ft)

    if ( q == 0.0 ) return q;

    // --- case where inlet node is a storage node: 
    //     prevent node volume from going negative
    if ( Node[j].type == STORAGE ) return node_getMaxOutflow(j, q, dt); 

    // --- case where inlet is a non-storage node
    switch ( Pump[k].type )
    {
      // --- for Type1 pump, a volume is computed for inlet node,
      //     so make sure it doesn't go negative
      case TYPE1_PUMP:
        return node_getMaxOutflow(j, q, dt);

      // --- for other types of pumps, if pumping rate would make depth
      //     at upstream node negative, then set pumping rate = inflow
      case TYPE2_PUMP:
      case TYPE4_PUMP:
      case TYPE3_PUMP:
         newNetInflow = Node[j].inflow - Node[j].outflow - q;
         netFlowVolume = 0.5 * (Node[j].oldNetInflow + newNetInflow ) * dt;
         y = Node[j].oldDepth + netFlowVolume / Xnode[j].newSurfArea;
         if ( y <= 0.0 ) return Node[j].inflow;
    }
    return q;
}

//=============================================================================

void  findNonConduitSurfArea(int i)
//
//  Input:   i = link index
//  Output:  none
//  Purpose: finds the surface area contributed by a non-conduit
//           link to its upstream and downstream nodes.
//
{
    if ( Link[i].type == ORIFICE )
    {
        Xlink[i].surfArea1 = Orifice[Link[i].subIndex].surfArea / 2.;
    }

	// --- no surface area for weirs to maintain SWMM 4 compatibility
/*
    else if ( Link[i].type == WEIR )
    {
        Xlink[i].surfArea1 = Weir[Link[i].subIndex].surfArea / 2.;
    }
*/

    else Xlink[i].surfArea1 = 0.0;

////  Following segment modified for release 5.0.019.  ////                    //(5.0.019 - LR)
    Xlink[i].surfArea2 = Xlink[i].surfArea1;
    if ( Link[i].flowClass == UP_CRITICAL ||
        Node[Link[i].node1].type == STORAGE ) Xlink[i].surfArea1 = 0.0;
    if ( Link[i].flowClass == DN_CRITICAL ||
        Node[Link[i].node2].type == STORAGE ) Xlink[i].surfArea2 = 0.0;
}

//=============================================================================

void updateNodeFlows(int i, double q)
//
//  Input:   i = link index
//           q = link flow rate (cfs)
//  Output:  none
//  Purpose: updates cumulative inflow & outflow at link's end nodes.
//
{
    if ( q >= 0.0 )
    {
        Node[Link[i].node1].outflow += q;
        Node[Link[i].node2].inflow  += q;
    }
    else
    {
        Node[Link[i].node1].inflow   -= q;
        Node[Link[i].node2].outflow  -= q;
    }
}

//=============================================================================

double  getConduitFlow(int j, double qOld, double dt)
//
//  Input:   j        = link index
//           qOld     = flow from previous iteration (cfs)
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
    double aOld;                       // area from previous time step (ft2)
    double v;                          // velocity (ft/sec)
    double rho;                        // upstream weighting factor
    double sigma;                      // inertial damping factor
    double length;                     // effective conduit length (ft)
    double dq1, dq2, dq3, dq4, dq5;    // terms in momentum eqn.
    double denom;                      // denominator of flow update formula
    double q;                          // new flow value (cfs)
    double barrels;                    // number of barrels in conduit
    TXsect* xsect = &Link[j].xsect;    // ptr. to conduit's cross section data
    char   isFull = FALSE;             // TRUE if conduit flowing full

    // --- get most current heads at upstream and downstream ends of conduit
    k =  Link[j].subIndex;
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

    // --- get flow from last time step & previous iteration 
    barrels = Conduit[k].barrels;
    qOld /= barrels;
    qLast = Conduit[k].q1;

    // -- get area from solution at previous time step
    aOld = Conduit[k].a2;
    aOld = MAX(aOld, FUDGE);

    // --- use Courant-modified length instead of conduit's actual length
    length = Conduit[k].modLength;

    // --- find flow classification & corresponding surface area
    //     contributions to upstream and downstream nodes
    Link[j].flowClass = getFlowClass(j, qLast, h1, h2, y1, y2);
    findSurfArea(j, length, &h1, &h2, &y1, &y2);

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
         Link[j].isClosed ||
         aMid <= FUDGE )
    {
        Conduit[k].a1 = 0.5 * (a1 + a2);
        Conduit[k].q1 = 0.0;;
        Conduit[k].q2 = 0.0;
        Link[j].dqdh  = GRAVITY * dt * aMid / length * barrels;
        Link[j].froude = 0.0;
        Link[j].newDepth = MIN(yMid, Link[j].xsect.yFull);
        Link[j].newVolume = Conduit[k].a1 * link_getLength(j) * barrels;       //(5.0.015 - LR)
        return 0.0;
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

    // --- combine terms to find new conduit flow
    denom = 1.0 + dq1 + dq5;
    q = (qOld - dq2 + dq3 + dq4) / denom;

    // --- compute derivative of flow w.r.t. head
    Link[j].dqdh = 1.0 / denom  * GRAVITY * dt * aWtd / length * barrels;      //(5.0.014 - LR)

    // --- check if any flow limitation applies
    if ( q > 0.0 )
    {
        // --- open channels can't have more than full normal flow
        if ( isFull )
        {
            if ( xsect_isOpen(xsect->type) ) q = MIN(q, Link[j].qFull);
        }

        // --- check for inlet controlled culvert flow                         //(5.0.014 - LR)
        if ( xsect->culvertCode > 0 && !isFull )                               //(5.0.014 - LR)
            q = culvert_getInflow(j, q, h1);                                   //(5.0.014 - LR)

        // --- check for normal flow limitation based on surface slope & Fr
        else                                                                   //(5.0.014 - LR)
        if ( y1 < Link[j].xsect.yFull &&
               ( Link[j].flowClass == SUBCRITICAL ||
                 Link[j].flowClass == SUPCRITICAL)
           ) q = checkNormalFlow(j, q, y1, y2, a1, r1);                        //(5.0.019 - LR)
    }

    // --- apply under-relaxation weighting between new & old flows;
    // --- do not allow change in flow direction without first being zero 
    if ( Steps > 0 )
    {
        q = (1.0 - Omega) * qLast + Omega * q;
        if ( q * qLast < 0.0 ) q = 0.001 * SGN(q);
    }

    // --- check if user-supplied flow limit applies
    if ( Link[j].qLimit > 0.0 )
    {
         if ( fabs(q) > Link[j].qLimit ) q = SGN(q) * Link[j].qLimit;
    }

    // --- check for reverse flow with closed flap gate
    if ( link_setFlapGate(j, n1, n2, q) ) q = 0.0;                             //(5.0.014 - LR)

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
    Link[j].newVolume = aMid * link_getLength(j) * barrels;                    //(5.0.015 - LR)
    return q * barrels;
}

//=============================================================================

int getFlowClass(int j, double q, double h1, double h2, double y1, double y2)
//
//  Input:   j  = conduit link index
//           q  = current conduit flow (cfs)
//           h1 = head at upstream end of conduit (ft)
//           h2 = head at downstream end of conduit (ft)
//           y1 = upstream flow depth in conduit (ft)
//           y2 = downstream flow depth in conduit (ft)
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
    Fasnh = 1.0;

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
                NormalDepth   = link_getYnorm(j, fabs(q));
                CriticalDepth = link_getYcrit(j, fabs(q));
                ycMin = MIN(NormalDepth, CriticalDepth);
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
                NormalDepth = link_getYnorm(j, fabs(q));
                CriticalDepth = link_getYcrit(j, fabs(q));
                ycMin = MIN(NormalDepth, CriticalDepth);
                ycMax = MAX(NormalDepth, CriticalDepth);
                if ( y2 < ycMin ) flowClass = DN_CRITICAL;
                else if ( y2 < ycMax )
                {
                    if ( ycMax - ycMin < FUDGE ) Fasnh = 0.0;
                    else Fasnh = (ycMax - y2) / (ycMax - ycMin);
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
            NormalDepth   = link_getYnorm(j, fabs(q));
            CriticalDepth = link_getYcrit(j, fabs(q));
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
            NormalDepth = link_getYnorm(j, fabs(q));
            CriticalDepth = link_getYcrit(j, fabs(q));
            flowClass = DN_CRITICAL;
        }
    }
    return flowClass;
}

//=============================================================================

void findSurfArea(int j, double length, double* h1, double* h2,
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
    TXsect* xsect = &Link[j].xsect;

    // --- get node indexes & current flow depths
    n1 = Link[j].node1;
    n2 = Link[j].node2;
    flowDepth1 = *y1;
    flowDepth2 = *y2;

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
        surfArea2 = (widthMid + width2) * length / 4. * Fasnh;
        break;

      case UP_CRITICAL:
        flowDepth1 = CriticalDepth;
        if ( NormalDepth < CriticalDepth ) flowDepth1 = NormalDepth;
        flowDepth1 = MAX(flowDepth1, FUDGE);                                   //(5.0.022 - LR)
        *h1 = Node[n1].invertElev + Link[j].offset1 + flowDepth1;
        flowDepthMid = 0.5 * (flowDepth1 + flowDepth2);
        if ( flowDepthMid < FUDGE ) flowDepthMid = FUDGE;
        width2   = getWidth(xsect, flowDepth2);
        widthMid = getWidth(xsect, flowDepthMid);
        surfArea2 = (widthMid + width2) * length * 0.5;
        break;

      case DN_CRITICAL:
        flowDepth2 = CriticalDepth;
        if ( NormalDepth < CriticalDepth ) flowDepth2 = NormalDepth;
        flowDepth2 = MAX(flowDepth2, FUDGE);                                   //(5.0.022 - LR)
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
    Xlink[j].surfArea1 = surfArea1;
    Xlink[j].surfArea2 = surfArea2;
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
//  if ( yNorm < 0.04 ) y = 0.04*xsect->yFull;                                 //(5.0.015 - LR)
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

////  This function was removed for release 5.0.014.  ////                     //(5.0.014 - LR)
//double checkFlapGate(int j, int n1, int n2, double q)                        //(5.0.014 - LR)

//=============================================================================

//  =====  This function was completely re-written for release 5.0.019  =====  //(5.0.019 - LR)

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
        return MIN(q, qNorm);
    }
    else return q;
}

//=============================================================================

void setNodeDepth(int i, double dt)
//
//  Input:   i  = node index
//           dt = time step (sec)
//  Output:  none
//  Purpose: sets depth at non-outfall node after current time step.
//
{
    int     canPond;                   // TRUE if node can pond overflows
    int     isPonded;                  // TRUE if node is currently ponded     //(5.0.016 - LR)
    double  dQ;                        // inflow minus outflow at node (cfs)
    double  dV;                        // change in node volume (ft3)
    double  dy;                        // change in node depth (ft)
    double  yMax;                      // max. depth at node (ft)
    double  yOld;                      // node depth at previous time step (ft)
    double  yLast;                     // previous node depth (ft)
    double  yNew;                      // new node depth (ft)
    double  yCrown;                    // depth to node crown (ft)
    double  surfArea;                  // node surface area (ft2)
    double  denom;                     // denominator term
    double  corr;                      // correction factor
    double  f;                         // relative surcharge depth

    // --- see if node can pond water above it
    canPond = (AllowPonding && Node[i].pondedArea > 0.0);
    isPonded = (canPond && Node[i].newDepth > Node[i].fullDepth);              //(5.0.016 - LR)

    // --- initialize values
    yCrown = Node[i].crownElev - Node[i].invertElev;
    yOld = Node[i].oldDepth;
    yLast = Node[i].newDepth;
    Node[i].overflow = 0.0;
    surfArea = Xnode[i].newSurfArea;

    // --- determine average net flow volume into node over the time step
    dQ = Node[i].inflow - Node[i].outflow;
    dV = 0.5 * (Node[i].oldNetInflow + dQ) * dt - node_getLosses(i, dt);       //(5.0.019 - LR)

    // --- if node not surcharged, base depth change on surface area        
    if ( yLast <= yCrown || Node[i].type == STORAGE || isPonded )              //(5.0.016 - LR)
    {
        dy = dV / surfArea;
        Xnode[i].oldSurfArea = Xnode[i].newSurfArea;
        if ( !isPonded ) Xnode[i].oldSurfArea = Xnode[i].newSurfArea;          //(5.0.019 - LR)
        yNew = yOld + dy;

        // --- apply under-relaxation to new depth estimate
        if ( Steps > 0 )
        {
            yNew = (1.0 - Omega) * yLast + Omega * yNew;
        }

        // --- don't allow a ponded node to drop much below full depth         //(5.0.019 - LR)
        if ( isPonded && yNew < Node[i].fullDepth )                            //(5.0.019 - LR)
            yNew = Node[i].fullDepth - FUDGE;                                  //(5.0.019 - LR)
    }

    // --- if node surcharged, base depth change on dqdh
    //     NOTE: depth change is w.r.t depth from previous
    //     iteration; also, do not apply under-relaxation.
    else
    {
        // --- apply correction factor for upstream terminal nodes
        corr = 1.0;
        if ( Node[i].degree < 0 ) corr = 0.6;

        // --- allow surface area from last non-surcharged condition
        //     to influence dqdh if depth close to crown depth
        denom = Xnode[i].sumdqdh;
        if ( yLast < 1.25 * yCrown )
        {
            f = (yLast - yCrown) / yCrown;
            denom += (Xnode[i].oldSurfArea/dt -
                      Xnode[i].sumdqdh) * exp(-15.0 * f);
        }

        // --- compute new estimate of node depth
        if ( denom == 0.0 ) dy = 0.0;
        else dy = corr * dQ / denom;
        yNew = yLast + dy;
        if ( yNew < yCrown ) yNew = yCrown - FUDGE;

        // --- don't allow a ponded node to rise much above full depth         //(5.0.019 - LR)
        if ( canPond && yNew > Node[i].fullDepth )                             //(5.0.019 - LR)
            yNew = Node[i].fullDepth + FUDGE;                                  //(5.0.019 - LR)
    }

    // --- depth cannot be negative
    if ( yNew < 0 ) yNew = 0.0;

    // --- determine max. non-flooded depth
    yMax = Node[i].fullDepth;
    if ( canPond == FALSE ) yMax += Node[i].surDepth;

    // --- find flooded depth & volume
    if ( yNew > yMax )                                                         //(5.0.019 - LR)
    {
        yNew = getFloodedDepth(i, canPond, dV, yNew, yMax, dt);                //(5.0.014 - LR)
    }
    else Node[i].newVolume = node_getVolume(i, yNew);

////  Computation of dy/dt moved to here  ////                                 //(5.0.017 - LR)
    // --- compute change in depth w.r.t. time
    Xnode[i].dYdT = fabs(yNew - yOld) / dt;

    // --- save new depth for node
    Node[i].newDepth = yNew;
}

//=============================================================================

//  =====  This function was completely re-written for release 5.0.019  =====  //(5.0.019 - LR)

double getFloodedDepth(int i, int canPond, double dV, double yNew,
                       double yMax, double dt)
//
//  Input:   i  = node index
//           canPond = TRUE if water can pond over node
//           isPonded = TRUE if water is currently ponded
//           dV = change in volume over time step (ft3)
//           yNew = current depth at node (ft)
//           yMax = max. depth at node before ponding (ft)
//           dt = time step (sec)
//  Output:  returns depth at node when flooded (ft)
//  Purpose: computes depth, volume and overflow for a flooded node.
//
{
    if ( canPond == FALSE )
    {
        Node[i].overflow = dV / dt;
        Node[i].newVolume = Node[i].fullVolume;
        yNew = yMax;
    }
    else
    {
        Node[i].newVolume = MAX((Node[i].oldVolume+dV), Node[i].fullVolume);
        Node[i].overflow = (Node[i].newVolume - 
            MAX(Node[i].oldVolume, Node[i].fullVolume)) / dt;
    }
    if ( Node[i].overflow < FUDGE ) Node[i].overflow = 0.0;
    return yNew;

}

//=============================================================================

double getVariableStep(double maxStep)
//
//  Input:   maxStep = user-supplied max. time step (sec)
//  Output:  returns time step (sec)
//  Purpose: finds time step that satisfies stability criterion but
//           is no greater than the user-supplied max. time step.
//
{
    int    minLink = -1;                // index of link w/ min. time step
    int    minNode = -1;                // index of node w/ min. time step
    double tMin;                        // allowable time step (sec)
    double tMinLink;                    // allowable time step for links (sec)
    double tMinNode;                    // allowable time step for nodes (sec)

    // --- find stable time step for links & then nodes
    tMin = maxStep;
    tMinLink = getLinkStep(tMin, &minLink);
    tMinNode = getNodeStep(tMinLink, &minNode);

    // --- use smaller of the link and node time step
    tMin = tMinLink;
    if ( tMinNode < tMin )
    {
        tMin = tMinNode ;
        minLink = -1;
    }

    // --- update count of times the minimum node or link was critical
    stats_updateCriticalTimeCount(minNode, minLink);

    // --- don't let time step go below an absolute minimum
    if ( tMin < MINTIMESTEP ) tMin = MINTIMESTEP;
    return tMin;
}

//=============================================================================

double getLinkStep(double tMin, int *minLink)
//
//  Input:   tMin = critical time step found so far (sec)
//  Output:  minLink = index of link with critical time step;
//           returns critical time step (sec)
//  Purpose: finds critical time step for conduits based on Courant criterion.
//
{
    int    i;                           // link index
    int    k;                           // conduit index
    double q;                           // conduit flow (cfs)
    double t;                           // time step (sec)
    double tLink = tMin;                // critical link time step (sec)

    // --- examine each conduit link
    for ( i = 0; i < Nobjects[LINK]; i++ )
    {
        if ( Link[i].type == CONDUIT )
        {
           // --- skip conduits with negligible flow, area or Fr
            k = Link[i].subIndex;
            q = fabs(Link[i].newFlow) / Conduit[k].barrels;
            if ( q <= 0.05 * Link[i].qFull
            ||   Conduit[k].a1 <= FUDGE
            ||   Link[i].froude <= 0.01 
               ) continue;

            // --- compute time step to satisfy Courant condition
            t = Link[i].newVolume / Conduit[k].barrels / q;
            t = t * Conduit[k].modLength / link_getLength(i);                  //(5.0.015 - LR)
            t = t * Link[i].froude / (1.0 + Link[i].froude) * CourantFactor;

            // --- update critical link time step
            if ( t < tLink )
            {
                tLink = t;
                *minLink = i;
            }
        }
    }
    return tLink;
}

//=============================================================================

double getNodeStep(double tMin, int *minNode)
//
//  Input:   tMin = critical time step found so far (sec)
//  Output:  minNode = index of node with critical time step;
//           returns critical time step (sec)
//  Purpose: finds critical time step for nodes based on max. allowable
//           projected change in depth.
//
{
    int    i;                           // node index
    double maxDepth;                    // max. depth allowed at node (ft)
    double dYdT;                        // change in depth per unit time (ft/sec)
    double t1;                          // time needed to reach depth limit (sec)
    double tNode = tMin;                // critical node time step (sec)

    // --- find smallest time so that estimated change in nodal depth
    //     does not exceed safety factor * maxdepth
    for ( i = 0; i < Nobjects[NODE]; i++ )
    {
        // --- see if node can be skipped
        if ( Node[i].type == OUTFALL ) continue;
        if ( Node[i].newDepth <= FUDGE) continue;
        if ( Node[i].newDepth  + FUDGE >=
             Node[i].crownElev - Node[i].invertElev ) continue;

        // --- define max. allowable depth change using crown elevation
        maxDepth = (Node[i].crownElev - Node[i].invertElev) * 0.25;
        if ( maxDepth < FUDGE ) continue;
        dYdT = Xnode[i].dYdT;
        if (dYdT < FUDGE ) continue;

        // --- compute time to reach max. depth & compare with critical time
        t1 = maxDepth / dYdT;
        if ( t1 < tNode )
        {
            tNode = t1;
            *minNode = i;
        }
    }
    return tNode;
}

//=============================================================================

void  checkCapacity(int j)
//
//  Input:   j = link index
//  Output:  none
//  Purpose: determines if a conduit link is capacity limited.
//
{
    int    n1, n2, k;
    double h1, h2;

    // ---- check only conduit links
    if ( Link[j].type != CONDUIT ) return;

    // --- check that upstream end is full
    k = Link[j].subIndex;
    Conduit[k].capacityLimited = FALSE;
    if ( Conduit[k].a1 >= Link[j].xsect.aFull )
    {
        // --- check if HGL slope > conduit slope
        n1 = Link[j].node1;
        n2 = Link[j].node2;
        h1 = Node[n1].newDepth + Node[n1].invertElev;
        h2 = Node[n2].newDepth + Node[n2].invertElev;
        if ( (h1 - h2) > fabs(Conduit[k].slope) * Conduit[k].length )          //(5.0.018 - LR)
            Conduit[k].capacityLimited = TRUE;
    }
}

