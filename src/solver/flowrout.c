//-----------------------------------------------------------------------------
//   flowrout.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/19/14  (Build 5.1.001)
//             09/15/14  (Build 5.1.007)
//             03/19/15  (Build 5.1.008)
//             03/14/17  (Build 5.1.012)
//   Author:   L. Rossman (EPA)
//             M. Tryby (EPA)
//
//   Flow routing functions.
//
//
//   Build 5.1.007:
//   - updateStorageState() modified in response to node outflow being 
//     initialized with current evap & seepage losses in routing_execute().
//
//   Build 5.1.008:
//   - Determination of node crown elevations moved to dynwave.c.
//   - Support added for new way of recording conduit's fullness state.
//
//   Build 5.1.012:
//   - Overflow computed in updateStorageState() must be non-negative.
//   - Terminal storage nodes now updated corectly.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include "headers.h"
#include <stdlib.h>
#include <math.h>

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
static const double OMEGA   = 0.55;    // under-relaxation parameter
static const int    MAXITER = 10;      // max. iterations for storage updating
static const double STOPTOL = 0.005;   // storage updating stopping tolerance

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  flowrout_init            (called by routing_open)
//  flowrout_close           (called by routing_close)
//  flowrout_getRoutingStep  (called routing_getRoutingStep)
//  flowrout_execute         (called routing_execute)

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void   initLinkDepths(void);
static void   initNodeDepths(void);
static void   initNodes(void);
static void   initLinks(int routingModel);
static void   validateTreeLayout(void);      
static void   validateGeneralLayout(void);
static void   updateStorageState(int i, int j, int links[], double dt);
static double getStorageOutflow(int node, int j, int links[], double dt);
static double getLinkInflow(int link, double dt);
static void   setNewNodeState(int node, double dt);
static void   setNewLinkState(int link);
static void   updateNodeDepth(int node, double y);
static int    steadyflow_execute(int link, double* qin, double* qout,
              double tStep);


//=============================================================================

void flowrout_init(int routingModel)
//
//  Input:   routingModel = routing model code
//  Output:  none
//  Purpose: initializes flow routing system.
//
{
    // --- initialize for dynamic wave routing 
    if ( routingModel == DW )
    {
        // --- check for valid conveyance network layout
        validateGeneralLayout();
        dynwave_init();

        // --- initialize node & link depths if not using a hotstart file
        if ( Fhotstart1.mode == NO_FILE )
        {
            initNodeDepths();
            initLinkDepths();
        }
    }

    // --- validate network layout for kinematic wave routing
    else validateTreeLayout();

    // --- initialize node & link volumes
    initNodes();
    initLinks(routingModel);
}

//=============================================================================

void  flowrout_close(int routingModel)
//
//  Input:   routingModel = routing method code
//  Output:  none
//  Purpose: closes down routing method used.
//
{
    if ( routingModel == DW ) dynwave_close();
}

//=============================================================================

double flowrout_getRoutingStep(int routingModel, double fixedStep)
//
//  Input:   routingModel = type of routing method used
//           fixedStep = user-assigned max. routing step (sec)
//  Output:  returns adjusted value of routing time step (sec)
//  Purpose: finds variable time step for dynamic wave routing.
//
{
    if ( routingModel == DW )
    {
        return dynwave_getRoutingStep(fixedStep);
    }
    return fixedStep;
}

//=============================================================================

int flowrout_execute(int links[], int routingModel, double tStep)
//
//  Input:   links = array of link indexes in topo-sorted order
//           routingModel = type of routing method used
//           tStep = routing time step (sec)
//  Output:  returns number of computational steps taken
//  Purpose: routes flow through conveyance network over current time step.
//
{
    int   i, j;
    int   n1;                          // upstream node of link
    double qin;                        // link inflow (cfs)
    double qout;                       // link outflow (cfs)
    double steps;                      // computational step count

    // --- set overflows to drain any ponded water
    if ( ErrorCode ) return 0;
    for (j = 0; j < Nobjects[NODE]; j++)
    {
        Node[j].updated = FALSE;
        Node[j].overflow = 0.0;
        if ( Node[j].type != STORAGE
        &&   Node[j].newVolume > Node[j].fullVolume )
        {
            Node[j].overflow = (Node[j].newVolume - Node[j].fullVolume)/tStep;
        }
    }

    // --- execute dynamic wave routing if called for
    if ( routingModel == DW )
    {
        return dynwave_execute(tStep);
    }

    // --- otherwise examine each link, moving from upstream to downstream
    steps = 0.0;
    for (i = 0; i < Nobjects[LINK]; i++)
    {
        // --- see if upstream node is a storage unit whose state needs updating
        j = links[i];
        n1 = Link[j].node1;
        if ( Node[n1].type == STORAGE ) updateStorageState(n1, i, links, tStep);

        // --- retrieve inflow at upstream end of link
        qin  = getLinkInflow(j, tStep);

        // route flow through link
        if ( routingModel == SF )
            steps += steadyflow_execute(j, &qin, &qout, tStep);
        else steps += kinwave_execute(j, &qin, &qout, tStep);
        Link[j].newFlow = qout;

        // adjust outflow at upstream node and inflow at downstream node
        Node[ Link[j].node1 ].outflow += qin;
        Node[ Link[j].node2 ].inflow += qout;
    }
    if ( Nobjects[LINK] > 0 ) steps /= Nobjects[LINK];

    // --- update state of each non-updated node and link
    for ( j=0; j<Nobjects[NODE]; j++) setNewNodeState(j, tStep);
    for ( j=0; j<Nobjects[LINK]; j++) setNewLinkState(j);
    return (int)(steps+0.5);
}

//=============================================================================

void validateTreeLayout()
//
//  Input:   none
//  Output:  none
//  Purpose: validates tree-like conveyance system layout used for Steady
//           and Kinematic Wave flow routing
//
{
    int    j;

    // --- check nodes
    for ( j = 0; j < Nobjects[NODE]; j++ )
    {
        switch ( Node[j].type )
        {
          // --- dividers must have only 2 outlet links
          case DIVIDER:
            if ( Node[j].degree > 2 )
            {
                report_writeErrorMsg(ERR_DIVIDER, Node[j].ID);
            }
            break;

          // --- outfalls cannot have any outlet links
          case OUTFALL:
            if ( Node[j].degree > 0 )
            {
                report_writeErrorMsg(ERR_OUTFALL, Node[j].ID);
            }
            break;

          // --- storage nodes can have multiple outlets
          case STORAGE: break;

          // --- all other nodes allowed only one outlet link
          default:
            if ( Node[j].degree > 1 )
            {
                report_writeErrorMsg(ERR_MULTI_OUTLET, Node[j].ID);
            }
        }
    }

    // ---  check links 
    for (j=0; j<Nobjects[LINK]; j++)
    {
        switch ( Link[j].type )
        {
          // --- non-dummy conduits cannot have adverse slope
          case CONDUIT:
              if ( Conduit[Link[j].subIndex].slope < 0.0 &&
                   Link[j].xsect.type != DUMMY )
              {
                  report_writeErrorMsg(ERR_SLOPE, Link[j].ID);
              }
              break;

          // --- regulator links must be outlets of storage nodes
          case ORIFICE:
          case WEIR:
          case OUTLET:
            if ( Node[Link[j].node1].type != STORAGE )
            {
                report_writeErrorMsg(ERR_REGULATOR, Link[j].ID);
            }
        }
    }
}

//=============================================================================

void validateGeneralLayout()
//
//  Input:   none
//  Output:  nonw
//  Purpose: validates general conveyance system layout.
//
{
    int i, j;
    int outletCount = 0;

    // --- use node inflow attribute to count inflow connections
    for ( i=0; i<Nobjects[NODE]; i++ ) Node[i].inflow = 0.0;

    // --- examine each link
    for ( j = 0; j < Nobjects[LINK]; j++ )
    {
        // --- update inflow link count of downstream node
        i = Link[j].node1;
        if ( Node[i].type != OUTFALL ) i = Link[j].node2;
        Node[i].inflow += 1.0;

        // --- if link is dummy link or ideal pump then it must
        //     be the only link exiting the upstream node 
        if ( (Link[j].type == CONDUIT && Link[j].xsect.type == DUMMY) ||
             (Link[j].type == PUMP &&
              Pump[Link[j].subIndex].type == IDEAL_PUMP) )
        {
            i = Link[j].node1;
            if ( Link[j].direction < 0 ) i = Link[j].node2;
            if ( Node[i].degree > 1 )
            {
                report_writeErrorMsg(ERR_DUMMY_LINK, Node[i].ID);
            }
        }
    }

    // --- check each node to see if it qualifies as an outlet node
    //     (meaning that degree = 0)
    for ( i = 0; i < Nobjects[NODE]; i++ )
    {
        // --- if node is of type Outfall, check that it has only 1
        //     connecting link (which can either be an outflow or inflow link)
        if ( Node[i].type == OUTFALL )
        {
            if ( Node[i].degree + (int)Node[i].inflow > 1 )
            {
                report_writeErrorMsg(ERR_OUTFALL, Node[i].ID);
            }
            else outletCount++;
        }
    }
    if ( outletCount == 0 ) report_writeErrorMsg(ERR_NO_OUTLETS, "");

    // --- reset node inflows back to zero
    for ( i = 0; i < Nobjects[NODE]; i++ )
    {
        if ( Node[i].inflow == 0.0 ) Node[i].degree = -Node[i].degree;
        Node[i].inflow = 0.0;
    }
}

//=============================================================================

void initNodeDepths(void)
//
//  Input:   none
//  Output:  none
//  Purpose: sets initial depth at nodes for Dynamic Wave flow routing.
//
{
    int   i;                           // link or node index
    int   n;                           // node index
    double y;                          // node water depth (ft)

    // --- use Node[].inflow as a temporary accumulator for depth in 
    //     connecting links and Node[].outflow as a temporary counter
    //     for the number of connecting links
    for (i = 0; i < Nobjects[NODE]; i++)
    {
        Node[i].inflow  = 0.0;
        Node[i].outflow = 0.0;
    }

    // --- total up flow depths in all connecting links into nodes
    for (i = 0; i < Nobjects[LINK]; i++)
    {
        if ( Link[i].newDepth > FUDGE ) y = Link[i].newDepth + Link[i].offset1;
        else y = 0.0;
        n = Link[i].node1;
        Node[n].inflow += y;
        Node[n].outflow += 1.0;
        n = Link[i].node2;
        Node[n].inflow += y;
        Node[n].outflow += 1.0;
    }

    // --- if no user-supplied depth then set initial depth at non-storage/
    //     non-outfall nodes to average of depths in connecting links
    for ( i = 0; i < Nobjects[NODE]; i++ )
    {
        if ( Node[i].type == OUTFALL ) continue;
        if ( Node[i].type == STORAGE ) continue;
        if ( Node[i].initDepth > 0.0 ) continue;
        if ( Node[i].outflow > 0.0 )
        {
            Node[i].newDepth = Node[i].inflow / Node[i].outflow;
        }
    }

    // --- compute initial depths at all outfall nodes
    for ( i = 0; i < Nobjects[LINK]; i++ ) link_setOutfallDepth(i);
}

//=============================================================================
         
void initLinkDepths()
//
//  Input:   none
//  Output:  none
//  Purpose: sets initial flow depths in conduits under Dyn. Wave routing.
//
{
    int    i;                          // link index
    double y, y1, y2;                  // depths (ft)

    // --- examine each link
    for (i = 0; i < Nobjects[LINK]; i++)
    {
        // --- examine each conduit
        if ( Link[i].type == CONDUIT )
        {
            // --- skip conduits with user-assigned initial flows
            //     (their depths have already been set to normal depth)
            if ( Link[i].q0 != 0.0 ) continue;

            // --- set depth to average of depths at end nodes
            y1 = Node[Link[i].node1].newDepth - Link[i].offset1;
            y1 = MAX(y1, 0.0);
            y1 = MIN(y1, Link[i].xsect.yFull);
            y2 = Node[Link[i].node2].newDepth - Link[i].offset2;
            y2 = MAX(y2, 0.0);
            y2 = MIN(y2, Link[i].xsect.yFull);
            y = 0.5 * (y1 + y2);
            y = MAX(y, FUDGE);
            Link[i].newDepth = y;
        }
    }
}

//=============================================================================

void initNodes()
//
//  Input:   none
//  Output:  none
//  Purpose: sets initial inflow/outflow and volume for each node
//
{
    int i;

    for ( i = 0; i < Nobjects[NODE]; i++ )
    {
        // --- initialize node inflow and outflow
        Node[i].inflow = Node[i].newLatFlow;
        Node[i].outflow = 0.0;

        // --- initialize node volume
        Node[i].newVolume = 0.0;
        if ( AllowPonding &&
             Node[i].pondedArea > 0.0 &&
             Node[i].newDepth > Node[i].fullDepth )
        {
            Node[i].newVolume = Node[i].fullVolume +
                                (Node[i].newDepth - Node[i].fullDepth) *
                                Node[i].pondedArea;
        }
        else Node[i].newVolume = node_getVolume(i, Node[i].newDepth);
    }

    // --- update nodal inflow/outflow at ends of each link
    //     (needed for Steady Flow & Kin. Wave routing)
    for ( i = 0; i < Nobjects[LINK]; i++ )
    {
        if ( Link[i].newFlow >= 0.0 )
        {
            Node[Link[i].node1].outflow += Link[i].newFlow;
            Node[Link[i].node2].inflow  += Link[i].newFlow;
        }
        else
        {
            Node[Link[i].node1].inflow   -= Link[i].newFlow;
            Node[Link[i].node2].outflow  -= Link[i].newFlow;
        }
    }
}

//=============================================================================

void initLinks(int routingModel)
//
//  Input:   none
//  Output:  none
//  Purpose: sets initial upstream/downstream conditions in links.
//
{
    int    i;                          // link index
    int    k;                          // conduit or pump index

    // --- examine each link
    for ( i = 0; i < Nobjects[LINK]; i++ )
    {
        if ( routingModel == SF) Link[i].newFlow = 0.0;

        // --- otherwise if link is a conduit
        else if ( Link[i].type == CONDUIT )
        {
            // --- assign initial flow to both ends of conduit
            k = Link[i].subIndex;
            Conduit[k].q1 = Link[i].newFlow / Conduit[k].barrels;
            Conduit[k].q2 = Conduit[k].q1;

            // --- find areas based on initial flow depth
            Conduit[k].a1 = xsect_getAofY(&Link[i].xsect, Link[i].newDepth);
            Conduit[k].a2 = Conduit[k].a1;

            // --- compute initial volume from area
            {
                Link[i].newVolume = Conduit[k].a1 * link_getLength(i) *
                                    Conduit[k].barrels;
            }
            Link[i].oldVolume = Link[i].newVolume;
        }
    }
}

//=============================================================================

double getLinkInflow(int j, double dt)
//
//  Input:   j  = link index
//           dt = routing time step (sec)
//  Output:  returns link inflow (cfs)
//  Purpose: finds flow into upstream end of link at current time step under
//           Steady or Kin. Wave routing.
//
{
    int   n1 = Link[j].node1;
    double q;
    if ( Link[j].type == CONDUIT ||
         Link[j].type == PUMP ||
         Node[n1].type == STORAGE ) q = link_getInflow(j);
    else q = 0.0;
    return node_getMaxOutflow(n1, q, dt);
}

//=============================================================================

void updateStorageState(int i, int j, int links[], double dt)
//
//  Input:   i = index of storage node
//           j = current position in links array
//           links = array of topo-sorted link indexes
//           dt = routing time step (sec)
//  Output:  none
//  Purpose: updates depth and volume of a storage node using successive
//           approximation with under-relaxation for Steady or Kin. Wave
//           routing.
//
{
    int    iter;                       // iteration counter
    int    stopped;                    // TRUE when iterations stop
    double vFixed;                     // fixed terms of flow balance eqn.
    double v2;                         // new volume estimate (ft3)
    double d1;                         // initial value of storage depth (ft)
    double d2;                         // updated value of storage depth (ft)

    // --- see if storage node needs updating
    if ( Node[i].type != STORAGE ) return;
    if ( Node[i].updated ) return;

    // --- compute terms of flow balance eqn.
    //       v2 = v1 + (inflow - outflow)*dt
    //     that do not depend on storage depth at end of time step
    vFixed = Node[i].oldVolume + 
             0.5 * (Node[i].oldNetInflow + Node[i].inflow - 
                    Node[i].outflow) * dt;
    d1 = Node[i].newDepth;

    // --- iterate finding outflow (which depends on depth) and subsequent
    //     new volume and depth until negligible depth change occurs
    iter = 1;
    stopped = FALSE;
    while ( iter < MAXITER && !stopped )
    {
        // --- find new volume from flow balance eqn.
        v2 = vFixed - 0.5 * getStorageOutflow(i, j, links, dt) * dt;

        // --- limit volume to full volume if no ponding
        //     and compute overflow rate
        v2 = MAX(0.0, v2);
        Node[i].overflow = 0.0;
        if ( v2 > Node[i].fullVolume )
        {
            Node[i].overflow = (v2 - MAX(Node[i].oldVolume,
                                         Node[i].fullVolume)) / dt;
            if ( Node[i].overflow < FUDGE ) Node[i].overflow = 0.0;
            if ( !AllowPonding || Node[i].pondedArea == 0.0 )
                v2 = Node[i].fullVolume;
        }

        // --- update node's volume & depth 
        Node[i].newVolume = v2;
        d2 = node_getDepth(i, v2);
        Node[i].newDepth = d2;

        // --- use under-relaxation to estimate new depth value
        //     and stop if close enough to previous value
        d2 = (1.0 - OMEGA)*d1 + OMEGA*d2;
        if ( fabs(d2 - d1) <= STOPTOL ) stopped = TRUE;

        // --- update old depth with new value and continue to iterate
        Node[i].newDepth = d2;
        d1 = d2;
        iter++;
    }

    // --- mark node as being updated
    Node[i].updated = TRUE;
}

//=============================================================================

double getStorageOutflow(int i, int j, int links[], double dt)
//
//  Input:   i = index of storage node
//           j = current position in links array
//           links = array of topo-sorted link indexes
//           dt = routing time step (sec)
//  Output:  returns total outflow from storage node (cfs)
//  Purpose: computes total flow released from a storage node.
//
{
    int   k, m;
    double outflow = 0.0;

    for (k = j; k < Nobjects[LINK]; k++)
    {
        m = links[k];
        if ( Link[m].node1 != i ) break;
        outflow += getLinkInflow(m, dt);
    }
    return outflow;        
}

//=============================================================================

void setNewNodeState(int j, double dt)
//
//  Input:   j  = node index
//           dt = time step (sec)
//  Output:  none
//  Purpose: updates state of node after current time step
//           for Steady Flow or Kinematic Wave flow routing.
//
{
    int   canPond;                     // TRUE if ponding can occur at node  
    double newNetInflow;               // inflow - outflow at node (cfs)

    // --- update terminal storage nodes
    if ( Node[j].type == STORAGE )
    {	
	if ( Node[j].updated == FALSE )
	    updateStorageState(j, Nobjects[LINK], NULL, dt);
        return; 
    }

    // --- update stored volume using mid-point integration
    newNetInflow = Node[j].inflow - Node[j].outflow - Node[j].losses;
    Node[j].newVolume = Node[j].oldVolume +
                        0.5 * (Node[j].oldNetInflow + newNetInflow) * dt;
    if ( Node[j].newVolume < FUDGE ) Node[j].newVolume = 0.0;

    // --- determine any overflow lost from system
    Node[j].overflow = 0.0;
    canPond = (AllowPonding && Node[j].pondedArea > 0.0);
    if ( Node[j].newVolume > Node[j].fullVolume )
    {
        Node[j].overflow = (Node[j].newVolume - MAX(Node[j].oldVolume,
                            Node[j].fullVolume)) / dt;
        if ( Node[j].overflow < FUDGE ) Node[j].overflow = 0.0;
        if ( !canPond ) Node[j].newVolume = Node[j].fullVolume;
    }

    // --- compute a depth from volume
    //     (depths at upstream nodes are subsequently adjusted in
    //     setNewLinkState to reflect depths in connected conduit)
    Node[j].newDepth = node_getDepth(j, Node[j].newVolume);
}

//=============================================================================

void setNewLinkState(int j)
//
//  Input:   j = link index
//  Output:  none
//  Purpose: updates state of link after current time step under
//           Steady Flow or Kinematic Wave flow routing
//
{
    int   k;
    double a, y1, y2;

    Link[j].newDepth = 0.0;
    Link[j].newVolume = 0.0;

    if ( Link[j].type == CONDUIT )
    {
        // --- find avg. depth from entry/exit conditions
        k = Link[j].subIndex;
        a = 0.5 * (Conduit[k].a1 + Conduit[k].a2);
        Link[j].newVolume = a * link_getLength(j) * Conduit[k].barrels;
        y1 = xsect_getYofA(&Link[j].xsect, Conduit[k].a1);
        y2 = xsect_getYofA(&Link[j].xsect, Conduit[k].a2);
        Link[j].newDepth = 0.5 * (y1 + y2);

        // --- update depths at end nodes
        updateNodeDepth(Link[j].node1, y1 + Link[j].offset1);
        updateNodeDepth(Link[j].node2, y2 + Link[j].offset2);

        // --- check if capacity limited
        if ( Conduit[k].a1 >= Link[j].xsect.aFull )
        {
             Conduit[k].capacityLimited = TRUE;
             Conduit[k].fullState = ALL_FULL;
        }
        else
        {    
            Conduit[k].capacityLimited = FALSE;
            Conduit[k].fullState = 0;
        }
    }
}

//=============================================================================

void updateNodeDepth(int i, double y)
//
//  Input:   i = node index
//           y = flow depth (ft)
//  Output:  none
//  Purpose: updates water depth at a node with a possibly higher value.
//
{
    // --- storage nodes were updated elsewhere
    if ( Node[i].type == STORAGE ) return;

    // --- if non-outfall node is flooded, then use full depth
    if ( Node[i].type != OUTFALL &&
         Node[i].overflow > 0.0 ) y = Node[i].fullDepth;

    // --- if current new depth below y
    if ( Node[i].newDepth < y )
    {
        // --- update new depth
        Node[i].newDepth = y;

        // --- depth cannot exceed full depth (if value exists)
        if ( Node[i].fullDepth > 0.0 && y > Node[i].fullDepth )
        {
            Node[i].newDepth = Node[i].fullDepth;
        }
    }
}

//=============================================================================

int steadyflow_execute(int j, double* qin, double* qout, double tStep)
//
//  Input:   j = link index
//           qin = inflow to link (cfs)
//           tStep = time step (sec)
//  Output:  qin = adjusted inflow to link (limited by flow capacity) (cfs)
//           qout = link's outflow (cfs)
//           returns 1 if successful
//  Purpose: performs steady flow routing through a single link.
//
{
    int   k;
    double s;
    double q;

    // --- use Manning eqn. to compute flow area for conduits
    if ( Link[j].type == CONDUIT )
    {
        k = Link[j].subIndex;
        q = (*qin) / Conduit[k].barrels;
        if ( Link[j].xsect.type == DUMMY ) Conduit[k].a1 = 0.0;
        else 
        {
            // --- adjust flow for evap and infil losses
            q -= link_getLossRate(j, q, tStep);
         
            // --- flow can't exceed full flow 
            if ( q > Link[j].qFull )
            {
                q = Link[j].qFull;
                Conduit[k].a1 = Link[j].xsect.aFull;
                (*qin) = q * Conduit[k].barrels;
            }

            // --- infer flow area from flow rate 
            else
            {
                s = q / Conduit[k].beta;
                Conduit[k].a1 = xsect_getAofS(&Link[j].xsect, s);
            }
        }
        Conduit[k].a2 = Conduit[k].a1;
        
        Conduit[k].q1Old = Conduit[k].q1;
        Conduit[k].q2Old = Conduit[k].q2;
        
        Conduit[k].q1 = q;
        Conduit[k].q2 = q;
        (*qout) = q * Conduit[k].barrels;
    }
    else (*qout) = (*qin);
    return 1;
}

//=============================================================================
