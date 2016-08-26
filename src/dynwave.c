//-----------------------------------------------------------------------------
//   dynwave.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (5.1.001)
//             03/28/14   (5.1.002)
//             09/15/14   (5.1.007)
//             03/19/15   (5.1.008)
//             08/01/16   (5.1.011)
//   Author:   L. Rossman (EPA)
//             M. Tryby (EPA)
//             R. Dickinson (CDM)
//
//   Dynamic wave flow routing functions.
//
//   This module solves the dynamic wave flow routing equations using
//   Picard Iterations (i.e., a method of successive approximations)
//   to solve the explicit form of the continuity and momentum equations
//   for conduits.
//
//   Build 5.1.002:
//   - Only non-ponded nodal surface area is saved for use in
//     surcharge algorithm.
//
//   Build 5.1.007:
//   - Node losses added to node outflow variable instead of treated
//     as a separate item when computing change in node flow volume.
//
//   Build 5.1.008:
//   - Module-specific constants moved here from project.c.
//   - Support added for user-specified minimum variable time step.
//   - Node crown elevations found here instead of in flowrout.c module.
//   - OpenMP use to parallelize findLinkFlows() & findNodeDepths().
//   - Bug in finding complete list of capacity limited links fixed.
//
//   Build 5.1.011:
//   - Added test for failed memory allocation.
//   - Fixed illegal array index bug for Ideal Pumps.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include "headers.h"
#include <malloc.h>
#include <math.h>
#include <omp.h>                                                               //(5.1.008)

//-----------------------------------------------------------------------------
//     Constants 
//-----------------------------------------------------------------------------
static const double MINTIMESTEP =  0.001;   // min. time step (sec)            //(5.1.008)
static const double OMEGA       =  0.5;     // under-relaxation parameter

//  Constants moved here from project.c  //                                    //(5.1.008)
const double DEFAULT_SURFAREA  = 12.566; // Min. nodal surface area (~4 ft diam.)
const double DEFAULT_HEADTOL   = 0.005;  // Default head tolerance (ft)
const int    DEFAULT_MAXTRIALS = 8;      // Max. trials per time step


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

//-----------------------------------------------------------------------------
//  Shared Variables
//-----------------------------------------------------------------------------
static double  VariableStep;           // size of variable time step (sec)
static TXnode* Xnode;                  // extended nodal information

static double  Omega;                  // actual under-relaxation parameter
static int     Steps;                  // number of Picard iterations

//-----------------------------------------------------------------------------
//  Function declarations
//-----------------------------------------------------------------------------
static void   initRoutingStep(void);
static void   initNodeStates(void);
static void   findBypassedLinks();
static void   findLimitedLinks();

static void   findLinkFlows(double dt);
static int    isTrueConduit(int link);
static void   findNonConduitFlow(int link, double dt);
static void   findNonConduitSurfArea(int link);
static double getModPumpFlow(int link, double q, double dt);
static void   updateNodeFlows(int link);

static int    findNodeDepths(double dt);
static void   setNodeDepth(int node, double dt);
static double getFloodedDepth(int node, int canPond, double dV, double yNew,
              double yMax, double dt);

static double getVariableStep(double maxStep);
static double getLinkStep(double tMin, int *minLink);
static double getNodeStep(double tMin, int *minNode);

//=============================================================================

////  This function was modified for release 5.1.008.  ////                    //(5.1.008)

void dynwave_init()
//
//  Input:   none
//  Output:  none
//  Purpose: initializes dynamic wave routing method.
//
{
    int i, j;
    double z;

    VariableStep = 0.0;
    Xnode = (TXnode *) calloc(Nobjects[NODE], sizeof(TXnode));

////  Added to release 5.1.011.  ////                                          //(5.1.011)
    if ( Xnode == NULL )
    {
        report_writeErrorMsg(ERR_MEMORY,
            " Not enough memory for dynamic wave routing.");
        return;
    }
//////////////////////////////////////

    // --- initialize node surface areas & crown elev.
    for (i = 0; i < Nobjects[NODE]; i++ )
    {
        Xnode[i].newSurfArea = 0.0;
        Xnode[i].oldSurfArea = 0.0;
        Node[i].crownElev = Node[i].invertElev;
    }

    // --- update node crown elev. & initialize links
    for (i = 0; i < Nobjects[LINK]; i++)
    {
        j = Link[i].node1;
        z = Node[j].invertElev + Link[i].offset1 + Link[i].xsect.yFull;
        Node[j].crownElev = MAX(Node[j].crownElev, z);
        j = Link[i].node2;
        z = Node[j].invertElev + Link[i].offset2 + Link[i].xsect.yFull;
        Node[j].crownElev = MAX(Node[j].crownElev, z);
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
}

//=============================================================================

////  New function added to release 5.1.008.  ////                             //(5.1.008)

void dynwave_validate()
//
//  Input:   none
//  Output:  none
//  Purpose: adjusts dynamic wave routing options.
//
{
    if ( MinRouteStep > RouteStep ) MinRouteStep = RouteStep;
    if ( MinRouteStep < MINTIMESTEP ) MinRouteStep = MINTIMESTEP;
	if ( MinSurfArea == 0.0 ) MinSurfArea = DEFAULT_SURFAREA;
	else MinSurfArea /= UCF(LENGTH) * UCF(LENGTH);
    if ( HeadTol == 0.0 ) HeadTol = DEFAULT_HEADTOL;
    else HeadTol /= UCF(LENGTH);
	if ( MaxTrials == 0 ) MaxTrials = DEFAULT_MAXTRIALS;
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
        VariableStep = MinRouteStep;                                           //(5.1.008)
    }

    // --- otherwise compute variable step based on current flow solution
    else VariableStep = getVariableStep(fixedStep);

    // --- adjust step to be a multiple of a millisecond
    VariableStep = floor(1000.0 * VariableStep) / 1000.0;
    return VariableStep;
}

//=============================================================================

int dynwave_execute(double tStep)
//
//  Input:   links = array of topo sorted links indexes
//           tStep = time step (sec)
//  Output:  returns number of iterations used
//  Purpose: routes flows through drainage network over current time step.
//
{
    int converged;

    // --- initialize
    if ( ErrorCode ) return 0;
    Steps = 0;
    converged = FALSE;
    Omega = OMEGA;
    initRoutingStep();

    // --- keep iterating until convergence 
    while ( Steps < MaxTrials )
    {
        // --- execute a routing step & check for nodal convergence
        initNodeStates();
        findLinkFlows(tStep);
        converged = findNodeDepths(tStep);
        Steps++;
        if ( Steps > 1 )
        {
            if ( converged ) break;

            // --- check if link calculations can be skipped in next step
            findBypassedLinks();
        }
    }
    if ( !converged ) NonConvergeCount++;

    //  --- identify any capacity-limited conduits
    findLimitedLinks();
    return Steps;
}

//=============================================================================

void   initRoutingStep()
{
    int i;
    for (i = 0; i < Nobjects[NODE]; i++)
    {
        Xnode[i].converged = FALSE;
        Xnode[i].dYdT = 0.0;
    }
    for (i = 0; i < Nobjects[LINK]; i++)
    {
        Link[i].bypassed = FALSE;
        Link[i].surfArea1 = 0.0;
        Link[i].surfArea2 = 0.0;
    }

    // --- a2 preserves conduit area from solution at last time step
    for ( i = 0; i < Nlinks[CONDUIT]; i++) Conduit[i].a2 = Conduit[i].a1;
}

//=============================================================================

void initNodeStates()
//
//  Input:   none
//  Output:  none
//  Purpose: initializes node's surface area, inflow & outflow
//
{
    int i;

    for (i = 0; i < Nobjects[NODE]; i++)
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
        if ( Xnode[i].newSurfArea < MinSurfArea )
        {
            Xnode[i].newSurfArea = MinSurfArea;
        }

////  Following code section modified for release 5.1.007  ////                //(5.1.007)
        // --- initialize nodal inflow & outflow
        Node[i].inflow = 0.0;
        Node[i].outflow = Node[i].losses;
        if ( Node[i].newLatFlow >= 0.0 )
        {    
            Node[i].inflow += Node[i].newLatFlow;
        }
        else
        {    
            Node[i].outflow -= Node[i].newLatFlow;
        }
        Xnode[i].sumdqdh = 0.0;
    }
}

//=============================================================================

void   findBypassedLinks()
{
    int i;
    for (i = 0; i < Nobjects[LINK]; i++)
    {
        if ( Xnode[Link[i].node1].converged &&
             Xnode[Link[i].node2].converged )
             Link[i].bypassed = TRUE;
        else Link[i].bypassed = FALSE;
    }
}

//=============================================================================

void  findLimitedLinks()
//
//  Input:   none
//  Output:  none
//  Purpose: determines if a conduit link is capacity limited.
//
{
    int    j, n1, n2, k;
    double h1, h2;

    for (j = 0; j < Nobjects[LINK]; j++)
    {
        // ---- check only non-dummy conduit links
        if ( !isTrueConduit(j) ) continue;                                     //(5.1.008)

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
            if ( (h1 - h2) > fabs(Conduit[k].slope) * Conduit[k].length )
                Conduit[k].capacityLimited = TRUE;
        }
    }
}

//=============================================================================

void findLinkFlows(double dt)
{
    int i;

    // --- find new flow in each non-dummy conduit
#pragma omp parallel num_threads(NumThreads)                                   //(5.1.008)
{
    #pragma omp for                                                            //(5.1.008)
    for ( i = 0; i < Nobjects[LINK]; i++)
    {
        if ( isTrueConduit(i) && !Link[i].bypassed )
            dwflow_findConduitFlow(i, Steps, Omega, dt);
    }
}

    // --- update inflow/outflows for nodes attached to non-dummy conduits
    for ( i = 0; i < Nobjects[LINK]; i++)
    {
        if ( isTrueConduit(i) ) updateNodeFlows(i);
    }

    // --- find new flows for all dummy conduits, pumps & regulators
    for ( i = 0; i < Nobjects[LINK]; i++)
    {
        if ( !isTrueConduit(i) )
        {	
            if ( !Link[i].bypassed ) findNonConduitFlow(i, dt);
            updateNodeFlows(i);
        }
    }
}

//=============================================================================

int isTrueConduit(int j)
{
    return ( Link[j].type == CONDUIT && Link[j].xsect.type != DUMMY );
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
    if ( Steps > 0 && Link[i].type != PUMP ) 
    {
        qNew = (1.0 - Omega) * qLast + Omega * qNew;
        if ( qNew * qLast < 0.0 ) qNew = 0.001 * SGN(qNew);
    }
    Link[i].newFlow = qNew;
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
        Link[i].surfArea1 = Orifice[Link[i].subIndex].surfArea / 2.;
    }

    // --- no surface area for weirs to maintain SWMM 4 compatibility
/*
    else if ( Link[i].type == WEIR )
    {
        Xlink[i].surfArea1 = Weir[Link[i].subIndex].surfArea / 2.;
    }
*/

    else Link[i].surfArea1 = 0.0;
    Link[i].surfArea2 = Link[i].surfArea1;
    if ( Link[i].flowClass == UP_CRITICAL ||
        Node[Link[i].node1].type == STORAGE ) Link[i].surfArea1 = 0.0;
    if ( Link[i].flowClass == DN_CRITICAL ||
        Node[Link[i].node2].type == STORAGE ) Link[i].surfArea2 = 0.0;
}

//=============================================================================

void updateNodeFlows(int i)
//
//  Input:   i = link index
//           q = link flow rate (cfs)
//  Output:  none
//  Purpose: updates cumulative inflow & outflow at link's end nodes.
//
{
    int    k;                                                                  //(5.1.011)
    int    barrels = 1;
    int    n1 = Link[i].node1;
    int    n2 = Link[i].node2;
    double q = Link[i].newFlow;
    double uniformLossRate = 0.0;

    // --- compute any uniform seepage loss from a conduit
    if ( Link[i].type == CONDUIT )
    {
        k = Link[i].subIndex;
        uniformLossRate = Conduit[k].evapLossRate + Conduit[k].seepLossRate; 
        barrels = Conduit[k].barrels;
    }

    // --- update total inflow & outflow at upstream/downstream nodes
    if ( q >= 0.0 )
    {
        Node[n1].outflow += q + uniformLossRate;
        Node[n2].inflow  += q;
    }
    else
    {
        Node[n1].inflow   -= q;
        Node[n2].outflow  -= q - uniformLossRate;
    }

    // --- add surf. area contributions to upstream/downstream nodes
    Xnode[Link[i].node1].newSurfArea += Link[i].surfArea1 * barrels;
    Xnode[Link[i].node2].newSurfArea += Link[i].surfArea2 * barrels;

    // --- update summed value of dqdh at each end node
    Xnode[Link[i].node1].sumdqdh += Link[i].dqdh;
    if ( Link[i].type == PUMP )
    {
        k = Link[i].subIndex;
        if ( Pump[k].type != TYPE4_PUMP )                                      //(5.1.011)
        {
            Xnode[n2].sumdqdh += Link[i].dqdh;
        }
    }
    else Xnode[n2].sumdqdh += Link[i].dqdh;
}

//=============================================================================

int findNodeDepths(double dt)
{
    int i;
    int converged;      // convergence flag
    double yOld;        // previous node depth (ft)

    // --- compute outfall depths based on flow in connecting link
    for ( i = 0; i < Nobjects[LINK]; i++ ) link_setOutfallDepth(i);

    // --- compute new depth for all non-outfall nodes and determine if
    //     depth change from previous iteration is below tolerance
    converged = TRUE;
#pragma omp parallel num_threads(NumThreads)                                   //(5.1.008)
{
    #pragma omp for private(yOld)                                              //(5.1.008)
    for ( i = 0; i < Nobjects[NODE]; i++ )
    {
        if ( Node[i].type == OUTFALL ) continue;
        yOld = Node[i].newDepth;
        setNodeDepth(i, dt);
        Xnode[i].converged = TRUE;
        if ( fabs(yOld - Node[i].newDepth) > HeadTol )
        {
            converged = FALSE;
            Xnode[i].converged = FALSE;
        }
    }
}                                                                              //(5.1.008)
    return converged;
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
    int     isPonded;                  // TRUE if node is currently ponded 
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
    isPonded = (canPond && Node[i].newDepth > Node[i].fullDepth);

    // --- initialize values
    yCrown = Node[i].crownElev - Node[i].invertElev;
    yOld = Node[i].oldDepth;
    yLast = Node[i].newDepth;
    Node[i].overflow = 0.0;
    surfArea = Xnode[i].newSurfArea;

    // --- determine average net flow volume into node over the time step
    dQ = Node[i].inflow - Node[i].outflow;
    dV = 0.5 * (Node[i].oldNetInflow + dQ) * dt;

    // --- if node not surcharged, base depth change on surface area        
    if ( yLast <= yCrown || Node[i].type == STORAGE || isPonded )
    {
        dy = dV / surfArea;
        yNew = yOld + dy;

        // --- save non-ponded surface area for use in surcharge algorithm     //(5.1.002)
        if ( !isPonded ) Xnode[i].oldSurfArea = surfArea;                      //(5.1.002)

        // --- apply under-relaxation to new depth estimate
        if ( Steps > 0 )
        {
            yNew = (1.0 - Omega) * yLast + Omega * yNew;
        }

        // --- don't allow a ponded node to drop much below full depth
        if ( isPonded && yNew < Node[i].fullDepth )
            yNew = Node[i].fullDepth - FUDGE;
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

        // --- don't allow a newly ponded node to rise much above full depth
        if ( canPond && yNew > Node[i].fullDepth )
            yNew = Node[i].fullDepth + FUDGE;
    }

    // --- depth cannot be negative
    if ( yNew < 0 ) yNew = 0.0;

    // --- determine max. non-flooded depth
    yMax = Node[i].fullDepth;
    if ( canPond == FALSE ) yMax += Node[i].surDepth;

    // --- find flooded depth & volume
    if ( yNew > yMax )
    {
        yNew = getFloodedDepth(i, canPond, dV, yNew, yMax, dt);
    }
    else Node[i].newVolume = node_getVolume(i, yNew);

    // --- compute change in depth w.r.t. time
    Xnode[i].dYdT = fabs(yNew - yOld) / dt;

    // --- save new depth for node
    Node[i].newDepth = yNew;
}

//=============================================================================

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
    if ( tMin < MinRouteStep ) tMin = MinRouteStep;                            //(5.1.008)
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
            t = t * Conduit[k].modLength / link_getLength(i);
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
