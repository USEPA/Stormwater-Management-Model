//-----------------------------------------------------------------------------
//   qualrout.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//             04/02/15   (Build 5.1.008)
//   Author:   L. Rossman
//
//   Water quality routing functions.
//
//   Build 5.1.008:
//   - Pollutant mass lost to seepage flow added to mass balance totals.
//   - Pollutant concen. increased when evaporation occurs.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  qualrout_init            (called by swmm_start)
//  qualrout_execute         (called by routing_execute)

//-----------------------------------------------------------------------------
//  Function declarations
//-----------------------------------------------------------------------------
static void  findLinkMassFlow(int i, double tStep);
static void  findNodeQual(int j);
static void  findLinkQual(int i, double tStep);
static void  findSFLinkQual(int i, double tStep);
static void  findStorageQual(int j, double tStep);
static void  updateHRT(int j, double v, double q, double tStep);
static double getReactedQual(int p, double c, double v1, double tStep);
static double getMixedQual(double c, double v1, double wIn, double qIn,
              double tStep);
static double getLinkLosses(int j, double tStep);                              //(5.1.008)
static double getStorageLosses(int i, double tStep);                           //(5.1.008)

//=============================================================================

void    qualrout_init()
//
//  Input:   none
//  Output:  none
//  Purpose: initializes water quality concentrations in all nodes and links.
//
{
    int     i, p, isWet;
    double  c;

    for (i = 0; i < Nobjects[NODE]; i++)
    {
	    isWet = ( Node[i].newDepth > FUDGE );
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            c = 0.0;
            if ( isWet ) c = Pollut[p].initConcen;
            Node[i].oldQual[p] = c;
            Node[i].newQual[p] = c;
        }
    }

    for (i = 0; i < Nobjects[LINK]; i++)
    {
        isWet = ( Link[i].newDepth > FUDGE );
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            c = 0.0;
            if ( isWet ) c = Pollut[p].initConcen;
            Link[i].oldQual[p] = c;
            Link[i].newQual[p] = c;
        }
    }
}

//=============================================================================

void qualrout_execute(double tStep)
//
//  Input:   tStep = routing time step (sec)
//  Output:  none
//  Purpose: routes water quality constituents through the drainage
//           network over the current time step.
//
{
    int    i, j;
    double qIn, vAvg;

    // --- find mass flow each link contributes to its downstream node
    for ( i = 0; i < Nobjects[LINK]; i++ ) findLinkMassFlow(i, tStep);

    // --- find new water quality concentration at each node  
    for (j = 0; j < Nobjects[NODE]; j++)
    {
        // --- get node inflow and average volume
        qIn = Node[j].inflow;
        vAvg = (Node[j].oldVolume + Node[j].newVolume) / 2.0;
        
        // --- save inflow concentrations if treatment applied
        if ( Node[j].treatment )
        {
            if ( qIn < ZERO ) qIn = 0.0;
            treatmnt_setInflow(qIn, Node[j].newQual);
        }
       
        // --- find new quality at the node 
        if ( Node[j].type == STORAGE || Node[j].oldVolume > FUDGE )
        {
            findStorageQual(j, tStep);
        }
        else findNodeQual(j);

        // --- apply treatment to new quality values
        if ( Node[j].treatment ) treatmnt_treat(j, qIn, vAvg, tStep);
    }

    // --- find new water quality in each link
    for ( i=0; i<Nobjects[LINK]; i++ ) findLinkQual(i, tStep);
}

//=============================================================================

double getMixedQual(double c, double v1, double wIn, double qIn, double tStep)
//
//  Input:   c = pollutant concentration at start of time step (mass/ft3)
//           v1 = volume at start of time step (ft3)
//           wIn = mass inflow rate (mass/sec)
//           qIn = flow inflow rate (cfs)
//           tStep = time step (sec)
//  Output:  returns pollutant concentration at end of time step (mass/ft3)
//  Purpose: finds pollutant concentration within a completely mixed volume
//
{
    double vIn, cIn, cMax;

    // --- compute concentration of any inflow
    if ( qIn <= ZERO ) return c;
    vIn = qIn * tStep;
    cIn = wIn * tStep / vIn;

    // --- find limit on final concentration
    cMax = MAX(c, cIn);

    // --- mix inflow with reacted contents
    c = (c*v1 + wIn*tStep) / (v1 + vIn);
    c = MIN(c, cMax);
    c = MAX(c, 0.0);
    return c;
}


//=============================================================================

void findLinkMassFlow(int i, double tStep)
//
//  Input:   i = link index
//           tStep = time step (sec)
//  Output:  none
//  Purpose: adds constituent mass flow out of link to the total
//           accumulation at the link's downstream node.
//
//  Note:    Node[].newQual[], the accumulator variable, already contains
//           contributions from runoff and other external inflows from
//           calculations made in routing_execute().
{
    int    j, p;
    double qLink, w;

    // --- find inflow to downstream node
    qLink = Link[i].newFlow;

    // --- identify index of downstream node
    j = Link[i].node2;
    if ( qLink < 0.0 ) j = Link[i].node1;
    qLink = fabs(qLink);

    // --- add mass inflow from link to total at downstream node
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
	    w = qLink * Link[i].oldQual[p];
        Node[j].newQual[p] += w;
	    Link[i].totalLoad[p] += w * tStep;
    }
}

//=============================================================================

void findNodeQual(int j)
//
//  Input:   j = node index
//  Output:  none
//  Purpose: finds new quality in a node with no storage volume.
//
{
    int    p;
    double qNode;

    // --- if there is flow into node then concen. = mass inflow/node flow
    qNode = Node[j].inflow;
    if ( qNode > ZERO )
    {
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            Node[j].newQual[p] /= qNode;
        }
    }

    // --- otherwise concen. is 0
    else for (p = 0; p < Nobjects[POLLUT]; p++) Node[j].newQual[p] = 0.0;
}

//=============================================================================

void findLinkQual(int i, double tStep)
//
//  Input:   i = link index
//           tStep = routing time step (sec)
//  Output:  none
//  Purpose: finds new quality in a link at end of the current time step.
//
{
    int    j,                // upstream node index
           k,                // conduit index
           p;                // pollutant index
    double wIn,              // pollutant mass inflow rate (mass/sec)
           qIn,              // inflow rate (cfs)
           qOut,             // outflow rate (cfs)
           v1,               // link volume at start of time step (ft3)
           v2,               // link volume at end of time step (ft3)
           c1,               // current concentration within link (mass/ft3)
           c2,               // new concentration within link (mass/ft3)
           vLosses;          // evap + seepage volume loss (ft3)

    // --- identify index of upstream node
    j = Link[i].node1;
    if ( Link[i].newFlow < 0.0 ) j = Link[i].node2;

    // --- link concentration equals that of upstream node when
    //     link is not a conduit or is a dummy link
    if ( Link[i].type != CONDUIT || Link[i].xsect.type == DUMMY )
    {
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            Link[i].newQual[p] = Node[j].newQual[p];
        }
        return;
    }

////  Added for release 5.1.008.  ////                                         //(5.1.008)
    // --- get volume of evap. & seepage losses
    //     (while updating mass balance for these losses)
    vLosses = getLinkLosses(i, tStep);
////

    // --- concentrations are zero in an empty conduit
    if ( Link[i].newDepth <= FUDGE )
    {
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            Link[i].newQual[p] = 0.0;
        }
        return;
    }

    // --- Steady Flow routing requires special treatment
    if ( RouteModel == SF )
    {
        findSFLinkQual(i, tStep);
        return;
    }

    // --- get inlet & outlet flow
    k = Link[i].subIndex;
    qIn  = fabs(Conduit[k].q1) * (double)Conduit[k].barrels;
    qOut = fabs(Conduit[k].q2) * (double)Conduit[k].barrels;

    // --- get starting and ending volumes
    v1 = Link[i].oldVolume - vLosses;                                          //(5.1.008)
    v1 = MAX(v1, 0.0);                                                         //(5.1.008)
    v2 = Link[i].newVolume;

    // --- adjust inflow to compensate for volume change when inflow = outflow
    if (qIn == qOut)                                        
    {
        qIn = qIn + (v2 - v1) / tStep; 
        qIn = MAX(qIn, 0.0);
    }

    // --- for each pollutant
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        // --- determine mass lost to first order decay
        c1 = Link[i].oldQual[p];
        c2 = getReactedQual(p, c1, v1, tStep);

        // --- mix inflow to conduit with previous contents
        wIn = Node[j].newQual[p]*qIn;
        c2 = getMixedQual(c2, v1, wIn, qIn, tStep);

        // --- assign new concen. to link
        Link[i].newQual[p] = c2;
    }
}

//=============================================================================

void  findSFLinkQual(int i, double tStep)
//
//  Input:   i = link index
//           tStep = routing time step (sec)
//  Output:  none
//  Purpose: finds new quality in a link at end of the current time step for
//           Steady Flow routing.
//
{
    int j = Link[i].node1;
    int p;
    double c1, c2;
    double lossRate;
    double t = tStep;

    // --- for each pollutant
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        // --- conduit's quality equals upstream node quality
        c1 = Node[j].newQual[p];
        c2 = c1;

        // --- apply first-order decay over travel time
        if ( Pollut[p].kDecay > 0.0 )
        {
            c2 = c1 * exp(-Pollut[p].kDecay * t);
            c2 = MAX(0.0, c2);
            lossRate = (c1 - c2) * Link[i].newFlow;
            massbal_addReactedMass(p, lossRate);
        }
        Link[i].newQual[p] = c2;
    }
}

//=============================================================================

void  findStorageQual(int j, double tStep)
//
//  Input:   j = node index
//           tStep = routing time step (sec)
//  Output:  none
//  Purpose: finds new quality in a node with storage volume.
//  
{
    int    p;                // pollutant index
    double qIn,              // inflow rate (cfs)
           wIn,              // pollutant mass inflow rate (mass)
           v1,               // volume at start of time step (ft3)
           c1,               // initial pollutant concentration (mass/ft3)
           c2;               // final pollutant concentration (mass/ft3)

    // --- get inflow rate & initial volume
    qIn = Node[j].inflow;
    v1 = Node[j].oldVolume;

////  Modified for release 5.1.008.  ////                                      //(5.1.008)
////
    // -- for storage nodes
    if ( Node[j].type == STORAGE )
    {    
        // --- update hydraulic residence time
        //     (HRT can be used in treatment functions)
        updateHRT(j, Node[j].oldVolume, qIn, tStep);

        //  --- reduce volume by evap & exfiltration losses
        //      (while also updating mass balances)
        v1 -= getStorageLosses(j, tStep);
        v1 = MAX(v1, 0.0);
    }
////

    // --- for each pollutant
    for (p = 0; p < Nobjects[POLLUT]; p++)
    {
        // --- get current concentration 
        c1 = Node[j].oldQual[p];

        // --- apply first order decay only if no separate treatment function
        if ( Node[j].treatment == NULL ||
             Node[j].treatment[p].equation == NULL )
        {
            c1 = getReactedQual(p, c1, v1, tStep);
        }

        // --- mix inflow with current contents (mass inflow rate was
        //     temporarily saved in Node[j].newQual)
        wIn = Node[j].newQual[p];
        c2 = getMixedQual(c1, v1, wIn, qIn, tStep);

        // --- assign new concen. to node
        Node[j].newQual[p] = c2;
    }
}

//=============================================================================

void updateHRT(int j, double v, double q, double tStep)
//
//  Input:   j = node index
//           v = storage volume (ft3)
//           q = inflow rate (cfs)
//           tStep = time step (sec)
//  Output:  none
//  Purpose: updates hydraulic residence time (i.e., water age) at a 
//           storage node.
//
{
    int    k = Node[j].subIndex;
    double hrt = Storage[k].hrt;
    if ( v < ZERO ) hrt = 0.0;
    else hrt = (hrt + tStep) * v / (v + q*tStep);
    Storage[k].hrt = MAX(hrt, 0.0);
}

//=============================================================================

double getReactedQual(int p, double c, double v1, double tStep)
//
//  Input:   p = pollutant index
//           c = initial concentration (mass/ft3)
//           v1 = initial volume (ft3)
//           tStep = time step (sec)
//  Output:  none
//  Purpose: applies a first order reaction to a pollutant over a given
//           time step.
//
{
    double c2, lossRate;
    double kDecay = Pollut[p].kDecay;

    if ( kDecay == 0.0 ) return c;
    c2 = c * (1.0 - kDecay * tStep);
    c2 = MAX(0.0, c2);
    lossRate = (c - c2) * v1 / tStep;
    massbal_addReactedMass(p, lossRate);
    return c2;
}

//=============================================================================

////  New function added to release 5.1.008.  ////                             //(5.1.008)

double getLinkLosses(int i, double tStep)
//
//  Input:   i = link index
//           tStep = time step (sec)
//  Output:  total volume of conduit losses over the time step (ft3)
//  Purpose: updates mass balance and conduit concentrations for evap. &
//           seepage losses.
//
{
    int k, p;
    double seepRate = 0.0,   // seepage loss rate (cfs)
           evapRate = 0.0,   // evaporation loss rate (cfs)
           v1,               // link volume after seepage (ft3)
           v2,               // link volume after seepage & evaporation (ft3)
           vRatio;           // v1 / v2

    // --- add any seepage mass loss to mass balance totals
    if ( Link[i].type != CONDUIT ) return 0.0;
    k = Link[i].subIndex;
    if ( Conduit[k].seepLossRate > 0.0 )
    {
        seepRate = Conduit[k].seepLossRate * (double)Conduit[k].barrels;
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            massbal_addSeepageLoss(p, seepRate*Link[i].oldQual[p]);
        }
    }
 
    // --- adjust conduit concentrations for any evaporation loss
    evapRate = Conduit[k].evapLossRate * (double)Conduit[k].barrels;
    if ( evapRate > 0.0 && Link[i].oldVolume > FUDGE )
    {
        // --- find link volume after seepage and evaporation
        v1 = Link[i].oldVolume - seepRate * tStep;    // after seepage
        v2 = v1 - evapRate * tStep;                   // after seepage & evap

        // --- no volume remaining; send remaining mass to final storage
        if ( v2 < FUDGE )
        {
            for (p = 0; p < Nobjects[POLLUT]; p++)
                massbal_addToFinalStorage(p, Link[i].oldQual[p] * v1);
        }
        // --- otherwise increase concentrations by loss in volume
        else
        {
            vRatio = v1 / v2;
            for ( p = 0; p < Nobjects[POLLUT]; p++ ) Link[i].oldQual[p] *= vRatio;
        }
    }

    // --- return volume of total losses over the time step
    return (seepRate + evapRate) * tStep;
}

//=============================================================================

////  New function added to release 5.1.008.  ////                             //(5.1.008)

double getStorageLosses(int i, double tStep)
//
//  Input:   i = node index
//           tStep = time step (sec)
//  Output:  total volume of storage losses over the time step (ft3)
//  Purpose: updates mass balance and storage node concentrations for evap. &
//           seepage losses.
//
{
    int k, p;
    double seepRate = 0.0,   // seepage loss rate (cfs)
           evapRate = 0.0,   // evaporation loss rate (cfs)
           v1,               // link volume after seepage (ft3)
           v2,               // link volume after seepage & evaporation (ft3)
           vRatio;           // v1 / v2

    // --- add any seepage mass loss to mass balance totals
    if ( Node[i].type != STORAGE ) return 0.0;
    k = Node[i].subIndex;
    if ( Storage[k].exfilLoss > 0.0 )
    {
        seepRate = Storage[k].exfilLoss / tStep;
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            massbal_addSeepageLoss(p, seepRate*Node[i].oldQual[p]);
        }
    }
 
    // --- adjust storage node concentrations for any evaporation loss
    evapRate = Storage[k].evapLoss / tStep;
    if ( evapRate > 0.0 && Node[i].oldDepth > FUDGE )
    {
        // --- find storage volume after seepage and evaporation
        v1 = Node[i].oldVolume - seepRate * tStep;    // after seepage
        v2 = v1 - evapRate * tStep;                   // after seepage & evap

        // --- no volume remaining; send remaining mass to final storage
        if ( v2 < FUDGE )
        {
            for (p = 0; p < Nobjects[POLLUT]; p++)
                massbal_addToFinalStorage(p, Node[i].oldQual[p] * v1);
        }
        // --- otherwise increase concentrations by loss in volume
        else
        {
            vRatio = v1 / v2;
            for ( p = 0; p < Nobjects[POLLUT]; p++ ) Node[i].oldQual[p] *= vRatio;
        }
    }

    // --- return volume of total losses over the time step
    return (seepRate + evapRate) * tStep;
}
