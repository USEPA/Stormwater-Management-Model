//-----------------------------------------------------------------------------
//   stats.c
//
//   Project:  EPA SWMM5
//   Version:  5.0
//   Date:     6/19/07   (Build 5.0.010)
//             2/4/08    (Build 5.0.012)
//             10/9/09   (Build 5.0.017)
//             11/18/09  (Build 5.0.018)
//             07/30/10  (Build 5.0.019)
//             04/20/11  (Build 5.0.022)
//   Author:   L. Rossman (EPA)
//             R. Dickinson (CDM)
//
//   Simulation statistics functions.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <math.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
#define MAX_STATS 5
static TSysStats       SysStats;
static TMaxStats       MaxMassBalErrs[MAX_STATS];
static TMaxStats       MaxCourantCrit[MAX_STATS];
static TMaxStats       MaxFlowTurns[MAX_STATS];                                //(5.0.010 - LR)
static double          SysOutfallFlow;

//-----------------------------------------------------------------------------
//  Exportable variables (shared with statsrpt.c)
//-----------------------------------------------------------------------------
TSubcatchStats* SubcatchStats;
TNodeStats*     NodeStats;
TLinkStats*     LinkStats;
TStorageStats*  StorageStats;
TOutfallStats*  OutfallStats;
TPumpStats*     PumpStats;
double          MaxOutfallFlow;
double          MaxRunoffFlow;

//-----------------------------------------------------------------------------
//  Imported variables
//-----------------------------------------------------------------------------
extern double*         NodeInflow;     // defined in massbal.c
extern double*         NodeOutflow;    // defined in massbal.c

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  stats_open                    (called from swmm_start in swmm5.c)
//  stats_close                   (called from swmm_end in swmm5.c)
//  stats_report                  (called from swmm_end in swmm5.c)
//  stats_updateSubcatchStats     (called from subcatch_getRunoff)
//  stats_updateFlowStats         (called from routing_execute)
//  stats_updateCriticalTimeCount (called from getVariableStep in dynwave.c)

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void stats_updateNodeStats(int node, double tStep, DateTime aDate);
static void stats_updateLinkStats(int link, double tStep, DateTime aDate);
static void stats_findMaxStats(void);
static void stats_updateMaxStats(TMaxStats maxStats[], int i, int j, double x);

//=============================================================================

int  stats_open()
//
//  Input:   none
//  Output:  returns an error code
//  Purpose: opens the simulation statistics system.
//
{
    int j, k;

    // --- set all pointers to NULL
    NodeStats = NULL;
    LinkStats = NULL;
    StorageStats = NULL;
    OutfallStats = NULL;
    PumpStats = NULL;                                                          //(5.0.012 - LR)

    // --- allocate memory for & initialize subcatchment statistics
    SubcatchStats = NULL;
    if ( Nobjects[SUBCATCH] > 0 )
    {
        SubcatchStats = (TSubcatchStats *) calloc(Nobjects[SUBCATCH],
                                               sizeof(TSubcatchStats));
        if ( !SubcatchStats )
        {
            report_writeErrorMsg(ERR_MEMORY, "");
            return ErrorCode;
        }
        for (j=0; j<Nobjects[SUBCATCH]; j++)
        {
            SubcatchStats[j].precip  = 0.0;
            SubcatchStats[j].runon   = 0.0;
            SubcatchStats[j].evap    = 0.0;
            SubcatchStats[j].infil   = 0.0;
            SubcatchStats[j].runoff  = 0.0;
            SubcatchStats[j].maxFlow = 0.0;
        }
    }

    // --- allocate memory for node & link stats
    if ( Nobjects[LINK] > 0 )
    {
        NodeStats = (TNodeStats *) calloc(Nobjects[NODE], sizeof(TNodeStats));
        LinkStats = (TLinkStats *) calloc(Nobjects[LINK], sizeof(TLinkStats));
        if ( !NodeStats || !LinkStats )
        {
            report_writeErrorMsg(ERR_MEMORY, "");
            return ErrorCode;
        }
    }

    // --- initialize node stats
    if ( NodeStats ) for ( j = 0; j < Nobjects[NODE]; j++ )
    {
        NodeStats[j].avgDepth = 0.0;
        NodeStats[j].maxDepth = 0.0;
        NodeStats[j].maxDepthDate = StartDateTime;
        NodeStats[j].maxDepthChange = 0.0;                                     //(5.0.012 - LR)
        NodeStats[j].volFlooded = 0.0;
        NodeStats[j].timeFlooded = 0.0;
        NodeStats[j].timeSurcharged = 0.0;                                     //(5.0.012 - LR)
        NodeStats[j].timeCourantCritical = 0.0;
        NodeStats[j].totLatFlow = 0.0;                                         //(5.0.012 - LR)
        NodeStats[j].maxLatFlow = 0.0;
        NodeStats[j].maxInflow = 0.0;
        NodeStats[j].maxOverflow = 0.0;
        NodeStats[j].maxPondedVol = 0.0;                                       //(5.0.012 - LR)
        NodeStats[j].maxInflowDate = StartDateTime;
        NodeStats[j].maxOverflowDate = StartDateTime;
    }

    // --- initialize link stats
    if ( LinkStats ) for ( j = 0; j < Nobjects[LINK]; j++ )
    {
        LinkStats[j].maxFlow = 0.0;
        LinkStats[j].maxVeloc = 0.0;
        LinkStats[j].maxDepth = 0.0;
        LinkStats[j].avgFlowChange = 0.0;
        LinkStats[j].avgFroude = 0.0;
        LinkStats[j].timeSurcharged = 0.0;
        LinkStats[j].timeFullUpstream = 0.0;                                   //(5.0.012 - LR)
        LinkStats[j].timeFullDnstream = 0.0;                                   //(5.0.012 - LR)
        LinkStats[j].timeFullFlow = 0.0;                                       //(5.0.012 - LR)
        LinkStats[j].timeCapacityLimited = 0.0;                                //(5.0.012 - LR)
        LinkStats[j].timeCourantCritical = 0.0;
        for (k=0; k<MAX_FLOW_CLASSES; k++)
            LinkStats[j].timeInFlowClass[k] = 0.0;
        LinkStats[j].flowTurns = 0;                                            //(5.0.010 - LR)
        LinkStats[j].flowTurnSign = 0;                                         //(5.0.010 - LR)
    }

    // --- allocate memory for & initialize storage unit statistics
    if ( Nnodes[STORAGE] > 0 )
    {
        StorageStats = (TStorageStats *) calloc(Nnodes[STORAGE],
                           sizeof(TStorageStats));
        if ( !StorageStats )
        {
            report_writeErrorMsg(ERR_MEMORY, "");
            return ErrorCode;
        }
        else for ( j = 0; j < Nnodes[STORAGE]; j++ )
        {
            StorageStats[j].avgVol = 0.0;
            StorageStats[j].maxVol = 0.0;
            StorageStats[j].maxFlow = 0.0;
            StorageStats[j].losses = 0.0;                                      //(5.0.018-LR)
            StorageStats[j].maxVolDate = StartDateTime;
        }
    }

    // --- allocate memory for & initialize outfall statistics
    if ( Nnodes[OUTFALL] > 0 )
    {
        OutfallStats = (TOutfallStats *) calloc(Nnodes[OUTFALL],
                           sizeof(TOutfallStats));
        if ( !OutfallStats )
        {
            report_writeErrorMsg(ERR_MEMORY, "");
            return ErrorCode;
        }
        else for ( j = 0; j < Nnodes[OUTFALL]; j++ )
        {
            OutfallStats[j].avgFlow = 0.0;
            OutfallStats[j].maxFlow = 0.0;
            OutfallStats[j].totalPeriods = 0;
            if ( Nobjects[POLLUT] > 0 )
            {
                OutfallStats[j].totalLoad =
                    (double *) calloc(Nobjects[POLLUT], sizeof(double));
                if ( !OutfallStats[j].totalLoad )
                {
                    report_writeErrorMsg(ERR_MEMORY, "");
                    return ErrorCode;
                }
                for (k=0; k<Nobjects[POLLUT]; k++)
                    OutfallStats[j].totalLoad[k] = 0.0;
            }
            else OutfallStats[j].totalLoad = NULL;
        }
    }

    // --- allocate memory & initialize pumping statistics                     //(5.0.012 - LR)
    if ( Nlinks[PUMP] > 0 )                                                    //(5.0.012 - LR)
    {                                                                          //(5.0.012 - LR)
        PumpStats = (TPumpStats *) calloc(Nlinks[PUMP], sizeof(TPumpStats));   //(5.0.012 - LR)
        if ( !PumpStats )                                                      //(5.0.012 - LR)
        {                                                                      //(5.0.012 - LR)
            report_writeErrorMsg(ERR_MEMORY, "");                              //(5.0.012 - LR)
            return ErrorCode;                                                  //(5.0.012 - LR)
        }                                                                      //(5.0.012 - LR)
        else for ( j = 0; j < Nlinks[PUMP]; j++ )                              //(5.0.012 - LR)
        {                                                                      //(5.0.012 - LR)
            PumpStats[j].utilized = 0.0;                                       //(5.0.012 - LR)
            PumpStats[j].minFlow  = 0.0;                                       //(5.0.022 - LR)
            PumpStats[j].avgFlow  = 0.0;                                       //(5.0.012 - LR)
            PumpStats[j].maxFlow  = 0.0;                                       //(5.0.012 - LR)
            PumpStats[j].volume   = 0.0;                                       //(5.0.012 - LR)
            PumpStats[j].energy   = 0.0;                                       //(5.0.012 - LR)
            PumpStats[j].startUps = 0;                                         //(5.0.022 - LR)
            PumpStats[j].offCurveLow = 0.0;                                    //(5.0.022 - LR)
            PumpStats[j].offCurveHigh = 0.0;                                   //(5.0.022 - LR)
        }                                                                      //(5.0.012 - LR) 
    }                                                                          //(5.0.012 - LR)

    // --- initialize system stats
    MaxRunoffFlow = 0.0;
    MaxOutfallFlow = 0.0;
    SysStats.maxTimeStep = 0.0;
    SysStats.minTimeStep = RouteStep;
    SysStats.avgTimeStep = 0.0;
    SysStats.avgStepCount = 0.0;
    SysStats.steadyStateCount = 0.0;
    return 0;
}

//=============================================================================

void  stats_close()
//
//  Input:   none
//  Output:  
//  Purpose: closes the simulation statistics system.
//
{
    int j;

    FREE(SubcatchStats);
    FREE(NodeStats);
    FREE(LinkStats);
    FREE(StorageStats); 
    if ( OutfallStats )
    {
        for ( j=0; j<Nnodes[OUTFALL]; j++ )
            FREE(OutfallStats[j].totalLoad);
        FREE(OutfallStats);
    }
    FREE(PumpStats);                                                           //(5.0.012 - LR)
}

//=============================================================================

////  This function was completely re-written ///                              //(5.0.012 - LR)

void  stats_report()
//
//  Input:   none
//  Output:  none
//  Purpose: reports simulation statistics.
//
{

    // --- report flow routing accuracy statistics
    if ( Nobjects[LINK] > 0 && RouteModel != NO_ROUTING )
    {
        stats_findMaxStats();
        report_writeMaxStats(MaxMassBalErrs, MaxCourantCrit, MAX_STATS);
        report_writeMaxFlowTurns(MaxFlowTurns, MAX_STATS);
        report_writeSysStats(&SysStats);
    }

    // --- report summary statistics
    statsrpt_writeReport();
}

//=============================================================================

void   stats_updateSubcatchStats(int j, double rainVol, double runonVol,
                                 double evapVol, double infilVol,
                                 double runoffVol, double runoff)
//
//  Input:   j = subcatchment index

////  Volumes are now passed in as ft3 instead of ft3/ft2  ////                //(5.0.019 - LR)

//           rainVol   = rainfall + snowfall volume (ft3)
//           runonVol  = runon volume from other subcatchments (ft3)
//           evapVol   = evaporation volume (ft3)
//           infilVol  = infiltration volume (ft3)
//           runoffVol = runoff volume (ft3)
//           runoff    = runoff rate (cfs)
//  Output:  none
//  Purpose: updates totals of runoff components for a specific subcatchment.
//
{
    SubcatchStats[j].precip += rainVol;
    SubcatchStats[j].runon  += runonVol;
    SubcatchStats[j].evap   += evapVol;
    SubcatchStats[j].infil  += infilVol;
    SubcatchStats[j].runoff += runoffVol;
    SubcatchStats[j].maxFlow = MAX(SubcatchStats[j].maxFlow, runoff);
}

//=============================================================================

void  stats_updateMaxRunoff()
//
//   Input:   none
//   Output:  updates global variable MaxRunoffFlow
//   Purpose: updates value of maximum system runoff rate.
//
{
    int j;
    double sysRunoff = 0.0;
    
    for (j=0; j<Nobjects[SUBCATCH]; j++) sysRunoff += Subcatch[j].newRunoff;
    MaxRunoffFlow = MAX(MaxRunoffFlow, sysRunoff);
}    

//=============================================================================

void   stats_updateFlowStats(double tStep, DateTime aDate, int stepCount,
                             int steadyState)
//
//  Input:   tStep = routing time step (sec)
//           aDate = current date/time
//           stepCount = # steps required to solve routing at current time period
//           steadyState = TRUE if steady flow conditions exist
//  Output:  none
//  Purpose: updates various flow routing statistics at current time period.
//
{
    int   j;

    // --- update stats only after reporting period begins
    if ( aDate < ReportStart ) return;
    SysOutfallFlow = 0.0;

    // --- update node & link stats
    for ( j=0; j<Nobjects[NODE]; j++ )
        stats_updateNodeStats(j, tStep, aDate);
    for ( j=0; j<Nobjects[LINK]; j++ )
        stats_updateLinkStats(j, tStep, aDate);

    // --- update time step stats
    //     (skip initial time step for min. value)
    if ( StepCount > 1 )
    {
        SysStats.minTimeStep = MIN(SysStats.minTimeStep, tStep);
    }
    SysStats.avgTimeStep += tStep;
    SysStats.maxTimeStep = MAX(SysStats.maxTimeStep, tStep);

    // --- update iteration step count stats
    SysStats.avgStepCount += stepCount;

    // --- update count of times in steady state
    SysStats.steadyStateCount += steadyState;

    // --- update max. system outfall flow
    MaxOutfallFlow = MAX(MaxOutfallFlow, SysOutfallFlow);
}

//=============================================================================
   
void stats_updateCriticalTimeCount(int node, int link)
//
//  Input:   node = node index
//           link = link index
//  Output:  none
//  Purpose: updates count of times a node or link was time step-critical.
//
{
    if      ( node >= 0 ) NodeStats[node].timeCourantCritical += 1.0;
    else if ( link >= 0 ) LinkStats[link].timeCourantCritical += 1.0;
}

//=============================================================================

void stats_updateNodeStats(int j, double tStep, DateTime aDate)
//
//  Input:   j = node index
//           tStep = routing time step (sec)
//           aDate = current date/time
//  Output:  none
//  Purpose: updates flow statistics for a node.
//
{
    int    k, p;
    double newVolume = Node[j].newVolume;
    double newDepth = Node[j].newDepth;
    double delta;                                                              //(5.0.012 - LR)
    int    canPond = (AllowPonding && Node[j].pondedArea > 0.0);               //(5.0.019 - LR)

    // --- update depth statistics                                             //(5.0.019 - LR)
    NodeStats[j].avgDepth += newDepth;
    if ( newDepth > NodeStats[j].maxDepth )
    {
        NodeStats[j].maxDepth = newDepth;
        NodeStats[j].maxDepthDate = aDate;
    }
    delta = fabs(newDepth - Node[j].oldDepth);                                 //(5.0.012 - LR)
    if ( delta > NodeStats[j].maxDepthChange )                                 //(5.0.012 - LR)
        NodeStats[j].maxDepthChange = delta;                                   //(5.0.012 - LR)
    
////  Following code segment was modified for release 5.0.019.  ////           //(5.0.019 - LR)
    // --- update flooding, ponding, and surcharge statistics
    if ( Node[j].type != OUTFALL )
    {
        if ( newVolume > Node[j].fullVolume || Node[j].overflow > 0.0 )
        {
            NodeStats[j].timeFlooded += tStep;
            NodeStats[j].volFlooded += Node[j].overflow * tStep;
            if ( canPond ) NodeStats[j].maxPondedVol =
                MAX(NodeStats[j].maxPondedVol,
                    (newVolume - Node[j].fullVolume));
        }
        if ( newDepth + Node[j].invertElev + FUDGE >= Node[j].crownElev )
        {
                NodeStats[j].timeSurcharged += tStep;                          //(5.0.022 - LR)
        }
    }
////  End of modified code segment.  ////                                      //(5.0.019 - LR)

    // --- update storage statistics
    if ( Node[j].type == STORAGE )
    {
        k = Node[j].subIndex;
        StorageStats[k].avgVol += newVolume;
        StorageStats[k].losses += Storage[Node[j].subIndex].losses;            //(5.0.018-LR)
        newVolume = MIN(newVolume, Node[j].fullVolume);                        //(5.0.019 - LR)
        if ( newVolume > StorageStats[k].maxVol )
        {
            StorageStats[k].maxVol = newVolume;
            StorageStats[k].maxVolDate = aDate;
        }
        StorageStats[k].maxFlow = MAX(StorageStats[k].maxFlow, Node[j].outflow);
    }

    // --- update outfall statistics
    if ( Node[j].type == OUTFALL && Node[j].inflow >= MIN_RUNOFF_FLOW )
    {
        k = Node[j].subIndex;
        OutfallStats[k].avgFlow += Node[j].inflow;
        OutfallStats[k].maxFlow = MAX(OutfallStats[k].maxFlow, Node[j].inflow);
        OutfallStats[k].totalPeriods++;
        for (p=0; p<Nobjects[POLLUT]; p++)
        {
            OutfallStats[k].totalLoad[p] += Node[j].inflow *
                Node[j].newQual[p] * LperFT3 * tStep * Pollut[p].mcf;
        }
        SysOutfallFlow += Node[j].inflow;
    }

    // --- update inflow statistics                                            //(5.0.019 - LR)
    NodeStats[j].totLatFlow += ( (Node[j].oldLatFlow + Node[j].newLatFlow) *   //(5.0.012 - LR)
                                 0.5 * tStep );                                //(5.0.012 - LR)

    NodeStats[j].maxLatFlow = MAX(Node[j].newLatFlow, NodeStats[j].maxLatFlow);
    if ( Node[j].inflow > NodeStats[j].maxInflow )
    {
        NodeStats[j].maxInflow = Node[j].inflow;
        NodeStats[j].maxInflowDate = aDate;
    }

    // --- update overflow statistics                                          //(5.0.019 - LR)
    if ( Node[j].overflow > NodeStats[j].maxOverflow )
    {
        NodeStats[j].maxOverflow = Node[j].overflow;
        NodeStats[j].maxOverflowDate = aDate;
    }
}

//=============================================================================

void  stats_updateLinkStats(int j, double tStep, DateTime aDate)
//
//  Input:   j = link index
//           tStep = routing time step (sec)
//           aDate = current date/time
//  Output:  none
//  Purpose: updates flow statistics for a link.
//
{
    int    k;
    double q, v;
    double dq;                                                                 //(5.0.010 - LR)

    // --- update max. flow
    dq = Link[j].newFlow - Link[j].oldFlow;                                    //(5.0.010 - LR)
    q = fabs(Link[j].newFlow);
    if ( q > LinkStats[j].maxFlow )
    {
        LinkStats[j].maxFlow = q;
        LinkStats[j].maxFlowDate = aDate;
    }

    // --- update max. velocity
    v = link_getVelocity(j, q, Link[j].newDepth);
    if ( v > LinkStats[j].maxVeloc )
    {
        LinkStats[j].maxVeloc = v;
        LinkStats[j].maxVelocDate = aDate;
    }

    // --- update max. depth
    if ( Link[j].newDepth > LinkStats[j].maxDepth )
    {
        LinkStats[j].maxDepth = Link[j].newDepth;
    }

    if ( Link[j].type == PUMP )
    {
        if ( q >= Link[j].qFull )
            LinkStats[j].timeFullFlow += tStep;                                //(5.0.012 - LR)
        if ( q > MIN_RUNOFF_FLOW )                                             //(5.0.012 - LR)
        {                                                                      //(5.0.012 - LR)
            k = Link[j].subIndex;                                              //(5.0.012 - LR)
            PumpStats[k].minFlow = MIN(PumpStats[k].minFlow, q);               //(5.0.022 - LR)
            PumpStats[k].maxFlow = LinkStats[j].maxFlow;                       //(5.0.012 - LR)
            PumpStats[k].avgFlow += q;                                         //(5.0.012 - LR)
            PumpStats[k].volume += q*tStep;                                    //(5.0.012 - LR)
            PumpStats[k].utilized += tStep;                                    //(5.0.012 - LR)
            PumpStats[k].energy += link_getPower(j)*tStep/3600.0;              //(5.0.012 - LR)
            if ( Link[j].flowClass == DN_DRY )                                 //(5.0.022 - LR)
                PumpStats[k].offCurveLow += tStep;                             //(5.0.022 - LR)
            if ( Link[j].flowClass == UP_DRY )                                 //(5.0.022 - LR)
                PumpStats[k].offCurveHigh += tStep;                            //(5.0.022 - LR)
            if ( Link[j].oldFlow < MIN_RUNOFF_FLOW )                           //(5.0.022 - LR)
                PumpStats[k].startUps++;                                       //(5.0.022 - LR)
            PumpStats[k].totalPeriods++;                                       //(5.0.012 - LR)
            LinkStats[j].timeSurcharged += tStep;                              //(5.0.012 - LR)
            LinkStats[j].timeFullUpstream += tStep;                            //(5.0.012 - LR)
            LinkStats[j].timeFullDnstream += tStep;                            //(5.0.012 - LR)
        }                                                                      //(5.0.012 - LR) 
    }
    else if ( Link[j].type == CONDUIT )
    {
        // --- update sums used to compute avg. Fr and flow change
        LinkStats[j].avgFroude += Link[j].froude; 
        LinkStats[j].avgFlowChange += fabs(dq);
    
        // --- update flow classification distribution
        k = Link[j].flowClass;
        if ( k >= 0 && k < MAX_FLOW_CLASSES )
        {
            ++LinkStats[j].timeInFlowClass[k];
        }

        // --- update time conduit is full
        k = Link[j].subIndex;
        if ( q >= Link[j].qFull ) LinkStats[j].timeFullFlow += tStep;          //(5.0.012 - LR) )
        if ( Conduit[k].capacityLimited )                                      //(5.0.012 - LR)
            LinkStats[j].timeCapacityLimited += tStep;                         //(5.0.012 - LR)
        if ( Link[j].newDepth >= Link[j].xsect.yFull )                         //(5.0.012 - LR)
        {
            LinkStats[j].timeSurcharged += tStep;
            LinkStats[j].timeFullUpstream += tStep;                            //(5.0.012 - LR)
            LinkStats[j].timeFullDnstream += tStep;                            //(5.0.012 - LR)
        }
        else if ( Conduit[k].a1 >= Link[j].xsect.aFull )                       //(5.0.012 - LR)
            LinkStats[j].timeFullUpstream += tStep;                            //(5.0.012 - LR)
        else if ( Conduit[k].a2 >= Link[j].xsect.aFull )                       //(5.0.012 - LR)
            LinkStats[j].timeFullDnstream += tStep;                            //(5.0.012 - LR)
    }

    // --- update flow turn count                                              //(5.0.010 - LR)
    k = LinkStats[j].flowTurnSign;                                             //(5.0.010 - LR)
    LinkStats[j].flowTurnSign = SGN(dq);                                       //(5.0.010 - LR)
    if ( fabs(dq) > 0.001 &&  k * LinkStats[j].flowTurnSign < 0 )              //(5.0.010 - LR)
            LinkStats[j].flowTurns++;                                          //(5.0.010 - LR)
}

//=============================================================================

void  stats_findMaxStats()
//
//  Input:   none
//  Output:  none
//  Purpose: finds nodes & links with highest mass balance errors
//           & highest times Courant time-step critical.
//
{
    int    j;
    double x;

    // --- initialize max. stats arrays
    for (j=0; j<MAX_STATS; j++)
    {
        MaxMassBalErrs[j].objType = NODE;
        MaxMassBalErrs[j].index   = -1;
        MaxMassBalErrs[j].value   = -1.0;                                      //(5.0.010 - LR)
        MaxCourantCrit[j].index   = -1;
        MaxCourantCrit[j].value   = -1.0;                                      //(5.0.010 - LR)
        MaxFlowTurns[j].index     = -1;                                        //(5.0.010 - LR)
        MaxFlowTurns[j].value     = -1.0;                                      //(5.0.010 - LR)
    }

    // --- find links with most flow turns                                     //(5.0.010 - LR)
    if ( StepCount > 2 )                                                       //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR) 
        for (j=0; j<Nobjects[LINK]; j++)                                       //(5.0.010 - LR)
        {                                                                      //(5.0.010 - LR)
            x = 100.0 * LinkStats[j].flowTurns / (2./3.*(StepCount-2));        //(5.0.010 - LR)
            stats_updateMaxStats(MaxFlowTurns, LINK, j, x);                    //(5.0.010 - LR)
        }                                                                      //(5.0.010 - LR)
    }                                                                          //(5.0.010 - LR)

    // --- find nodes with largest mass balance errors
    for (j=0; j<Nobjects[NODE]; j++)
    {
        // --- skip terminal nodes and nodes with negligible inflow
        if ( Node[j].degree <= 0  ) continue;
        if ( NodeInflow[j] <= 0.1 ) continue;

        // --- evaluate mass balance error
        //     (Note: NodeInflow & NodeOutflow include any initial and final   //(5.0.018 - LR)
        //            stored volumes, respectively).                           //(5.0.018 - LR)
        if ( NodeInflow[j]  > 0.0 )
            x = 1.0 - NodeOutflow[j] / NodeInflow[j];                          //(5.0.018 - LR)
        else if ( NodeOutflow[j] > 0.0 ) x = -1.0;
        else                             x = 0.0;
        stats_updateMaxStats(MaxMassBalErrs, NODE, j, 100.0*x);
    }

    // --- stop if not using a variable time step
    if ( RouteModel != DW || CourantFactor == 0.0 ) return;

    // --- find nodes most frequently Courant critical
    for (j=0; j<Nobjects[NODE]; j++)
    {
        x = NodeStats[j].timeCourantCritical / StepCount;
        stats_updateMaxStats(MaxCourantCrit, NODE, j, 100.0*x);
    }

    // --- find links most frequently Courant critical
    for (j=0; j<Nobjects[LINK]; j++)
    {
        x = LinkStats[j].timeCourantCritical / StepCount;
        stats_updateMaxStats(MaxCourantCrit, LINK, j, 100.0*x);
    }
}

//=============================================================================

void  stats_updateMaxStats(TMaxStats maxStats[], int i, int j, double x)
//
//  Input:   maxStats[] = array of critical statistics values
//           i = object category (NODE or LINK)
//           j = object index
//           x = value of statistic for the object
//  Output:  none
//  Purpose: updates the collection of most critical statistics
//
{
    int   k;
    TMaxStats maxStats1, maxStats2;
    maxStats1.objType = i;
    maxStats1.index   = j;
    maxStats1.value   = x;
    for (k=0; k<MAX_STATS; k++)
    {
        if ( fabs(maxStats1.value) > fabs(maxStats[k].value) )
        {
            maxStats2 = maxStats[k];
            maxStats[k] = maxStats1;
            maxStats1 = maxStats2;
        }
    }
}

//=============================================================================
