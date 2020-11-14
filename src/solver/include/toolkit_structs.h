/*
 *  toolkit_structs.h
 *
 *  Created on: November 13, 2020
 *  Updated on:
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/CESER
 *
 */


#ifndef TOOLKIT_STRUCTS_H_
#define TOOLKIT_STRUCTS_H_


// Forward declaration
typedef double DateTime;

// --- Define the SWMM toolkit structures

/// Node stats structure
/** @struct SM_NodeStats
 *  @brief Node Statatistics
 *
 *  @var SM_NodeStats::avgDepth
 *    average node depth (level)
 *  @var SM_NodeStats::maxDepth
 *    max node depth (level) (from routing step)
 *  @var SM_NodeStats::maxDepthDate
 *    date of maximum depth
 *  @var SM_NodeStats::maxRptDepth
 *    max node depth (level) (from reporting step)
 *  @var SM_NodeStats::volFlooded
 *    total volume flooded (volume)
 *  @var SM_NodeStats::timeFlooded
 *    total time flooded
 *  @var SM_NodeStats::timeSurcharged
 *    total time surcharged
 *  @var SM_NodeStats::timeCourantCritical
 *    total time courant critical
 *  @var SM_NodeStats::totLatFlow
 *    total lateral inflow (volume)
 *  @var SM_NodeStats::maxLatFlow
 *    maximum lateral inflow (flowrate)
 *  @var SM_NodeStats::maxInflow
 *    maximum total inflow (flowrate)
 *  @var SM_NodeStats::maxOverflow
 *    maximum flooding (flowrate)
 *  @var SM_NodeStats::maxPondedVol
 *    maximum ponded volume (volume)
 *  @var SM_NodeStats::maxInflowDate
 *    date of maximum inflow
 *  @var SM_NodeStats::maxOverflowDate
 *    date of maximum overflow
 */

typedef struct
{
   double        avgDepth;
   double        maxDepth;
   DateTime      maxDepthDate;
   double        maxRptDepth;
   double        volFlooded;
   double        timeFlooded;
   double        timeSurcharged;
   double        timeCourantCritical;
   double        totLatFlow;
   double        maxLatFlow;
   double        maxInflow;
   double        maxOverflow;
   double        maxPondedVol;
   DateTime      maxInflowDate;
   DateTime      maxOverflowDate;
}  SM_NodeStats;

/** @struct SM_StorageStats
 *  @brief Storage Statatistics
 *
 *  @var SM_StorageStats::initVol
 *    initial volume (volume)
 *  @var SM_StorageStats::avgVol
 *    average volume (volume) (from routing step)
 *  @var SM_StorageStats::maxVol
 *    maximum volume (volume) (from routing step)
 *  @var SM_StorageStats::maxFlow
 *    maximum total inflow (flowrate) (from routing step)
 *  @var SM_StorageStats::evapLosses
 *    evaporation losses (volume)
 *  @var SM_StorageStats::exfilLosses
 *    exfiltration losses (volume)
 *  @var SM_StorageStats::maxVolDate
 *    date of maximum volume
 */
typedef struct
{
   double        initVol;
   double        avgVol;
   double        maxVol;
   double        maxFlow;
   double        evapLosses;
   double        exfilLosses;
   DateTime      maxVolDate;
}  SM_StorageStats;

/** @struct SM_OutfallStats
 *  @brief Outfall Statatistics
 *
 *  @var SM_OutfallStats::avgFlow
 *    average flow (flowrate)
 *  @var SM_OutfallStats::maxFlow
 *    maximum flow (flowrate) (from routing step)
 *  @var SM_OutfallStats::totalLoad
 *    total pollutant load (mass)
 *  @var SM_OutfallStats::totalPeriods
 *    total simulation steps (from routing step)
 */
typedef struct
{
   double       avgFlow;
   double       maxFlow;
   double*      totalLoad;
   int          totalPeriods;
}  SM_OutfallStats;

/** @struct SM_LinkStats
 *  @brief Link Statatistics
 *
 * @var SM_LinkStats::maxFlow
 *   maximum flow (flowrate) (from routing step)
 * @var SM_LinkStats::maxFlowDate
 *   date of maximum flowrate
 * @var SM_LinkStats::maxVeloc
 *   maximum velocity (from routing step)
 * @var SM_LinkStats::maxDepth
 *   maximum depth (level)
 * @var SM_LinkStats::timeNormalFlow
 *   time in normal flow
 * @var SM_LinkStats::timeInletControl
 *   time under inlet control
 * @var SM_LinkStats::timeSurcharged
 *   time surcharged
 * @var SM_LinkStats::timeFullUpstream
 *   time full upstream
 * @var SM_LinkStats::timeFullDnstream
 *   time full downstream
 * @var SM_LinkStats::timeFullFlow
 *   time full flow
 * @var SM_LinkStats::timeCapacityLimited
 *   time capacity limited
 * @var SM_LinkStats::timeInFlowClass
 *   time in flow class:
 *     | class | description |
 *     | :--- | --- |
 *     | DRY | dry conduit |
 *     | UP_DRY | upstream end is dry |
 *     | DN_DRY | downstream end is dry |
 *     | SUBCRITICAL | sub-critical flow |
 *     | SUPCRITICAL | super-critical flow |
 *     | UP_CRITICAL | free-fall at upstream end |
 *     | DN_CRITICAL | free-fall at downstream end |
 * @var SM_LinkStats::timeCourantCritical
 *   time courant critical
 * @var SM_LinkStats::flowTurns
 *   number of flow turns
 * @var SM_LinkStats::flowTurnSign
 *   number of flow turns sign
 */
typedef struct
{
   double        maxFlow;
   DateTime      maxFlowDate;
   double        maxVeloc;
   double        maxDepth;
   double        timeNormalFlow;
   double        timeInletControl;
   double        timeSurcharged;
   double        timeFullUpstream;
   double        timeFullDnstream;
   double        timeFullFlow;
   double        timeCapacityLimited;
   double        timeInFlowClass[7];
   double        timeCourantCritical;
   long          flowTurns;
   int           flowTurnSign;
}  SM_LinkStats;

/** @struct SM_PumpStats
 *  @brief Pump Statistics
 *
 * @var SM_PumpStats::utilized
 *   time utilized
 * @var SM_PumpStats::minFlow
 *   minimum flowrate
 * @var SM_PumpStats::avgFlow
 *   average flowrate
 * @var SM_PumpStats::maxFlow
 *   maximum flowrate
 * @var SM_PumpStats::volume
 *   total pumping volume (volume)
 * @var SM_PumpStats::energy
 *   total energy demand
 * @var SM_PumpStats::offCurveLow
 *   hysteresis low (off depth wrt curve)
 * @var SM_PumpStats::offCurveHigh
 *   hysteresis high (on depth wrt curve)
 * @var SM_PumpStats::startUps
 *   number of start ups
 * @var SM_PumpStats::totalPeriods
 *   total simulation steps (from routing step)
 */
typedef struct
{
   double       utilized;
   double       minFlow;
   double       avgFlow;
   double       maxFlow;
   double       volume;
   double       energy;
   double       offCurveLow;
   double       offCurveHigh;
   int          startUps;
   int          totalPeriods;
}  SM_PumpStats;


/** @struct SM_SubcatchStats
 *  @brief Subcatchment Statistics
 *
 * @var SM_SubcatchStats::precip
 *   total precipication (length)
 * @var SM_SubcatchStats::runon
 *   total runon (volume)
 * @var SM_SubcatchStats::evap
 *   total evaporation (volume)
 * @var SM_SubcatchStats::infil
 *   total infiltration (volume)
 * @var SM_SubcatchStats::runoff
 *   total runoff (volume)
 * @var SM_SubcatchStats::maxFlow
 *   maximum runoff rate (flowrate)
 */
typedef struct
{
    double       precip;
    double       runon;
    double       evap;
    double       infil;
    double       runoff;
    double       maxFlow;
    double       impervRunoff;
    double       pervRunoff;
}  SM_SubcatchStats;


/** @struct SM_RoutingTotals
 *  @brief System Flow Routing Statistics
 *
 * @var SM_RoutingTotals::dwInflow
 *   dry weather inflow
 * @var SM_RoutingTotals::wwInflow
 *   wet weather inflow
 * @var SM_RoutingTotals::gwInflow
 *   groundwater inflow
 * @var SM_RoutingTotals::iiInflow
 *   RDII inflow
 * @var SM_RoutingTotals::exInflow
 *   direct inflow
 * @var SM_RoutingTotals::flooding
 *   internal flooding
 * @var SM_RoutingTotals::outflow
 *   external outflow
 * @var SM_RoutingTotals::evapLoss
 *   evaporation loss
 * @var SM_RoutingTotals::seepLoss
 *   seepage loss
 * @var SM_RoutingTotals::reacted
 *   reaction losses
 * @var SM_RoutingTotals::initStorage
 *   initial storage volume
 * @var SM_RoutingTotals::finalStorage
 *   final storage volume
 * @var SM_RoutingTotals::pctError
 *   continuity error
 */
typedef struct
{
   double        dwInflow;
   double        wwInflow;
   double        gwInflow;
   double        iiInflow;
   double        exInflow;
   double        flooding;
   double        outflow;
   double        evapLoss;
   double        seepLoss;
   double        reacted;
   double        initStorage;
   double        finalStorage;
   double        pctError;
}  SM_RoutingTotals;

/// System runoff stats structure

/** @struct SM_RunoffTotals
 *  @brief System Runoff Statistics
 *
 * @var SM_RunoffTotals::rainfall
 *   rainfall total (depth)
 * @var SM_RunoffTotals::evap
 *   evaporation loss (volume)
 * @var SM_RunoffTotals::infil
 *   infiltration loss (volume)
 * @var SM_RunoffTotals::runoff
 *   runoff volume (volume)
 * @var SM_RunoffTotals::drains
 *   LID drains (volume)
 * @var SM_RunoffTotals::runon
 *   runon from outfalls (volume)
 * @var SM_RunoffTotals::initStorage
 *   inital surface storage (depth)
 * @var SM_RunoffTotals::finalStorage
 *   final surface storage (depth)
 * @var SM_RunoffTotals::initSnowCover
 *   initial snow cover (depth)
 * @var SM_RunoffTotals::finalSnowCover
 *   final snow cover (depth)
 * @var SM_RunoffTotals::snowRemoved
 *   snow removal (depth)
 * @var SM_RunoffTotals::pctError
 *   continuity error (%)
 */
typedef struct
{
   double        rainfall;
   double        evap;
   double        infil;
   double        runoff;
   double        drains;
   double        runon;
   double        initStorage;
   double        finalStorage;
   double        initSnowCover;
   double        finalSnowCover;
   double        snowRemoved;
   double        pctError;
}  SM_RunoffTotals;


#endif /* TOOLKIT_STRUCTS_H_ */