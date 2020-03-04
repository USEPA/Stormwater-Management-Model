//-----------------------------------------------------------------------------
//   stats.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//             09/15/14   (Build 5.1.007)
//             03/19/15   (Build 5.1.008)
//             08/01/16   (Build 5.1.011)
//             03/14/17   (Build 5.1.012)
//             05/10/18   (Build 5.1.013)
//   Author:   L. Rossman (EPA)
//             R. Dickinson (CDM)
//
//   Simulation statistics functions.
//
//   Build 5.1.007:
//   - Exfiltration losses added to storage node statistics.
//
//   Build 5.1.008:
//   - Support for updating groundwater statistics added.
//   - Support for updating maximum reported nodal depths added.
//   - OpenMP parallelization applied to updating node and link flow statistics.
//   - Updating of time that conduit is upstrm/dnstrm full was modified.
//
//   Build 5.1.011:
//   - Surcharging is now evaluated only under dynamic wave flow routing and
//     storage nodes cannot be classified as surcharged.
//
//   Build 5.1.012:
//   - Time step statistics now evaluated only in non-steady state periods.
//   - Check for full conduit flow now accounts for number of barrels.
//
//   Build 5.1.013:
//   - Include omp.h protected against lack of compiler support for OpenMP.
//   - Statistics on impervious and pervious runoff totals added.
//   - Storage nodes with a non-zero surcharge depth (e.g. enclosed tanks)
//     can now be classified as being surcharged.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "headers.h"
#include "swmm5.h"
#if defined(_OPENMP)                                                           //(5.1.013)
#include <omp.h>
#endif

//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
#define MAX_STATS 5
static TSysStats       SysStats;
static TMaxStats       MaxMassBalErrs[MAX_STATS];
static TMaxStats       MaxCourantCrit[MAX_STATS];
static TMaxStats       MaxFlowTurns[MAX_STATS];
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
//  stats_updateGwaterStats       (called from gwater_getGroundwater)
//  stats_updateFlowStats         (called from routing_execute)
//  stats_updateCriticalTimeCount (called from getVariableStep in dynwave.c)
//  stats_updateMaxNodeDepth      (called from output_saveNodeResults)

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
    PumpStats = NULL;

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
            SubcatchStats[j].impervRunoff = 0.0;                               //(5.1.013)
            SubcatchStats[j].pervRunoff   = 0.0;                               //
        }

        for (j=0; j<Nobjects[SUBCATCH]; j++)
        {
            if ( Subcatch[j].groundwater == NULL ) continue;
            Subcatch[j].groundwater->stats.avgUpperMoist = 0.0;
            Subcatch[j].groundwater->stats.avgWaterTable = 0.0;
            Subcatch[j].groundwater->stats.infil = 0.0;
            Subcatch[j].groundwater->stats.latFlow = 0.0;
            Subcatch[j].groundwater->stats.deepFlow = 0.0;
            Subcatch[j].groundwater->stats.evap = 0.0;
            Subcatch[j].groundwater->stats.maxFlow = 0.0;
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
        NodeStats[j].maxRptDepth = 0.0;
        NodeStats[j].volFlooded = 0.0;
        NodeStats[j].timeFlooded = 0.0;
        NodeStats[j].timeSurcharged = 0.0;
        NodeStats[j].timeCourantCritical = 0.0;
        NodeStats[j].totLatFlow = 0.0;
        NodeStats[j].maxLatFlow = 0.0;
        NodeStats[j].maxInflow = 0.0;
        NodeStats[j].maxOverflow = 0.0;
        NodeStats[j].maxPondedVol = 0.0;
        NodeStats[j].maxInflowDate = StartDateTime;
        NodeStats[j].maxOverflowDate = StartDateTime;
    }

    // --- initialize link stats
    if ( LinkStats ) for ( j = 0; j < Nobjects[LINK]; j++ )
    {
        LinkStats[j].maxFlow = 0.0;
        LinkStats[j].maxVeloc = 0.0;
        LinkStats[j].maxDepth = 0.0;
        LinkStats[j].timeSurcharged = 0.0;
        LinkStats[j].timeFullUpstream = 0.0;
        LinkStats[j].timeFullDnstream = 0.0;
        LinkStats[j].timeFullFlow = 0.0;
        LinkStats[j].timeCapacityLimited = 0.0;
        LinkStats[j].timeCourantCritical = 0.0;
        for (k=0; k<MAX_FLOW_CLASSES; k++)
            LinkStats[j].timeInFlowClass[k] = 0.0;
        LinkStats[j].flowTurns = 0;
        LinkStats[j].flowTurnSign = 0;
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
        else for ( k = 0; k < Nobjects[NODE]; k++ )
        {
            if ( Node[k].type != STORAGE ) continue;
            j = Node[k].subIndex;
            StorageStats[j].initVol = Node[k].newVolume;
            StorageStats[j].avgVol = 0.0;
            StorageStats[j].maxVol = 0.0;
            StorageStats[j].maxFlow = 0.0;
            StorageStats[j].evapLosses = 0.0;
            StorageStats[j].exfilLosses = 0.0;
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

    // --- allocate memory & initialize pumping statistics
    if ( Nlinks[PUMP] > 0 ) 
    { 
        PumpStats = (TPumpStats *) calloc(Nlinks[PUMP], sizeof(TPumpStats));
        if ( !PumpStats ) 
        {
            report_writeErrorMsg(ERR_MEMORY, "");
            return ErrorCode;
        }
        else for ( j = 0; j < Nlinks[PUMP]; j++ )
        {
            PumpStats[j].utilized = 0.0;
            PumpStats[j].minFlow  = 0.0;
            PumpStats[j].avgFlow  = 0.0;
            PumpStats[j].maxFlow  = 0.0; 
            PumpStats[j].volume   = 0.0;
            PumpStats[j].energy   = 0.0;
            PumpStats[j].startUps = 0;
            PumpStats[j].offCurveLow = 0.0; 
            PumpStats[j].offCurveHigh = 0.0;
        } 
    } 

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
    FREE(PumpStats);
}

//=============================================================================

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
	                             double impervVol, double pervVol,
                                 double runoffVol, double runoff)
//
//  Input:   j = subcatchment index
//           rainVol   = rainfall + snowfall volume (ft3)
//           runonVol  = runon volume from other subcatchments (ft3)
//           evapVol   = evaporation volume (ft3)
//           infilVol  = infiltration volume (ft3)
//           impervVol = impervious runoff volume (ft3)
//           pervVol   = pervious runoff volume (ft3)
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
	SubcatchStats[j].impervRunoff += impervVol;                                //(5.1.013)
	SubcatchStats[j].pervRunoff += pervVol;                                    //
}

//=============================================================================

void  stats_updateGwaterStats(int j, double infil, double evap, double latFlow,
                              double deepFlow, double theta, double waterTable,
                              double tStep)
{
    Subcatch[j].groundwater->stats.infil += infil * tStep;
    Subcatch[j].groundwater->stats.evap += evap * tStep;
    Subcatch[j].groundwater->stats.latFlow += latFlow * tStep;
    Subcatch[j].groundwater->stats.deepFlow += deepFlow * tStep;
    Subcatch[j].groundwater->stats.avgUpperMoist += theta * tStep;
    Subcatch[j].groundwater->stats.avgWaterTable += waterTable * tStep;
    Subcatch[j].groundwater->stats.finalUpperMoist = theta;
    Subcatch[j].groundwater->stats.finalWaterTable = waterTable;
    if ( fabs(latFlow) > fabs(Subcatch[j].groundwater->stats.maxFlow) )
    {
        Subcatch[j].groundwater->stats.maxFlow = latFlow;
    }
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

void   stats_updateMaxNodeDepth(int j, double depth)
//
//   Input:   j = node index
//            depth = water depth at node at current reporting time (ft)
//   Output:  none
//   Purpose: updates a node's maximum depth recorded at reporting times.
//
{
    if ( NodeStats != NULL )
        NodeStats[j].maxRptDepth = MAX(NodeStats[j].maxRptDepth, depth);
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
#pragma omp parallel num_threads(NumThreads)
{
    #pragma omp for
    for ( j=0; j<Nobjects[NODE]; j++ )
        stats_updateNodeStats(j, tStep, aDate);
    #pragma omp for
    for ( j=0; j<Nobjects[LINK]; j++ )
        stats_updateLinkStats(j, tStep, aDate);
}

    // --- update count of times in steady state
    SysStats.steadyStateCount += steadyState;

    // --- update time step stats if not in steady state
	if ( steadyState == FALSE )
	{
        // --- skip initial time step for min. value)
        if ( OldRoutingTime > 0 )
        {
            SysStats.minTimeStep = MIN(SysStats.minTimeStep, tStep);
        }
        SysStats.avgTimeStep += tStep;
        SysStats.maxTimeStep = MAX(SysStats.maxTimeStep, tStep);

        // --- update iteration step count stats
        SysStats.avgStepCount += stepCount;
	}

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
    double yCrown = Node[j].crownElev - Node[j].invertElev;
    int    canPond = (AllowPonding && Node[j].pondedArea > 0.0);

    // --- update depth statistics
    NodeStats[j].avgDepth += newDepth;
    if ( newDepth > NodeStats[j].maxDepth )
    {
        NodeStats[j].maxDepth = newDepth;
        NodeStats[j].maxDepthDate = aDate;
    }
    
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

        // --- for dynamic wave routing, classify a node as                    //(5.1.013)
        //     surcharged if its water level exceeds its crown elev.
        if (RouteModel == DW)                                                  //(5.1.013)
        {
            if ((Node[j].type != STORAGE || Node[j].surDepth > 0.0) &&         //(5.1.013)
                newDepth + Node[j].invertElev + FUDGE >= Node[j].crownElev)
            {
                NodeStats[j].timeSurcharged += tStep;
            }
        }
    }

    // --- update storage statistics
    if ( Node[j].type == STORAGE )
    {
        k = Node[j].subIndex;
        StorageStats[k].avgVol += newVolume;
        StorageStats[k].evapLosses += 
            Storage[Node[j].subIndex].evapLoss; 
        StorageStats[k].exfilLosses +=
            Storage[Node[j].subIndex].exfilLoss; 

        newVolume = MIN(newVolume, Node[j].fullVolume);
        if ( newVolume > StorageStats[k].maxVol )
        {
            StorageStats[k].maxVol = newVolume;
            StorageStats[k].maxVolDate = aDate;
        }
        StorageStats[k].maxFlow = MAX(StorageStats[k].maxFlow, Node[j].outflow);
    }

    // --- update outfall statistics
    if ( Node[j].type == OUTFALL ) 
    {
        k = Node[j].subIndex;
        if ( Node[j].inflow >= MIN_RUNOFF_FLOW )
        {
            OutfallStats[k].avgFlow += Node[j].inflow;
            OutfallStats[k].maxFlow = MAX(OutfallStats[k].maxFlow, Node[j].inflow);
            OutfallStats[k].totalPeriods++;
        }
        for (p=0; p<Nobjects[POLLUT]; p++)
        {
            OutfallStats[k].totalLoad[p] += Node[j].inflow * 
            Node[j].newQual[p] * tStep;
        }
        SysOutfallFlow += Node[j].inflow;
    }

    // --- update inflow statistics
    NodeStats[j].totLatFlow += ( (Node[j].oldLatFlow + Node[j].newLatFlow) * 
                                 0.5 * tStep );
    if ( fabs(Node[j].newLatFlow) > fabs(NodeStats[j].maxLatFlow) )
        NodeStats[j].maxLatFlow = Node[j].newLatFlow;
    if ( Node[j].inflow > NodeStats[j].maxInflow )
    {
        NodeStats[j].maxInflow = Node[j].inflow;
        NodeStats[j].maxInflowDate = aDate;
    }

    // --- update overflow statistics
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
    double dq;

    // --- update max. flow
    dq = Link[j].newFlow - Link[j].oldFlow;
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
    }

    // --- update max. depth
    if ( Link[j].newDepth > LinkStats[j].maxDepth )
    {
        LinkStats[j].maxDepth = Link[j].newDepth;
    }

    if ( Link[j].type == PUMP )
    {
        if ( q >= Link[j].qFull )
            LinkStats[j].timeFullFlow += tStep;
        if ( q > MIN_RUNOFF_FLOW )
        {
            k = Link[j].subIndex;
            PumpStats[k].minFlow = MIN(PumpStats[k].minFlow, q);
            PumpStats[k].maxFlow = LinkStats[j].maxFlow;
            PumpStats[k].avgFlow += q;
            PumpStats[k].volume += q*tStep;
            PumpStats[k].utilized += tStep;
            PumpStats[k].energy += link_getPower(j)*tStep/3600.0;
            if ( Link[j].flowClass == DN_DRY )
                PumpStats[k].offCurveLow += tStep;
            if ( Link[j].flowClass == UP_DRY )
                PumpStats[k].offCurveHigh += tStep;
            if ( Link[j].oldFlow < MIN_RUNOFF_FLOW )
                PumpStats[k].startUps++;
            PumpStats[k].totalPeriods++;
            LinkStats[j].timeSurcharged += tStep;
            LinkStats[j].timeFullUpstream += tStep;
            LinkStats[j].timeFullDnstream += tStep;
        }
    }
    else if ( Link[j].type == CONDUIT )
    {

        // --- update time under normal flow & inlet control 
        if ( Link[j].normalFlow ) LinkStats[j].timeNormalFlow += tStep;
        if ( Link[j].inletControl ) LinkStats[j].timeInletControl += tStep;
    
        // --- update flow classification distribution
        k = Link[j].flowClass;
        if ( k >= 0 && k < MAX_FLOW_CLASSES )
        {
            ++LinkStats[j].timeInFlowClass[k];
        }

        // --- update time conduit is full
        k = Link[j].subIndex;
        if ( q >= Link[j].qFull * (double)Conduit[k].barrels )
            LinkStats[j].timeFullFlow += tStep; 
        if ( Conduit[k].capacityLimited )
            LinkStats[j].timeCapacityLimited += tStep;

        switch (Conduit[k].fullState)
        {
        case ALL_FULL:
            LinkStats[j].timeSurcharged += tStep;
            LinkStats[j].timeFullUpstream += tStep;
            LinkStats[j].timeFullDnstream += tStep;
            break;
        case UP_FULL:
            LinkStats[j].timeFullUpstream += tStep;
            break;
        case DN_FULL:
            LinkStats[j].timeFullDnstream += tStep;
        }
    }

    // --- update flow turn count
    k = LinkStats[j].flowTurnSign;
    LinkStats[j].flowTurnSign = SGN(dq);
    if ( fabs(dq) > 0.001 &&  k * LinkStats[j].flowTurnSign < 0 )
            LinkStats[j].flowTurns++;
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
        MaxMassBalErrs[j].value   = -1.0;
        MaxCourantCrit[j].index   = -1;
        MaxCourantCrit[j].value   = -1.0;
        MaxFlowTurns[j].index     = -1; 
        MaxFlowTurns[j].value     = -1.0;
    }

    // --- find links with most flow turns 
    if ( StepCount > 2 )
    {
        for (j=0; j<Nobjects[LINK]; j++)
        {
            x = 100.0 * LinkStats[j].flowTurns / (2./3.*(StepCount-2));
            stats_updateMaxStats(MaxFlowTurns, LINK, j, x);
        }
    }

    // --- find nodes with largest mass balance errors
    for (j=0; j<Nobjects[NODE]; j++)
    {
        // --- skip terminal nodes and nodes with negligible inflow
        if ( Node[j].degree <= 0  ) continue;
        if ( NodeInflow[j] <= 0.1 ) continue;

        // --- evaluate mass balance error
        //     (Note: NodeInflow & NodeOutflow include any initial and final
        //            stored volumes, respectively).
        if ( NodeInflow[j]  > 0.0 )
            x = 1.0 - NodeOutflow[j] / NodeInflow[j];
        else if ( NodeOutflow[j] > 0.0 ) x = -1.0;
        else                             x = 0.0;
        stats_updateMaxStats(MaxMassBalErrs, NODE, j, 100.0*x);
    }

    // --- stop if not using a variable time step
    if ( RouteModel != DW || CourantFactor == 0.0 ) return;

    // --- find nodes most frequently Courant critical
    if ( StepCount == 0 ) return;
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
