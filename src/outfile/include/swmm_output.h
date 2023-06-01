 /*!
  *  \file swmm_output.h
  *  \brief Header file for SWMM output API
  *  \author Colleen Barr (US EPA - ORD/NHEERL)
  *  \author Michael Tryby (US EPA - ORD/CESER) (Editor)
  *	 \author Bryant McDonnell (EmNet LLC) (Editor)
  *  \author Caleb Buahin (US EPA - ORD/CESER) (Last Editor)
  *  \date Created On: 10/18/2019
  *  \date Last Edited: 05/26/2023
  *  \remarks
  *  \see
  *  \bug
  *  \warning
  *  \todo
  *  \note
  */
#ifndef SWMM_OUTPUT_H_
#define SWMM_OUTPUT_H_

#ifdef WINDOWS
/*!
* \def EXPORT_OUT_API 
* \brief Cross-platform DLL export/import specifier
*/
#define EXPORT_OUT_API __declspec(dllexport) __stdcall
#else
#define EXPORT_OUT_API
#endif


/*!
* \def MAXFILENAME
* \brief Maximum number of characters in file path
*/
#define MAXFILENAME 259    // Max characters in file path1

/*!
* \def MAXELENAME
* \brief Maximum number of characters in element name
*/
#define MAXELENAME 31      // Max characters in element name

/*!
* \typedef SMO_Handle
* \brief SWMM output handle. This is an opaque pointer to struct. Do not access variables.
*/
typedef void *SMO_Handle;


#include "swmm_output_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
* \brief Initializes output file handle to NULL
* \param[out] p_handle Pointer to a SMO_Handle
* \return Error code
* \see smo_close
*/
int EXPORT_OUT_API SMO_init(SMO_Handle *p_handle);

/*!
* \brief Closes a SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \return Error code
* \see smo_init
* \see smo_open
*/
int EXPORT_OUT_API SMO_close(SMO_Handle *p_handle);

/*!
* \brief Opens a SWMM output file
* \param[out] p_handle Pointer to a SMO_Handle
* \param[in] path Path to the SWMM output file
* \return Error code
* \see smo_close
*/
int EXPORT_OUT_API SMO_open(SMO_Handle p_handle, const char *path);

/*!
* \brief Gets SWMM output file version
* \param[in] p_handle Pointer to a SMO_Handle
* \param[out] version Version number
* \return Error code
*/
int EXPORT_OUT_API SMO_getVersion(SMO_Handle p_handle, int *version);

/*!
* \brief Gets number of elements in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[out] elementCount array to store number of elements
* \param[out] length Number of elements
* \return Error code
*/
int EXPORT_OUT_API SMO_getProjectSize(SMO_Handle p_handle, int **elementCount, int *length);

/*!
* \brief Gets units used in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[out] unitFlag array to store units
* \param[out] length Number of units
* \return Error code
*/
int EXPORT_OUT_API SMO_getUnits(SMO_Handle p_handle, int **unitFlag, int *length);

/*!
* \brief Gets flow units used in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[out] unitFlag Flow units
* \return Error code
*/
int EXPORT_OUT_API SMO_getFlowUnits(SMO_Handle p_handle, int *unitFlag);

/*!
* \brief Gets pollutant units used in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[out] unitFlag array to store pollutant units
* \param[out] length Number of pollutant units
* \return Error code
*/
int EXPORT_OUT_API SMO_getPollutantUnits(SMO_Handle p_handle, int **unitFlag, int *length);

/*!
* \brief Gets start date of SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[out] date Start date
* \return Error code
*/
int EXPORT_OUT_API SMO_getStartDate(SMO_Handle p_handle, double *date);

/*!
* \brief Gets Times used in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] code Time code
* \param[out] time Time
* \return Error code
*/
int EXPORT_OUT_API SMO_getTimes(SMO_Handle p_handle, SMO_time code, int *time);

/*!
* \brief Gets element name used in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] type Element type
* \param[in] elementIndex Element index
* \param[out] elementName Element name
* \param[out] size Size of element name
* \return Error code
*/
int EXPORT_OUT_API SMO_getElementName(SMO_Handle p_handle, SMO_elementType type, int elementIndex, char **elementName, int *size);

/*!
* \brief Gets time series for subcatchment in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] subcatchIndex Subcatchment index
* \param[in] attr Subcatchment attribute
* \param[in] startPeriod Start period
* \param[in] endPeriod End period
* \param[out] outValueArray Array to store time series
* \param[out] length Length of time series
* \return Error code
*/
int EXPORT_OUT_API SMO_getSubcatchSeries(SMO_Handle p_handle, int subcatchIndex, SMO_subcatchAttribute attr, int startPeriod, int endPeriod, float **outValueArray, int *length);

/*!
* \brief Gets time series for node in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] nodeIndex Node index
* \param[in] attr Node attribute
* \param[in] startPeriod Start period
* \param[in] endPeriod End period
* \param[out] outValueArray Array to store time series
* \param[out] length Length of time series
* \return Error code
*/
int EXPORT_OUT_API SMO_getNodeSeries(SMO_Handle p_handle, int nodeIndex, SMO_nodeAttribute attr, int startPeriod, int endPeriod, float **outValueArray, int *length);

/*!
* \brief Gets time series for link in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] linkIndex Link index
* \param[in] attr Link attribute
* \param[in] startPeriod Start period
* \param[in] endPeriod End period
* \param[out] outValueArray Array to store time series
* \param[out] length Length of time series
* \return Error code
*/
int EXPORT_OUT_API SMO_getLinkSeries(SMO_Handle p_handle, int linkIndex, SMO_linkAttribute attr, int startPeriod, int endPeriod, float **outValueArray, int *length);

/*!
* \brief Gets time series for system in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] attr System attribute
* \param[in] startPeriod Start period
* \param[in] endPeriod End period
* \param[out] outValueArray Array to store time series
* \param[out] length Length of time series
* \return Error code
*/
int EXPORT_OUT_API SMO_getSystemSeries(SMO_Handle p_handle, SMO_systemAttribute attr, int startPeriod, int endPeriod, float **outValueArray, int *length);

/*!
* \brief Gets attribute for subcatchment in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] timeIndex Time index
* \param[in] attr Subcatchment attribute
* \param[out] outValueArray Array to store attribute
* \param[out] length Length of attribute
* \return Error code
*/
int EXPORT_OUT_API SMO_getSubcatchAttribute(SMO_Handle p_handle, int timeIndex, SMO_subcatchAttribute attr, float **outValueArray, int *length);

/*!
* \brief Gets attribute for node in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] timeIndex Time index
* \param[in] attr Node attribute
* \param[out] outValueArray Array to store attribute
* \param[out] length Length of attribute
* \return Error code
*/
int EXPORT_OUT_API SMO_getNodeAttribute(SMO_Handle p_handle, int timeIndex, SMO_nodeAttribute attr, float **outValueArray, int *length);

/*!
* \brief Gets attribute for link in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] timeIndex Time index
* \param[in] attr Link attribute
* \param[out] outValueArray Array to store attribute
* \param[out] length Length of attribute
* \return Error code
*/
int EXPORT_OUT_API SMO_getLinkAttribute(SMO_Handle p_handle, int timeIndex, SMO_linkAttribute attr, float **outValueArray, int *length);

/*!
* \brief Gets attribute for system in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] timeIndex Time index
* \param[in] attr System attribute
* \param[out] outValueArray Array to store attribute
* \param[out] length Length of attribute
* \return Error code
*/
int EXPORT_OUT_API SMO_getSystemAttribute(SMO_Handle p_handle, int timeIndex, SMO_systemAttribute attr, float **outValueArray, int *length);

/*!
* \brief Gets result for subcatchment in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] timeIndex Time index
* \param[in] subcatchIndex Subcatchment index
* \param[out] outValueArray Array to store result
* \param[out] length Length of result
* \return Error code
*/
int EXPORT_OUT_API SMO_getSubcatchResult(SMO_Handle p_handle, int timeIndex, int subcatchIndex, float **outValueArray, int *length);

/*!
* \brief Gets result for node in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] timeIndex Time index
* \param[in] nodeIndex Node index
* \param[out] outValueArray Array to store result
* \param[out] length Length of result
* \return Error code
*/
int EXPORT_OUT_API SMO_getNodeResult(SMO_Handle p_handle, int timeIndex, int nodeIndex, float **outValueArray, int *length);

/*!
* \brief Gets result for link in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] timeIndex Time index
* \param[in] linkIndex Link index
* \param[out] outValueArray Array to store result
* \param[out] length Length of result
* \return Error code
*/
int EXPORT_OUT_API SMO_getLinkResult(SMO_Handle p_handle, int timeIndex, int linkIndex, float **outValueArray, int *length);

/*!
* \brief Gets result for system in SWMM output file
* \param[in] p_handle Pointer to a SMO_Handle
* \param[in] timeIndex Time index
* \param[in] dummyIndex Dummy index not used. Included for consistency with other get functions and for automatic generation of wrappers.
* \param[out] outValueArray Array to store result
* \param[out] length Length of result
* \return Error code
*/
int EXPORT_OUT_API SMO_getSystemResult(SMO_Handle p_handle, int timeIndex, int dummyIndex, float **outValueArray, int *length);

/*!
* \brief frees array allocated by SMO_getSubcatchSeries, SMO_getNodeSeries, SMO_getLinkSeries, SMO_getSystemSeries, SMO_getSubcatchAttribute, SMO_getNodeAttribute, SMO_getLinkAttribute, SMO_getSystemAttribute, SMO_getSubcatchResult, SMO_getNodeResult, SMO_getLinkResult, SMO_getSystemResult
* \param[in] array Array to be freed
* \todo Consider using a macro instead of a function
*/
void EXPORT_OUT_API SMO_free(void **array);

/*!
* \brief clears current error message
* \param[in] p_handle Pointer to a SMO_Handle
*/
void EXPORT_OUT_API SMO_clearError(SMO_Handle p_handle_in);

/*!
* \brief gets current error message
* \param[in] p_handle Pointer to a SMO_Handle
* \param[out] msg_buffer Buffer to store error message
* \return Error code
*/
int EXPORT_OUT_API SMO_checkError(SMO_Handle p_handle_in, char **msg_buffer);

#ifdef __cplusplus
}
#endif

#endif /* SWMM_OUTPUT_H_ */
