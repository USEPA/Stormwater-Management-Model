/*
* outputAPI.h
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

/*------------------- Error Messages --------------------*/
#define ERR410 "Error 410: SMO_init() has not been called"
#define ERR411 "Error 411: SMO_open() has not been called"
#define ERR414 "Error 414: memory allocation failure"

#define ERR421 "Input Error 421: invalid parameter code"
#define ERR422 "Input Error 422: reporting period index out of range"
#define ERR423 "Input Error 423: element index out of range"
#define ERR424 "Input Error 424: no memory allocated for results"

#define ERR434 "File Error 434: unable to open binary output file"
#define ERR435 "File Error 435: invalid file - not created by SWMM"
#define ERR436 "File Error 436: invalid file - contains no results"
#define ERR437 "File Error 437: invalid file - model run issued warnings"

#define ERR440 "ERROR 440: an unspecified error has occurred"


typedef struct SMOutputAPI SMOutputAPI; // opaque pointer

typedef enum {
	subcatchCount,
	nodeCount,
	linkCount,
	pollutantCount

} SMO_elementCount;

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
  #define DLLEXPORT extern "C" __declspec(dllexport) __stdcall
  #else
  #define DLLEXPORT __declspec(dllexport) __stdcall
  #endif
#else
  #ifdef __cplusplus
  #define DLLEXPORT extern "C"
  #else
  #define DLLEXPORT
  #endif
#endif

SMOutputAPI* DLLEXPORT SMO_init(void);
int DLLEXPORT SMO_open(SMOutputAPI* smoapi, const char* path);

int DLLEXPORT SMO_getProjectSize(SMOutputAPI* smoapi, SMO_elementCount code,
		int* count);

int DLLEXPORT SMO_getUnits(SMOutputAPI* smoapi, SMO_unit code, int* unitFlag);
int DLLEXPORT SMO_getStartTime(SMOutputAPI* smoapi, double* time);
int DLLEXPORT SMO_getTimes(SMOutputAPI* smoapi, SMO_time code, int* time);

int DLLEXPORT SMO_getElementName(SMOutputAPI* smoapi, SMO_elementType type,
		int elementIndex, char* elementName, int length);

float* DLLEXPORT SMO_newOutValueSeries(SMOutputAPI* smoapi, long seriesStart,
	long seriesLength, long* length, int* errcode);
float* DLLEXPORT SMO_newOutValueArray(SMOutputAPI* smoapi, SMO_apiFunction func,
		SMO_elementType type, long* length, int* errcode);

int DLLEXPORT SMO_getSubcatchSeries(SMOutputAPI* smoapi, int subcatchIndex,
	SMO_subcatchAttribute attr, long timeIndex, long length, float* outValueSeries);
int DLLEXPORT SMO_getNodeSeries(SMOutputAPI* smoapi, int nodeIndex, SMO_nodeAttribute attr,
	long timeIndex, long length, float* outValueSeries);
int DLLEXPORT SMO_getLinkSeries(SMOutputAPI* smoapi, int linkIndex, SMO_linkAttribute attr,
	long timeIndex, long length, float* outValueSeries);
int DLLEXPORT SMO_getSystemSeries(SMOutputAPI* smoapi, SMO_systemAttribute attr,
	long timeIndex, long length, float *outValueSeries);

int DLLEXPORT SMO_getSubcatchAttribute(SMOutputAPI* smoapi, long timeIndex,
	SMO_subcatchAttribute attr, float* outValueArray);
int DLLEXPORT SMO_getNodeAttribute(SMOutputAPI* smoapi, long timeIndex,
	SMO_nodeAttribute attr, float* outValueArray);
int DLLEXPORT SMO_getLinkAttribute(SMOutputAPI* smoapi, long timeIndex,
	SMO_linkAttribute attr, float* outValueArray);
int DLLEXPORT SMO_getSystemAttribute(SMOutputAPI* smoapi, long timeIndex,
	SMO_systemAttribute attr, float* outValueArray);

int DLLEXPORT SMO_getSubcatchResult(SMOutputAPI* smoapi, long timeIndex,
	int subcatchIndex, float* outValueArray, int* arrayLength);
int DLLEXPORT SMO_getNodeResult(SMOutputAPI* smoapi, long timeIndex,
	int nodeIndex, float* outValueArray, int* arrayLength);
int DLLEXPORT SMO_getLinkResult(SMOutputAPI* smoapi, long timeIndex,
	int linkIndex, float* outValueArray, int* arrayLength);
int DLLEXPORT SMO_getSystemResult(SMOutputAPI* smoapi, long timeIndex,
	int dummyIndex, float* outValueArray, int* arrayLength);

void DLLEXPORT SMO_free(float *array);

int DLLEXPORT SMO_close(SMOutputAPI* smoapi);
void DLLEXPORT SMO_errMessage(int errcode, char* errmsg, int n);

#endif /* OUTPUTAPI_H_ */
