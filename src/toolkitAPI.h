//-----------------------------------------------------------------------------
//   toolkitAPI.h
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    08/30/2016
//   Author:  B. McDonnell (EmNet LLC)
//
//   Exportable Functions for Project Definition API.
//
//-----------------------------------------------------------------------------

#include <math.h>

//#ifndef DLLEXPORT
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
//#endif

#ifdef __cplusplus
extern "C" {
#endif

#define _CRT_SECURE_NO_DEPRECATE

void DLLEXPORT swmm_getAPIError(int errcode, char *s);

// Input API Exportable Functions
/**
 @brief runs a complete EPANET simulation
 @param inpFile pointer to name of input file (must exist)
 @param rptFile pointer to name of report file (to be created)
 @param binOutFile pointer to name of binary output file (to be created)
 @param callback a callback function that takes a character string (char *) as
 its only parameter.
 @return error code

 The callback function should reside in and be used by the calling
 code to display the progress messages that EPANET generates
 as it carries out its computations. If this feature is not
 needed then the argument should be NULL.
 */
int DLLEXPORT swmm_getSimulationUnit(int type, int *value);
int DLLEXPORT swmm_getSimulationAnalysisSetting(int type, int *value);
int DLLEXPORT swmm_getSimulationParam(int type, double *value);

int DLLEXPORT swmm_countObjects(int type, int *count);
int DLLEXPORT swmm_getObjectId(int type, int index, char *id);

int DLLEXPORT swmm_getNodeType(int index, int *Ntype);
int DLLEXPORT swmm_getLinkType(int index, int *Ltype);

int DLLEXPORT swmm_getLinkConnections(int index, int *Node1, int *Node2);
int DLLEXPORT swmm_getSubcatchOutConnection(int index, int *type, int *Index);

// Nodes
int DLLEXPORT swmm_getNodeParam(int index, int Param, double *value);
int DLLEXPORT swmm_setNodeParam(int index, int Param, double value);
// Links
int DLLEXPORT swmm_getLinkParam(int index, int Param, double *value);
int DLLEXPORT swmm_setLinkParam(int index, int Param, double value);
int DLLEXPORT swmm_getLinkDirection(int index, signed char *value);
// Subcatchments
int DLLEXPORT swmm_getSubcatchParam(int index, int Param, double *value);
int DLLEXPORT swmm_setSubcatchParam(int index, int Param, double value);
//
int DLLEXPORT swmm_getSimulationDateTime(int timetype, int *year, int *month,
                                         int *day, int *hour, int *minute,
                                         int *seconds);
int DLLEXPORT swmm_setSimulationDateTime(int timetype, char *dtimestr);

//-------------------------------
// Active Simulation Results API
//-------------------------------
int DLLEXPORT swmm_getCurrentDateTimeStr(char *dtimestr);

int DLLEXPORT swmm_getNodeStats(int index, TNodeStats *nodeStats);
int DLLEXPORT swmm_getStorageStats(int index, TStorageStats *storageStats);
int DLLEXPORT swmm_getOutfallStats(int index, TOutfallStats *outfallStats);
void DLLEXPORT swmm_freeOutfallStats(TOutfallStats *outfallStats);

int DLLEXPORT swmm_getLinkStats(int index, TLinkStats *linkStats);
int DLLEXPORT swmm_getPumpStats(int index, TPumpStats *pumpStats);

int DLLEXPORT swmm_getSubcatchStats(int index, TSubcatchStats *subcatchStats);

int DLLEXPORT swmm_getSystemRoutingStats(TRoutingTotals *routingTot);
int DLLEXPORT swmm_getSystemRunoffStats(TRunoffTotals *runoffTot);

//-------------------------------
// Setters API
//-------------------------------
int DLLEXPORT swmm_setLinkSetting(int index, double setting);
int DLLEXPORT swmm_setNodeInflow(int index, double flowrate);

#ifdef __cplusplus
}    // matches the linkage specification from above */
#endif
