/*!
 * \file consts.h
 * \brief Header file for SWMM constants.
 * \author L. Rossman
 * \date Created: 2023-07-15
 * \date Last updated: 2024-12-30
 * \version 5.3.0
 * \details
 * Various constants used in the SWMM model.
 *
 * Update history
 * ==============
 * - Build 5.3.0 (2024-12-30)
 *   - Added MAXHOTSTARTFILES constant to support saving multiple hotstart files
 *     at different times.
 */
#ifndef CONSTS_H
#define CONSTS_H

#include "version.h"

/*!
 * \defgroup SWMM_Constants SWMM Constants, Variables, and Local and API Functions
 * \brief Constants, variables, local, and API functions used in the SWMM model.
 * \{
 */

/*!
 * \addtogroup SWMM_General_Constants SWMM General Constants
 * \brief General constants used in the SWMM model.
 * \ingroup SWMM_Constants
 * \{
 */

/*!
 * \def VERSION
 * \brief SWMM version number
 */
#define VERSION PROJECT_VERSION_SWMM_FORMAT

/*!
 * \def MAGICNUMBER
 * \brief Magic number used to identify a SWMM binary file
 */
#define MAGICNUMBER 516114522

/*!
 * \def EOFMARK
 * \brief End of file mark used in SWMM binary files
 * \note Use 0x04 for UNIX systems
 */
#define EOFMARK 0x1A // Use 0x04 for UNIX systems

/*!
 * \def MAXTITLE
 * \brief Maximum number of title lines
 */
#define MAXTITLE 3 // Max. # title lines

/*!
 * \def MAXMSG
 * \brief Maximum number of characters in a message
 */
#define MAXMSG 1024 // Max. # characters in message text

/*!
 * \def MAXLINE
 * \brief Maximum number of characters per input line
 */
#define MAXLINE 1024 // Max. # characters per input line

/*!
 * \def MAXFNAME
 * \brief Maximum number of characters in a file name
 */
#define MAXFNAME 259 // Max. # characters in file name

/*!
 * \def MAXTOKS
 * \brief Maximum number of items per line of input
 */
#define MAXTOKS 40 // Max. items per line of input

/*!
 * \def MAXSTATES
 * \brief Maximum number of computed hydraulic variables
 */
#define MAXSTATES 10 // Max. # computed hyd. variables

/*!
 * \def MAXODES
 * \brief Maximum number of ODE's to be solved
 */
#define MAXODES 4 // Max. # ODE's to be solved

/*!
 * \def NA
 * \brief NOT APPLICABLE code
 */
#define NA -1 // NOT APPLICABLE code

/*!
 * \def TRUE
 * \brief Value for TRUE state
 */
#define TRUE 1 // Value for TRUE state

/*!
 * \def FALSE
 * \brief Value for FALSE state
 */
#define FALSE 0 // Value for FALSE state

/*!
 * \def BIG
 * \brief Generic large value
 */
#define BIG 1.E10 // Generic large value

/*!
 * \def TINY
 * \brief Generic small value
 */
#define TINY 1.E-6 // Generic small value

/*!
 * \def ZERO
 * \brief Effective zero value
 */
#define ZERO 1.E-10 // Effective zero value

/*!
 * \def MISSING
 * \brief Missing value code
 */
#define MISSING -1.E10 // Missing value code

/*!
 * \def PI
 * \brief Value of pi
 */
#define PI 3.141592654 // Value of pi

/*!
 * \def GRAVITY
 * \brief Acceleration of gravity in US units
 */
#define GRAVITY 32.2 // accel. of gravity in US units

/*!
 * \def SI_GRAVITY
 * \brief Acceleration of gravity in SI units
 */
#define SI_GRAVITY 9.81 // accel of gravity in SI units
/* DEPRECATED
#define   MAXFILESIZE        2147483647L    // largest file size in bytes
*/

/*!
 * \def MAXHOTSTARTFILES
 * \brief Maximum number of hotstart files
 */
#define MAXHOTSTARTFILES 10 // largest file size in bytes

/*!
 * \def MAXTIMESERIESCACHESIZE
 * \brief Maximum number of time series rows that can be cached to memory to speed up simulation
 * \TODO Allow users to set this value in the input file and GUI
 */
#define MAXTIMESERIESCACHESIZE 10 // maximum number of time series that can be cached

/*!
 * \}
 */

/*!
 * \addtogroup Mannings_Unit_Factor Units factor in Manning Equation
 * \brief Units factor in Manning Equation
 * \ingroup SWMM_Constants
 * \{
 */

/*!
 * \def PHI
 * \brief Units factor in Manning Equation
 */
#define PHI 1.486
/*!
 * \}
 */

/*!
 * \addtogroup Definitions_Measurable_Runoff_Flow_Depth Definition of Measureable Runoff, Flow, and Depth
 * \brief Definition of measureable runoff, flow, and depth
 * \ingroup SWMM_Constants
 * \{
 */

/*!
 * \def MIN_RUNOFF_FLOW
 * \brief Minimum runoff flow (cfs)
 */
#define MIN_RUNOFF_FLOW 0.001 // cfs

/*!
 * \def MIN_EXCESS_DEPTH
 * \brief Minimum excess depth (ft) = 0.03 mm
 * \note Currently not used
 */
#define MIN_EXCESS_DEPTH 0.0001 // ft, = 0.03 mm  <NOT USED>

/*!
 * \def MIN_TOTAL_DEPTH
 * \brief Minimum total depth (ft). = 0.05 inches
 */
#define MIN_TOTAL_DEPTH 0.004167 // ft, = 0.05 inches

/*!
 * \def MIN_RUNOFF
 * \brief Minimum runoff (ft/sec) = 0.001 in/hr
 */
#define MIN_RUNOFF 2.31481e-8 // ft/sec = 0.001 in/hr
/*!
 * \}
 */

/*!
 * \addtogroup Min_Flow_Depth_and_Vol Minimum Flow, Depth, and Volume Used to Evaluate Steady State Conditions
 * \brief Minimum flow, depth, and volume used to evaluate steady state conditions
 * \ingroup SWMM_Constants
 * \{
 */
/*!
 * \def FLOW_TOL
 * \brief Flow tolerance (cfs)
 */
#define FLOW_TOL 0.00001 // cfs

/*!
 * \def DEPTH_TOL
 * \brief Depth tolerance (ft)
 * \note Currently not used
 */
#define DEPTH_TOL 0.00001 // ft    <NOT USED>

/*!
 * \def VOLUME_TOL
 * \brief Volume tolerance (ft3)
 * \note Currently not used
 */
#define VOLUME_TOL 0.01 // ft3   <NOT USED>
/*!
 * \}
 */

//---------------------------------------------------
// Minimum depth for reporting non-zero water quality
//---------------------------------------------------
// #define   MIN_WQ_DEPTH  0.01     // ft (= 3 mm)
// #define   MIN_WQ_FLOW   0.001    // cfs

/*!
 * \addtogroup Fudge Minimum Flow Depth and Area for Dynamic Wave Routing
 * \brief Minimum Flow Depth and Area for Dynamic Wave Routing
 * \ingroup SWMM_Constants
 * \{
 */
/*!
 * \def FUDGE
 * \brief Fudge factor (ft or ft2)
 */
#define FUDGE 0.0001 // ft or ft2
/*!
 * \}
 */

/*!
 * \addtogroup Conversion_Factors Various Conversion Factors
 * \brief Various Conversion Factors
 * \ingroup SWMM_Constants
 * \{
 */
/*!
 * \def GPMperCFS
 * \brief Gallons per minute per cfs
 */
#define GPMperCFS 448.831

/*!
 * \def AFDperCFS
 * \brief Acre-feet per day per cfs
 */
#define AFDperCFS 1.9837

/*!
 * \def MGDperCFS
 * \brief Million gallons per day per cfs
 */
#define MGDperCFS 0.64632

/*!
 * \def IMGDperCFS
 * \brief Imperial million gallons per day per cfs
 */
#define IMGDperCFS 0.5382

/*!
 * \def LPSperCFS
 * \brief Liters per second per cfs
 */
#define LPSperCFS 28.317

/*!
 * \def LPMperCFS
 * \brief Liters per minute per cfs
 */
#define LPMperCFS 1699.0

/*!
 * \def CMHperCFS
 * \brief Cubic meters per hour per cfs
 */
#define CMHperCFS 101.94

/*!
 * \def CMDperCFS
 * \brief Cubic meters per day per cfs
 */
#define CMDperCFS 2446.6

/*!
 * \def MLDperCFS
 * \brief Million liters per day per cfs
 */
#define MLDperCFS 2.4466

/*!
 * \def M3perFT3
 * \brief Cubic meters per ft3
 */
#define M3perFT3 0.028317

/*!
 * \def LperFT3
 * \brief Liters per ft3
 */
#define LperFT3 28.317

/*!
 * \def MperFT
 * \brief Meters per ft
 */
#define MperFT 0.3048

/*!
 * \def PSIperFT
 * \brief Pounds per square inch per ft
 */
#define PSIperFT 0.4333

/*!
 * \def KPAperPSI
 * \brief Kilopascals per psi
 */
#define KPAperPSI 6.895

/*!
 * \def KWperHP
 * \brief Kilowatts per horsepower
 */
#define KWperHP 0.7457

/*!
 * \def SECperDAY
 * \brief Seconds per day
 */
#define SECperDAY 86400

/*!
 * \def MSECperDAY
 * \brief Milliseconds per day
 */
#define MSECperDAY 8.64e7

/*!
 * \def MMperINCH
 * \brief Millimeters per inch
 */
#define MMperINCH 25.40
/*!
 * \}
 */

/*!
 * \addtogroup Token_Separators Token Separator Characters
 * \brief Token separator characters
 * \ingroup SWMM_Constants
 * \{
 */

/*!
 * \def SEPSTR
 * \brief Separator string
 */
#define SEPSTR " \t\n\r"

/*!
 * \addtogroup Token_Separators Table Separator Characters
 * \brief Table separator characters
 */
#define TBLSEPSTR " \t\n\r,"

/*!
 * \}
 */

/*!
 * \}
 */

#endif // CONSTS_H
