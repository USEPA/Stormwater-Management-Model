/** @file toolkitAPI.h
 @see http://github.com/openwateranalytics/stormwater-management-model
 
 toolkitAPI.h
 @brief Exportable Functions for Toolkit API.
 @date 08/30/2016 (First Contribution)
 @authors B. McDonnell (EmNet LLC), OpenWaterAnalytics members: see <a href="https://github.com/OpenWaterAnalytics/Stormwater-Management-Model/blob/develop/AUTHORS">AUTHORS</a>.
 

*/
#ifndef TOOLKITAPI_H
#define TOOLKITAPI_H

#ifdef WINDOWS
#ifdef __MINGW32__
// <- More wrapper friendly
#define DLLEXPORT __declspec(dllexport) __cdecl
#else
#define DLLEXPORT __declspec(dllexport) __stdcall
#endif
#else
#define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define _CRT_SECURE_NO_DEPRECATE

#include "../src/datetime.h"

// --- Define the SWMM toolkit constants

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

typedef enum {
    SM_SYSTEMUNIT   = 0, /**< System Units */
    SM_FLOWUNIT     = 1, /**< Flow Units */
} SM_Units; 

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

/// Subcatchment result property codes
typedef enum {
    SM_SUBCRAIN      = 0,  /**< Rainfall Rate */
    SM_SUBCEVAP      = 1,  /**< Evaporation Loss */
    SM_SUBCINFIL     = 2,  /**< Infiltration Loss */
    SM_SUBCRUNON     = 3,  /**< Runon Rate */
    SM_SUBCRUNOFF    = 4,  /**< Runoff Rate */
    SM_SUBCSNOW      = 5,  /**< Snow Depth */
} SM_SubcResult;

/// Gage precip array property codes
typedef enum {
    SM_TOTALPRECIP  = 0,   //**< Total Precipitation Rate */
    SM_RAINFALL      = 1,  //**< Rainfall Rate */
    SM_SNOWFALL      = 2   //**< Snowfall Rate */)
} SM_GagePrecip;

// --- Define the SWMM toolkit structures

/// Node stats structure
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

/// Storage stats structure
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

/// Outfall stats structure
typedef struct
{
   double       avgFlow;
   double       maxFlow;
   double*      totalLoad;   
   int          totalPeriods;
}  SM_OutfallStats;

/// Link stats structure
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
   double        timeInFlowClass[7];//MAX_FLOW_CLASSES
   double        timeCourantCritical;
   long          flowTurns;
   int           flowTurnSign;
}  SM_LinkStats;

/// Pump stats structure
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

/// Subcatchment stats structure
typedef struct
{
    double       precip;
    double       runon;
    double       evap;
    double       infil;
    double       runoff;
    double       maxFlow;
    double*      surfaceBuildup;
}  SM_SubcatchStats;

/// System routing stats structure
typedef struct
{                                  // All routing totals are in ft3.
   double        dwInflow;         // dry weather inflow
   double        wwInflow;         // wet weather inflow
   double        gwInflow;         // groundwater inflow
   double        iiInflow;         // RDII inflow
   double        exInflow;         // direct inflow
   double        flooding;         // internal flooding
   double        outflow;          // external outflow
   double        evapLoss;         // evaporation loss
   double        seepLoss;         // seepage loss
   double        reacted;          // reaction losses
   double        initStorage;      // initial storage volume
   double        finalStorage;     // final storage volume
   double        pctError;         // continuity error
}  SM_RoutingTotals;

/// System runoff stats structure
typedef struct
{                                 // All volume totals are in ft3.
   double        rainfall;        // rainfall volume 
   double        evap;            // evaporation loss
   double        infil;           // infiltration loss
   double        runoff;          // runoff volume
   double        drains;          // LID drains
   double        runon;           // runon from outfalls
   double        initStorage;     // inital surface storage
   double        finalStorage;    // final surface storage
   double        initSnowCover;   // initial snow cover
   double        finalSnowCover;  // final snow cover
   double        snowRemoved;     // snow removal
   double        pctError;        // continuity error (%)
}  SM_RunoffTotals;


// --- Declare SWMM toolkit API Function

/**
 @brief Get the text of an error code.
 @param errcode The error code
 @param[out] s The error string represented by the code
*/
void DLLEXPORT swmm_getAPIError(int errcode, char *s);

/**
 @brief Gets Simulation Unit
 @param type Option code (see @ref SM_Units)
 @param[out] value Option value
 @return Error code
 */
int DLLEXPORT swmm_getSimulationUnit(int type, int *value);

/**
 @brief Gets Simulation Analysis Setting
 @param type Option code (see @ref SM_SimOption)
 @param[out] value Option value
 @return Error code
 */
int DLLEXPORT swmm_getSimulationAnalysisSetting(int type, int *value);

/**
 @brief Gets Simulation Analysis Setting
 @param type Option code (see @ref SM_SimSetting)
 @param[out] value Option value
 @return Error code
 */
int DLLEXPORT swmm_getSimulationParam(int type, double *value);

/**
 @brief Gets Object Count
 @param type Option code (see @ref SM_ObjectType)
 @param[out] count Option value
 @return Error code
 */
int DLLEXPORT swmm_countObjects(int type, int *count);

/**
 @brief Gets Object ID
 @param type Option code (see @ref SM_ObjectType)
 @param index of the Object
 @param[out] id The string ID of object.
 @return Error code
 */
int DLLEXPORT swmm_getObjectId(int type, int index, char *id);

/**
 @brief Gets Object ID Index
 @param type Option code (see @ref SM_ObjectType)
 @param id of the Object
 @param[out] Error code
 @return Object Injdex
 */
int DLLEXPORT swmm_getObjectIndex(int type, char *id, int *errcode);

/**
 @brief Get the type of node with specified index.
 @param index The index of a node
 @param[out] Ntype The type code for the node (@ref SM_NodeType). 
 id must be pre-allocated by the caller. 
 @return Error code
*/
int DLLEXPORT swmm_getNodeType(int index, int *Ntype);

/**
 @brief Get the type of link with specified index.
 @param index The index of a link
 @param[out] Ltype The type code for the link (@ref SM_LinkType).
 @return Error code
*/
int DLLEXPORT swmm_getLinkType(int index, int *Ltype);

/**
 @brief Get the link Connection Node Indeces. If the conduit has a 
 negative slope, the dynamic wave solver will automatically
 reverse the nodes. To check the direction, call @ref swmm_getLinkDirection().
 @param index The index of a link
 @param[out] Node1 The upstream node index.
 @param[out] Node2 The downstream node index.
 @return Error code
*/
int DLLEXPORT swmm_getLinkConnections(int index, int *Node1, int *Node2);

/**
 @brief Get the link flow direction (see @ref swmm_getLinkType() for notes.
 @param index The index of a link
 @param[out] value The link flow direction.
 @return Error code
*/
int DLLEXPORT swmm_getLinkDirection(int index, signed char *value);

/**
 @brief Get the Subcatchment connection. Subcatchments can load to a
 node, another subcatchment, or itself.
 @param index The index of a Subcatchment
 @param[out] type The type of object loading (See @ref SM_ObjectType)
 @param[out] Index The object index
 @return Error code
*/
int DLLEXPORT swmm_getSubcatchOutConnection(int index, int *type, int *Index);

/**
 @brief Get a property value for specified node.
 @param index The index of a node
 @param Param The property type code (See @ref SM_NodeProperty)
 @param[out] value The value of the node's property
 @return Error code
*/
int DLLEXPORT swmm_getNodeParam(int index, int Param, double *value);

/**
 @brief Set a property value for specified node.
 @param index The index of a node
 @param Param The property type code (See @ref SM_NodeProperty)
 @param value The new value of the node's property
 @return Error code
*/
int DLLEXPORT swmm_setNodeParam(int index, int Param, double value);

/**
 @brief Get a property value for specified link.
 @param index The index of a link
 @param Param The property type code (See @ref SM_LinkProperty)
 @param[out] value The value of the link's property
 @return Error code
*/
int DLLEXPORT swmm_getLinkParam(int index, int Param, double *value);

/**
 @brief Set a property value for specified link.
 @param index The index of a link
 @param Param The property type code (See @ref SM_LinkProperty)
 @param value The new value of the link's property
 @return Error code
*/
int DLLEXPORT swmm_setLinkParam(int index, int Param, double value);

/**
 @brief Get a property value for specified subcatchment.
 @param index The index of a subcatchment
 @param Param The property type code (See @ref SM_SubcProperty)
 @param[out] value The value of the subcatchment's property
 @return Error code
*/
int DLLEXPORT swmm_getSubcatchParam(int index, int Param, double *value);

/**
 @brief Set a property value for specified subcatchment.
 @param index The index of a subcatchment
 @param Param The property type code (See @ref SM_SubcProperty)
 @param value The new value of the subcatchment's property
 @return Error code
*/
int DLLEXPORT swmm_setSubcatchParam(int index, int Param, double value);

/**
 @brief Get the current simulation datetime information.
 @param Param timetype The property type code (See @ref SM_TimePropety)
 @param[out] year The year
 @param[out] month The month
 @param[out] day The day
 @param[out] hour The hour
 @param[out] minute The minute
 @param[out] seconds The seconds
 @return Error code
*/
int DLLEXPORT swmm_getSimulationDateTime(int timetype, int *year, int *month,
                                         int *day, int *hour, int *minute,
                                         int *seconds);

/**
 @brief Set simulation datetime information.
 @param Param timetype The property type code (See @ref SM_TimePropety)
 @param[out] dtimestr The current datetime. dtimestr must be pre-allocated by
 the caller.  This will copy 19 characters. 
 @return Error code
*/
int DLLEXPORT swmm_setSimulationDateTime(int timetype, char *dtimestr);

//-------------------------------
// Active Simulation Results API
//-------------------------------
/**
 @brief Get the simulation current datetime as a string.
 @param[out] dtimestr The current datetime. dtimestr must be pre-allocated by
 the caller.  This will copy 19 characters. 
 @return Error code
*/
int DLLEXPORT swmm_getCurrentDateTimeStr(char *dtimestr);

/**
 @brief Get a result value for specified node.
 @param index The index of a node
 @param result The property type code (See @ref SM_NodeResult)
 @param[out] value The value of the node's property
 @return Error code
*/
int DLLEXPORT swmm_getNodeResult(int index, int type, double *result);

/**
 @brief Get a result value for specified link.
 @param index The index of a link
 @param result The property type code (See @ref SM_LinkResult)
 @param[out] value The value of the link's property
 @return Error code
*/
int DLLEXPORT swmm_getLinkResult(int index, int type, double *result);

/**
 @brief Get a result value for specified subcatchment.
 @param index The index of a subcatchment
 @param result The property type code (See @ref SM_SubcResult)
 @param[out] value The value of the subcatchment's property
 @return Error code
*/
int DLLEXPORT swmm_getSubcatchResult(int index, int type, double *result);

/**
@brief Get precipitation rates for a gage.
@param index The index of gage
@param[out] Fage precipitation rates array [total, rainfall, snowfall]
@return Error code
*/
int DLLEXPORT swmm_getGagePrecip(int index, double **GageArray);

/**
 @brief Get a node statistics.
 @param index The index of a node
 @param[out] nodeStats The Node Stats struct (see @ref SM_NodeStats). 
 pre-allocated by the caller.
 @return Error code
*/
int DLLEXPORT swmm_getNodeStats(int index, SM_NodeStats *nodeStats);

/**
 @brief Get the cumulative inflow for a node.
 @param index The index of a node
 @param[out] value The total inflow.
 @return Error code
*/
int DLLEXPORT swmm_getNodeTotalInflow(int index, double *value);

/**
 @brief Get a storage statistics.
 @param index The index of a storage node
 @param[out] storageStats The storage Stats struct (see @ref SM_StorageStats). 
 pre-allocated by the caller.
 @return Error code
*/
int DLLEXPORT swmm_getStorageStats(int index, SM_StorageStats *storageStats);

/**
 @brief Get outfall statistics.
 @param index The index of a outfall node
 @param[out] outfallStats The outfall Stats struct (see @ref SM_OutfallStats). 
 pre-allocated by the caller. Caller is also responsible for freeing the 
 SM_OutfallStats structure using swmm_freeOutfallStats(). This frees any
 pollutants array.
 @return Error code
*/
int DLLEXPORT swmm_getOutfallStats(int index, SM_OutfallStats *outfallStats);

/**
 @brief Free outfall statistics structure.
 @param[out] outfallStats The outfall Stats struct. This frees any allocated
 pollutants array.
 @return Error code
*/
void DLLEXPORT swmm_freeOutfallStats(SM_OutfallStats *outfallStats);

/**
 @brief Get link statistics.
 @param index The index of a link
 @param[out] linkStats The link Stats struct (see @ref SM_LinkStats). 
 pre-allocated by the caller.
 @return Error code
*/
int DLLEXPORT swmm_getLinkStats(int index, SM_LinkStats *linkStats);

/**
 @brief Get pump statistics.
 @param index The index of a pump
 @param[out] pumpStats The link Stats struct (see @ref SM_PumpStats). 
 pre-allocated by the caller.
 @return Error code
*/
int DLLEXPORT swmm_getPumpStats(int index, SM_PumpStats *pumpStats);

/**
 @brief Get subcatchment statistics.
 @param index The index of a subcatchment
 @param[out] subcatchStats The link Stats struct (see @ref SM_SubcatchStats). 
 pre-allocated by the caller. Caller is also responsible for freeing the 
 SM_SubcatchStats structure using swmm_freeSubcatchStats(). This frees any
 pollutants array.
 @return Error code
*/
int DLLEXPORT swmm_getSubcatchStats(int index, SM_SubcatchStats *subcatchStats);

/**
 @brief Free subcatchment statistics structure.
 @param[out] subcatchStats The outfall Stats struct. This frees any allocated
 pollutants array.
 @return Error code
*/
void DLLEXPORT swmm_freeSubcatchStats(SM_SubcatchStats *subcatchStats);

/**
 @brief Get system routing statistics.
 @param[out] routingTot The system Routing Stats struct (see @ref SM_RoutingTotals). 
 pre-allocated by the caller.
 @return Error code
*/
int DLLEXPORT swmm_getSystemRoutingStats(SM_RoutingTotals *routingTot);

/**
 @brief Get system runoff statistics.
 @param[out] runoffTot The system Runoff Stats struct (see @ref SM_RunoffTotals). 
 pre-allocated by the caller.
 @return Error code
*/
int DLLEXPORT swmm_getSystemRunoffStats(SM_RunoffTotals *runoffTot);

//-------------------------------
// Setters API
//-------------------------------

/**
 @brief Set a link setting (pump, orifice, or weir). Setting for an orifice
 and a weir should be [0, 1]. A setting for a pump can range from [0, inf).
 However, if a pump is set to 1, it will pump at its maximum curve setting. 
 @param index The link index. 
 @param setting The new setting for the link. 
 @return Error code
*/
int DLLEXPORT swmm_setLinkSetting(int index, double setting);

/**
 @brief Set an inflow rate to a node. The inflow rate is held constant 
 until the caller changes it. 
 @param index The node index. 
 @param flowrate The new node inflow rate. 
 @return Error code
*/
int DLLEXPORT swmm_setNodeInflow(int index, double flowrate);

/**
 @brief Set outfall stage.
 @param index The outfall node index. 
 @param stage The outfall node stage (head). 
 @return Error code
*/
int DLLEXPORT swmm_setOutfallStage(int index, double stage);

/**
@brief Set a total precipitation intensity to the gage.
@param index The gage index.
@param total_precip The new total precipitation intensity.
@return Error code
*/
int DLLEXPORT swmm_setGagePrecip(int index, double total_precip);

/**
@brief Helper function to free memory array allocated in SWMM.
@param array The pointer to the array
*/
void DLLEXPORT freeArray(void** array);

#ifdef __cplusplus
}    // matches the linkage specification from above */
#endif


#endif
