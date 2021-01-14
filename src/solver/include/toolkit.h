/** @file toolkit.h
 @see http://github.com/openwateranalytics/stormwater-management-model

 toolkit.h
 @brief Exportable Functions for Toolkit API.
 @date 08/30/2016 (First Contribution)
 @authors B. McDonnell (EmNet LLC), OpenWaterAnalytics members: see <a href="https://github.com/OpenWaterAnalytics/Stormwater-Management-Model/blob/develop/AUTHORS">AUTHORS</a>.


*/
#ifndef TOOLKITAPI_H
#define TOOLKITAPI_H

#ifdef WINDOWS
#ifdef __MINGW32__
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


#include "toolkit_enums.h"
#include "toolkit_structs.h"


// /**
//  @brief Get full semantic version number
//  @param[out] semver sematic version (char array)
// */
// void DLLEXPORT   swmm_getSemVersion(char* semver);

/**
 @brief Get full semantic version number info
 @param[out] major sematic version major number
 @param[out] minor sematic version minor number
 @param[out] patch sematic version patch number
 @return error code
*/
int  DLLEXPORT   swmm_getVersionInfo(char **major, char **minor, char **patch);

/**
 @brief Opens SWMM input file, reads in network data, runs, and closes
 @param f1 pointer to name of input file (must exist)
 @param f2 pointer to name of report file (to be created)
 @param f3 pointer to name of binary output file (to be created)
 @param pointer to callback function (for printing progress)
 @return error code
*/
int  DLLEXPORT   swmm_run_cb(const char *f1, const char *f2, const char *f3,
    void (*callback) (double *));

/**
 @brief Get the text of an error code.
 @param errcode The error code
 @param[out] errorMsg The error string represented by the code
 @return Error code
*/
int DLLEXPORT swmm_getAPIError(int errorCode, char **errorMsg);

/**
 @brief Finds the index of an object given its ID.
 @param type An object type (see @ref SM_ObjectType)
 @param id The object ID
 @param[out] index The objects index
 @return Error code
*/
int DLLEXPORT swmm_project_findObject(SM_ObjectType type, char *id, int *index);

/**
@brief Gets Simulation Unit
@param type Option code (see @ref SM_Units)
@param[out] value Option value
@return Error code
*/
int DLLEXPORT swmm_getSimulationUnit(SM_Units type, int *value);

/**
 @brief Gets Simulation Analysis Setting
 @param type Option code (see @ref SM_SimOption)
 @param[out] value Option value
 @return Error code
 */
int DLLEXPORT swmm_getSimulationAnalysisSetting(SM_SimOption type, int *value);

/**
 @brief Gets Simulation Analysis Setting
 @param type Option code (see @ref SM_SimSetting)
 @param[out] value Option value
 @return Error code
 */
int DLLEXPORT swmm_getSimulationParam(SM_SimSetting type, double *value);

/**
 @brief Gets Object Count
 @param type Option code (see @ref SM_ObjectType)
 @param[out] count Option value
 @return Error code
 */
int DLLEXPORT swmm_countObjects(SM_ObjectType type, int *count);

/**
 @brief Gets Object ID
 @param type Option code (see @ref SM_ObjectType)
 @param index of the Object
 @param[out] id The string ID of object.
 @return Error code
 */
int DLLEXPORT swmm_getObjectId(SM_ObjectType type, int index, char **id);

/**
 @brief Gets Object Index
 @param type Option code (see @ref SM_ObjectType)
 @param[in] id of the Object
 @param[out] index of the Object
 @return errcode Error Code
 */
int DLLEXPORT swmm_getObjectIndex(SM_ObjectType type, char *id, int *index);

/**
 @brief Get the type of node with specified index.
 @param index The index of a node
 @param[out] Ntype The type code for the node (@ref SM_NodeType).
 id must be pre-allocated by the caller.
 @return Error code
*/
int DLLEXPORT swmm_getNodeType(int index, SM_NodeType *Ntype);

/**
 @brief Get the type of link with specified index.
 @param index The index of a link
 @param[out] Ltype The type code for the link (@ref SM_LinkType).
 @return Error code
*/
int DLLEXPORT swmm_getLinkType(int index, SM_LinkType *Ltype);

/**
 @brief Get the link Connection Node Indeces. If the conduit has a
 negative slope, the dynamic wave solver will automatically
 reverse the nodes. To check the direction, call @ref swmm_getLinkDirection().
 @param index The index of a link
 @param[out] Node1 The upstream node index.
 @param[out] Node2 The downstream node index.
 @return Error code
*/
int DLLEXPORT swmm_getLinkConnections(int index, int *node1, int *node2);

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
 @param[out] out_index The object index
 @return Error code
*/
int DLLEXPORT swmm_getSubcatchOutConnection(int index, SM_ObjectType *type, int *out_index);

/**
 @brief Get the number of lid units on a subcatchment.
 @param index The index of a subcatchment
 @param[out] value The number of lid units on a subcatchment
 @return Error code
*/
int DLLEXPORT swmm_getLidUCount(int index, int *value);

/**
 @brief Get a property value for a specified lid unit on a specified subcatchment
 @param index The index of a subcatchment
 @param lidIndex The index of specified lid unit
 @param Param The property type code (See @ref SM_LidUProperty)
 @param[out] value The value of the lid unit's property
 @return Error code
*/
int DLLEXPORT swmm_getLidUParam(int index, int lidIndex, SM_LidUProperty param, double *value);

/**
 @brief Set a property value for a specified lid unit on a specified subcatchment
 @param index The index of a subcatchment
 @param lidIndex The index of specified lid unit
 @param Param The property type code (See @ref SM_LidUProperty)
 @param value The new value of the lid unit's property
 @return Error code
*/
int DLLEXPORT swmm_setLidUParam(int index, int lidIndex, SM_LidUProperty param, double value);

/**
 @brief Get the lid option for a specified lid unit on a specified subcatchment
 @param index The index of a subcatchment
 @param lidIndex The index of specified lid unit
 @param Param The lid option type code (See @ref SM_LidUOptions)
 @param[out] value The value of the option for the lid unit
 @return Error code
*/
int DLLEXPORT swmm_getLidUOption(int index, int lidIndex, SM_LidUOptions param, int *value);

/**
 @brief Set the lid option for a specified lid unit on a specified subcatchment
 @param index The index of a subcatchment
 @param lidIndex The index of specified lid unit
 @param Param The lid option type code (See @ref SM_LidUOptions)
 @param value The new value of the option for the lid unit
 @return Error code
*/
int DLLEXPORT swmm_setLidUOption(int index, int lidIndex, SM_LidUOptions param, int value);

/**
 @brief Get the lid control surface immediate overflow condition
 @param lidControlIndex The index of specified lid control
 @param[out] condition The value of surface immediate overflow condition
 @return Error code
*/
int DLLEXPORT swmm_getLidCOverflow(int lidControlIndex, int *condition);

/**
 @brief Get a property value for specified lid control
 @param lidControlIndex The index of specified lid control
 @param layerIndex The index of specified lid layer (See @ref SM_LidLayer)
 @param Param The property type code (See @ref SM_LidLayerProperty)
 @param[out] value The value of lid control's property
 @return Error code
*/
int DLLEXPORT swmm_getLidCParam(int lidControlIndex, SM_LidLayer layerIndex, SM_LidLayerProperty param, double *value);

/**
 @brief Set a property value for specified lid control
 @param lidControlIndex The index of specified lid control
 @param layerIndex The index of specified lid layer (See @ref SM_LidLayer)
 @param Param The property type code (See @ref SM_LidLayerProperty)
 @param value The new value for the lid control's property
 @return Error code
*/
int DLLEXPORT swmm_setLidCParam(int lidControlIndex, SM_LidLayer layerIndex, SM_LidLayerProperty param, double value);

/**
 @brief Get the lid unit water balance simulated value at current time
 @param index The index of a subcatchment
 @param lidIndex The index of specified lid unit
 @param layerIndex The index of specified lid layer (See @ref SM_LidLayer)
 @param Param The result type code (See @ref SM_LidResult)
 @param[out] result The result for the specified lid unit
 @return Error code
*/
int DLLEXPORT swmm_getLidUFluxRates(int index, int lidIndex, SM_LidLayer layerIndex, double *result);

/**
 @brief Get the lid group of a specified subcatchment result at current time
 @param index The index of a subcatchment
 @param type The result type code (See @ref SM_LidResult)
 @param[out] result The result for the specified lid group
 @return Error code
*/
int DLLEXPORT swmm_getLidGResult(int index, SM_LidResult type, double *result);

/**
 @brief Get the lid unit of a specified subcatchment result at current time
 @param index The index of a subcatchment
 @param lidIndex The index of specified lid unit
 @param type The result type code (See @ref SM_LidResult)
 @param[out] result The result for the specified lid unit
 @return Error code
*/
int DLLEXPORT swmm_getLidUResult(int index, int lidIndex, SM_LidResult type, double *result);

/**
 @brief Get a property value for specified node.
 @param index The index of a node
 @param Param The property type code (See @ref SM_NodeProperty)
 @param[out] value The value of the node's property
 @return Error code
*/
int DLLEXPORT swmm_getNodeParam(int index, SM_NodeProperty param, double *value);

/**
 @brief Set a property value for specified node.
 @param index The index of a node
 @param Param The property type code (See @ref SM_NodeProperty)
 @param value The new value of the node's property
 @return Error code
*/
int DLLEXPORT swmm_setNodeParam(int index, SM_NodeProperty param, double value);

/**
 @brief Get a property value for specified link.
 @param index The index of a link
 @param Param The property type code (See @ref SM_LinkProperty)
 @param[out] value The value of the link's property
 @return Error code
*/
int DLLEXPORT swmm_getLinkParam(int index, SM_LinkProperty param, double *value);

/**
 @brief Set a property value for specified link.
 @param index The index of a link
 @param Param The property type code (See @ref SM_LinkProperty)
 @param value The new value of the link's property
 @return Error code
*/
int DLLEXPORT swmm_setLinkParam(int index, SM_LinkProperty param, double value);

/**
 @brief Get a property value for specified subcatchment.
 @param index The index of a subcatchment
 @param Param The property type code (See @ref SM_SubcProperty)
 @param[out] value The value of the subcatchment's property
 @return Error code
*/
int DLLEXPORT swmm_getSubcatchParam(int index, SM_SubcProperty param, double *value);

/**
 @brief Set a property value for specified subcatchment.
 @param index The index of a subcatchment
 @param Param The property type code (See @ref SM_SubcProperty)
 @param value The new value of the subcatchment's property
 @return Error code
*/
int DLLEXPORT swmm_setSubcatchParam(int index, SM_SubcProperty param, double value);

/**
 @brief Get the current simulation datetime information.
 @param type The property type code (See @ref SM_TimePropety)
 @param[out] year The year
 @param[out] month The month
 @param[out] day The day
 @param[out] hour The hour
 @param[out] minute The minute
 @param[out] second The seconds
 @return Error code
*/
int DLLEXPORT swmm_getSimulationDateTime(SM_TimePropety type, int *year, int *month,
                                         int *day, int *hour, int *minute,
                                         int *second);

/**
 @brief Set simulation datetime information.
 @param type The property type code (See @ref SM_TimePropety)
 @param year The year
 @param month The month
 @param day The day
 @param hour The hour
 @param minute The minute
 @param second The second
 @return Error code
*/
int DLLEXPORT swmm_setSimulationDateTime(SM_TimePropety type, int year, int month,
                                         int day, int hour, int minute,
                                         int second);

/**
 @brief Get the current simulation datetime information.
 @param[out] year The year
 @param[out] month The month
 @param[out] day The day
 @param[out] hour The hour
 @param[out] minute The minute
 @param[out] second The seconds
 @return Error code
*/
int DLLEXPORT swmm_getCurrentDateTime(int *year, int *month, int *day,
                                      int *hour, int *minute, int *second);

/**
 @brief Get a result value for specified node.
 @param index The index of a node
 @param type The property type code (See @ref SM_NodeResult)
 @param[out] result The result of the node's property
 @return Error code
*/
int DLLEXPORT swmm_getNodeResult(int index, SM_NodeResult type, double *result);

/**
 @brief Gets pollutant values for a specified node.
 @param index The index of a node
 @param type The property type code (see @ref SM_NodePollut)
 @param[out] PollutArray result array
 @return Error code
*/
int DLLEXPORT swmm_getNodePollut(int index, SM_NodePollut type, double **pollutArray, int *length);

/**
 @brief Get a result value for specified link.
 @param index The index of a link
 @param type The property type code (See @ref SM_LinkResult)
 @param[out] result The result of the link's property
 @return Error code
*/
int DLLEXPORT swmm_getLinkResult(int index, SM_LinkResult type, double *result);

/**
 @brief Gets pollutant values for a specified link.
 @param index The index of a link
 @param type The property type code (see @ref SM_LinkPollut)
 @param[out] PollutArray result array
 @return Error code
*/
int DLLEXPORT swmm_getLinkPollut(int index, SM_LinkPollut type, double **pollutArray, int *length);

/**
 @brief Get a result value for specified subcatchment.
 @param index The index of a subcatchment
 @param type The property type code (See @ref SM_SubcResult)
 @param[out] result The result of the subcatchment's property
 @return Error code
*/
int DLLEXPORT swmm_getSubcatchResult(int index, SM_SubcResult type, double *result);

/**
 @brief Gets pollutant values for a specified subcatchment.
 @param index The index of a subcatchment
 @param type The property type code (see @ref SM_SubcPollut)
 @param[out] PollutArray result array
 @return Error code
*/
int DLLEXPORT swmm_getSubcatchPollut(int index, SM_SubcPollut type, double **pollutArray, int *length);

/**
@brief Get precipitation rates for a gage.
@param index The index of gage
@param type The property type code (see @ref SM_GagePrecip)
@param[out] GageArray precipitation rate
@return Error code
*/
int DLLEXPORT swmm_getGagePrecip(int index, SM_GagePrecip type, double *result);

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

// /**
//  @brief Free outfall statistics structure.
//  @param[out] outfallStats The outfall Stats struct. This frees any allocated
//  pollutants array.
//  @return Error code
// */
// void DLLEXPORT swmm_freeOutfallStats(SM_OutfallStats *outfallStats);

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
 @brief Get system routing totals.
 @param[out] routingTot The system Routing Stats struct (see @ref SM_RoutingTotals).
 pre-allocated by the caller.
 @return Error code
*/
int DLLEXPORT swmm_getSystemRoutingTotals(SM_RoutingTotals *routingTotals);

/**
 @brief Get system runoff totals.
 @param[out] runoffTot The system Runoff Stats struct (see @ref SM_RunoffTotals).
 pre-allocated by the caller.
 @return Error code
*/
int DLLEXPORT swmm_getSystemRunoffTotals(SM_RunoffTotals *runoffTotals);

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
 @return Void.
*/
void DLLEXPORT swmm_freeMemory(void *memory);

#ifdef __cplusplus
}    // matches the linkage specification from above */
#endif


#endif
