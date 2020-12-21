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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errormanager.h"
#include "messages.h"

#include "swmm_output.h"


// NOTE: These depend on machine data model and may change when porting
// F_OFF Must be a 8 byte / 64 bit integer for large file support
#ifdef _WIN32    // Windows (32-bit and 64-bit)
  #define F_OFF __int64
#else    // Other platforms
  #define F_OFF off_t
#endif

#define INT4 int      // Must be a 4 byte / 32 bit integer type
#define REAL4 float   // Must be a 4 byte / 32 bit real type

#define RECORDSIZE 4  // Memory alignment 4 byte word size for both int and real
#define DATESIZE 8    // Dates are stored as 8 byte word size

#define NELEMENTTYPES 5    // Number of element types
#define MEMCHECK(x) (((x) == NULL) ? 414 : 0)


struct IDentry {
    char* IDname;
    int   length;
};
typedef struct IDentry idEntry;


//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------

typedef struct Handle {
    char  name[MAXFILENAME + 1];    // file path/name
    FILE* file;                     // FILE structure pointer

    struct IDentry* elementNames;    // array of pointers to element names

    long Nperiods;     // number of reporting periods
    int  FlowUnits;    // flow units code

    int Nsubcatch;    // number of subcatchments
    int Nnodes;       // number of drainage system nodes
    int Nlinks;       // number of drainage system links
    int Npolluts;     // number of pollutants tracked

    int SubcatchVars;    // number of subcatch reporting variables
    int NodeVars;        // number of node reporting variables
    int LinkVars;        // number of link reporting variables
    int SysVars;         // number of system reporting variables

    double StartDate;     // start date of simulation
    int    ReportStep;    // reporting time step (seconds)

    F_OFF IDPos;             // file position where object ID names start
    F_OFF ObjPropPos;        // file position where object properties start
    F_OFF ResultsPos;        // file position where results start
    F_OFF BytesPerPeriod;    // bytes used for results in each period

    error_handle_t* error_handle;
} data_t, *SMO_Handle;


//-----------------------------------------------------------------------------
//   Local functions
//-----------------------------------------------------------------------------
void errorLookup(int errcode, char *errmsg, int length);
int  validateFile(data_t *p_data);
void initElementNames(data_t *p_data);

double getTimeValue(data_t *p_data, int timeIndex);
float  getSubcatchValue(data_t *p_data, int timeIndex, int subcatchIndex, SMO_subcatchAttribute attr);
float  getNodeValue(data_t *p_data, int timeIndex, int nodeIndex, SMO_nodeAttribute attr);
float  getLinkValue(data_t *p_data, int timeIndex, int linkIndex, SMO_linkAttribute attr);
float  getSystemValue(data_t *p_data, int timeIndex, SMO_systemAttribute attr);

int   _fopen(FILE **f, const char *name, const char *mode);
int   _fseek(FILE *stream, F_OFF offset, int whence);
F_OFF _ftell(FILE *stream);

float *newFloatArray(int n);
int   *newIntArray(int n);
char  *newCharArray(int n);


int EXPORT_OUT_API SMO_init(SMO_Handle *p_handle)
//  Purpose: Initialized pointer for the opaque SMO_Handle.
//
//  Returns: Error code 0 on success, -1 on failure
//
//  Note: The existence of this function has been carefully considered.
//   Don't change it.
//
{
    int     errorcode = 0;
    data_t *priv_data;

    // Allocate memory for private data
    priv_data = (data_t *)calloc(1, sizeof(data_t));

    if (priv_data != NULL) {
        priv_data->error_handle = new_errormanager(&errorLookup);
        *p_handle = priv_data;
    } 
    else
        errorcode = -1;

    // TODO: Need to handle errors during initialization better.
    return errorcode;
}

int EXPORT_OUT_API SMO_close(SMO_Handle p_handle)

//
//   Purpose: Clean up after and close Output API
//
{
    data_t *p_data;
    int i, n, errorcode = 0;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;

    else {
        if (p_data->elementNames != NULL) {
            n = p_data->Nsubcatch + p_data->Nnodes + p_data->Nlinks +
                p_data->Npolluts;

            for (i = 0; i < n; i++)
                free(p_data->elementNames[i].IDname);

            free(p_data->elementNames);
        }

        dst_errormanager(p_data->error_handle);

        if (p_data->file != NULL)
            fclose(p_data->file);

        free(p_data);
    }

    return errorcode;
}

int EXPORT_OUT_API SMO_open(SMO_Handle p_handle, const char *path)
//
//  Purpose: Open the output binary file and read the header.
//
{
    int   err, errorcode = 0;
    F_OFF offset;

    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        return -1;
    else {
        strncpy(p_data->name, path, MAXFILENAME);

        // Attempt to open binary output file for reading only
        if ((_fopen(&(p_data->file), path, "rb")) != 0)
            errorcode = 434;

        // --- validate the output file
        else if ((err = validateFile(p_data)) != 0)
            errorcode = err;

        // If a warning is encountered read file header
        if (errorcode < 400) {
            // --- otherwise read additional parameters from start of file
            fseek(p_data->file, 3 * RECORDSIZE, SEEK_SET);
            fread(&(p_data->Nsubcatch), RECORDSIZE, 1, p_data->file);
            fread(&(p_data->Nnodes), RECORDSIZE, 1, p_data->file);
            fread(&(p_data->Nlinks), RECORDSIZE, 1, p_data->file);
            fread(&(p_data->Npolluts), RECORDSIZE, 1, p_data->file);

            // Compute offset for saved subcatch/node/link input values
            offset =
                (p_data->Nsubcatch + 2) * RECORDSIZE    // Subcatchment area
                + (3 * p_data->Nnodes + 4) *
                      RECORDSIZE    // Node type, invert & max depth
                + (5 * p_data->Nlinks + 6) *
                      RECORDSIZE;    // Link type, z1, z2, max depth & length
            offset += p_data->ObjPropPos;

            // Read number & codes of computed variables
            _fseek(p_data->file, offset, SEEK_SET);
            fread(&(p_data->SubcatchVars), RECORDSIZE, 1,
                  p_data->file);    // # Subcatch variables

            _fseek(p_data->file, p_data->SubcatchVars * RECORDSIZE, SEEK_CUR);
            fread(&(p_data->NodeVars), RECORDSIZE, 1,
                  p_data->file);    // # Node variables

            _fseek(p_data->file, p_data->NodeVars * RECORDSIZE, SEEK_CUR);
            fread(&(p_data->LinkVars), RECORDSIZE, 1,
                  p_data->file);    // # Link variables

            _fseek(p_data->file, p_data->LinkVars * RECORDSIZE, SEEK_CUR);
            fread(&(p_data->SysVars), RECORDSIZE, 1,
                  p_data->file);    // # System variables

            // --- read data just before start of output results
            offset = p_data->ResultsPos - 3 * RECORDSIZE;
            _fseek(p_data->file, offset, SEEK_SET);
            fread(&(p_data->StartDate), DATESIZE, 1, p_data->file);
            fread(&(p_data->ReportStep), RECORDSIZE, 1, p_data->file);

            // --- compute number of bytes of results values used per time
            // period
            p_data->BytesPerPeriod =
                DATESIZE +
                (p_data->Nsubcatch * p_data->SubcatchVars +
                 p_data->Nnodes * p_data->NodeVars +
                 p_data->Nlinks * p_data->LinkVars + p_data->SysVars) *
                    RECORDSIZE;
        }
    }
    // If error close the binary file
    if (errorcode > 400) {
        set_error(p_data->error_handle, errorcode);
        SMO_close(p_handle);
    }

    return errorcode;
}

int EXPORT_OUT_API SMO_getVersion(SMO_Handle p_handle, int *version)
//
//  Input:   p_handle = pointer to SMO_Handle struct
//  Output:  version SWMM version
//  Returns: error code
//
//  Purpose: Returns SWMM version that wrote binary file
//
{
    int     errorcode = 0;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        return -1;
    else {
        fseek(p_data->file, 1 * RECORDSIZE, SEEK_SET);
        if (fread(version, RECORDSIZE, 1, p_data->file) != 1)
            errorcode = 436;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getProjectSize(SMO_Handle p_handle, int **elementCount, int *length)
//
//   Purpose: Returns project size.
//
{
    int     errorcode = 0;
    int     *temp;
    data_t  *p_data;

    p_data = (data_t*)p_handle;

    *elementCount = NULL;
    *length = NELEMENTTYPES;

    if (p_data == NULL)
        errorcode = -1;
    else if (MEMCHECK(temp = newIntArray(*length)))
        errorcode = 414;
    else {
        temp[0] = p_data->Nsubcatch;
        temp[1] = p_data->Nnodes;
        temp[2] = p_data->Nlinks;
        temp[3] = 1;   // NSystems
        temp[4] = p_data->Npolluts;

        *elementCount = temp;
    }

    return set_error(p_data->error_handle, errorcode);
}


int EXPORT_OUT_API SMO_getUnits(SMO_Handle p_handle, int **unitFlag, int *length)
//
//  Purpose: Returns unit flags for unit_system, flow, and pollutants.
//
{
    int     errorcode = 0;
    int*    temp;
    F_OFF   offset;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    *unitFlag = NULL;
    if (p_data->Npolluts > 0)
        *length = 2 + p_data->Npolluts;
    else
        *length = 3;

    p_data = (data_t*)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (MEMCHECK(temp = newIntArray(*length)))
        errorcode = 414;
    else {
        // Set flow units flag
        fseek(p_data->file, 2*RECORDSIZE, SEEK_SET);
        fread(&temp[1], RECORDSIZE, 1, p_data->file);

        // Set unit system based on flow flag
        if (temp[1] < SMO_CMS)
            temp[0] = SMO_US;
        else
            temp[0] = SMO_SI;

        // Set conc units flag
        if (p_data->Npolluts == 0)
            temp[2] = SMO_NONE;
        else {
            offset = p_data->ObjPropPos - (p_data->Npolluts * RECORDSIZE);
            _fseek(p_data->file, offset, SEEK_SET);
            fread(&temp[2], RECORDSIZE, p_data->Npolluts, p_data->file);
        }
        *unitFlag = temp;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getStartDate(SMO_Handle p_handle, double *date)
//
//	Purpose: Returns start date.
//
{
    int     errorcode = 0;
    data_t *p_data;

    *date = -1.0;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else
        *date = p_data->StartDate;

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getTimes(SMO_Handle p_handle, SMO_time code, int *time)
//
//   Purpose: Returns step size and number of periods.
//
{
    int    errorcode = 0;
    data_t *p_data;

    *time = -1;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else {
        switch (code) {
            case SMO_reportStep:
                *time = p_data->ReportStep;
                break;
            case SMO_numPeriods:
                *time = p_data->Nperiods;
                break;
            default:
                errorcode = 421;
        }
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getElementName(SMO_Handle p_handle, SMO_elementType type,
    int index, char **name, int *length)
//
//  Purpose: Given an element index returns the element name.
//
{
    int    idx = -1, errorcode = 0;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = 410;
    else if (p_data->file == NULL)
        errorcode = 411;
    else {
        // Initialize the name array if necessary
        if (p_data->elementNames == NULL)
            initElementNames(p_data);

        switch (type) {
            case SMO_subcatch:
                if (index < 0 || index >= p_data->Nsubcatch)
                    errorcode = 423;
                else
                    idx = index;
                break;

            case SMO_node:
                if (index < 0 || index >= p_data->Nnodes)
                    errorcode = 423;
                else
                    idx = p_data->Nsubcatch + index;
                break;

            case SMO_link:
                if (index < 0 || index >= p_data->Nlinks)
                    errorcode = 423;
                else
                    idx = p_data->Nsubcatch + p_data->Nnodes + index;
                break;

            case SMO_pollut:
                if (index < 0 || index >= p_data->Npolluts)
                    errorcode = 423;
                else
                    idx = p_data->Nsubcatch + p_data->Nnodes + p_data->Nlinks +
                          index;
                break;

            default:
                errorcode = 421;
        }

        if (!errorcode) {
            *length = p_data->elementNames[idx].length;
            *name   = newCharArray(*length + 1);
            // Writes IDname and an additional null character to name
            strncpy(*name, p_data->elementNames[idx].IDname,
                    (*length + 1) * sizeof(char));
        }
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getSubcatchSeries(SMO_Handle p_handle, int subcatchIndex,
    SMO_subcatchAttribute attr, int startPeriod, int endPeriod,
    float **outValueArray, int *length)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using timeIndex and length respectively.
//
{
    int    k, len, errorcode = 0;
    float  *temp;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (subcatchIndex < 0 || subcatchIndex > p_data->Nsubcatch)
        errorcode = 420;
    else if (startPeriod < 0 || startPeriod >= p_data->Nperiods ||
             endPeriod <= startPeriod)
        errorcode = 422;
    // Check memory for outValues
    else if
        MEMCHECK(temp = newFloatArray(len = endPeriod - startPeriod))
    errorcode = 411;
    else {
        // loop over and build time series
        for (k = 0; k < len; k++)
            temp[k] =
                getSubcatchValue(p_data, startPeriod + k, subcatchIndex, attr);

        *outValueArray = temp;
        *length         = len;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getNodeSeries(SMO_Handle p_handle, int nodeIndex,
    SMO_nodeAttribute attr, int startPeriod, int endPeriod,
    float **outValueArray, int *length)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using timeIndex and length respectively.
//
{
    int    k, len, errorcode = 0;
    float  *temp;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (nodeIndex < 0 || nodeIndex > p_data->Nnodes)
        errorcode = 420;
    else if (startPeriod < 0 || startPeriod >= p_data->Nperiods ||
             endPeriod <= startPeriod)
        errorcode = 422;
    // Check memory for outValues
    else if
        MEMCHECK(temp = newFloatArray(len = endPeriod - startPeriod))
    errorcode = 411;
    else {
        // loop over and build time series
        for (k = 0; k < len; k++)
            temp[k] = getNodeValue(p_data, startPeriod + k, nodeIndex, attr);

        *outValueArray = temp;
        *length         = len;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getLinkSeries(SMO_Handle p_handle, int linkIndex,
    SMO_linkAttribute attr, int startPeriod, int endPeriod,
    float **outValueArray, int *length)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using timeIndex and length respectively.
//
{
    int    k, len, errorcode = 0;
    float  *temp;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (linkIndex < 0 || linkIndex > p_data->Nlinks)
        errorcode = 420;
    else if (startPeriod < 0 || startPeriod >= p_data->Nperiods ||
             endPeriod <= startPeriod)
        errorcode = 422;
    // Check memory for outValues
    else if
        MEMCHECK(temp = newFloatArray(len = endPeriod - startPeriod))
    errorcode = 411;
    else {
        // loop over and build time series
        for (k = 0; k < len; k++)
            temp[k] = getLinkValue(p_data, startPeriod + k, linkIndex, attr);

        *outValueArray = temp;
        *length         = len;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getSystemSeries(SMO_Handle p_handle, SMO_systemAttribute attr,
    int startPeriod, int endPeriod, float **outValueArray, int *length)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using timeIndex and length respectively.
//
{
    int    k, len, errorcode = 0;
    float  *temp;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (startPeriod < 0 || startPeriod >= p_data->Nperiods ||
             endPeriod <= startPeriod)
        errorcode = 422;
    // Check memory for outValues
    else if
        MEMCHECK(temp = newFloatArray(len = endPeriod - startPeriod))
    errorcode = 411;
    else {
        // loop over and build time series
        for (k = 0; k < len; k++)
            temp[k] = getSystemValue(p_data, startPeriod + k, attr);

        *outValueArray = temp;
        *length         = len;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getSubcatchAttribute(SMO_Handle p_handle, int periodIndex,
    SMO_subcatchAttribute attr, float **outValueArray, int *length)
//
//   Purpose: For all subcatchments at given time, get a particular attribute.
//
{
    int    k, errorcode = 0;
    float  *temp;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods)
        errorcode = 422;
    // Check memory for outValues
    else if
        MEMCHECK(temp = newFloatArray(p_data->Nsubcatch)) errorcode = 411;
    else {
        // loop over and pull result
        for (k = 0; k < p_data->Nsubcatch; k++)
            temp[k] = getSubcatchValue(p_data, periodIndex, k, attr);

        *outValueArray = temp;
        *length        = p_data->Nsubcatch;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getNodeAttribute(SMO_Handle p_handle, int periodIndex,
    SMO_nodeAttribute attr, float **outValueArray, int *length)
//
//  Purpose: For all nodes at given time, get a particular attribute.
//
{
    int    k, errorcode = 0;
    float  *temp;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods)
        errorcode = 422;
    // Check memory for outValues
    else if
        MEMCHECK(temp = newFloatArray(p_data->Nnodes)) errorcode = 411;
    else {
        // loop over and pull result
        for (k = 0; k < p_data->Nnodes; k++)
            temp[k] = getNodeValue(p_data, periodIndex, k, attr);

        *outValueArray = temp;
        *length        = p_data->Nnodes;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getLinkAttribute(SMO_Handle p_handle, int periodIndex,
    SMO_linkAttribute attr, float **outValueArray, int *length)
//
//  Purpose: For all links at given time, get a particular attribute.
//
{
    int    k, errorcode = 0;
    float  *temp;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods)
        errorcode = 422;
    // Check memory for outValues
    else if
        MEMCHECK(temp = newFloatArray(p_data->Nlinks)) errorcode = 411;
    else {
        // loop over and pull result
        for (k = 0; k < p_data->Nlinks; k++)
            temp[k] = getLinkValue(p_data, periodIndex, k, attr);

        *outValueArray = temp;
        *length        = p_data->Nlinks;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getSystemAttribute(SMO_Handle p_handle, int periodIndex,
    SMO_systemAttribute attr, float **outValueArray, int *length)
//
//  Purpose: For the system at given time, get a particular attribute.
//
{
    int     errorcode = 0;
    float   temp;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods)
        errorcode = 422;
    else {
        // don't need to loop since there's only one system
        temp = getSystemValue(p_data, periodIndex, attr);

        *outValueArray = &temp;
        *length        = 1;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getSubcatchResult(SMO_Handle p_handle, int periodIndex,
    int subcatchIndex, float **outValueArray, int *arrayLength)
//
// Purpose: For a subcatchment at given time, get all attributes.
//
{
    int    errorcode = 0;
    float  *temp;
    F_OFF  offset;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods)
        errorcode = 422;
    else if (subcatchIndex < 0 || subcatchIndex > p_data->Nsubcatch)
        errorcode = 423;
    else if
        MEMCHECK(temp = newFloatArray(p_data->SubcatchVars)) errorcode = 411;
    else {
        // --- compute offset into output file
        offset = p_data->ResultsPos + (periodIndex)*p_data->BytesPerPeriod +
                 2 * RECORDSIZE;
        // add offset for subcatchment
        offset += (subcatchIndex * p_data->SubcatchVars) * RECORDSIZE;

        _fseek(p_data->file, offset, SEEK_SET);
        fread(temp, RECORDSIZE, p_data->SubcatchVars, p_data->file);

        *outValueArray = temp;
        *arrayLength   = p_data->SubcatchVars;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getNodeResult(SMO_Handle p_handle, int periodIndex,
    int nodeIndex, float **outValueArray, int *arrayLength)
//
//	Purpose: For a node at given time, get all attributes.
//
{
    int    errorcode = 0;
    float  *temp;
    F_OFF  offset;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods)
        errorcode = 422;
    else if (nodeIndex < 0 || nodeIndex > p_data->Nnodes)
        errorcode = 423;
    else if
        MEMCHECK(temp = newFloatArray(p_data->NodeVars)) errorcode = 411;
    else {
        // calculate byte offset to start time for series
        offset = p_data->ResultsPos + (periodIndex)*p_data->BytesPerPeriod +
                 2 * RECORDSIZE;
        // add offset for subcatchment and node
        offset += (p_data->Nsubcatch * p_data->SubcatchVars +
                   nodeIndex * p_data->NodeVars) *
                  RECORDSIZE;

        _fseek(p_data->file, offset, SEEK_SET);
        fread(temp, RECORDSIZE, p_data->NodeVars, p_data->file);

        *outValueArray = temp;
        *arrayLength   = p_data->NodeVars;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getLinkResult(SMO_Handle p_handle, int periodIndex,
    int linkIndex, float **outValueArray, int *arrayLength)
//
//	Purpose: For a link at given time, get all attributes.
//
{
    int    errorcode = 0;
    float  *temp;
    F_OFF  offset;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods)
        errorcode = 422;
    else if (linkIndex < 0 || linkIndex > p_data->Nlinks)
        errorcode = 423;
    else if
        MEMCHECK(temp = newFloatArray(p_data->LinkVars)) errorcode = 411;
    else {
        // calculate byte offset to start time for series
        offset = p_data->ResultsPos + (periodIndex)*p_data->BytesPerPeriod +
                 2 * RECORDSIZE;
        // add offset for subcatchment and node and link
        offset +=
            (p_data->Nsubcatch * p_data->SubcatchVars +
             p_data->Nnodes * p_data->NodeVars + linkIndex * p_data->LinkVars) *
            RECORDSIZE;

        _fseek(p_data->file, offset, SEEK_SET);
        fread(temp, RECORDSIZE, p_data->LinkVars, p_data->file);

        *outValueArray = temp;
        *arrayLength   = p_data->LinkVars;
    }

    return set_error(p_data->error_handle, errorcode);
}

int EXPORT_OUT_API SMO_getSystemResult(SMO_Handle p_handle, int periodIndex,
    int dummyIndex, float **outValueArray, int *arrayLength)
//
//	Purpose: For the system at given time, get all attributes.
//
{
    int    errorcode = 0;
    float  *temp;
    F_OFF  offset;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        errorcode = -1;
    else if (periodIndex < 0 || periodIndex >= p_data->Nperiods)
        errorcode = 422;
    else if
        MEMCHECK(temp = newFloatArray(p_data->SysVars)) errorcode = 411;
    {
        // calculate byte offset to start time for series
        offset = p_data->ResultsPos + (periodIndex)*p_data->BytesPerPeriod +
                 2 * RECORDSIZE;
        // add offset for subcatchment and node and link (system starts after
        // the last link)
        offset += (p_data->Nsubcatch * p_data->SubcatchVars +
                   p_data->Nnodes * p_data->NodeVars +
                   p_data->Nlinks * p_data->LinkVars) *
                  RECORDSIZE;

        _fseek(p_data->file, offset, SEEK_SET);
        fread(temp, RECORDSIZE, p_data->SysVars, p_data->file);

        *outValueArray = temp;
        *arrayLength   = p_data->SysVars;
    }

    return set_error(p_data->error_handle, errorcode);
}

void EXPORT_OUT_API SMO_freeMemory(void *array)
//
//  Purpose: Frees memory allocated by API calls
//
{
    free(array);
}

void EXPORT_OUT_API SMO_clearError(SMO_Handle p_handle) {
    data_t *p_data;

    p_data = (data_t *)p_handle;
    clear_error(p_data->error_handle);
}

int EXPORT_OUT_API SMO_checkError(SMO_Handle p_handle, char **msg_buffer) {
    int    errorcode = 0;
    char   *temp      = NULL;
    data_t *p_data;

    p_data = (data_t *)p_handle;

    if (p_data == NULL)
        return -1;
    else {
        errorcode = p_data->error_handle->error_status;
        if (errorcode)
            temp = check_error(p_data->error_handle);

        *msg_buffer = temp;
    }

    return errorcode;
}

void errorLookup(int errcode, char *dest_msg, int dest_len)
//
//  Purpose: takes error code returns error message
//
{
    const char *msg;

    switch (errcode) {
        case 10:
            msg = WARN10;
            break;
        case 411:
            msg = ERR411;
            break;
        case 421:
            msg = ERR421;
            break;
        case 422:
            msg = ERR422;
            break;
        case 423:
            msg = ERR423;
            break;
        case 424:
            msg = ERR424;
            break;
        case 434:
            msg = ERR434;
            break;
        case 435:
            msg = ERR435;
            break;
        case 436:
            msg = ERR436;
            break;
        default:
            msg = ERR440;
    }

    strncpy(dest_msg, msg, MAXMSG);
}

// Local functions:
int validateFile(data_t *p_data) {
    INT4 magic1, magic2, errcode;
    int  errorcode = 0;

    // --- fast forward to end and read epilogue
    _fseek(p_data->file, -6 * RECORDSIZE, SEEK_END);
    fread(&(p_data->IDPos), RECORDSIZE, 1, p_data->file);
    fread(&(p_data->ObjPropPos), RECORDSIZE, 1, p_data->file);
    fread(&(p_data->ResultsPos), RECORDSIZE, 1, p_data->file);
    fread(&(p_data->Nperiods), RECORDSIZE, 1, p_data->file);
    fread(&errcode, RECORDSIZE, 1, p_data->file);
    fread(&magic2, RECORDSIZE, 1, p_data->file);

    // --- rewind and read magic number from beginning of the file
    _fseek(p_data->file, 0L, SEEK_SET);
    fread(&magic1, RECORDSIZE, 1, p_data->file);

    // Is this a valid SWMM binary output file?
    if (magic1 != magic2)
        errorcode = 435;
    // Does the binary file contain results?
    else if (p_data->Nperiods <= 0)
        errorcode = 436;
    // Were there problems with the model run?
    else if (errcode != 0)
        errorcode = 10;

    return errorcode;
}

void initElementNames(data_t *p_data) {
    int j, numNames;

    numNames =
        p_data->Nsubcatch + p_data->Nnodes + p_data->Nlinks + p_data->Npolluts;

    // allocate memory for array of idEntries
    p_data->elementNames = (idEntry*)calloc(numNames, sizeof(idEntry));

    // Position the file to the start of the ID entries
    _fseek(p_data->file, p_data->IDPos, SEEK_SET);

    for (j = 0; j < numNames; j++) {
        fread(&(p_data->elementNames[j].length), RECORDSIZE, 1, p_data->file);

        p_data->elementNames[j].IDname =
            (char*)calloc(p_data->elementNames[j].length + 1, sizeof(char));

        fread(p_data->elementNames[j].IDname, sizeof(char),
              p_data->elementNames[j].length, p_data->file);
    }
}

double getTimeValue(data_t *p_data, int timeIndex) {

    F_OFF  offset;
    double value;

    // --- compute offset into output file
    offset = p_data->ResultsPos + timeIndex * p_data->BytesPerPeriod;

    // --- re-position the file and read the result
    _fseek(p_data->file, offset, SEEK_SET);
    fread(&value, RECORDSIZE * 2, 1, p_data->file);

    return value;
}

float getSubcatchValue(data_t *p_data, int timeIndex, int subcatchIndex,
    SMO_subcatchAttribute attr) {

    F_OFF offset;
    float value;

    // --- compute offset into output file
    offset = p_data->ResultsPos + timeIndex * p_data->BytesPerPeriod +
             2 * RECORDSIZE;
    // offset for subcatch
    offset += RECORDSIZE * (subcatchIndex * p_data->SubcatchVars + attr);

    // --- re-position the file and read the result
    _fseek(p_data->file, offset, SEEK_SET);
    fread(&value, RECORDSIZE, 1, p_data->file);

    return value;
}

float getNodeValue(data_t *p_data, int timeIndex, int nodeIndex,
    SMO_nodeAttribute attr) {

    F_OFF offset;
    float value;

    // --- compute offset into output file
    offset = p_data->ResultsPos + timeIndex * p_data->BytesPerPeriod +
             2 * RECORDSIZE;
    // offset for node
    offset += RECORDSIZE * (p_data->Nsubcatch * p_data->SubcatchVars +
                            nodeIndex * p_data->NodeVars + attr);

    // --- re-position the file and read the result
    _fseek(p_data->file, offset, SEEK_SET);
    fread(&value, RECORDSIZE, 1, p_data->file);

    return value;
}

float getLinkValue(data_t *p_data, int timeIndex, int linkIndex,
    SMO_linkAttribute attr) {

    F_OFF offset;
    float value;

    // --- compute offset into output file
    offset = p_data->ResultsPos + timeIndex * p_data->BytesPerPeriod +
             2 * RECORDSIZE;
    // offset for link
    offset += RECORDSIZE * (p_data->Nsubcatch * p_data->SubcatchVars +
                            p_data->Nnodes * p_data->NodeVars +
                            linkIndex * p_data->LinkVars + attr);

    // --- re-position the file and read the result
    _fseek(p_data->file, offset, SEEK_SET);
    fread(&value, RECORDSIZE, 1, p_data->file);

    return value;
}

float getSystemValue(data_t *p_data, int timeIndex, SMO_systemAttribute attr) {

    F_OFF offset;
    float value;

    // --- compute offset into output file
    offset = p_data->ResultsPos + timeIndex * p_data->BytesPerPeriod +
             2 * RECORDSIZE;
    //  offset for system
    offset += RECORDSIZE * (p_data->Nsubcatch * p_data->SubcatchVars +
                            p_data->Nnodes * p_data->NodeVars +
                            p_data->Nlinks * p_data->LinkVars + attr);

    // --- re-position the file and read the result
    _fseek(p_data->file, offset, SEEK_SET);
    fread(&value, RECORDSIZE, 1, p_data->file);

    return value;
}

int _fopen(FILE **f, const char *name, const char *mode) {
    //
    //  Purpose: Substitute for fopen_s on platforms where it doesn't exist
    //  Note: fopen_s is part of C++11 standard
    //
    int ret = 0;
#ifdef _MSC_VER
    ret = (int)fopen_s(f, name, mode);
#else
    *f = fopen(name, mode);
    if (!*f)
        ret = -1;
#endif
    return ret;
}

int _fseek(FILE *stream, F_OFF offset, int whence)
//
//  Purpose: Selects platform fseek() for large file support
//
{
#ifdef _MSC_VER
#define FSEEK64 _fseeki64
#else
#define FSEEK64 fseeko
#endif

    return FSEEK64(stream, offset, whence);
}

F_OFF _ftell(FILE *stream)
//
//  Purpose: Selects platform ftell() for large file support
//
{
#ifdef _MSC_VER
#define FTELL64 _ftelli64
#else
#define FTELL64 ftello
#endif
    return FTELL64(stream);
}

float *newFloatArray(int n)
//
//  Warning: Caller must free memory allocated by this function.
//
{
    return (float *)malloc((n) * sizeof(float));
}

int *newIntArray(int n)
//
//  Warning: Caller must free memory allocated by this function.
//
{
    return (int *)malloc((n) * sizeof(int));
}

char *newCharArray(int n)
//
//  Warning: Caller must free memory allocated by this function.
//
{
    return (char *)malloc((n) * sizeof(char));
}
