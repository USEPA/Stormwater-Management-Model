/*
* outputAPI.c - SWMM
*
*      Author: Colleen Barr
*      Modified by: Michael E. Tryby,
*                   Bryant McDonnell
*
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "outputapi.h"
#include "errormanager.h"
#include "messages.h"
//#include "datetime.h"


// NOTE: These depend on machine data model and may change when porting
#define F_OFF off_t      // Must be a 8 byte / 64 bit integer for large file support
#define INT4  int        // Must be a 4 byte / 32 bit integer type
#define REAL4 float      // Must be a 4 byte / 32 bit real type

#define RECORDSIZE  4    // Memory alignment 4 byte word size for both int and real
#define DATESIZE    8    // Dates are stored as 8 byte word size

#define NELEMENTTYPES  4 // Number of element types

#define MEMCHECK(x)  (((x) == NULL) ? 414 : 0 )

struct IDentry {
	char* IDname;
	int length;
};
typedef struct IDentry idEntry;

//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------

typedef struct {
	char name[MAXFILENAME + 1];           // file path/name
	FILE* file;                        // FILE structure pointer

	struct IDentry *elementNames;      // array of pointers to element names

	long Nperiods;                     // number of reporting periods
	int FlowUnits;                     // flow units code

	int Nsubcatch;                     // number of subcatchments
	int Nnodes;                        // number of drainage system nodes
	int Nlinks;                        // number of drainage system links
	int Npolluts;                      // number of pollutants tracked

	int SubcatchVars;                  // number of subcatch reporting variables
	int NodeVars;                      // number of node reporting variables
	int LinkVars;                      // number of link reporting variables
	int SysVars;                       // number of system reporting variables

	double StartDate;                  // start date of simulation
	int    ReportStep;                 // reporting time step (seconds)

	F_OFF IDPos;					   // file position where object ID names start
	F_OFF ObjPropPos;				   // file position where object properties start
	F_OFF ResultsPos;                  // file position where results start
	F_OFF BytesPerPeriod;              // bytes used for results in each period

	error_handle_t* error_handle;
} data_t;

//typedef data_t* p_data_t;

//-----------------------------------------------------------------------------
//   Local functions
//-----------------------------------------------------------------------------
void errorLookup(int errcode, char* errmsg, int length);
int    validateFile(data_t* p_data);
void   initElementNames(data_t* p_data);

double getTimeValue(data_t* p_data, long timeIndex);
float  getSubcatchValue(data_t* p_data, long timeIndex, int subcatchIndex, SMO_subcatchAttribute attr);
float  getNodeValue(data_t* p_data, long timeIndex, int nodeIndex, SMO_nodeAttribute attr);
float  getLinkValue(data_t* p_data, long timeIndex, int linkIndex, SMO_linkAttribute attr);
float  getSystemValue(data_t* p_data, long timeIndex, SMO_systemAttribute attr);

float* newArray(int n);


int DLLEXPORT SMO_init(SMO_Handle* p_handle)
//  Purpose: Initialized pointer for the opaque SMO_Handle.
//
//  Returns: Error code 0 on success, -1 on failure
//
//  Note: The existence of this function has been carefully considered.
//   Don't change it.
//
{
	int errorcode = 0;
	data_t* priv_data;

	// Allocate memory for private data
	priv_data = (data_t*)calloc(1, sizeof(data_t));

	if (priv_data != NULL){
		priv_data->error_handle = new_errormanager(&errorLookup);
		*p_handle = priv_data;
	}
	else
		errorcode = -1;

    // TODO: Need to handle errors during initialization better.
	return errorcode;
}

int DLLEXPORT SMO_close(SMO_Handle* p_handle)
//
//   Purpose: Clean up after and close Output API
//
{
	data_t* p_data;
	int i, n, errorcode = 0;

	p_data = (data_t*)*p_handle;

	if (p_data == NULL || p_data->file == NULL)
		errorcode = -1;

	else
	{
		if (p_data->elementNames != NULL)
		{
			n = p_data->Nsubcatch + p_data->Nnodes + p_data->Nlinks + p_data->Npolluts;

			for(i = 0; i < n; i++)
				free(p_data->elementNames[i].IDname);
		}

		dst_errormanager(p_data->error_handle);

		fclose(p_data->file);
		free(p_data);

		*p_handle = NULL;
	}

	return errorcode;
}

int DLLEXPORT SMO_open(SMO_Handle p_handle, const char* path)
//
//  Purpose: Open the output binary file and read the header.
//
{
	int err, errorcode = 0;
	F_OFF offset;

	data_t* p_data;

	p_data = (data_t*)p_handle;

	if (p_data == NULL) return -1;
    else
    {
    	strncpy(p_data->name, path, MAXFILENAME);

    	// --- open the output file
    	if ((p_data->file = fopen(path, "rb")) == NULL) errorcode = 434;
    	// --- validate the output file
    	else if ((err = validateFile(p_data)) != 0) errorcode = err;

    	else {
    		// --- otherwise read additional parameters from start of file
    		fseek(p_data->file, 3*RECORDSIZE, SEEK_SET);
    		fread(&(p_data->Nsubcatch), RECORDSIZE, 1, p_data->file);
    		fread(&(p_data->Nnodes), RECORDSIZE, 1, p_data->file);
    		fread(&(p_data->Nlinks), RECORDSIZE, 1, p_data->file);
    		fread(&(p_data->Npolluts), RECORDSIZE, 1, p_data->file);

    		// Compute offset for saved subcatch/node/link input values
    		offset = (p_data->Nsubcatch + 2) * RECORDSIZE  // Subcatchment area
    			+ (3 * p_data->Nnodes + 4) * RECORDSIZE  // Node type, invert & max depth
				+ (5 * p_data->Nlinks + 6) * RECORDSIZE; // Link type, z1, z2, max depth & length
    		offset += p_data->ObjPropPos;

    		// Read number & codes of computed variables
    		fseeko64(p_data->file, offset, SEEK_SET);
    		fread(&(p_data->SubcatchVars), RECORDSIZE, 1, p_data->file); // # Subcatch variables

    		fseeko64(p_data->file, p_data->SubcatchVars*RECORDSIZE, SEEK_CUR);
    		fread(&(p_data->NodeVars), RECORDSIZE, 1, p_data->file);     // # Node variables

    		fseeko64(p_data->file, p_data->NodeVars*RECORDSIZE, SEEK_CUR);
    		fread(&(p_data->LinkVars), RECORDSIZE, 1, p_data->file);     // # Link variables

    		fseeko64(p_data->file, p_data->LinkVars*RECORDSIZE, SEEK_CUR);
    		fread(&(p_data->SysVars), RECORDSIZE, 1, p_data->file);     // # System variables

    		// --- read data just before start of output results
    		offset = p_data->ResultsPos - 3 * RECORDSIZE;
    		fseeko64(p_data->file, offset, SEEK_SET);
    		fread(&(p_data->StartDate), DATESIZE, 1, p_data->file);
    		fread(&(p_data->ReportStep), RECORDSIZE, 1, p_data->file);

    		// --- compute number of bytes of results values used per time period
    		p_data->BytesPerPeriod = DATESIZE +
    				(p_data->Nsubcatch*p_data->SubcatchVars +
    						p_data->Nnodes*p_data->NodeVars +
							p_data->Nlinks*p_data->LinkVars +
							p_data->SysVars)*RECORDSIZE;
    	}
    }

	if (errorcode)
		SMO_close(&p_handle);

	return errorcode;
}

int DLLEXPORT SMO_getVersion(SMO_Handle p_handle, int* version)
//
//  Input:   p_handle = pointer to SMO_Handle struct
//  Output:  version SWMM version
//  Returns: error code
//
//  Purpose: Returns SWMM version that wrote binary file
//
{
	int errorcode = 0;
	data_t* p_data;

	p_data = (data_t*)p_handle;

	if (p_data == NULL) return -1;
	else
	{
		fseek(p_data->file, 1*RECORDSIZE, SEEK_SET);
		if (fread(version, RECORDSIZE, 1, p_data->file) != 1)
			errorcode = 436;
	}

	return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT SMO_getProjectSize(SMO_Handle p_handle, int** elementCount, int* length)
//
//   Purpose: Returns project size.
//
{
	int errorcode = 0;
	int* temp = new int[NELEMENTTYPES];
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
	else
	{
		temp[0] = p_data->Nsubcatch;
		temp[1] = p_data->Nnodes;
		temp[2] = p_data->Nlinks;
		temp[3] = p_data->Npolluts;

		*elementCount = temp;
		*length = NELEMENTTYPES;
	}

	return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT SMO_getFlowUnits(SMO_Handle p_handle, int* unitFlag)
//
//   Purpose: Returns unit flag for flow.
//
//   Returns:
//            0: CFS  (cubic feet per second)
//            1: GPM  (gallons per minute)
//            2: MGD  (million gallons per day)
//            3: CMS  (cubic meters per second)
//            4: LPS  (liters per second)
//            5: MLD  (million liters per day)
//
{
	int errorcode = 0;
	data_t* p_data;

	*unitFlag = -1;

	p_data = (data_t*)p_handle;

	if (p_data == NULL) return -1;
	else
	{
		fseek(p_data->file, 2*RECORDSIZE, SEEK_SET);
		fread(unitFlag, RECORDSIZE, 1, p_data->file);
	}

	return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT SMO_getPollutantUnits(SMO_Handle p_handle, int** unitFlag, int* length)
//
//   Purpose:
//     Return integer flag representing the units that the given pollutant is
//     measured in. Concentration units are located after the pollutant ID
//     names and before the object properties start, and are stored for each
//     pollutant.  They're stored as 4-byte integers with the following codes:
//       0: mg/L
//       1: ug/L
//       2: count/L
//
//   Args:
//     pollutantIndex: valid values are 0 to Npolluts-1
{
	int errorcode = 0;
	int* temp;
	F_OFF offset;
	data_t* p_data;

	p_data = (data_t*)p_handle;

	temp = new int[p_data->Npolluts];

	if (p_data == NULL) errorcode = -1;
	else if (MEMCHECK(temp)) errorcode = 414;
	else
    {
        offset = p_data->ObjPropPos - (p_data->Npolluts * RECORDSIZE);
        fseek(p_data->file, offset, SEEK_SET);
        fread(temp, RECORDSIZE, p_data->Npolluts, p_data->file);

        *unitFlag = temp;
        *length = p_data->Npolluts;
    }

	return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT SMO_getStartDate(SMO_Handle p_handle, double* date)
//
//	Purpose: Returns start date.
//
{
	int errorcode = 0;
	data_t* p_data;

	*date = -1.0;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
	else
		*date = p_data->StartDate;

	return set_error(p_data->error_handle, errorcode);
}


int DLLEXPORT SMO_getTimes(SMO_Handle p_handle, SMO_time code, int* time)
//
//   Purpose: Returns step size and number of periods.
//
{
	int errorcode = 0;
	data_t* p_data;

	*time = -1.0;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
	else
	{
		switch (code)
		{
		case reportStep:  *time = p_data->ReportStep;
			break;
		case numPeriods:  *time = p_data->Nperiods;
			break;
		default:           errorcode = 421;
		}
	}

	return errorcode;
}

int DLLEXPORT SMO_getElementName(SMO_Handle p_handle, SMO_elementType type,
		int index, char** name, int* length)
//
//  Purpose: Given an element index returns the element name.
//
//  Note: The caller is responsible for allocating memory for the char array
//    name. The caller passes the length of the array allocated. The name may
//    be truncated if an array of adequate length is not passed.
//
{
	int idx, errorcode = 0;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = 410;
    else if (p_data->file == NULL) errorcode = 411;
	else
	{
		// Initialize the name array if necessary
		if (p_data->elementNames == NULL) initElementNames(p_data);

		switch (type)
		{
		case subcatch:
			if (index < 0 || index >= p_data->Nsubcatch)
				errorcode = 423;
			else
				idx = index;
			break;

		case node:
			if (index < 0 || index >= p_data->Nnodes)
				errorcode = 423;
			else
				idx = p_data->Nsubcatch + index;
			break;

		case link:
			if (index < 0 || index >= p_data->Nlinks)
				errorcode = 423;
			else
				idx = p_data->Nsubcatch + p_data->Nnodes + index;
			break;

		case sys:
			if (index < 0 || index >= p_data->Npolluts)
				errorcode = 423;
			else
				idx = p_data->Nsubcatch + p_data->Nnodes + p_data->Nlinks + index;
			break;

		default:
			errorcode = 421;
		}

		if (!errorcode) {
			*length = p_data->elementNames[idx].length;
			*name = new char[*length + 1];

			strncpy(*name, p_data->elementNames[idx].IDname, (size_t)(*length));
		}
	}

	return errorcode;
}


//float* DLLEXPORT SMO_newOutValueSeries(SMOutputAPI* smoapi, long startPeriod,
//	long endPeriod, long* length, int* errcode)
////
////  Purpose: Allocates memory for outValue Series.
////
////  Warning: Caller must free memory allocated by this function using SMO_free().
////
//{
//	long size;
//	float* array;
//
//    if (smoapi == NULL) *errcode = 410;
//    else if (smoapi->file == NULL) *errcode = 411;
//	else if (startPeriod < 0 || endPeriod >= smoapi->Nperiods ||
//			endPeriod <= startPeriod) *errcode = 422;
//	else
//	{
//
//		size = endPeriod - startPeriod;
//		if (size > smoapi->Nperiods)
//			size = smoapi->Nperiods;
//
//		array = (float*)calloc(size, sizeof(float));
//		*errcode = (MEMCHECK(array));
//
//		*length = size;
//		return array;
//	}
//
//	return NULL;
//}
//
//
//float* DLLEXPORT SMO_newOutValueArray(SMOutputAPI* smoapi, SMO_apiFunction func,
//	SMO_elementType type, long* length, int* errcode)
////
//// Purpose: Allocates memory for outValue Array.
////
////  Warning: Caller must free memory allocated by this function using SMO_free().
////
//{
//	long size;
//	float* array;
//
//    if (smoapi == NULL) *errcode = 410;
//    else if (smoapi->file == NULL) *errcode = 411;
//	else
//	{
//		switch (func)
//		{
//		case getAttribute:
//			if (type == subcatch)
//				size = smoapi->Nsubcatch;
//			else if (type == node)
//				size = smoapi->Nnodes;
//			else if (type == link)
//				size = smoapi->Nlinks;
//			else // system
//				size = 1;
//		break;
//
//		case getResult:
//			if (type == subcatch)
//				size = smoapi->SubcatchVars;
//			else if (type == node)
//				size = smoapi->NodeVars;
//			else if (type == link)
//				size = smoapi->LinkVars;
//			else // system
//				size = smoapi->SysVars;
//		break;
//
//		default: *errcode = 421;
//			return NULL;
//		}
//
//		// Allocate memory for outValues
//		array = (float*)calloc(size, sizeof(float));
//		*errcode = (MEMCHECK(array));
//
//		*length = size;
//		return array;
//	}
//
//	return NULL;
//}


int DLLEXPORT SMO_getSubcatchSeries(SMO_Handle p_handle, int subcatchIndex,
	SMO_subcatchAttribute attr, int startPeriod, int endPeriod,
	float** outValueSeries, int* dim)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using timeIndex and length respectively.
//
{
	int k, length, errorcode = 0;
	float* temp;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
	else if (subcatchIndex < 0 || subcatchIndex > p_data->Nsubcatch) errorcode = 420;
    else if (startPeriod < 0 || startPeriod >= p_data->Nperiods ||
    		endPeriod <= startPeriod) errorcode = 422;
	// Check memory for outValues
	else if MEMCHECK(temp = newArray(length = endPeriod - startPeriod)) errorcode = 411;
	else
	{
		// loop over and build time series
		for (k = 0; k < length; k++)
			temp[k] = getSubcatchValue(p_data, startPeriod + k,
			subcatchIndex, attr);

		*outValueSeries = temp;
		*dim = length;
	}

	return set_error(p_data->error_handle, errorcode);
}


int DLLEXPORT SMO_getNodeSeries(SMO_Handle p_handle, int nodeIndex, SMO_nodeAttribute attr,
		int startPeriod, int endPeriod, float** outValueSeries, int* dim)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using timeIndex and length respectively.
//
{
	int k, length, errorcode = 0;
	float* temp;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
	else if (nodeIndex < 0 || nodeIndex > p_data->Nnodes) errorcode = 420;
    else if (startPeriod < 0 || startPeriod >= p_data->Nperiods ||
    		endPeriod <= startPeriod) errorcode = 422;
	// Check memory for outValues
	else if MEMCHECK(temp = newArray(length = endPeriod - startPeriod)) errorcode = 411;
	else
	{
		// loop over and build time series
		for (k = 0; k < length; k++)
			temp[k] = getNodeValue(p_data, startPeriod + k,
			nodeIndex, attr);

	    *outValueSeries = temp;
	    *dim = length;
    }

    return set_error(p_data->error_handle, errorcode);
}


int DLLEXPORT SMO_getLinkSeries(SMO_Handle p_handle, int linkIndex, SMO_linkAttribute attr,
		int startPeriod, int endPeriod, float** outValueSeries, int* dim)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using timeIndex and length respectively.
//
{
	int k, length, errorcode = 0;
	float* temp;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
	else if (linkIndex < 0 || linkIndex > p_data->Nlinks) errorcode = 420;
    else if (startPeriod < 0 || startPeriod >= p_data->Nperiods ||
       		endPeriod <= startPeriod) errorcode = 422;
	// Check memory for outValues
	else if MEMCHECK(temp = newArray(length = endPeriod - startPeriod)) errorcode = 411;
	else
	{
		// loop over and build time series
		for (k = 0; k < length; k++)
			temp[k] = getLinkValue(p_data, startPeriod + k, linkIndex, attr);

		*outValueSeries = temp;
	    *dim = length;
    }

    return set_error(p_data->error_handle, errorcode);
}



int DLLEXPORT SMO_getSystemSeries(SMO_Handle p_handle, SMO_systemAttribute attr,
		int startPeriod, int endPeriod, float** outValueSeries, int* dim)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using timeIndex and length respectively.
//
{
	int k, length, errorcode = 0;
	float* temp;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
    else if (startPeriod < 0 || startPeriod >= p_data->Nperiods ||
    		endPeriod <= startPeriod) errorcode = 422;
	// Check memory for outValues
	else if MEMCHECK(temp = newArray(length = endPeriod - startPeriod)) errorcode = 411;
	else
	{
		// loop over and build time series
		for (k = 0; k < length; k++)
			temp[k] = getSystemValue(p_data, startPeriod + k, attr);

		*outValueSeries = temp;
	    *dim = length;
    }

    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT SMO_getSubcatchAttribute(SMO_Handle p_handle, int periodIndex,
	SMO_subcatchAttribute attr, float** outValueArray, int* length)
//
//   Purpose: For all subcatchments at given time, get a particular attribute.
//
{
	int k, errorcode = 0;
	float* temp;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods) errorcode = 422;
	// Check memory for outValues
	else if MEMCHECK(temp = newArray(p_data->Nsubcatch)) errorcode = 411;
	else
	{
		// loop over and pull result
		for (k = 0; k < p_data->Nsubcatch; k++)
			temp[k] = getSubcatchValue(p_data, periodIndex, k, attr);

		*outValueArray = temp;
		*length = p_data->Nsubcatch;
	}

	return set_error(p_data->error_handle, errorcode);
}



int DLLEXPORT SMO_getNodeAttribute(SMO_Handle p_handle, int periodIndex,
	SMO_nodeAttribute attr, float** outValueArray, int* length)
//
//  Purpose: For all nodes at given time, get a particular attribute.
//
{
	int k, errorcode = 0;
	float* temp;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods) errorcode = 422;
	// Check memory for outValues
	else if MEMCHECK(temp = newArray(p_data->Nnodes)) errorcode = 411;
	else
	{
		// loop over and pull result
		for (k = 0; k < p_data->Nnodes; k++)
			temp[k] = getNodeValue(p_data, periodIndex, k, attr);

		*outValueArray = temp;
		*length = p_data->Nnodes;
	}

	return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT SMO_getLinkAttribute(SMO_Handle p_handle, int periodIndex,
	SMO_linkAttribute attr, float** outValueArray, int* length)
//
//  Purpose: For all links at given time, get a particular attribute.
//
{
	int k, errorcode = 0;
	float* temp;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods) errorcode = 422;
	// Check memory for outValues
	else if MEMCHECK(temp = newArray(p_data->Nlinks)) errorcode = 411;
	else
	{
		// loop over and pull result
		for (k = 0; k < p_data->Nlinks; k++)
			temp[k] = getLinkValue(p_data, periodIndex, k, attr);

		*outValueArray = temp;
		*length = p_data->Nlinks;
	}

	return set_error(p_data->error_handle, errorcode);
}


int DLLEXPORT SMO_getSystemAttribute(SMO_Handle p_handle, int periodIndex,
	SMO_systemAttribute attr, float** outValue, int* length)
//
//  Purpose: For the system at given time, get a particular attribute.
//
{
	int errorcode = 0;
	float temp;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods) errorcode = 422;
	else
	{
		// don't need to loop since there's only one system
		temp = getSystemValue(p_data, periodIndex, attr);

		*outValue = &temp;
		*length = 1;
	}

	return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT SMO_getSubcatchResult(SMO_Handle p_handle, long periodIndex,
		int subcatchIndex, float** outValueArray, int* arrayLength)
//
// Purpose: For a subcatchment at given time, get all attributes.
//
{
	int errorcode = 0;
	float* temp;
	F_OFF offset;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods) errorcode = 422;
    else if (subcatchIndex < 0 || subcatchIndex > p_data->Nsubcatch) errorcode = 423;
	else if MEMCHECK(temp = newArray(p_data->SubcatchVars)) errorcode = 411;
	else
	{
		// --- compute offset into output file
		offset = p_data->ResultsPos + (periodIndex)*p_data->BytesPerPeriod + 2 * RECORDSIZE;
		// add offset for subcatchment
		offset += (subcatchIndex*p_data->SubcatchVars)*RECORDSIZE;

		fseeko64(p_data->file, offset, SEEK_SET);
		fread(temp, RECORDSIZE, p_data->SubcatchVars, p_data->file);

		*outValueArray = temp;
		*arrayLength = p_data->SubcatchVars;
	}

	return set_error(p_data->error_handle, errorcode);
}


int DLLEXPORT SMO_getNodeResult(SMO_Handle p_handle, long periodIndex,
		int nodeIndex, float** outValueArray, int* arrayLength)
//
//	Purpose: For a node at given time, get all attributes.
//
{
	int errorcode = 0;
	float* temp;
	F_OFF offset;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods) errorcode = 422;
    else if (nodeIndex < 0 || nodeIndex > p_data->Nnodes) errorcode = 423;
	else if MEMCHECK(temp = newArray(p_data->NodeVars)) errorcode = 411;
	else
	{
		// calculate byte offset to start time for series
		offset = p_data->ResultsPos + (periodIndex)*p_data->BytesPerPeriod + 2 * RECORDSIZE;
		// add offset for subcatchment and node
		offset += (p_data->Nsubcatch*p_data->SubcatchVars + nodeIndex*p_data->NodeVars)*RECORDSIZE;

		fseeko64(p_data->file, offset, SEEK_SET);
		fread(temp, RECORDSIZE, p_data->NodeVars, p_data->file);

		*outValueArray = temp;
		*arrayLength = p_data->NodeVars;
	}

	return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT SMO_getLinkResult(SMO_Handle p_handle, long periodIndex,
		int linkIndex, float** outValueArray, int* arrayLength)
//
//	Purpose: For a link at given time, get all attributes.
//
{
	int errorcode = 0;
	float* temp;
	F_OFF offset;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods) errorcode = 422;
    else if (linkIndex < 0 || linkIndex > p_data->Nlinks) errorcode = 423;
	else if MEMCHECK(temp = newArray(p_data->LinkVars)) errorcode = 411;
	else
	{
		// calculate byte offset to start time for series
		offset = p_data->ResultsPos + (periodIndex)*p_data->BytesPerPeriod + 2 * RECORDSIZE;
		// add offset for subcatchment and node and link
		offset += (p_data->Nsubcatch*p_data->SubcatchVars
			+ p_data->Nnodes*p_data->NodeVars + linkIndex*p_data->LinkVars)*RECORDSIZE;

		fseeko64(p_data->file, offset, SEEK_SET);
		fread(temp, RECORDSIZE, p_data->LinkVars, p_data->file);

		*outValueArray = temp;
		*arrayLength = p_data->LinkVars;
	}

	return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT SMO_getSystemResult(SMO_Handle p_handle, long periodIndex,
		int dummyIndex, float** outValueArray, int* arrayLength)
//
//	Purpose: For the system at given time, get all attributes.
//
{
	int errorcode = 0;
	float* temp;
	F_OFF offset;
	data_t* p_data;

	p_data = (data_t*)p_handle;

    if (p_data == NULL) errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods) errorcode = 422;
	else if MEMCHECK(temp = newArray(p_data->SysVars)) errorcode = 411;
	{
		// calculate byte offset to start time for series
		offset = p_data->ResultsPos + (periodIndex)*p_data->BytesPerPeriod + 2 * RECORDSIZE;
		// add offset for subcatchment and node and link (system starts after the last link)
		offset += (p_data->Nsubcatch*p_data->SubcatchVars + p_data->Nnodes*p_data->NodeVars
			+ p_data->Nlinks*p_data->LinkVars)*RECORDSIZE;

		fseeko64(p_data->file, offset, SEEK_SET);
		fread(temp, RECORDSIZE, p_data->SysVars, p_data->file);

		*outValueArray = temp;
		*arrayLength = p_data->SysVars;
	}

	return errorcode;
}

void DLLEXPORT SMO_free(void** array)
//
//  Purpose: Frees memory allocated by API calls
//
{
	if (array != NULL) {
		free(*array);
		*array = NULL;
	}
}

void DLLEXPORT SMO_clearError(SMO_Handle p_handle)
{
	data_t* p_data;

	p_data = (data_t*)p_handle;
	clear_error(p_data->error_handle);
}

int DLLEXPORT SMO_checkError(SMO_Handle p_handle, char** msg_buffer)
{
	int errorcode = 0;
	char *temp = NULL;
	data_t* p_data;

	p_data = (data_t*)p_handle;

	if (p_data == NULL) return -1;
	else
	{
		errorcode = p_data->error_handle->error_status;
		if (errorcode)
			temp = check_error(p_data->error_handle);

		*msg_buffer = temp;
	}

	return errorcode;
}

void errorLookup(int errcode, char* dest_msg, int dest_len)
//
//  Purpose: takes error code returns error message
//
// ERR411 "Error 411: memory allocation failure"
//
// ERR421 "Input Error 421: invalid parameter code"
// ERR422 "Input Error 422: reporting period index out of range"
// ERR423 "Input Error 423: element index out of range"
// ERR424 "Input Error 424: no memory allocated for results"
//
// ERR434 "File Error  434: unable to open binary output file"
// ERR435 "File Error  435: invalid file - not created by SWMM"
// ERR436 "File Error  436: invalid file - contains no results"
// ERR437 "File Error  437: invalid file - model run issued warnings"
//
// ERR440 "ERROR 440: an unspecified error has occurred"
{
	const char* msg;

	switch (errcode)
	{
	case 411: msg = ERR411;
		break;
	case 421: msg = ERR421;
		break;
	case 422: msg = ERR422;
		break;
	case 423: msg = ERR423;
		break;
	case 424: msg = ERR424;
		break;
	case 434: msg = ERR434;
		break;
	case 435: msg = ERR435;
		break;
	case 436: msg = ERR436;
		break;
	case 437: msg = ERR437;
		break;
	default: msg = ERR440;
	}

	strncpy(dest_msg, msg, MAXMSG);
}


// Local functions:
int validateFile(data_t* p_data)
{
	INT4 magic1, magic2, errcode;
	int errorcode = 0;

	// --- fast forward to end and read epilogue
	fseeko64(p_data->file, -6 * RECORDSIZE, SEEK_END);
	fread(&(p_data->IDPos), RECORDSIZE, 1, p_data->file);
	fread(&(p_data->ObjPropPos), RECORDSIZE, 1, p_data->file);
	fread(&(p_data->ResultsPos), RECORDSIZE, 1, p_data->file);
	fread(&(p_data->Nperiods), RECORDSIZE, 1, p_data->file);
	fread(&errcode, RECORDSIZE, 1, p_data->file);
	fread(&magic2, RECORDSIZE, 1, p_data->file);

	// --- rewind and read magic number from beginning of the file
	fseeko(p_data->file, 0L, SEEK_SET);
	fread(&magic1, RECORDSIZE, 1, p_data->file);

	// Is this a valid SWMM binary output file?
	if (magic1 != magic2) errorcode = 435;
	// Does the binary file contain results?
	else if (p_data->Nperiods <= 0) errorcode = 436;
	// Were there problems with the model run?
	else if (errcode != 0) errorcode = 437;

	return errorcode;
}

void initElementNames(data_t* p_data)
{
	int j, numNames;

	numNames = p_data->Nsubcatch + p_data->Nnodes + p_data->Nlinks + p_data->Npolluts;

	// allocate memory for array of idEntries
	p_data->elementNames = (idEntry*)calloc(numNames, sizeof(idEntry));

	// Position the file to the start of the ID entries
	fseeko64(p_data->file, p_data->IDPos, SEEK_SET);

	for(j=0;j<numNames;j++)
	{
		fread(&(p_data->elementNames[j].length), RECORDSIZE, 1, p_data->file);

		p_data->elementNames[j].IDname =
				(char*)calloc(p_data->elementNames[j].length + 1, sizeof(char));

		fread(p_data->elementNames[j].IDname, sizeof(char),
				p_data->elementNames[j].length, p_data->file);
	}
}

double getTimeValue(data_t* p_data, long timeIndex)
{
	F_OFF offset;
	double value;

	// --- compute offset into output file
	offset = p_data->ResultsPos + timeIndex*p_data->BytesPerPeriod;

	// --- re-position the file and read the result
	fseeko64(p_data->file, offset, SEEK_SET);
	fread(&value, RECORDSIZE * 2, 1, p_data->file);

	return value;
}

float getSubcatchValue(data_t* p_data, long timeIndex, int subcatchIndex,
		SMO_subcatchAttribute attr)
{
	F_OFF offset;
	float value;

	// --- compute offset into output file
	offset = p_data->ResultsPos + timeIndex*p_data->BytesPerPeriod + 2*RECORDSIZE;
	// offset for subcatch
	offset += RECORDSIZE*(subcatchIndex*p_data->SubcatchVars + attr);

	// --- re-position the file and read the result
	fseeko64(p_data->file, offset, SEEK_SET);
	fread(&value, RECORDSIZE, 1, p_data->file);

	return value;
}

float getNodeValue(data_t* p_data, long timeIndex, int nodeIndex,
		SMO_nodeAttribute attr)
{
	F_OFF offset;
	float value;

	// --- compute offset into output file
	offset = p_data->ResultsPos + timeIndex*p_data->BytesPerPeriod + 2*RECORDSIZE;
	// offset for node
	offset += RECORDSIZE*(p_data->Nsubcatch*p_data->SubcatchVars + nodeIndex*p_data->NodeVars + attr);

	// --- re-position the file and read the result
	fseeko64(p_data->file, offset, SEEK_SET);
	fread(&value, RECORDSIZE, 1, p_data->file);

	return value;
}

float getLinkValue(data_t* p_data, long timeIndex, int linkIndex,
		SMO_linkAttribute attr)
{
	F_OFF offset;
	float value;

	// --- compute offset into output file
	offset = p_data->ResultsPos + timeIndex*p_data->BytesPerPeriod + 2*RECORDSIZE;
	// offset for link
	offset += RECORDSIZE*(p_data->Nsubcatch*p_data->SubcatchVars + p_data->Nnodes*p_data->NodeVars +
		linkIndex*p_data->LinkVars + attr);

	// --- re-position the file and read the result
	fseeko64(p_data->file, offset, SEEK_SET);
	fread(&value, RECORDSIZE, 1, p_data->file);

	return value;
}

float getSystemValue(data_t* p_data, long timeIndex,
		SMO_systemAttribute attr)
{
	F_OFF offset;
	float value;

	// --- compute offset into output file
	offset = p_data->ResultsPos + timeIndex*p_data->BytesPerPeriod + 2*RECORDSIZE;
	//  offset for system
	offset += RECORDSIZE*(p_data->Nsubcatch*p_data->SubcatchVars + p_data->Nnodes*p_data->NodeVars +
		p_data->Nlinks*p_data->LinkVars + attr);

	// --- re-position the file and read the result
	fseeko64(p_data->file, offset, SEEK_SET);
	fread(&value, RECORDSIZE, 1, p_data->file);

	return value;
}

float* newArray(int n)
//
//  Warning: Caller must free memory allocated by this function.
//
{
	return (float*) malloc((n)*sizeof(float));
}
