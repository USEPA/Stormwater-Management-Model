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
typedef void *SMO_Handle;


#include "swmm_output_enums.h"
#include "swmm_output_export.h"

#ifdef __cplusplus
extern "C" {
#endif

int EXPORT_OUT_API SMO_init(SMO_Handle *p_handle);
int EXPORT_OUT_API SMO_close(SMO_Handle *p_handle);
int EXPORT_OUT_API SMO_open(SMO_Handle p_handle, const char *path);
int EXPORT_OUT_API SMO_getVersion(SMO_Handle p_handle, int *version);
int EXPORT_OUT_API SMO_getProjectSize(SMO_Handle p_handle, int **elementCount, int *length);

int EXPORT_OUT_API SMO_getUnits(SMO_Handle p_handle, int **unitFlag, int *length);
int EXPORT_OUT_API SMO_getFlowUnits(SMO_Handle p_handle, int *unitFlag);
int EXPORT_OUT_API SMO_getPollutantUnits(SMO_Handle p_handle, int **unitFlag, int *length);
int EXPORT_OUT_API SMO_getStartDate(SMO_Handle p_handle, double *date);
int EXPORT_OUT_API SMO_getTimes(SMO_Handle p_handle, SMO_time code, int *time);
int EXPORT_OUT_API SMO_getElementName(SMO_Handle p_handle, SMO_elementType type, int elementIndex, char **elementName, int *size);

int EXPORT_OUT_API SMO_getSubcatchSeries(SMO_Handle p_handle, int subcatchIndex, SMO_subcatchAttribute attr, int startPeriod, int endPeriod, float **outValueArray, int *length);
int EXPORT_OUT_API SMO_getNodeSeries(SMO_Handle p_handle, int nodeIndex, SMO_nodeAttribute attr, int startPeriod, int endPeriod, float **outValueArray, int *length);
int EXPORT_OUT_API SMO_getLinkSeries(SMO_Handle p_handle, int linkIndex, SMO_linkAttribute attr, int startPeriod, int endPeriod, float **outValueArray, int *length);
int EXPORT_OUT_API SMO_getSystemSeries(SMO_Handle p_handle, SMO_systemAttribute attr, int startPeriod, int endPeriod, float **outValueArray, int *length);

int EXPORT_OUT_API SMO_getSubcatchAttribute(SMO_Handle p_handle, int timeIndex, SMO_subcatchAttribute attr, float **outValueArray, int *length);
int EXPORT_OUT_API SMO_getNodeAttribute(SMO_Handle p_handle, int timeIndex, SMO_nodeAttribute attr, float **outValueArray, int *length);
int EXPORT_OUT_API SMO_getLinkAttribute(SMO_Handle p_handle, int timeIndex, SMO_linkAttribute attr, float **outValueArray, int *length);
int EXPORT_OUT_API SMO_getSystemAttribute(SMO_Handle p_handle, int timeIndex, SMO_systemAttribute attr, float **outValueArray, int *length);

int EXPORT_OUT_API SMO_getSubcatchResult(SMO_Handle p_handle, int timeIndex, int subcatchIndex, float **outValueArray, int *length);
int EXPORT_OUT_API SMO_getNodeResult(SMO_Handle p_handle, int timeIndex, int nodeIndex, float **outValueArray, int *length);
int EXPORT_OUT_API SMO_getLinkResult(SMO_Handle p_handle, int timeIndex, int linkIndex, float **outValueArray, int *length);
int EXPORT_OUT_API SMO_getSystemResult(SMO_Handle p_handle, int timeIndex, int dummyIndex, float **outValueArray, int *length);

void EXPORT_OUT_API SMO_free(void **array);
void EXPORT_OUT_API SMO_clearError(SMO_Handle p_handle_in);
int EXPORT_OUT_API SMO_checkError(SMO_Handle p_handle_in, char **msg_buffer);

#ifdef __cplusplus
}
#endif

#endif /* SWMM_OUTPUT_H_ */
