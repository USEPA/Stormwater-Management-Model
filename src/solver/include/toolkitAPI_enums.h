#ifndef TOOLKITAPI_ENUMS_H_
#define TOOLKITAPI_ENUMS_H_

#include "../datetime.h"

/// Object type codes
typedef enum {
    SM_GAGE         = 0,  /**< Rain gage */
    SM_SUBCATCH     = 1,  /**< Subcatchment */
    SM_NODE         = 2,  /**< Conveyance system node */
    SM_LINK         = 3,  /**< Conveyance system link */
    SM_POLLUT       = 4,  /**< Pollutant */
    SM_LANDUSE      = 5,  /**< Land use category */
    SM_TIMEPATTERN  = 6,  /**< Dry weather flow time pattern */
    SM_CURVE        = 7,  /**< Generic table of values */
    SM_TSERIES      = 8,  /**< Generic time series of values */
    SM_CONTROL      = 9,  /**< Conveyance system control rules */
    SM_TRANSECT     = 10, /**< Irregular channel cross-section */
    SM_AQUIFER      = 11, /**< Groundwater aquifer */
    SM_UNITHYD      = 12, /**< RDII unit hydrograph */
    SM_SNOWMELT     = 13, /**< Snowmelt parameter set */
    SM_SHAPE        = 14, /**< Custom conduit shape */
    SM_LID          = 15  /**< LID treatment units */
} SM_ObjectType;

/// Node object type codes
typedef enum {
    SM_JUNCTION    = 0,  /**< Manhole Junction */
    SM_OUTFALL     = 1,  /**< Outfall */
    SM_STORAGE     = 2,  /**< Storage */
    SM_DIVIDER     = 3   /**< Divider */
} SM_NodeType;

/// Link object type codes
typedef enum {
    SM_CONDUIT     = 0,  /**< Conduit */
    SM_PUMP        = 1,  /**< Pump */
    SM_ORIFICE     = 2,  /**< Orifice */
    SM_WEIR        = 3,  /**< Weir */
    SM_OUTLET      = 4   /**< Outlet */
} SM_LinkType;

/// Simulation Option codes
typedef enum {
    SM_STARTDATE    = 0, /**< Simulation Start Date */
    SM_ENDDATE      = 1, /**< Simulation End Date */
    SM_REPORTDATE   = 2  /**< Simulation Report Start Date */
} SM_TimePropety;

/// Simulation Unit Codes
typedef enum {
    SM_SYSTEMUNIT   = 0, /**< System Units */
    SM_FLOWUNIT     = 1, /**< Flow Units */
} SM_Units;

/// Simulation Options
typedef enum {
    SM_ALLOWPOND    = 0, /**< Allow Ponding */
    SM_SKIPSTEADY   = 1, /**< Skip Steady State*/
    SM_IGNORERAIN   = 2, /**< Ignore Rainfall*/
    SM_IGNORERDII   = 3, /**< Ignore RDII */
    SM_IGNORESNOW   = 4, /**< Ignore Snowmelt */
    SM_IGNOREGW     = 5, /**< Ignore Groundwater */
    SM_IGNOREROUTE  = 6, /**< Ignore Routing */
    SM_IGNORERQUAL  = 7  /**< Ignore Quality */
} SM_SimOption;

/// Simulation Settings
typedef enum {
    SM_ROUTESTEP     = 0,  /**< Routing Step (sec) */
    SM_MINROUTESTEP  = 1,  /**< Minimum Routing Step (sec) */
    SM_LENGTHSTEP    = 2,  /**< Lengthening Step (sec) */
    SM_STARTDRYDAYS  = 3,  /**< Antecedent dry days */
    SM_COURANTFACTOR = 4,  /**< Courant time step factor */
    SM_MINSURFAREA   = 5,  /**< Minimum nodal surface area */
    SM_MINSLOPE      = 6,  /**< Minimum conduit slope */
    SM_RUNOFFERROR   = 7,  /**< Runoff continuity error */
    SM_GWERROR       = 8,  /**< Groundwater continuity error */
    SM_FLOWERROR     = 9,  /**< Flow routing error */
    SM_QUALERROR     = 10, /**< Quality routing error */
    SM_HEADTOL       = 11, /**< DW routing head tolerance (ft) */
    SM_SYSFLOWTOL    = 12, /**< Tolerance for steady system flow */
    SM_LATFLOWTOL    = 13  /**< Tolerance for steady nodal inflow */
} SM_SimSetting;

/// Node property codes
typedef enum {
    SM_INVERTEL     = 0,  /**< Invert Elevation */
    SM_FULLDEPTH    = 1,  /**< Full Depth */
    SM_SURCHDEPTH   = 2,  /**< Surcharge Depth */
    SM_PONDAREA     = 3,  /**< Ponding Area */
    SM_INITDEPTH    = 4,  /**< Initial Depth */
} SM_NodeProperty;

/// Link property codes
typedef enum {
    SM_OFFSET1      = 0,  /**< Inlet Offset */
    SM_OFFSET2      = 1,  /**< Outlet Offset */
    SM_INITFLOW     = 2,  /**< Initial Flow Rate */
    SM_FLOWLIMIT    = 3,  /**< Flow limit */
    SM_INLETLOSS    = 4,  /**< Inlet Loss */
    SM_OUTLETLOSS   = 5,  /**< Outles Loss */
    SM_AVELOSS      = 6,  /**< Average Loss */
} SM_LinkProperty;

/// Subcatchment property codes
typedef enum {
    SM_WIDTH        = 0,  /**< Width */
    SM_AREA         = 1,  /**< Area */
    SM_FRACIMPERV   = 2,  /**< Impervious Fraction */
    SM_SLOPE        = 3,  /**< Slope */
    SM_CURBLEN      = 4   /**< Curb Length */
} SM_SubcProperty;

/// Node result property codes
typedef enum {
    SM_TOTALINFLOW    = 0,  /**< Total Inflow */
    SM_TOTALOUTFLOW   = 1,  /**< Total Outflow */
    SM_LOSSES         = 2,  /**< Node Losses */
    SM_NODEVOL        = 3,  /**< Stored Volume */
    SM_NODEFLOOD      = 4,  /**< Flooding Rate */
    SM_NODEDEPTH      = 5,  /**< Node Depth */
    SM_NODEHEAD       = 6,  /**< Node Head */
    SM_LATINFLOW      = 7   /**< Lateral Inflow Rate */
} SM_NodeResult;

/// Node pollutant result property codes
typedef enum {
    SM_NODEQUAL       = 0,  /**< Current Node Quality */
} SM_NodePollut;

/// Link result property codes
typedef enum {
    SM_LINKFLOW        = 0,  /**< Flowrate */
    SM_LINKDEPTH       = 1,  /**< Depth */
    SM_LINKVOL         = 2,  /**< Volume */
    SM_USSURFAREA      = 3,  /**< Upstream Surface Area */
    SM_DSSURFAREA      = 4,  /**< Downstream Surface Area */
    SM_SETTING         = 5,  /**< Setting */
    SM_TARGETSETTING   = 6,  /**< Target Setting */
    SM_FROUDE          = 7   /**< Froude Number */
} SM_LinkResult;

/// Link pollutant result property codes
typedef enum {
    SM_LINKQUAL      = 0,  /**< Current Link Quality */
    SM_TOTALLOAD     = 1,  /**< Total Quality Mass Loading */
} SM_LinkPollut;

/// Subcatchment result property codes
typedef enum {
    SM_SUBCRAIN      = 0,  /**< Rainfall Rate */
    SM_SUBCEVAP      = 1,  /**< Evaporation Loss */
    SM_SUBCINFIL     = 2,  /**< Infiltration Loss */
    SM_SUBCRUNON     = 3,  /**< Runon Rate */
    SM_SUBCRUNOFF    = 4,  /**< Runoff Rate */
    SM_SUBCSNOW      = 5,  /**< Snow Depth */
} SM_SubcResult;

/// Subcatchment pollutant result property codes
typedef enum {
    SM_BUILDUP        = 0,  /**< Pollutant Buildup Load */
    SM_CPONDED        = 1,  /**< Ponded Pollutant Concentration */
    SM_SUBCQUAL       = 2,  /**< Current Pollutant Runoff Quality */
    SM_SUBCTOTALLOAD  = 3,  /**< Total Pollutant Washoff load */
} SM_SubcPollut;

/// Gage precip array property codes
typedef enum {
    SM_TOTALPRECIP   = 0,  /**< Total Precipitation Rate */
    SM_RAINFALL      = 1,  /**< Rainfall Rate */
    SM_SNOWFALL      = 2   /**< Snowfall Rate */
} SM_GagePrecip;

/// Lid control layer codes
typedef enum {
    SM_SURFACE      = 0,  /**< Lid Surface Layer */
    SM_SOIL         = 1,  /**< Lid Soil Layer */
    SM_STOR         = 2,  /**< Lid Storage Layer */
    SM_PAVE         = 3,  /**< Lid Pavement Layer */
    SM_DRAIN        = 4,  /**< Lid Underdrain Layer */
    SM_DRAINMAT     = 5,  /**< Lid Drainage Mat Layer */
} SM_LidLayer;

/// Lid control layer property codes
typedef enum {
    SM_THICKNESS    = 0,  /**< Storage Height */
    SM_VOIDFRAC     = 1,  /**< Available Fraction of Storage Volume */
    SM_ROUGHNESS    = 2,  /**< Manning n */
    SM_SURFSLOPE    = 3,  /**< Surface Slope (fraction) */
    SM_SIDESLOPE    = 4,  /**< Side Slope (run/rise) */
    SM_ALPHA        = 5,  /**< Slope/Roughness Term in Manning Eqn */
    SM_POROSITY     = 6,  /**< Void Volume / Total Volume */
    SM_FIELDCAP     = 7,  /**< Field Capacity */
    SM_WILTPOINT    = 8,  /**< Wilting Point */
    SM_SUCTION      = 9,  /**< Suction Head at Wetting Front */
    SM_KSAT         = 10, /**< Saturated Hydraulic Conductivity */
    SM_KSLOPE       = 11, /**< Slope of Log(k) v. Moisture Content Curve */
    SM_CLOGFACTOR   = 12, /**< Clogging Factor */
    SM_IMPERVFRAC   = 13, /**< Impervious Area Fraction */
    SM_COEFF        = 14, /**< Underdrain Flow Coefficient */
    SM_EXPON        = 15, /**< Underdrain Head Exponent */
    SM_OFFSET       = 16, /**< Offset Height of Underdrain */
    SM_DELAY        = 17, /**< Rain Barrel Drain Delay Time */
    SM_HOPEN        = 18, /**< Head When Drain Opens */
    SM_HCLOSE       = 19, /**< Head When Drain Closes */
    SM_QCURVE       = 20, /**< Curve Controlling FLow Rate (Optional) */
    SM_REGENDAYS    = 21, /**< Clogging Regeneration Interval (Days) */
    SM_REGENDEGREE  = 22, /**< Degree of Clogging Regeneration */
} SM_LidLayerProperty;

/// Lid unit property codes
typedef enum {
    SM_UNITAREA     = 0,  /**< Area of Single Replicate Unit */
    SM_FWIDTH       = 1,  /**< Full Top Width of Single Unit */
    SM_BWIDTH       = 2,  /**< Bottom Width of Single Unit */
    SM_INITSAT      = 3,  /**< Initial Saturation of Soil and Storage Layer */
    SM_FROMIMPERV   = 4,  /**< Fraction of Impervious Area Runoff Treated */
    SM_FROMPERV     = 5,  /**< Fraction of Pervious Area Runoff Treated */
} SM_LidUProperty;

/// Lid unit option codes
typedef enum {
    SM_INDEX        = 0,  /**< Lid Process Index */
    SM_NUMBER       = 1,  /**< Number of Replicate Units */
    SM_TOPERV       = 2,  /**< Outflow to Pervious Area */
    SM_DRAINSUB     = 3,  /**< Subcatchment Recieving Drain Flow */
    SM_DRAINNODE    = 4,  /**< Node Recieving Drain Flow */
} SM_LidUOptions;

/// Lid unit result codes
typedef enum {
    SM_INFLOW       = 0,  /**< Total Inflow */
    SM_EVAP         = 1,  /**< Total Evaporation */
    SM_INFIL        = 2,  /**< Total Infiltration */
    SM_SURFFLOW     = 3,  /**< Total Surface runoff */
    SM_DRAINFLOW    = 4,  /**< Total Underdrain flow */
    SM_INITVOL      = 5,  /**< Initial Stored Volume */
    SM_FINALVOL     = 6,  /**< Final Stored Volume */
    SM_SURFDEPTH    = 7,  /**< Depth of Ponded Water on Surface Layer */
    SM_PAVEDEPTH    = 8,  /**< Depth of Water in Porous Pavement Layer */
    SM_SOILMOIST    = 9,  /**< Moisture Content of Biocell Soil Layer */
    SM_STORDEPTH    = 10, /**< Depth of Water in Storage Layer */
    SM_DRYTIME      = 11, /**< Time Since Last Rainfall */
    SM_OLDDRAINFLOW = 12, /**< Previous Drain Flow (unit and group) */
    SM_NEWDRAINFLOW = 13, /**< Current Drain Flow (unit and group) */
    SM_PERVAREA     = 14, /**< Amount of Pervious Area (group) */
    SM_FLOWTOPERV   = 15, /**< Total Flow Sent to Pervious Area */
    SM_EVAPRATE     = 16, /**< Evaporate Rate */
    SM_NATIVEINFIL  = 17, /**< Native soil infil. rate limit */
    SM_SURFINFLOW   = 18, /**< Precip. + runon to LID unit */
    SM_SURFINFIL    = 19, /**< Infil. rate from surface layer */
    SM_SURFEVAP     = 20, /**< Evaporate rate from surface layer */
    SM_SURFOUTFLOW  = 21, /**< Outflow from surface layer */
    SM_PAVEEVAP     = 22, /**< Evaporation from pavement layer */
    SM_PAVEPERC     = 23, /**< Percolation from pavement layer */
    SM_SOILEVAP     = 24, /**< Evaporation from soil layer */
    SM_SOILPERC     = 25, /**< Percolation from soil layer */
    SM_STORAGEINFLOW= 26, /**< Inflow rate to storage layer */
    SM_STORAGEEXFIL = 27, /**< Exfilration rate from storage layer */
    SM_STORAGEEVAP  = 28, /**< Evaporation from storage layer */
    SM_STORAGEDRAIN = 29, /**< Underdrain flow rate layer */
} SM_LidResult;

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


#endif /* TOOLKITAPI_ENUMS_H_ */
