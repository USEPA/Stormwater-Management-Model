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

#define MAXFILENAME 259    // Max characters in file path
#define MAXELENAME 31      // Max characters in element name

// This is an opaque pointer to struct. Do not access variables.
typedef struct Handle *SMO_Handle;


#include "swmm_output_enums.h"
#include "swmm_output_export.h"

#ifdef __cplusplus
extern "C" {
#endif

EXPORT_OUT_API int SMO_init(SMO_Handle *p_handle);
EXPORT_OUT_API int SMO_close(SMO_Handle p_handle);
EXPORT_OUT_API int SMO_open(SMO_Handle p_handle, const char *path);
EXPORT_OUT_API int SMO_getVersion(SMO_Handle p_handle, int *version);
EXPORT_OUT_API int SMO_getProjectSize(SMO_Handle p_handle, int **int_out, int *int_dim);

EXPORT_OUT_API int SMO_getUnits(SMO_Handle p_handle, int **int_out, int *int_dim);;
EXPORT_OUT_API int SMO_getFlowUnits(SMO_Handle p_handle, int *unitFlag);
EXPORT_OUT_API int SMO_getPollutantUnits(SMO_Handle p_handle, int **unitFlag, int *length);
EXPORT_OUT_API int SMO_getStartDate(SMO_Handle p_handle, double *date);
EXPORT_OUT_API int SMO_getTimes(SMO_Handle p_handle, SMO_time code, int *time);
EXPORT_OUT_API int SMO_getElementName(SMO_Handle p_handle, SMO_elementType type, int elementIndex, char **elementName, int *size);

EXPORT_OUT_API int SMO_getSubcatchSeries(SMO_Handle p_handle, int subcatchIndex, SMO_subcatchAttribute attr, int startPeriod, int endPeriod, float **float_out, int *int_dim);
EXPORT_OUT_API int SMO_getNodeSeries(SMO_Handle p_handle, int nodeIndex, SMO_nodeAttribute attr, int startPeriod, int endPeriod, float **float_out, int *int_dim);
EXPORT_OUT_API int SMO_getLinkSeries(SMO_Handle p_handle, int linkIndex, SMO_linkAttribute attr, int startPeriod, int endPeriod, float **float_out, int *int_dim);
EXPORT_OUT_API int SMO_getSystemSeries(SMO_Handle p_handle, SMO_systemAttribute attr, int startPeriod, int endPeriod, float **float_out, int *int_dim);

EXPORT_OUT_API int SMO_getSubcatchAttribute(SMO_Handle p_handle, int timeIndex, SMO_subcatchAttribute attr, float **float_out, int *int_dim);
EXPORT_OUT_API int SMO_getNodeAttribute(SMO_Handle p_handle, int timeIndex, SMO_nodeAttribute attr, float **float_out, int *int_dim);
EXPORT_OUT_API int SMO_getLinkAttribute(SMO_Handle p_handle, int timeIndex, SMO_linkAttribute attr, float **float_out, int *int_dim);
EXPORT_OUT_API int SMO_getSystemAttribute(SMO_Handle p_handle, int timeIndex, SMO_systemAttribute attr, float **float_out, int *int_dim);

EXPORT_OUT_API int SMO_getSubcatchResult(SMO_Handle p_handle, int timeIndex, int subcatchIndex, float **float_out, int *int_dim);
EXPORT_OUT_API int SMO_getNodeResult(SMO_Handle p_handle, int timeIndex, int nodeIndex, float **float_out, int *int_dim);
EXPORT_OUT_API int SMO_getLinkResult(SMO_Handle p_handle, int timeIndex, int linkIndex, float **float_out, int *int_dim);
EXPORT_OUT_API int SMO_getSystemResult(SMO_Handle p_handle, int timeIndex, int dummyIndex, float **float_out, int *int_dim);

EXPORT_OUT_API int SMO_getBufferSize(SMO_Handle p_handle, SMO_elementType type, int *bufferSize);
EXPORT_OUT_API int SMO_bufferSubcatchResult(SMO_Handle p_handle, int timeIndex, int subcatchIndex, float *buffer, int bufferSize);
EXPORT_OUT_API int SMO_bufferNodeResult(SMO_Handle p_handle, int periodIndex, int nodeIndex, float *buffer, int bufferSize);
EXPORT_OUT_API int SMO_bufferLinkResult(SMO_Handle p_handle, int timeIndex, int linkIndex, float *buffer, int bufferSize);
EXPORT_OUT_API int SMO_bufferSystemResult(SMO_Handle p_handle, int timeIndex, int dummyIndex, float *buffer, int bufferize);

EXPORT_OUT_API void SMO_freeMemory(void *array);
EXPORT_OUT_API void SMO_clearError(SMO_Handle p_handle_in);
EXPORT_OUT_API int SMO_checkError(SMO_Handle p_handle_in, char **msg_buffer);

#ifdef __cplusplus
}
#endif

#endif /* SWMM_OUTPUT_H_ */
