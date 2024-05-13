/*!
 * \file consts.h
 * \brief Constants used in the SWMM model
 * \author L. Rossman
 * \author Caleb Buahin (Last Editor)
 * \date 02/12/2023 (Build 5.2.3)
 * \date 05/28/2023 (Last Updated)
 * \version 5.2
 */

#ifndef CONSTS_H
#define CONSTS_H

/*!
 * \defgroup general_constants General Constants
 * \brief General constants used in the SWMM model
 * @{
 */

/*!
 * \def VERSION
 * \brief model version number
 */
#define VERSION 52003

/*!
 * \def MAGICNUMBER
 * \brief magic number used to validate a SWMM output file
 */
#define MAGICNUMBER 516114522

/*!
 * \def EOFMARK
 * \brief end of file mark.  Use 0x04 for UNIX systems
 */
#define EOFMARK 0x1A

/*!
 * \def MAXTITLE
 * \brief maximum number of title lines
 */
#define MAXTITLE 3

/*!
 * \def MAXMSG
 * \brief maximum number of characters in message text
 */
#define MAXMSG 1024

/*!
 * \def MAXLINE
 * \brief maximum number of characters per input line
 */
#define MAXLINE 1024

/*!
 * \def MAXFNAME
 * \brief maximum number of characters in file name
 */
#define MAXFNAME 259

/*!
 * \def MAXTOKS
 * \brief maximum number of items per input line
 */
#define MAXTOKS 40

/*!
 * \def MAXSTATES
 * \brief maximum number of computed hydraulic variables
 */
#define MAXSTATES 10

/*!
 * \def MAXODES
 * \brief maximum number of ODEs to be solved
 */
#define MAXODES 4

/*!
 * \def NA
 * \brief NOT APPLICABLE code
 */
#define NA -1

/*!
 * \def TRUE
 * \brief Value for TRUE state
 */
#define TRUE 1

/*!
 * \def FALSE
 * \brief Value for FALSE state
 */
#define FALSE 0

/*!
 * \def BIG
 * \brief Generic large value
 */
#define BIG 1.E10

/*!
 * \def TINY
 * \brief Generic small value
 */
#define TINY 1.E-6

/*!
 * \def ZERO
 * \brief Effective zero value
 */
#define ZERO 1.E-10

/*!
 * \def MISSING
 * \brief Missing value code
 */
#define MISSING -1.E10

/*!
 * \def PI
 * \brief Value of pi
 */
#define PI 3.141592654

/*!
 * \def GRAVITY
 * \brief accel. of gravity in US units
 */
#define GRAVITY 32.2

/*!
 * \def SI_GRAVITY
 * \brief accel. of gravity in SI units
 */
#define SI_GRAVITY 9.81

/*!
 * \def MAXFILESIZE
 * \brief largest file size in bytes
 * \deprecated This constant is no longer used in the SWMM source code.
 */
#define MAXFILESIZE 2147483647L // largest file size in bytes

/*!
 * @}
 */

/*!
 * \defgroup manning_factors Mannings Units Conversion Factor
 * \brief Conversion factor used in the Manning Equation
 * @{
 */
/*!
 * \def PHI
 * \brief Conversion factor used in the Manning Equation
 */
#define PHI 1.486
/*!
 * @}
 */

/*!
 * \defgroup measurable_runoff_depth Measurable Runoff Flow & Depth
 * \brief Minimum measurable runoff flow & depth
 * @{
 */
/*!
 * \def MIN_RUNOFF_FLOW
 * \brief Minimum measurable runoff flow
 */
#define MIN_RUNOFF_FLOW 0.001

/*!
 * \def MIN_EXCESS_DEPTH
 * \brief Minimum measurable excess depth. ft, = 0.03 mm  <NOT USED>
 */
#define MIN_EXCESS_DEPTH 0.0001

/*!
 * \def MIN_TOTAL_DEPTH
 * \brief Minimum measurable total depth. ft, = 0.05 inches
 */
#define MIN_TOTAL_DEPTH 0.004167

/*!
 * \def MIN_RUNOFF
 * \brief Minimum measurable runoff depth. ft, = 0.001 inches
 */
#define MIN_RUNOFF 2.31481e-8
/*!
 * @}
 */

/*!
 * \defgroup min_flow_depth_volume Minimum Flow, Depth & Volume
 * \brief Minimum flow, depth & volume used to evaluate steady state conditions
 * @{
 */
/*!
 * \def FLOW_TOL
 * \brief Minimum flow used to evaluate steady state conditions. cfs <NOT USED>
 */
#define FLOW_TOL 0.00001

/*!
 * \def DEPTH_TOL
 * \brief Minimum depth used to evaluate steady state conditions. ft <NOT USED>
 */
#define DEPTH_TOL 0.00001

/*!
 * \def VOLUME_TOL
 * \brief Minimum volume used to evaluate steady state conditions. ft3 <NOT
 * USED>
 */
#define VOLUME_TOL 0.01
/*!
 * @}
 */

//---------------------------------------------------
// Minimum depth for reporting non-zero water quality
//---------------------------------------------------
// #define   MIN_WQ_DEPTH  0.01     // ft (= 3 mm)
// #define   MIN_WQ_FLOW   0.001    // cfs

/*!
 * \defgroup min_flow_area_dynwave Minimum Flow Depth & Area for Dynamic Wave
 * Routing \brief Minimum flow depth & area for dynamic wave routing
 * @{
 */

/*!
 * \def FUDGE
 * \brief Fudge factor used to evaluate steady state conditions. ft or ft2
 */
#define FUDGE 0.0001 // ft or ft2
/*!
 * @}
 */

/*!
 * \defgroup conversion_factors Conversion Factors
 * \brief Various conversion factors
 * @{
 */

/*!
 * \def GPMperCFS
 * \brief Conversion factor for gallons per minute to cubic feet per second
 */
#define GPMperCFS 448.831

/*!
 * \def AFDperCFS
 * \brief Conversion factor for acre-feet per day to cubic feet per second
 */
#define AFDperCFS 1.9837

/*!
 * \def MGDperCFS
 * \brief Conversion factor for million gallons per day to cubic feet per second
 */
#define MGDperCFS 0.64632

/*!
 * \def IMGDperCFS
 * \brief Conversion factor for million gallons per day to cubic feet per second
 */
#define IMGDperCFS 0.5382

/*!
 * \def CMSperCFS
 * \brief Conversion factor for cubic meters per second to cubic feet per second
 */
#define LPSperCFS 28.317

/*!
 * \def LPMperCFS
 * \brief Conversion factor for liters per minute to cubic feet per second
 */
#define LPMperCFS 1699.0

/*!
 * \def CMHperCFS
 * \brief Conversion factor for cubic meters per hour to cubic feet per second
 */
#define CMHperCFS 101.94

/*!
 * \def CMDperCFS
 * \brief Conversion factor for cubic meters per day to cubic feet per second
 */
#define CMDperCFS 2446.6

/*!
 * \def MLDperCFS
 * \brief Conversion factor for megaliters per day to cubic feet per second
 */
#define MLDperCFS 2.4466

/*!
 * \def M3perFT3
 * \brief Conversion factor for cubic meters to cubic feet
 */
#define M3perFT3 0.028317

/*!
 * \def LperFT3
 * \brief Conversion factor for liters to cubic feet
 */
#define LperFT3 28.317

/*!
 * \def MperFT
 * \brief Conversion factor for meters to feet
 */
#define MperFT 0.3048

/*!
 * \def PSIperFT
 * \brief Conversion factor for pounds per square inch to feet
 */
#define PSIperFT 0.4333

/*!
 * \def KPAperPSI
 * \brief Conversion factor for kilopascals to pounds per square inch
 */
#define KPAperPSI 6.895

/*!
 * \def KWperHP
 * \brief Conversion factor for kilowatts to horsepower
 */
#define KWperHP 0.7457

/*!
 * \def SECperDAY
 * \brief Conversion factor for seconds to days
 */
#define SECperDAY 86400

/*!
 * \def MSECperDAY
 * \brief Conversion factor for milliseconds to days
 */
#define MSECperDAY 8.64e7

/*!
 * \def MMperINCH
 * \brief Conversion factor for millimeters to inches
 */
#define MMperINCH 25.40

/*!
 * @}
 */

/*!
 * \defgroup token_seperators Token Separators
 * \brief Token separator characters
 * @{
 */
/*!
 * \def SEPSTR
 * \brief Token separator characters
 */
#define SEPSTR " \t\n\r"
/*!
 * @}
 */

#endif // CONSTS_H
