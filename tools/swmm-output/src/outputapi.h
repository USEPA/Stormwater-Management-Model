/*
* outputAPI.h - SWMM
*
*      Author: Colleen Barr
*      Modified by: Michael Tryby,
*                   Bryant McDonnell
*
*
*/

#ifndef OUTPUTAPI_H_
#define OUTPUTAPI_H_

#define MAXFILENAME     259   // Max characters in file path
#define MAXELENAME       31   // Max characters in element name

// This is an opaque pointer to struct. Do not access variables.
typedef void* SMO_Handle;

typedef enum {
	flow_rate,
	concentration
} SMO_unit;

typedef enum {
	//getSeries,
	getAttribute,
	getResult
} SMO_apiFunction;

typedef enum {
	subcatch,
	node,
	link,
	sys
} SMO_elementType;

typedef enum {
//	reportStart,
	reportStep,
	numPeriods
} SMO_time;

typedef enum {
	rainfall_subcatch,  	// (in/hr or mm/hr),
	snow_depth_subcatch,	// (in or mm),
	evap_loss, 				// (in/hr or mm/hr),
	infil_loss,				// (in/hr or mm/hr),
	runoff_rate,     		// (flow units),
	gwoutflow_rate,  		// (flow units),
	gwtable_elev,    		// (ft or m),
	soil_moisture,			// unsaturated zone moisture content (-),
	pollutant_conc_subcatch	// first pollutant
} SMO_subcatchAttribute;

typedef enum {
	invert_depth,          	// (ft or m),
	hydraulic_head,        	// (ft or m),
	stored_ponded_volume,  	// (ft3 or m3),
	lateral_inflow,        	// (flow units),
	total_inflow,          	// lateral + upstream (flow units),
	flooding_losses,       	// (flow units),
	pollutant_conc_node     // first pollutant,
} SMO_nodeAttribute;

typedef enum {
	flow_rate_link,      	// (flow units),
	flow_depth,     		// (ft or m),
	flow_velocity,  		// (ft/s or m/s),
	flow_volume,			// (ft3 or m3),
	capacity,       		// (fraction of conduit filled),
	pollutant_conc_link  	// first pollutant,
} SMO_linkAttribute;

typedef enum {
	air_temp,              	// (deg. F or deg. C),
	rainfall_system,        // (in/hr or mm/hr),
	snow_depth_system,      // (in or mm),
	evap_infil_loss,	  	// (in/hr or mm/hr),
	runoff_flow,           	// (flow units),
	dry_weather_inflow,    	// (flow units),
	groundwater_inflow,    	// (flow units),
	RDII_inflow,           	// (flow units),
	direct_inflow,         	// user defined (flow units),
	total_lateral_inflow,  	// (sum of variables 4 to 8) //(flow units),
	flood_losses,       	// (flow units),
	outfall_flows,         	// (flow units),
	volume_stored,         	// (ft3 or m3),
	evap_rate             	// (in/day or mm/day),
	//p_evap_rate		    // (in/day or mm/day)
} SMO_systemAttribute;


#ifdef WINDOWS
  #ifdef __cplusplus
  #define DLLEXPORT __declspec(dllexport) __cdecl
  #else
  #define DLLEXPORT __declspec(dllexport) __stdcall
  #endif
#else
  #ifdef __cplusplus
  #define DLLEXPORT
  #else
  #define DLLEXPORT
  #endif
#endif

#ifdef __cplusplus
  extern "C" {
#endif

int DLLEXPORT SMO_init(SMO_Handle* p_handle);
int DLLEXPORT SMO_close(SMO_Handle* p_handle);
int DLLEXPORT SMO_open(SMO_Handle p_handle, const char* path);
int DLLEXPORT SMO_getVersion(SMO_Handle p_handle, int* version);
int DLLEXPORT SMO_getProjectSize(SMO_Handle p_handle, int** element_count, int* length);

int DLLEXPORT SMO_getFlowUnits(SMO_Handle p_handle, int* unitFlag);
int DLLEXPORT SMO_getPollutantUnits(SMO_Handle p_handle, int** unitFlag, int* length);

int DLLEXPORT SMO_getStartDate(SMO_Handle p_handle, double* date);
int DLLEXPORT SMO_getTimes(SMO_Handle p_handle, SMO_time code, int* time);
int DLLEXPORT SMO_getElementName(SMO_Handle p_handle, SMO_elementType type,
		int elementIndex, char** elementName, int* length);

//float* DLLEXPORT SMO_newOutValueSeries(SMO_Handle* p_handle, long seriesStart,
//	long seriesLength, long* length, int* errcode);
//float* DLLEXPORT SMO_newOutValueArray(SMO_Handle* p_handle, SMO_apiFunction func,
//		SMO_elementType type, long* length, int* errcode);

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
	SMO_systemAttribute attr, float** outValue, int* length);

int DLLEXPORT SMO_getSubcatchResult(SMO_Handle p_handle, long timeIndex,
	int subcatchIndex, float** outValueArray, int* arrayLength);
int DLLEXPORT SMO_getNodeResult(SMO_Handle p_handle, long timeIndex,
	int nodeIndex, float** outValueArray, int* arrayLength);
int DLLEXPORT SMO_getLinkResult(SMO_Handle p_handle, long timeIndex,
	int linkIndex, float** outValueArray, int* arrayLength);
int DLLEXPORT SMO_getSystemResult(SMO_Handle p_handle, long timeIndex,
	int dummyIndex, float** outValueArray, int* arrayLength);

void DLLEXPORT SMO_free(void** array);

void DLLEXPORT SMO_clearError(SMO_Handle p_handle_in);

int DLLEXPORT SMO_checkError(SMO_Handle p_handle_in, char** msg_buffer);

#ifdef __cplusplus
  }
#endif

#endif /* OUTPUTAPI_H_ */
