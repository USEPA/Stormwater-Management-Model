/*
 * swmm_output.c - SWMM Output API
 *
 *      Author: Colleen Barr
 *           US EPA - ORD/NHEERL
 *
 *      Modified by: Michael E. Tryby,
 *                   Bryant McDonnell
 *
 */

#ifndef SWMM_OUTPUT_H_
#define SWMM_OUTPUT_H_

#define MAXFILENAME     259   // Max characters in file path
#define MAXELENAME       31   // Max characters in element name

// This is an opaque pointer to struct. Do not access variables.
typedef void* SMO_Handle;

typedef enum {
	SMO_flow_rate,
	SMO_concentration
} SMO_unit;

typedef enum {
	SMO_subcatch,
	SMO_node,
	SMO_link,
	SMO_sys
} SMO_elementType;

typedef enum {
	SMO_reportStep,
	SMO_numPeriods
} SMO_time;

typedef enum {
	SMO_rainfall_subcatch,  	// (in/hr or mm/hr),
	SMO_snow_depth_subcatch,	// (in or mm),
	SMO_evap_loss, 				// (in/hr or mm/hr),
	SMO_infil_loss,				// (in/hr or mm/hr),
	SMO_runoff_rate,     		// (flow units),
	SMO_gwoutflow_rate,  		// (flow units),
	SMO_gwtable_elev,    		// (ft or m),
	SMO_soil_moisture,			// unsaturated zone moisture content (-),
	SMO_pollutant_conc_subcatch	// first pollutant
} SMO_subcatchAttribute;

typedef enum {
	SMO_invert_depth,          	// (ft or m),
	SMO_hydraulic_head,        	// (ft or m),
	SMO_stored_ponded_volume,  	// (ft3 or m3),
	SMO_lateral_inflow,        	// (flow units),
	SMO_total_inflow,          	// lateral + upstream (flow units),
	SMO_flooding_losses,       	// (flow units),
	SMO_pollutant_conc_node     // first pollutant,
} SMO_nodeAttribute;

typedef enum {
	SMO_flow_rate_link,      	// (flow units),
	SMO_flow_depth,     		// (ft or m),
	SMO_flow_velocity,  		// (ft/s or m/s),
	SMO_flow_volume,			// (ft3 or m3),
	SMO_capacity,       		// (fraction of conduit filled),
	SMO_pollutant_conc_link  	// first pollutant,
} SMO_linkAttribute;

typedef enum {
	SMO_air_temp,              	// (deg. F or deg. C),
	SMO_rainfall_system,        // (in/hr or mm/hr),
	SMO_snow_depth_system,      // (in or mm),
	SMO_evap_infil_loss,	  	// (in/hr or mm/hr),
	SMO_runoff_flow,           	// (flow units),
	SMO_dry_weather_inflow,    	// (flow units),
	SMO_groundwater_inflow,    	// (flow units),
	SMO_RDII_inflow,           	// (flow units),
	SMO_direct_inflow,         	// user defined (flow units),
	SMO_total_lateral_inflow,  	// (sum of variables 4 to 8) //(flow units),
	SMO_flood_losses,       	// (flow units),
	SMO_outfall_flows,         	// (flow units),
	SMO_volume_stored,         	// (ft3 or m3),
	SMO_evap_rate             	// (in/day or mm/day),
	//p_evap_rate		    // (in/day or mm/day)
} SMO_systemAttribute;


//#ifdef WINDOWS
//  #ifdef __cplusplus
//    #define DLLEXPORT __declspec(dllexport) __cdecl
//  #else
//    #define DLLEXPORT __declspec(dllexport) __stdcall
//  #endif
//#else
//  #define DLLEXPORT
//#endif

#include "swmm_output_export.h"


#ifdef __cplusplus
  extern "C" {
#endif

int DLLEXPORT SMO_init(SMO_Handle* p_handle);
int DLLEXPORT SMO_close(SMO_Handle* p_handle);
int DLLEXPORT SMO_open(SMO_Handle p_handle, const char* path);
int DLLEXPORT SMO_getVersion(SMO_Handle p_handle, int* version);
int DLLEXPORT SMO_getProjectSize(SMO_Handle p_handle, int** elementCount, int* length);

int DLLEXPORT SMO_getFlowUnits(SMO_Handle p_handle, int* unitFlag);
int DLLEXPORT SMO_getPollutantUnits(SMO_Handle p_handle, int** unitFlag, int* length);
int DLLEXPORT SMO_getStartDate(SMO_Handle p_handle, double* date);
int DLLEXPORT SMO_getTimes(SMO_Handle p_handle, SMO_time code, int* time);
int DLLEXPORT SMO_getElementName(SMO_Handle p_handle, SMO_elementType type,
		int elementIndex, char** elementName, int* size);

int DLLEXPORT SMO_getSubcatchSeries(SMO_Handle p_handle, int subcatchIndex,
	SMO_subcatchAttribute attr, int startPeriod, int endPeriod, float** outValueSeries, int* dim);
int DLLEXPORT SMO_getNodeSeries(SMO_Handle p_handle, int nodeIndex, SMO_nodeAttribute attr,
	int startPeriod, int endPeriod, float** outValueSeries, int* dim);
int DLLEXPORT SMO_getLinkSeries(SMO_Handle p_handle, int linkIndex, SMO_linkAttribute attr,
	int startPeriod, int endPeriod, float** outValueSeries, int* dim);
int DLLEXPORT SMO_getSystemSeries(SMO_Handle p_handle, SMO_systemAttribute attr,
	int startPeriod, int endPeriod, float** outValueSeries, int* dim);

int DLLEXPORT SMO_getSubcatchAttribute(SMO_Handle p_handle, int timeIndex,
	SMO_subcatchAttribute attr, float** outValueArray, int* length);
int DLLEXPORT SMO_getNodeAttribute(SMO_Handle p_handle, int timeIndex,
	SMO_nodeAttribute attr, float** outValueArray, int* length);
int DLLEXPORT SMO_getLinkAttribute(SMO_Handle p_handle, int timeIndex,
	SMO_linkAttribute attr, float** outValueArray, int* length);
int DLLEXPORT SMO_getSystemAttribute(SMO_Handle p_handle, int timeIndex,
	SMO_systemAttribute attr, float** outValueArray, int* length);

int DLLEXPORT SMO_getSubcatchResult(SMO_Handle p_handle, int timeIndex,
	int subcatchIndex, float** outValueArray, int* arrayLength);
int DLLEXPORT SMO_getNodeResult(SMO_Handle p_handle, int timeIndex,
	int nodeIndex, float** outValueArray, int* arrayLength);
int DLLEXPORT SMO_getLinkResult(SMO_Handle p_handle, int timeIndex,
	int linkIndex, float** outValueArray, int* arrayLength);
int DLLEXPORT SMO_getSystemResult(SMO_Handle p_handle, int timeIndex,
	int dummyIndex, float** outValueArray, int* arrayLength);

void DLLEXPORT SMO_free(void** array);
void DLLEXPORT SMO_clearError(SMO_Handle p_handle_in);
int DLLEXPORT SMO_checkError(SMO_Handle p_handle_in, char** msg_buffer);

#ifdef __cplusplus
  }
#endif

#endif /* SWMM_OUTPUT_H_ */
