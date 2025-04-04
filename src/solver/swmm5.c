/*!
 * \file swmm5.c
 * \brief Main module of the computational engine for Version 5 of the
 * U.S. Environmental Protection Agency's Storm Water Management Model (SWMM).
 * \author L. Rossman
 * \date Created: 2021-11-01
 * \date Last edited: 2024-12-23
 * \version 5.3
 * \details This is the main module of the computational engine for Version 5 of
 * the U.S. Environmental Protection Agency's Storm Water Management Model
 * (SWMM). It contains functions that control the flow of computations.
 *
 * This engine should be compiled into a shared object library whose API
 * functions are listed in swmm5.h.
 *
 * Update History
 * ==============
 *  - Build 5.1.008:
 *      - Support added for the MinGW compiler.
 *      - Reporting of project options moved to swmm_start.
 *      - Hot start file now read before routing system opened.
 *      - Final routing step adjusted so that total duration not exceeded.
 *  - Build 5.1.011:
 *      - Made sure that MS exception handling only used with MS C compiler.
 *      - Added name of module handling an exception to error report.
 *      - Elapsed simulation time now saved to new global variable ElaspedTime.
 *      - Added swmm_getError() function that retrieves error code and message.
 *      - Changed WarningCode to Warnings (# warnings issued).
 *      - Added swmm_getWarnings() function to retrieve value of Warnings.
 *      - Fixed error code returned on swmm_xxx functions.
 *  - Build 5.1.012:
 *      - #include <direct.h> only used when compiled for Windows.
 *  - Build 5.1.013:
 *      - Support added for saving average results within a reporting period.
 *      - SWMM engine now always compiled to a shared object library.
 *  - Build 5.1.015:
 *      - Fixes bug in summary statistics when Report Start date > Start Date.
 *  - Build 5.2.0:
 *      - Added additional API functions.
 *      - Set max. number of open files to 8192.
 *      - Changed getElapsedTime function to use report start as base date/time.
 *      - Prevented possible infinite loop if swmm_step() called when ErrorCode > 0.
 *      - Prevented early exit from swmm_end() when ErrorCode > 0.
 *      - Support added for relative file names.
 *  - Build 5.3.0:
 *      - Added support for saving hot start files at specific times.
 *      - Expanded SWMM api to save and use prescribed hotstart files.
 *      - Expansions to the SWMM API to include attributes of more objects and water quality.
 */

/*!
 * \def _CRT_SECURE_NO_DEPRECATE
 * \brief Define to prevent deprecation warnings from MS Visual C++ compilers
 */
#define _CRT_SECURE_NO_DEPRECATE

#undef EXH // indicates if exception handling included
#ifdef OS_WINDOWS
#ifdef _MSC_VER
/*!
 * \def EXH
 * \brief Define for MS Windows exception handling
 */
#define EXH
#endif
#endif

// --- include Windows & exception handling headers
#ifdef OS_WINDOWS
#include <windows.h>
#include <direct.h>
#include <errno.h>
#else
#include <unistd.h>
#endif

#ifdef EXH
#include <excpt.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>

// Protect against lack of compiler support for OpenMP
#if defined(_OPENMP)
#include <omp.h>
#else
/*!
 * \brief Dummy function for getting threads available for OpenMP to prevent compiler errors.
 * \return 1
 */
int omp_get_max_threads(void) { return 1; }
#endif

//-----------------------------------------------------------------------------
//  SWMM's header files
//
//  Note: the directives listed below are also contained in headers.h which
//        is included at the start of most of SWMM's other code modules.
//-----------------------------------------------------------------------------
#include "macros.h"  // macros used throughout SWMM
#include "objects.h" // definitions of SWMM's data objects

/*!
 * \def EXTERN
 * \brief Define for 'extern' in headers.h
 */
#define EXTERN // defined as 'extern' in headers.h

#include "globals.h" // declaration of all global variables
#include "funcs.h"   // declaration of all global functions
#include "error.h"   // error message codes
#include "text.h"    // listing of all text strings

#include "swmm5.h" // declaration of SWMM's API functions

/*!
 * \def MAX_EXCEPTIONS
 * \brief Maximum number of exceptions handled
 */
#define MAX_EXCEPTIONS 100

/*!
 * \addtogroup UnitConversionFactors Unit Conversion Factors
 * \brief Conversion factors for units used in SWMM.
 * \ingroup SWMM_Constants
 * \{
 */

/*!
 * \var Ucf
 * \brief Unit conversion factors for different units used in SWMM.
 */
const double Ucf[10][2] =
    {
        //  US      SI
        {43200.0, 1097280.0},    // RAINFALL (in/hr, mm/hr --> ft/sec)
        {12.0, 304.8},           // RAINDEPTH (in, mm --> ft)
        {1036800.0, 26334720.0}, // EVAPRATE (in/day, mm/day --> ft/sec)
        {1.0, 0.3048},           // LENGTH (ft, m --> ft)
        {2.2956e-5, 0.92903e-5}, // LANDAREA (ac, ha --> ft2)
        {1.0, 0.02832},          // VOLUME (ft3, m3 --> ft3)
        {1.0, 1.608},            // WINDSPEED (mph, km/hr --> mph)
        {1.0, 1.8},              // TEMPERATURE (deg F, deg C --> deg F)
        {2.203e-6, 1.0e-6},      // MASS (lb, kg --> mg)
        {43560.0, 3048.0}        // GWFLOW (cfs/ac, cms/ha --> ft/sec)
};

/*!
 * \var Qcf
 * \brief Flow conversion factors for different flow units used in SWMM.
 */
const double Qcf[6] =          // Flow Conversion Factors:
    {1.0, 448.831, 0.64632,    // cfs, gpm, mgd --> cfs
     0.02832, 28.317, 2.4466}; // cms, lps, mld --> cfs
/*!
 * \}
 */

/*!
 * \addtogroup SWMM_API_Shared_Variable SWMM Shared Variables
 * \brief SWMM API shared variables.
 * \ingroup SWMM_Constants
 * \{
 */

/*!
 * \var IsOpenFlag
 * \brief TRUE if a project has been opened.
 */
static int IsOpenFlag; // TRUE if a project has been opened

/*!
 * \var IsStartedFlag
 * \brief TRUE if a simulation has been started.
 */
static int IsStartedFlag; // TRUE if a simulation has been started

/*!
 * \var SaveResultsFlag
 * \brief TRUE if output to be saved to binary file.
 */
static int SaveResultsFlag; // TRUE if output to be saved to binary file

/*!
 * \var ExceptionCount
 * \brief Number of exceptions handled.
 */
static int ExceptionCount; // number of exceptions handled

/*!
 * \var DoRunoff
 * \brief TRUE if runoff is computed.
 */
static int DoRunoff; // TRUE if runoff is computed

/*!
 * \var DoRouting
 * \brief TRUE if flow routing is computed.
 */
static int DoRouting; // TRUE if flow routing is computed

/*!
 * \var RoutingDuration
 * \brief Duration of a set of routing steps (msecs).
 */
static double RoutingDuration; // duration of a set of routing steps (msecs)
/*!
 * \}
 */

/*!
 * \addtogroup SWMM_API_Local_Functions SWMM Local Functions
 * \brief Local functions used by the SWMM API functions.
 * \ingroup SWMM_Constants
 * \{
 */

/*!
 * \brief Routes flow and WQ through drainage system over a single time step.
 */
static void execRouting(void);

/*!
 * \brief Saves current results to binary output file.
 */
static void saveResults(void);

/*!
 * \brief Retrieve rain gage value given its index and property type.
 * \param[in] property Property type
 * \param[in] index Object index
 * \return Property value
 */
static double getGageValue(int property, int index);

/*!
 * \brief Retrieve subcatchment value given its index, property type, and subindex.
 * \param[in] property Property type
 * \param[in] index Object index
 * \param[in] subIndex Subindex
 * \return Property value
 */
static double getSubcatchValue(int property, int index, int subIndex);

/*!
 * \brief Retrieve node value given its index, property type, and subindex.
 * \param[in] property Property type
 * \param[in] index Object index
 * \param[in] subIndex Subindex
 * \return Property value
 */
static double getNodeValue(int property, int index, int subIndex);

/*!
 * \brief Retrieve link value given its index, property type, and subindex.
 * \param[in] property Property type
 * \param[in] index Object index
 * \param[in] subIndex Subindex
 * \return Property value
 */
static double getLinkValue(int property, int index, int subIndex);

/*!
 * \brief Retrieves the date/time of a reporting period.
 * \param[in] period reporting period (starting at 1)
 * \return Returns the date/time of the reporting period in decimal days
 */
static double getSavedDate(int period);

/*!
 * \brief Retrieve saved value of a subcatchment, node, or link given its index, property type, and period.
 * \param[in] property Property type
 * \param[in] index Object index
 * \param[in] period Reporting period (starting at 1)
 * \return Property value
 */
static double getSavedSubcatchValue(int property, int index, int period);

/*!
 * \brief Retrieve saved value of a node given its index, property type, and period.
 * \param[in] property Property type
 * \param[in] index Object index
 * \param[in] period Reporting period (starting at 1)
 * \return Property value
 */
static double getSavedNodeValue(int property, int index, int period);

/*!
 * \brief Retrieve saved value of a link given its index, property type, and period.
 * \param[in] property Property type
 * \param[in] index Object index
 * \param[in] period Reporting period (starting at 1)
 * \return Property value
 */
static double getSavedLinkValue(int property, int index, int period);

/*!
 * \brief Retrieve system value given its property type.
 * \param[in] property Property type
 * \return Property value
 */
static double getSystemValue(int property);

/*!
 * \brief Retrieve the maximum routing step.
 * \return Maximum routing step
 */
static double getMaxRouteStep();

/*!
 * \brief Set gage value given its property type, index, subindex, and value.
 * \param[in] property Property type
 * \param[in] index Object index
 * \param[in] subIndex Subindex
 * \param[in] value Property value
 * \return Error code
 */
static int setGageValue(int property, int index, int subIndex, double value);

/*!
 * \brief Set subcatchment value given its property type, index, subindex, and value.
 * \param[in] property Property type
 * \param[in] index Object index
 * \param[in] subIndex Subindex
 * \param[in] value Property value
 * \return Error code
 */
static int setSubcatchValue(int property, int index, int subIndex, double value);

/*!
 * \brief Set node value given its property type, index, subindex, and value.
 * \param[in] property Property type
 * \param[in] index Object index
 * \param[in] subIndex Subindex
 * \param[in] value Property value
 * \return Error code
 */
static int setNodeValue(int property, int index, int subIndex, double value);

/*!
 * \brief Set link value given its property type, index, subindex, and value.
 * \param[in] property Property type
 * \param[in] index Object index
 * \param[in] subIndex Subindex
 * \param[in] value Property value
 * \return Error code
 */
static int setLinkValue(int property, int index, int subIndex, double value);

/*!
 * \brief Set node lateral inflow value given its index and value.
 * \param[in] index Object index
 * \param[in] value Property value
 * \return Error code
 */
static int setNodeLatFlow(int index, double value);

/*!
 * \brief Set outfall stage value given its index and value.
 * \param[in] index Object index
 * \param[in] value Property value
 * \return Error code
 */
static int setOutfallStage(int index, double value);

/*!
 * \brief Set link setting value given its index and value.
 * \param[in] index Object index
 * \param[in] value Property value
 * \return Error code
 */
static int setLinkSetting(int index, double value);

/*!
 * \brief Set routing step value.
 * \param[in] value Property value
 * \return Error code
 */
static int setRoutingStep(double value);

/*!
 * \brief Set system value given its property type and value.
 * \param[in] property Property type
 * \param[in] value Property value
 * \return Error code
 */
static int setSystemValue(int property, double value);

/*!
 * \brief Determine if a file name contains a relative or absolute path.
 * \param[in] fname File name
 * \return 1 if fname's path is relative or 0 if absolute
 */
static int isRelativePath(const char *fname);

/*!
 * \brief Finds the full path of the directory for a file name.
 * \param[in] fname File name
 * \param[out] absPath Absolute path of fname (including ending path delimiter)
 * \param[in] size Size of the absPath array
 */
static void getAbsolutePath(const char *fname, char *absPath, size_t size);

/*!
 * \}
 */

#ifdef EXH

/*!
 * \brief Exception filtering routine for operating system exceptions
 * under Windows and the Microsoft C compiler.
 * \param[in] xc Exception code
 * \param[in] module Module name of code module where exception was handled
 * \param[in] elapsedTime Simulation time when exception occurred (days)
 * \param[in] step Step count at time when exception occurred
 * \return Returns an exception handling code
 */
static int xfilter(int xc, char *module, double elapsedTime, long step);

#endif

/*!
 * \copydoc swmm_run
 */
int EXPORT_SWMM_SOLVER_API swmm_run(const char *inputFile, const char *reportFile, const char *outputFile)
{
    long newHour, oldHour = 0;
    long theDay, theHour;
    double elapsedTime = 0.0;

    // --- initialize flags
    IsOpenFlag = FALSE;
    IsStartedFlag = FALSE;
    SaveResultsFlag = TRUE;

    // --- open the files & read input data
    ErrorCode = 0;
    writecon("\n o  Retrieving project data");
    swmm_open(inputFile, reportFile, outputFile);

    // --- run the simulation if input data OK
    if (!ErrorCode)
    {
        // --- initialize values
        swmm_start(TRUE);

        // --- execute each time step until elapsed time is re-set to 0
        if (!ErrorCode)
        {
            writecon("\n o  Simulating day: 0     hour:  0");
            do
            {
                swmm_step(&elapsedTime);
                newHour = (long)(elapsedTime * 24.0);
                if (newHour > oldHour)
                {
                    theDay = (long)elapsedTime;
                    theHour = (long)((elapsedTime - floor(elapsedTime)) * 24.0);
                    writecon("\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                    snprintf(Msg, MAXMSG, "%-5ld hour: %-2ld", theDay, theHour);
                    writecon(Msg);
                    oldHour = newHour;
                }
            } while (elapsedTime > 0.0 && !ErrorCode);
            writecon("\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
                     "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            writecon("Simulation complete           ");
        }

        // --- clean up
        swmm_end();
    }

    // --- report results
    if (!ErrorCode && Fout.mode == SCRATCH_FILE)
    {
        writecon("\n o  Writing output report");
        swmm_report();
    }

    // --- close the system
    swmm_close();
    return ErrorCode;
}

/*!
 * \copydoc swmm_run_with_callback
 */
int EXPORT_SWMM_SOLVER_API swmm_run_with_callback(const char *inputFile, const char *reportFile, const char *outputFile, progress_callback callback)
{
    double progress = 0.0, elapsedTime = 0.0;

    // --- initialize flags
    IsOpenFlag = FALSE;
    IsStartedFlag = FALSE;
    SaveResultsFlag = TRUE;

    // --- open the files & read input data
    ErrorCode = 0;

    swmm_open(inputFile, reportFile, outputFile);

    // --- run the simulation if input data OK
    if (!ErrorCode)
    {
        // --- initialize values
        swmm_start(TRUE);

        // --- execute each time step until elapsed time is re-set to 0
        if (!ErrorCode)
        {
            do
            {
                swmm_step(&elapsedTime);

                // --- calculate progress
                if (callback != NULL)
                {
                    progress = NewRoutingTime / TotalDuration;
                    callback(progress);
                }

            } while (elapsedTime > 0.0 && !ErrorCode);
        }

        // --- clean up
        swmm_end();
    }

    // --- report results
    if (!ErrorCode && Fout.mode == SCRATCH_FILE)
    {
        swmm_report();
    }

    // --- close the system
    swmm_close();

    return ErrorCode;
}

/*!
 * \copydoc swmm_open
 */
int EXPORT_SWMM_SOLVER_API swmm_open(const char *inputFile, const char *reportFile, const char *outputFile)
{
// --- to be safe, reset the state of the floating point unit
#ifdef OS_WINDOWS
    _fpreset();
    _setmaxstdio(8192);
#endif

#ifdef EXH
    // --- begin exception handling here
    __try
#endif
    {
        // --- initialize error & warning codes
        datetime_setDateFormat(M_D_Y);
        ErrorCode = 0;
        ErrorMsg[0] = '\0';
        Warnings = 0;
        IsOpenFlag = FALSE;
        IsStartedFlag = FALSE;
        ExceptionCount = 0;

        // --- open a SWMM project
        strcpy(InpDir, "");
        project_open(inputFile, reportFile, outputFile);
        getAbsolutePath(inputFile, InpDir, sizeof(InpDir));
        if (ErrorCode)
            return ErrorCode;
        IsOpenFlag = TRUE;
        report_writeLogo();

        // --- retrieve project data from input file
        project_readInput();
        if (ErrorCode)
            return ErrorCode;

        // --- write project title to report file & validate data
        report_writeTitle();
        project_validate();
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except (xfilter(GetExceptionCode(), "swmm_open", 0.0, 0))
    {
        ErrorCode = ERR_SYSTEM;
    }
#endif
    return ErrorCode;
}

/*!
 * \copydoc swmm_start
 */
int EXPORT_SWMM_SOLVER_API swmm_start(int saveFlag)
{
    // --- check that a project is open & no run started
    if (ErrorCode)
        return ErrorCode;
    if (!IsOpenFlag)
        return (ErrorCode = ERR_API_NOT_OPEN);
    if (IsStartedFlag)
        return (ErrorCode = ERR_API_NOT_ENDED);

    // --- write input summary & project options to report file if requested
    if (!RptFlags.disabled)
    {
        if (RptFlags.input)
            inputrpt_writeInput();
        report_writeOptions();
    }

    // --- save saveResults flag to global variable
    SaveResultsFlag = saveFlag;
    ExceptionCount = 0;

#ifdef EXH
    // --- begin exception handling loop here
    __try
#endif
    {
        // --- initialize elapsed time in decimal days
        ElapsedTime = 0.0;
        RoutingDuration = TotalDuration;

        // --- initialize runoff, routing & reporting time (in milliseconds)
        NewRunoffTime = 0.0;
        NewRoutingTime = 0.0;
        ReportTime = 1000 * (double)ReportStep;
        TotalStepCount = 0;
        ReportStepCount = 0;
        NonConvergeCount = 0;
        IsStartedFlag = TRUE;

        // --- initialize global continuity errors
        RunoffError = 0.0;
        GwaterError = 0.0;
        FlowError = 0.0;
        QualError = 0.0;

        // --- open rainfall processor (creates/opens a rainfall
        //     interface file and generates any RDII flows)
        if (!IgnoreRainfall)
            rain_open();
        if (ErrorCode)
            return ErrorCode;

        // --- initialize state of each major system component
        project_init();

        // --- see if runoff & routing needs to be computed
        if (Nobjects[SUBCATCH] > 0)
            DoRunoff = TRUE;
        else
            DoRunoff = FALSE;
        if (Nobjects[NODE] > 0 && !IgnoreRouting)
            DoRouting = TRUE;
        else
            DoRouting = FALSE;

        // --- open binary output file
        output_open();

        // --- open runoff processor
        if (DoRunoff)
            runoff_open();

        // --- open & read hot start file if present
        if (!hotstart_open())
            return ErrorCode;

        // --- open routing processor
        if (DoRouting)
            routing_open();

        // --- open mass balance and statistics processors
        massbal_open();
        stats_open();

        // --- write heading for control actions listing
        if (!RptFlags.disabled && RptFlags.controls)
            report_writeControlActionsHeading();
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except (xfilter(GetExceptionCode(), "swmm_start", 0.0, 0))
    {
        ErrorCode = ERR_SYSTEM;
    }
#endif
    return ErrorCode;
}

/*!
 * \copydoc swmm_end
 */
int EXPORT_SWMM_SOLVER_API swmm_step(double *elapsedTime)
//
//  Input:   elapsedTime = current elapsed time in decimal days
//  Output:  updated value of elapsedTime,
//           returns error code
//  Purpose: advances the simulation by one routing time step.
//
{
    // --- check that simulation can proceed
    *elapsedTime = 0.0;
    if (ErrorCode)
        return ErrorCode;
    if (!IsOpenFlag)
        return (ErrorCode = ERR_API_NOT_OPEN);
    if (!IsStartedFlag)
        return (ErrorCode = ERR_API_NOT_STARTED);

#ifdef EXH
    // --- begin exception handling loop here
    __try
#endif
    {
        // --- if routing time has not exceeded total duration
        if (NewRoutingTime < RoutingDuration)
        {
            // --- route flow & WQ through drainage system
            //     (runoff will be calculated as needed)
            //     (NewRoutingTime is updated)
            execRouting();
        }

        // --- if saving results to the binary file
        if (SaveResultsFlag)
            saveResults();

        // --- save hostart files if applicable
        hotstart_save();

        // --- update elapsed time (days)
        if (NewRoutingTime < RoutingDuration)
            ElapsedTime = NewRoutingTime / MSECperDAY;

        // --- otherwise end the simulation
        else
            ElapsedTime = 0.0;
        *elapsedTime = ElapsedTime;
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except (xfilter(GetExceptionCode(), "swmm_step", ElapsedTime, TotalStepCount))
    {
        ErrorCode = ERR_SYSTEM;
    }
#endif
    return ErrorCode;
}

/*!
 * \copydoc swmm_stride
 */
int EXPORT_SWMM_SOLVER_API swmm_stride(int strideStep, double *elapsedTime)
{
    double realRouteStep = RouteStep;

    if (strideStep <= 0)
        return swmm_step(elapsedTime);

    // --- check that simulation can proceed
    *elapsedTime = 0.0;
    if (ErrorCode)
        return ErrorCode;
    if (!IsOpenFlag)
        return (ErrorCode = ERR_API_NOT_OPEN);
    if (!IsStartedFlag)
        return (ErrorCode = ERR_API_NOT_STARTED);

    // --- modify total duration to be strideStep seconds after current time
    RoutingDuration = NewRoutingTime + 1000.0 * strideStep;
    RoutingDuration = MIN(TotalDuration, RoutingDuration);

    // --- modify routing step to not exceed stride time step
    if (strideStep < RouteStep)
        RouteStep = strideStep;

    // --- step through simulation until next stride step is reached
    do
    {
        swmm_step(elapsedTime);
    } while (*elapsedTime > 0.0 && !ErrorCode);

    // --- restore original routing step and routing duration
    RouteStep = realRouteStep;
    RoutingDuration = TotalDuration;

    // --- restore actual elapsed time (days)
    if (NewRoutingTime < TotalDuration)
    {
        ElapsedTime = NewRoutingTime / MSECperDAY;
    }
    else
        ElapsedTime = 0.0;
    *elapsedTime = ElapsedTime;
    return ErrorCode;
}

/*!
 * \copydoc swmm_useHotStart
 */
int EXPORT_SWMM_SOLVER_API swmm_useHotStart(const char *hotStartFile)
{
    int errorCode = 0;
    int fileVersion = 0;
    char fname[MAXFNAME + 1];

    if (ErrorCode)
        return ErrorCode;
    else if (!IsOpenFlag)
        return (ErrorCode = ERR_API_NOT_OPEN);
    else if (IsStartedFlag)
        return (ErrorCode = ERR_API_NOT_ENDED);

    sstrncpy(fname, hotStartFile, MAXFNAME);

    // Try to open the hotstart file first to see if it is valid
    errorCode = hotstart_is_valid(addAbsolutePath(fname), &fileVersion);
    if (errorCode)
    {
        return errorCode;
    }
    else
    {
        FhotstartInput.mode = USE_FILE;
        sstrncpy(FhotstartInput.name, addAbsolutePath(fname), MAXFNAME);
    }

    return errorCode;
}

/*!
 * \copydoc swmm_saveHotStart
 */
int EXPORT_SWMM_SOLVER_API swmm_saveHotStart(const char *hotStartFile)
{
    int errorCode = 0;

    if (ErrorCode)
        return ErrorCode;
    else if (!IsOpenFlag)
        return (ErrorCode = ERR_API_NOT_OPEN);
    else if (!IsStartedFlag)
        return (ErrorCode = ERR_API_NOT_STARTED);

    errorCode = hotstart_save_to_file(hotStartFile);

    return errorCode;
}

/*!
 * \copydoc execRouting
 */
void execRouting()
{
    double nextRoutingTime; // updated elapsed routing time (msec)
    double routingStep;     // routing time step (sec)

#ifdef EXH
    // --- begin exception handling loop here
    __try
#endif
    {
        // --- determine when next routing time occurs
        TotalStepCount++;
        if (!DoRouting)
            routingStep = MIN(WetStep, ReportStep);
        else
            routingStep = routing_getRoutingStep(RouteModel, RouteStep);
        if (routingStep <= 0.0)
        {
            ErrorCode = ERR_TIMESTEP;
            return;
        }
        nextRoutingTime = NewRoutingTime + 1000.0 * routingStep;

        // --- adjust routing step so that total duration not exceeded
        if (nextRoutingTime > RoutingDuration)
        {
            routingStep = (RoutingDuration - NewRoutingTime) / 1000.0;
            routingStep = MAX(routingStep, 1. / 1000.0);
            nextRoutingTime = RoutingDuration;
        }

        // --- compute runoff until next routing time reached or exceeded
        if (DoRunoff)
            while (NewRunoffTime < nextRoutingTime)
            {
                runoff_execute();
                if (ErrorCode)
                    return;
            }

        // --- if no runoff analysis, update climate state (for evaporation)
        else
            climate_setState(getDateTime(NewRoutingTime));

        // --- route flows & pollutants through drainage system
        //     (while updating NewRoutingTime)
        if (DoRouting)
            routing_execute(RouteModel, routingStep);
        else
            NewRoutingTime = nextRoutingTime;
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except (xfilter(GetExceptionCode(), "execRouting",
                      ElapsedTime, TotalStepCount))
    {
        ErrorCode = ERR_SYSTEM;
        return;
    }
#endif
}

/*!
 * \copydoc saveResults
 */
void saveResults()
{
    if (NewRoutingTime >= ReportTime)
    {
        // --- if user requested that average results be saved:
        if (RptFlags.averages)
        {
            // --- include latest results in current averages
            //     if current time equals the reporting time
            if (NewRoutingTime == ReportTime)
                output_updateAvgResults();

            // --- save current average results to binary file
            //     (which will re-set averages to 0)
            output_saveResults(ReportTime);

            // --- if current time exceeds reporting period then
            //     start computing averages for next period
            if (NewRoutingTime > ReportTime)
                output_updateAvgResults();
        }

        // --- otherwise save interpolated point results
        else
            output_saveResults(ReportTime);

        // --- advance to next reporting period
        ReportTime = ReportTime + 1000 * (double)ReportStep;
    }

    // --- not a reporting period so update average results if applicable
    else if (RptFlags.averages)
        output_updateAvgResults();
}

/*!
 * \copydoc swmm_end
 */
int EXPORT_SWMM_SOLVER_API swmm_end(void)
{
    // --- check that project opened and run started
    if (!IsOpenFlag)
        return (ErrorCode = ERR_API_NOT_OPEN);

    if (IsStartedFlag)
    {
        // --- write ending records to binary output file
        if (Fout.file)
            output_end();

        // --- report mass balance results and system statistics
        if (!ErrorCode && RptFlags.disabled == 0)
        {
            massbal_report();
            stats_report();
        }

        // --- close all computing systems
        stats_close();
        massbal_close();
        if (!IgnoreRainfall)
            rain_close();
        if (DoRunoff)
            runoff_close();
        if (DoRouting)
            routing_close(RouteModel);
        hotstart_close();
        IsStartedFlag = FALSE;
    }
    return ErrorCode;
}

/*!
 * \copydoc swmm_report
 */
int EXPORT_SWMM_SOLVER_API swmm_report()
{
    if (!ErrorCode)
        report_writeReport();
    return ErrorCode;
}

/*!
 * \copydoc swmm_writeLine
 */
void EXPORT_SWMM_SOLVER_API swmm_writeLine(const char *line)
{
    if (IsOpenFlag)
        report_writeLine(line);
}

/*!
 * \copydoc swmm_close
 */
int EXPORT_SWMM_SOLVER_API swmm_close()
{
    if (Fout.file)
        output_close();
    if (IsOpenFlag)
        project_close();
    report_writeSysTime();
    if (Finp.file != NULL)
        fclose(Finp.file);
    if (Frpt.file != NULL)
        fclose(Frpt.file);
    if (Fout.file != NULL)
    {
        fclose(Fout.file);
        if (Fout.mode == SCRATCH_FILE)
            remove(Fout.name);
    }
    IsOpenFlag = FALSE;
    IsStartedFlag = FALSE;
    return 0;
}

/*!
 * \copydoc swmm_getMassBalErr
 */
int EXPORT_SWMM_SOLVER_API swmm_getMassBalErr(float *runoffErr, float *flowErr,
                                              float *qualErr)
{
    *runoffErr = 0.0;
    *flowErr = 0.0;
    *qualErr = 0.0;

    if (IsOpenFlag && !IsStartedFlag)
    {
        *runoffErr = (float)RunoffError;
        *flowErr = (float)FlowError;
        *qualErr = (float)QualError;
    }
    return 0;
}

/*!
 * \copydoc swmm_getVersion
 */
int EXPORT_SWMM_SOLVER_API swmm_getVersion()
{
    return VERSION;
}

/*!
 * \copydoc swmm_getWarnings
 */
int EXPORT_SWMM_SOLVER_API swmm_getWarnings()
{
    return Warnings;
}

/*!
 * \copydoc swmm_getError
 */
int EXPORT_SWMM_SOLVER_API swmm_getError(char *errMsg, int msgLen)
{
    // --- copy text of last error message into errMsg
    if (ErrorCode > 0 && strlen(ErrorMsg) == 0)
        error_getMsg(ErrorCode, ErrorMsg);

    sstrncpy(errMsg, ErrorMsg, msgLen);

    // --- remove leading line feed from errMsg
    if (msgLen > 0 && errMsg[0] == '\n')
        errMsg[0] = ' ';
    return ErrorCode;
}

/*!
 * \copydoc swmm_getErrorFromCode
 */
int EXPORT_SWMM_SOLVER_API swmm_getErrorFromCode(int errorCode, char *outErrMsg[1024])
{
    char err_msg[MAXMSG];
    error_getMsg(errorCode, err_msg);
    sstrncpy(*outErrMsg, err_msg, MAXMSG);

    return 0;
}

/*!
 * \copydoc swmm_getCount
 */
int EXPORT_SWMM_SOLVER_API swmm_getCount(int objType)
{
    if (!IsOpenFlag)
        return ERR_API_NOT_OPEN;
    if (objType < swmm_GAGE || objType >= swmm_SYSTEM)
        return ERR_API_OBJECT_TYPE;
    return Nobjects[objType];
}

/*!
 * \copydoc swmm_getName
 */
int EXPORT_SWMM_SOLVER_API swmm_getName(int objType, int index, char *name, int size)
{
    char *idName = NULL;

    name[0] = '\0';
    if (!IsOpenFlag)
        return ERR_API_NOT_OPEN;

    switch (objType)
    {
    case GAGE:
        idName = Gage[index].ID;
        break;
    case SUBCATCH:
        idName = Subcatch[index].ID;
        break;
    case NODE:
        idName = Node[index].ID;
        break;
    case LINK:
        idName = Link[index].ID;
        break;
    case POLLUT:
        idName = Pollut[index].ID;
        break;
    case LANDUSE:
        idName = Landuse[index].ID;
        break;
    case TIMEPATTERN:
        idName = Pattern[index].ID;
        break;
    case CURVE:
        idName = Curve[index].ID;
        break;
    case TSERIES:
        idName = Tseries[index].ID;
        break;
    case TRANSECT:
        idName = Transect[index].ID;
        break;
    case AQUIFER:
        idName = Aquifer[index].ID;
        break;
    case UNITHYD:
        idName = UnitHyd[index].ID;
        break;
    case SNOWMELT:
        idName = Snowmelt[index].ID;
        break;
    default:
        return ERR_API_OBJECT_TYPE;
    }

    if (idName)
        sstrncpy(name, idName, size);

    return 0;
}

/*!
 * \copydoc swmm_getIndex
 */
int EXPORT_SWMM_SOLVER_API swmm_getIndex(int objType, const char *name)
{
    if (!IsOpenFlag)
        return ERR_API_NOT_OPEN;
    if (objType < swmm_GAGE || objType >= swmm_SYSTEM)
        return ERR_API_OBJECT_TYPE;
    return project_findObject(objType, name);
}

/*!
 * \copydoc swmm_getValue
 */
double EXPORT_SWMM_SOLVER_API swmm_getValue(int property, int index)
{
    if (!IsOpenFlag)
        return ERR_API_NOT_OPEN;
    if (property < 100)
        return getSystemValue(property);
    if (property < 200)
        return getGageValue(property, index);
    if (property < 300)
        return getSubcatchValue(property, index, -1);
    if (property < 400)
        return getNodeValue(property, index, -1);
    if (property < 500)
        return getLinkValue(property, index, -1);

    return ERR_API_PROPERTY_TYPE;
}

/*!
 * \copydoc swmm_getValueExpanded
 */
double EXPORT_SWMM_SOLVER_API swmm_getValueExpanded(int objType, int property, int index, int subIndex)
{
    if (!IsOpenFlag)
        return ERR_API_NOT_OPEN;

    switch (objType)
    {
    case swmm_SYSTEM:
        return getSystemValue(property);
    case swmm_GAGE:
        return getGageValue(property, index);
    case swmm_SUBCATCH:
        return getSubcatchValue(property, index, subIndex);
    case swmm_NODE:
        return getNodeValue(property, index, subIndex);
    case swmm_LINK:
        return getLinkValue(property, index, subIndex);
    default:
        return ERR_API_OBJECT_TYPE;
    }
}

/*!
 * \copydoc swmm_setValue
 */
int EXPORT_SWMM_SOLVER_API swmm_setValue(int property, int index, double value)
{

    if (!IsOpenFlag)
        return ERR_API_NOT_OPEN;

    switch (property)
    {
    case swmm_GAGE_RAINFALL:
        if (index < 0 || index >= Nobjects[GAGE])
            return 0;
        if (value >= 0.0)
            Gage[index].apiRainfall = value;
        return 0;
    case swmm_SUBCATCH_RPTFLAG:
        if (!IsStartedFlag && index >= 0 && index < Nobjects[SUBCATCH])
            Subcatch[index].rptFlag = (value > 0.0);
        return 0;
    case swmm_NODE_LATFLOW:
        setNodeLatFlow(index, value);
        return 0;
    case swmm_NODE_HEAD:
        setOutfallStage(index, value);
        return 0;
    case swmm_NODE_RPTFLAG:
        if (!IsStartedFlag && index >= 0 && index < Nobjects[NODE])
            Node[index].rptFlag = (value > 0.0);
        return 0;
    case swmm_LINK_SETTING:
        setLinkSetting(index, value);
        return 0;
    case swmm_LINK_RPTFLAG:
        if (!IsStartedFlag && index >= 0 && index < Nobjects[LINK])
            Link[index].rptFlag = (value > 0.0);
        return 0;
    case swmm_ROUTESTEP:
        setRoutingStep(value);
        return 0;
    case swmm_REPORTSTEP:
        if (!IsStartedFlag && value > 0)
            ReportStep = (int)value;
        return 0;
    case swmm_NOREPORT:
        if (!IsStartedFlag)
            RptFlags.disabled = (value > 0.0);
        return 0;
    default:
        return ERR_API_PROPERTY_TYPE;
    }
}

/*!
 * \copydoc swmm_setValueExpanded
 */
int EXPORT_SWMM_SOLVER_API swmm_setValueExpanded(int objType, int property, int index, int subIndex, double value)
{
    if (!IsOpenFlag)
        return ERR_API_NOT_OPEN;

    switch (objType)
    {
    case swmm_SYSTEM:
        return setSystemValue(property, value);
    case swmm_GAGE:
        return setGageValue(property, index, subIndex, value);
    case swmm_SUBCATCH:
        return setSubcatchValue(property, index, subIndex, value);
    case swmm_NODE:
        return setNodeValue(property, index, subIndex, value);
    case swmm_LINK:
        return setLinkValue(property, index, subIndex, value);
    default:
        return ERR_API_OBJECT_TYPE;
    }
}

/*!
 * \copydoc setGageValue
 */
int setGageValue(int property, int index, int subIndex, double value)
{
    if (index < 0 || index >= Nobjects[GAGE])
        return ERR_API_OBJECT_INDEX;

    switch (property)
    {
    case swmm_GAGE_RAINFALL:
        if (value >= 0.0)
        {
            Gage[index].apiRainfall = value;
            return 0;
        }
        else
            return ERR_API_PROPERTY_VALUE;
    default:
        return ERR_API_PROPERTY_TYPE;
    }
}

/*!
 * \copydoc setSubcatchValue
 */
int setSubcatchValue(int property, int index, int subIndex, double value)
{

    if (IsOpenFlag == FALSE)
    {
        return ERR_API_NOT_OPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
        return ERR_API_OBJECT_INDEX;
    // Check if Simulation is Running
    else if (IsStartedFlag == TRUE)
    {
        // Set values that can be configured at runtime during simulation
        switch (property)
        {
        case swmm_SUBCATCH_API_RAINFALL:
            if (value >= 0.0)
            {
                Subcatch[index].apiRainfall = value / UCF(RAINFALL);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_SUBCATCH_API_SNOWFALL:
            if (value >= 0.0)
            {
                Subcatch[index].apiSnowfall = value / UCF(RAINFALL);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_SUBCATCH_EXTERNAL_POLLUTANT_BUILDUP:
            {
                if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
                    return ERR_API_OBJECT_INDEX;
                Subcatch[index].apiExtBuildup[subIndex] = value;
                return 0;
            }
        default:
            return ERR_API_IS_RUNNING;
        }
    }
    else
    {
        switch (property)
        {
        case swmm_SUBCATCH_AREA:
            if (value >= 0.0)
            {
                Subcatch[index].area = value / UCF(LANDAREA);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_SUBCATCH_WIDTH:
            if (value >= 0.0)
            {
                Subcatch[index].width = value / UCF(LENGTH);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_SUBCATCH_SLOPE:
            if (value >= 0.0)
            {
                Subcatch[index].slope = value;
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_SUBCATCH_CURB_LENGTH:
            if (value >= 0.0)
            {
                Subcatch[index].curbLength = value / UCF(LENGTH);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_SUBCATCH_API_RAINFALL:
            if (value >= 0.0)
            {
                Subcatch[index].apiRainfall = value / UCF(RAINFALL);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_SUBCATCH_API_SNOWFALL:
            if (value >= 0.0)
            {
                Subcatch[index].apiSnowfall = value / UCF(RAINFALL);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_SUBCATCH_RPTFLAG:
            if (value >= 0.0)
            {
                Subcatch[index].rptFlag = (value > 0.0);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        default:
            return ERR_API_PROPERTY_TYPE;
        }
    }
}

/*!
 * \copydoc setNodeValue
 */
int setNodeValue(int property, int index, int subIndex, double value)
{

    TNode *node = NULL;

    if (IsOpenFlag == FALSE)
    {
        return ERR_API_NOT_OPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        return ERR_API_OBJECT_INDEX;
    }
    // Check if Simulation is Running
    else if (IsStartedFlag == TRUE)
    {

        // Set values that can be configured at runtime during simulation
        switch (property)
        {
        case swmm_NODE_LATFLOW:
            Node[index].apiExtInflow = value / UCF(FLOW);
            return 0;
        case swmm_NODE_HEAD:
            {
                node = &Node[index];

                if (node->type != OUTFALL)
                    return ERR_API_OBJECT_TYPE;

                Outfall[node->subIndex].fixedStage = value / UCF(LENGTH);
                Outfall[node->subIndex].type = FIXED_OUTFALL;
                return 0;
            }
        case swmm_NODE_POLLUTANT_LATMASS_FLUX:
            {
                if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
                    return ERR_API_OBJECT_INDEX;

                Node[index].apiExtQualMassFlux[subIndex] = value;
                return 0;
            }
        default:
            return ERR_API_IS_RUNNING;
        }
    }
    else
    {
        // Set values that can only be configured before simulation starts
        switch (property)
        {
        case swmm_NODE_ELEV:
            Node[index].invertElev = value / UCF(LENGTH);
            return 0;
        case swmm_NODE_MAXDEPTH:
            if (value >= 0.0)
            {
                Node[index].fullDepth = value / UCF(LENGTH);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_NODE_SURCHARGE_DEPTH:
            if (value >= 0.0)
            {
                Node[index].surDepth = value / UCF(LENGTH);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_NODE_PONDED_AREA:
            if (value >= 0.0)
            {
                Node[index].pondedArea = value / UCF(LANDAREA);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_NODE_INITIAL_DEPTH:
            if (value >= 0.0)
            {
                Node[index].initDepth = value / UCF(LENGTH);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_NODE_LATFLOW:
            Node[index].apiExtInflow = value / UCF(FLOW);
            return 0;
        case swmm_NODE_HEAD:
            node = &Node[index];

            if (node->type != OUTFALL)
                return ERR_API_OBJECT_TYPE;

            Outfall[node->subIndex].fixedStage = value / UCF(LENGTH);
            Outfall[node->subIndex].type = FIXED_OUTFALL;
            return 0;
        case swmm_NODE_RPTFLAG:
            Node[index].rptFlag = (value > 0.0);
            return 0;
        case swmm_NODE_POLLUTANT_LATMASS_FLUX:
            Node[index].apiExtQualMassFlux[subIndex] = value;
            return 0;
        default:
            return ERR_API_PROPERTY_TYPE;
        }
    }
}

/*!
 * \copydoc setLinkValue
 */
int setLinkValue(int property, int index, int subIndex, double value)
{
    TLink *link = NULL;

    if (IsOpenFlag == FALSE)
    {
        return ERR_API_NOT_OPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        return ERR_API_OBJECT_INDEX;
    }
    // Check if Simulation is Running
    else if (IsStartedFlag == TRUE)
    {

        link = &Link[index];

        // Set values that can be configured at runtime during simulation
        switch (property)
        {
        case swmm_LINK_SETTING:
            return setLinkSetting(index, value);
        case swmm_LINK_FLOW_LIMIT:
            link->qLimit = value / UCF(FLOW);
            return 0;
        case swmm_LINK_SEEPAGE_RATE:
            if (value >= 0.0)
            {
                link->seepRate = value / UCF(RAINFALL);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
        case swmm_LINK_POLLUTANT_LATMASS_FLUX:
            {
                if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
                    return ERR_API_OBJECT_INDEX;

                link->apiExtQualMassFlux[subIndex] = value;
                return 0;
            }
        default:
            return ERR_API_IS_RUNNING;
        }
    }
    else
    {
        // Set values that can only be configured before simulation starts
        switch (property)
        {
        case swmm_LINK_SETTING:
            return setLinkSetting(index, value);
        case swmm_LINK_OFFSET1:
            Link[index].offset1 = value / UCF(LENGTH);
            return 0;
        case swmm_LINK_OFFSET2:
            Link[index].offset2 = value / UCF(LENGTH);
            return 0;
        case swmm_LINK_INITIAL_FLOW:
            Link[index].q0 = value / UCF(FLOW);
            return 0;
        case swmm_LINK_FLOW_LIMIT:
            Link[index].qLimit = value / UCF(FLOW);
            return 0;
        case swmm_LINK_INLET_LOSS:
            Link[index].cLossInlet = value;
            return 0;
        case swmm_LINK_OUTLET_LOSS:
            Link[index].cLossOutlet = value;
            return 0;
        case swmm_LINK_AVERAGE_LOSS:
            Link[index].cLossAvg = value;
            return 0;
        case swmm_LINK_SEEPAGE_RATE:
            if (value >= 0.0)
            {
                Link[index].seepRate = value / UCF(RAINFALL);
                return 0;
            }
            else
                return ERR_API_PROPERTY_VALUE;
            return 0;
        case swmm_LINK_HAS_FLAPGATE:
            Link[index].hasFlapGate = (value > 0.0);
            return 0;
        case swmm_LINK_POLLUTANT_LATMASS_FLUX:
            {
                if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
                    return ERR_API_OBJECT_INDEX;

                link->apiExtQualMassFlux[subIndex] = value;
                return 0;
            }
        default:
            return ERR_API_PROPERTY_TYPE;
        }
    }
}

/*!
 * \copydoc swmm_getSavedValue
 */
double EXPORT_SWMM_SOLVER_API swmm_getSavedValue(int property, int index, int period)
{
    if (!IsOpenFlag)
        return 0;
    if (IsStartedFlag)
        return 0;
    if (period < 1 || period > Nperiods)
        return 0;
    if (property == swmm_CURRENTDATE)
        return getSavedDate(period);
    if (property >= 200 && property < 300)
        return getSavedSubcatchValue(property, index, period);
    if (property < 400)
        return getSavedNodeValue(property, index, period);
    if (property < 500)
        return getSavedLinkValue(property, index, period);
    return 0;
}

/*!
 * \copydoc swmm_decodeDate
 */
void EXPORT_SWMM_SOLVER_API swmm_decodeDate(double date, int *year, int *month, int *day,
                                            int *hour, int *minute, int *second, int *dayOfWeek)
{
    datetime_decodeDate(date, year, month, day);
    datetime_decodeTime(date, hour, minute, second);
    *dayOfWeek = datetime_dayOfWeek(date);
}

/*!
 * \copydoc swmm_encodeDate
 */
double EXPORT_SWMM_SOLVER_API swmm_encodeDate(int year, int month, int day,
                                              int hour, int minute, int second)
{
    return datetime_encodeDate(year, month, day) + datetime_encodeTime(hour, minute, second);
}

/*!
 * \copydoc getGageValue
 */
double getGageValue(int property, int index)
{
    double total, snow, rain;

    if (index < 0 || index >= Nobjects[GAGE])
        return ERR_API_OBJECT_INDEX;

    total = gage_getPrecip(index, &rain, &snow);

    switch (property)
    {
    case swmm_GAGE_TOTAL_PRECIPITATION:
        return total * UCF(RAINFALL);
    case swmm_GAGE_RAINFALL:
        return rain * UCF(RAINFALL);
    case swmm_GAGE_SNOWFALL:
        return snow * UCF(RAINFALL);
    default:
        return ERR_API_PROPERTY_TYPE;
    }
}

/*!
 * \copydoc getSubcatchValue
 */
double getSubcatchValue(int property, int index, int subIndex)
{
    TSubcatch *subcatch;
    double sub_catch_area;
    double prop_results;
    int i;

    if (index < 0 || index >= Nobjects[SUBCATCH])
        return 0;

    subcatch = &Subcatch[index];

    switch (property)
    {
    case swmm_SUBCATCH_AREA:
        return subcatch->area * UCF(LANDAREA);
    case swmm_SUBCATCH_RAINGAGE:
        return subcatch->gage;
    case swmm_SUBCATCH_RAINFALL:
        return subcatch->rainfall * UCF(RAINFALL);
    case swmm_SUBCATCH_EVAP:
        return subcatch->evapLoss * UCF(EVAPRATE);
    case swmm_SUBCATCH_INFIL:
        return subcatch->infilLoss * UCF(RAINFALL);
    case swmm_SUBCATCH_RUNOFF:
        return subcatch->newRunoff * UCF(FLOW);
    case swmm_SUBCATCH_RPTFLAG:
        return (subcatch->rptFlag > 0);
    case swmm_SUBCATCH_WIDTH:
        return subcatch->width * UCF(LENGTH);
    case swmm_SUBCATCH_SLOPE:
        return subcatch->slope;
    case swmm_SUBCATCH_CURB_LENGTH:
        return subcatch->curbLength * UCF(LENGTH);
    case swmm_SUBCATCH_API_RAINFALL:
        return subcatch->apiRainfall * UCF(RAINFALL);
    case swmm_SUBCATCH_API_SNOWFALL:
        return subcatch->apiSnowfall * UCF(RAINFALL);
    case swmm_SUBCATCH_POLLUTANT_BUILDUP:
        if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
            return ERR_API_OBJECT_INDEX;

        prop_results = 0.0;

        for (i = 0; i < Nobjects[LANDUSE]; i++)
        {
            sub_catch_area = subcatch->area * UCF(LANDAREA) * subcatch->landFactor[i].fraction;

            if (sub_catch_area > 0)
            {
                prop_results += subcatch->landFactor[i].buildup[subIndex] / sub_catch_area;
            }
        }

        return prop_results;
    case swmm_SUBCATCH_EXTERNAL_POLLUTANT_BUILDUP:
        if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
            return ERR_API_OBJECT_INDEX;
        return subcatch->apiExtBuildup[subIndex] / UCF(LANDAREA);

    case swmm_SUBCATCH_POLLUTANT_RUNOFF_CONCENTRATION:
        if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
            return ERR_API_OBJECT_INDEX;
        return subcatch->newQual[subIndex];

    case swmm_SUBCATCH_POLLUTANT_PONDED_CONCENTRATION:
        if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
            return ERR_API_OBJECT_INDEX;
        return subcatch->pondedQual[subIndex] / (subcatch_getDepth(index) * MAX(0.0, subcatch->area - subcatch->lidArea) * UCF(LANDAREA));

    case swmm_SUBCATCH_POLLUTANT_TOTAL_LOAD:
        if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
            return ERR_API_OBJECT_INDEX;
        return subcatch->totalLoad[subIndex];

    default:
        return ERR_API_PROPERTY_TYPE;
    }
}

/*!
 * \copydoc getNodeValue
 */
double getNodeValue(int property, int index, int subIndex)
{
    TNode *node;
    if (index < 0 || index >= Nobjects[NODE])
        return 0;
    else
    {
        node = &Node[index];

        switch (property)
        {
        case swmm_NODE_TYPE:
            return node->type;
        case swmm_NODE_ELEV:
            return node->invertElev * UCF(LENGTH);
        case swmm_NODE_MAXDEPTH:
            return node->fullDepth * UCF(LENGTH);
        case swmm_NODE_DEPTH:
            return node->newDepth * UCF(LENGTH);
        case swmm_NODE_HEAD:
            return (node->newDepth + node->invertElev) * UCF(LENGTH);
        case swmm_NODE_VOLUME:
            return node->newVolume * UCF(VOLUME);
        case swmm_NODE_LATFLOW:
            return node->newLatFlow * UCF(FLOW);
        case swmm_NODE_INFLOW:
            return node->inflow * UCF(FLOW);
        case swmm_NODE_OVERFLOW:
            return node->overflow * UCF(FLOW);
        case swmm_NODE_RPTFLAG:
            return (node->rptFlag > 0);
        case swmm_NODE_SURCHARGE_DEPTH:
            return node->surDepth * UCF(LENGTH);
        case swmm_NODE_PONDED_AREA:
            return node->pondedArea * UCF(LANDAREA);
        case swmm_NODE_INITIAL_DEPTH:
            return node->initDepth * UCF(LENGTH);
        case swmm_NODE_POLLUTANT_CONCENTRATION:
            if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
                return ERR_API_OBJECT_INDEX;
            return node->newQual[subIndex];
        case swmm_NODE_POLLUTANT_LATMASS_FLUX:
            if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
                return ERR_API_OBJECT_INDEX;
            return node->apiExtQualMassFlux[subIndex];
        default:
            return ERR_API_OBJECT_TYPE;
        }
    }
}

/*!
 * \copydoc getLinkValue
 */
double getLinkValue(int property, int index, int subIndex)
{
    TLink *link;
    if (index < 0 || index >= Nobjects[LINK])
        return 0;
    link = &Link[index];
    switch (property)
    {
    case swmm_LINK_TYPE:
        return link->type;
    case swmm_LINK_NODE1:
        return link->node1;
    case swmm_LINK_NODE2:
        return link->node2;
    case swmm_LINK_LENGTH:
        if (link->type == CONDUIT)
            return Conduit[link->subIndex].length * UCF(LENGTH);
        else
            return ERR_API_OBJECT_TYPE;
    case swmm_LINK_SLOPE:
        if (link->type == CONDUIT)
            return Conduit[link->subIndex].slope;
        else
            return ERR_API_OBJECT_TYPE;
        break;
    case swmm_LINK_FULLDEPTH:
        return link->xsect.yFull * UCF(LENGTH);
    case swmm_LINK_FULLFLOW:
        return link->qFull * UCF(FLOW);
    case swmm_LINK_SETTING:
        return link->setting;
    case swmm_LINK_TIMEOPEN:
        if (link->setting > 0.0)
            return (getDateTime(NewRoutingTime) - link->timeLastSet) * 24.;
        else
            return 0;
    case swmm_LINK_TIMECLOSED:
        if (link->setting <= 0.0)
            return (getDateTime(NewRoutingTime) - link->timeLastSet) * 24.;
        else
            return 0;
    case swmm_LINK_FLOW:
        return link->newFlow * UCF(FLOW);
    case swmm_LINK_DEPTH:
        return link->newDepth * UCF(LENGTH);
    case swmm_LINK_VELOCITY:
        return link_getVelocity(index, fabs(link->newFlow), link->newDepth) * UCF(LENGTH);
    case swmm_LINK_TOPWIDTH:
        if (link->type == CONDUIT)
            return xsect_getWofY(&link->xsect, link->newDepth) * UCF(LENGTH);
        else
            return ERR_API_OBJECT_TYPE;
    case swmm_LINK_VOLUME:
        return link->newVolume * UCF(VOLUME);
    case swmm_LINK_CAPACITY:
        if (link->type == CONDUIT)
        {
            if (link->xsect.type != DUMMY)
                return xsect_getAofY(&link->xsect, link->newDepth) / link->xsect.aFull;
            else
                return 0.0;
        }
        else
            return link->setting;

    case swmm_LINK_RPTFLAG:
        return (link->rptFlag > 0);
    case swmm_LINK_OFFSET1:
        return link->offset1 * UCF(LENGTH);
    case swmm_LINK_OFFSET2:
        return link->offset2 * UCF(LENGTH);
    case swmm_LINK_INITIAL_FLOW:
        return link->q0 * UCF(FLOW);
    case swmm_LINK_FLOW_LIMIT:
        return link->qLimit * UCF(FLOW);
    case swmm_LINK_INLET_LOSS:
        return link->cLossInlet;
    case swmm_LINK_OUTLET_LOSS:
        return link->cLossOutlet;
    case swmm_LINK_AVERAGE_LOSS:
        return link->cLossAvg;
    case swmm_LINK_SEEPAGE_RATE:
        return link->seepRate * UCF(RAINFALL);
    case swmm_LINK_HAS_FLAPGATE:
        return (link->hasFlapGate > 0);
    case swmm_LINK_POLLUTANT_CONCENTRATION:
        if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
            return ERR_API_OBJECT_INDEX;
        return link->newQual[subIndex];
    case swmm_LINK_POLLUTANT_LOAD:
        if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
            return ERR_API_OBJECT_INDEX;
        return link->totalLoad[subIndex];
    case swmm_LINK_POLLUTANT_LATMASS_FLUX:
        if (subIndex < 0 || subIndex >= Nobjects[POLLUT])
            return ERR_API_OBJECT_INDEX;
        return link->apiExtQualMassFlux[subIndex];
    default:
        return ERR_API_OBJECT_TYPE;
    }
}

/*!
 * \copydoc getSystemValue
 */
double getSystemValue(int property)
{
    switch (property)
    {
    case swmm_STARTDATE:
        return StartDateTime;
    case swmm_CURRENTDATE:
        return StartDateTime + ElapsedTime;
    case swmm_ELAPSEDTIME:
        return ElapsedTime;
    case swmm_ROUTESTEP:
        return RouteStep;
    case swmm_MAXROUTESTEP:
        return getMaxRouteStep();
    case swmm_REPORTSTEP:
        return ReportStep;
    case swmm_TOTALSTEPS:
        return Nperiods;
    case swmm_NOREPORT:
        return RptFlags.disabled;
    case swmm_FLOWUNITS:
        return FlowUnits;
    case swmm_ENDDATE:
        return EndDateTime;
    case swmm_REPORTSTART:
        return ReportStart;
    case swmm_UNITSYSTEM:
        return UnitSystem;
    case swmm_SURCHARGEMETHOD:
        return SurchargeMethod;
    case swmm_ALLOWPONDING:
        return AllowPonding;
    case swmm_INERTIADAMPING:
        return InertDamping;
    case swmm_NORMALFLOWLTD:
        return NormalFlowLtd;
    case swmm_SKIPSTEADYSTATE:
        return SkipSteadyState;
    case swmm_IGNORERAINFALL:
        return IgnoreRainfall;
    case swmm_IGNORERDII:
        return IgnoreRDII;
    case swmm_IGNORESNOWMELT:
        return IgnoreSnowmelt;
    case swmm_IGNOREGROUNDWATER:
        return IgnoreGwater;
    case swmm_IGNOREROUTING:
        return IgnoreRouting;
    case swmm_IGNOREQUALITY:
        return IgnoreQuality;
    case swmm_ERROR_CODE:
        return ErrorCode;
    case swmm_RULESTEP:
        return RuleStep;
    case swmm_SWEEPSTART:
        return SweepStart;
    case swmm_SWEEPEND:
        return SweepEnd;
    case swmm_MAXTRIALS:
        return MaxTrials;
    case swmm_NUMTHREADS:
        return NumThreads;
    case swmm_MINROUTESTEP:
        return MinRouteStep;
    case swmm_LENGTHENINGSTEP:
        return LengtheningStep;
    case swmm_STARTDRYDAYS:
        return StartDryDays;
    case swmm_COURANTFACTOR:
        return CourantFactor;
    case swmm_MINSURFAREA:
        return MinSurfArea * UCF(LENGTH) * UCF(LENGTH);
    case swmm_MINSLOPE:
        return MinSlope;
    case swmm_RUNOFFERROR:
        return RunoffError;
    case swmm_FLOWERROR:
        return FlowError;
    case swmm_HEADTOL:
        return HeadTol * UCF(LENGTH);
    case swmm_SYSFLOWTOL:
        return SysFlowTol;
    case swmm_LATFLOWTOL:
        return LatFlowTol;
    default:
        return ERR_API_PROPERTY_TYPE;
    }
}

/*!
 * \copydoc setNodeLatFlow
 */
int setNodeLatFlow(int index, double value)
{
    if (index < 0 || index >= Nobjects[NODE])
        return ERR_API_OBJECT_INDEX;
    Node[index].apiExtInflow = value / UCF(FLOW);
    return 0;
}

/*!
 * \copydoc setOutfallStage
 */
int setOutfallStage(int index, double value)
{
    TNode *node;
    if (index < 0 || index >= Nobjects[NODE])
        return ERR_API_OBJECT_INDEX;

    node = &Node[index];

    if (node->type != OUTFALL)
        return ERR_API_OBJECT_TYPE;

    Outfall[node->subIndex].fixedStage = value / UCF(LENGTH);
    Outfall[node->subIndex].type = FIXED_OUTFALL;

    return 0;
}

/*!
 * \copydoc setLinkSetting
 */
int setLinkSetting(int index, double value)
{
    TLink *link;
    char control_rule_label[9] = "SWMM API";
    double currentTime;

    if (index < 0 || index >= Nobjects[LINK])
        return ERR_API_OBJECT_INDEX;

    link = &Link[index];
    if (value < 0.0 || link->type == CONDUIT)
        return ERR_API_OBJECT_INDEX;

    if (link->type != PUMP && value > 1.0)
        value = 1.0;

    if (link->targetSetting == value)
        return 0;

    link->targetSetting = value;

    if (link->targetSetting * link->setting == 0.0)
        link->timeLastSet = StartDateTime + ElapsedTime;

    link_setSetting(index, 0.0);

    // Add control action to RPT file if desired flagged
    if (RptFlags.controls)
    {
        currentTime = getDateTime(NewRoutingTime);
        report_writeControlAction(currentTime, Link[index].ID, value, control_rule_label);
    }

    return 0;
}

/*!
 * \copydoc getSavedDate
 */
double getSavedDate(int period)
{
    double days;
    output_readDateTime(period, &days);
    return days;
}

/*!
 * \copydoc getSavedSubcatchValue
 */
double getSavedSubcatchValue(int property, int index, int period)
{
    // --- SubcatchResults array is defined in output.c and contains
    //     computed results in user's units
    extern float *SubcatchResults;

    // --- order in which subcatchment was saved to output results file
    int outIndex = Subcatch[index].rptFlag - 1;
    if (outIndex < 0)
        return 0;

    output_readSubcatchResults(period, outIndex);
    switch (property)
    {
    case swmm_SUBCATCH_RAINFALL:
        return SubcatchResults[SUBCATCH_RAINFALL];
    case swmm_SUBCATCH_EVAP:
        return SubcatchResults[SUBCATCH_EVAP];
    case swmm_SUBCATCH_INFIL:
        return SubcatchResults[SUBCATCH_INFIL];
    case swmm_SUBCATCH_RUNOFF:
        return SubcatchResults[SUBCATCH_RUNOFF];
    default:
        return 0;
    }
}

/*!
 * \copydoc getSavedNodeValue
 */
double getSavedNodeValue(int property, int index, int period)
{
    // --- NodeResults array is defined in output.c and contains
    //     computed results in user's units
    extern float *NodeResults;

    // --- order in which node was saved to output results file
    int outIndex = Node[index].rptFlag - 1;
    if (outIndex < 0)
        return 0;

    output_readNodeResults(period, outIndex);
    switch (property)
    {
    case swmm_NODE_DEPTH:
        return NodeResults[NODE_DEPTH];
    case swmm_NODE_HEAD:
        return NodeResults[NODE_HEAD];
    case swmm_NODE_VOLUME:
        return NodeResults[NODE_VOLUME];
    case swmm_NODE_LATFLOW:
        return NodeResults[NODE_LATFLOW];
    case swmm_NODE_INFLOW:
        return NodeResults[NODE_INFLOW];
    case swmm_NODE_OVERFLOW:
        return NodeResults[NODE_OVERFLOW];
    default:
        return 0;
    }
}

/*!
 * \copydoc getSavedLinkValue
 */
double getSavedLinkValue(int property, int index, int period)
{
    double y, w;

    // --- LinkResults array is defined in output.c and contains
    //     computed results in user's units
    extern float *LinkResults;

    // --- order in which link was saved to output results file
    int outIndex = Link[index].rptFlag - 1;
    if (outIndex < 0)
        return 0;

    output_readLinkResults(period, outIndex);
    switch (property)
    {
    case swmm_LINK_FLOW:
        return LinkResults[LINK_FLOW];
    case swmm_LINK_DEPTH:
        return LinkResults[LINK_DEPTH];
    case swmm_LINK_VELOCITY:
        return LinkResults[LINK_VELOCITY];
    case swmm_LINK_TOPWIDTH:
        y = LinkResults[LINK_DEPTH] / UCF(LENGTH);
        w = xsect_getWofY(&Link[index].xsect, y);
        return w * UCF(LENGTH);
    case swmm_LINK_SETTING:
        return LinkResults[LINK_CAPACITY];
    default:
        return 0;
    }
}

/*!
 * \copydoc getMaxRouteStep
 */
double getMaxRouteStep()
{
    double tmpCourantFactor = CourantFactor;
    double result = RouteStep;

    if (!IsStartedFlag || RouteModel != DW)
        return result;
    CourantFactor = 1.0;
    result = routing_getRoutingStep(RouteModel, MinRouteStep);
    CourantFactor = tmpCourantFactor;
    return result;
}

/*!
 * \copydoc setRoutingStep
 */
int setRoutingStep(double value)
{
    if (value <= 0.0)
        return ERR_API_PROPERTY_VALUE;

    if (value <= MinRouteStep)
        value = MinRouteStep;

    CourantFactor = 0.0;
    RouteStep = value;

    return 0;
}

/*!
 * \copydoc setSystemValue
 */
int setSystemValue(int property, double value)
{
    int y, m, d, h, mm, s;

    if (IsStartedFlag)
        return ERR_API_NOT_ENDED;

    switch (property)
    {
    case swmm_STARTDATE:
        StartDateTime = value;
        datetime_decodeDate(value, &y, &m, &d);
        datetime_decodeTime(value, &h, &mm, &s);
        StartDate = datetime_encodeDate(y, m, d);
        StartTime = datetime_encodeTime(h, mm, s);
        TotalDuration = floor((EndDate - StartDate) * SECperDAY + (EndTime - StartTime) * SECperDAY);
        // convert total duration to milliseconds
        TotalDuration *= 1000.0;
        return 0;
    case swmm_ROUTESTEP:
        return setRoutingStep(value);

    case swmm_REPORTSTEP:
        if (value > 0)
        {
            ReportStep = (int)value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }

    case swmm_NOREPORT:
        RptFlags.disabled = (value > 0.0);
        return 0;

    case swmm_ENDDATE:
        EndDateTime = value;
        datetime_decodeDate(value, &y, &m, &d);
        datetime_decodeTime(value, &h, &mm, &s);
        EndDate = datetime_encodeDate(y, m, d);
        EndTime = datetime_encodeTime(h, mm, s);
        TotalDuration = floor((EndDate - StartDate) * SECperDAY + (EndTime - StartTime) * SECperDAY);
        // convert total duration to milliseconds
        TotalDuration *= 1000.0;
        return 0;
    case swmm_REPORTSTART:
        ReportStart = value;
        datetime_decodeDate(value, &y, &m, &d);
        datetime_decodeTime(value, &h, &mm, &s);
        ReportStartDate = datetime_encodeDate(y, m, d);
        ReportStartTime = datetime_encodeTime(h, mm, s);
        return 0;
    case swmm_NUMTHREADS:
        // possible over allocation of threads but we trust the user to know what they are doing. Limit to max threads.
        NumThreads = MAX(1, MIN((int)value, omp_get_max_threads()));
        return 0;
    case swmm_SURCHARGEMETHOD:
        if (value >= EXTRAN && value <= SLOT)
        {
            SurchargeMethod = (int)value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_ALLOWPONDING:
        AllowPonding = (value > 0.0);
        return 0;
    case swmm_INERTIADAMPING:
        if (value >= NO_DAMPING && value <= FULL_DAMPING)
        {
            InertDamping = (int)value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_NORMALFLOWLTD:
        if (value >= SLOPE && value <= NEITHER)
        {
            NormalFlowLtd = (int)value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_SKIPSTEADYSTATE:
        SkipSteadyState = (value > 0.0);
        return 0;
    case swmm_IGNORERAINFALL:
        IgnoreRainfall = (value > 0.0);
        return 0;
    case swmm_IGNORERDII:
        IgnoreRDII = (value > 0.0);
        return 0;
    case swmm_IGNORESNOWMELT:
        IgnoreSnowmelt = (value > 0.0);
        return 0;
    case swmm_IGNOREGROUNDWATER:
        IgnoreGwater = (value > 0.0);
        return 0;
    case swmm_IGNOREROUTING:
        IgnoreRouting = (value > 0.0);
        return 0;
    case swmm_IGNOREQUALITY:
        IgnoreQuality = (value > 0.0);
        return 0;
    case swmm_RULESTEP:
        if (value > 0)
        {
            RuleStep = (int)value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_SWEEPSTART:
        if (value >= 0.0 && value <= 365)
        {
            SweepStart = (int)value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_SWEEPEND:
        if (value >= 0.0 && value <= 365)
        {
            SweepEnd = (int)value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_MAXTRIALS:
        if (value >= 2)
        {
            MaxTrials = (int)value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_MINROUTESTEP:
        if (value > 0)
        {
            MinRouteStep = value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_LENGTHENINGSTEP:
        if (value > 0)
        {
            LengtheningStep = value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_STARTDRYDAYS:
        if (value >= 0)
        {
            StartDryDays = value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_COURANTFACTOR:
        if (value > 0 && value <= 2.0)
        {
            CourantFactor = value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_MINSURFAREA:
        if (value >= 0)
        {
            MinSurfArea = value / UCF(LENGTH) / UCF(LENGTH);
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    case swmm_MINSLOPE:
        if (value >= 0 && value < 100)
        {
            MinSlope = value;
            return 0;
        }
        else
        {
            return ERR_API_PROPERTY_VALUE;
        }
    default:
        return ERR_API_PROPERTY_TYPE;
    }
}

/*!
 * \copydoc UCF
 */
double UCF(int u)
{
    if (u < FLOW)
        return Ucf[u][UnitSystem];
    else
        return Qcf[FlowUnits];
}

/*!
 * \copydoc sstrncpy
 */
size_t sstrncpy(char *dest, const char *src, size_t n)
{
    int offset = 0;
    if (n > 0)
    {
        while (*(src + offset) != '\0')
        {
            if ((size_t)offset == n)
                break;
            *(dest + offset) = *(src + offset);
            offset++;
        }
    }
    *(dest + offset) = '\0';
    return strlen(dest);
}

/*!
 * \copydoc sstrcat
 */
size_t sstrcat(char *dest, const char *src, size_t destsize)
{
    size_t dest_len, offset = 0, src_index = 0;

    // obtain initial sizes
    dest_len = strlen(dest);

    // get the end of dest
    offset = dest_len;

    // append src
    src_index = 0;
    while (*(src + src_index) != '\0')
    {
        *(dest + offset) = *(src + src_index);
        offset++;
        src_index++;
        // don't copy more than size - dest_len - 1 characters
        if (offset == destsize - 1)
            break;
    }
    *(dest + offset) = '\0';
    return strlen(dest);
}

/*!
 * \copydoc strcomp
 */
int strcomp(const char *s1, const char *s2)
{
    int i;
    for (i = 0; UCHAR(s1[i]) == UCHAR(s2[i]); i++)
    {
        if (!s1[i + 1] && !s2[i + 1])
            return (1);
    }
    return (0);
}

/*!
 * \copydoc getTempFileName
 */
char *getTempFileName(char *fname)
{
// For Windows systems:
#ifdef OS_WINDOWS

    char *name = NULL;
    char *dir = NULL;

    // --- set dir to user's choice of a temporary directory
    if (strlen(TempDir) > 0)
    {
        if (_mkdir(TempDir) == 0 || errno == EEXIST)
            dir = TempDir;
    }

    // --- use _tempnam to get a pointer to an unused file name
    name = _tempnam(dir, "swmm");
    if (name == NULL)
        return NULL;

    // --- copy the file name to fname
    if (strlen(name) <= MAXFNAME)
        sstrncpy(fname, name, MAXFNAME);
    else
        fname = NULL;

    // --- free the pointer returned by _tempnam
    free(name);

    // --- return the new contents of fname
    return fname;

// For non-Windows systems:
#else

    // --- use system function mkstemp() to create a temporary file name
    sstrncpy(fname, "swmmXXXXXX", MAXFNAME);
    mkstemp(fname);
    return fname;

#endif
}

/*!
 * \copydoc getElapsedTime
 */
void getElapsedTime(DateTime aDate, int *days, int *hrs, int *mins)
{
    DateTime x;
    int secs;
    x = aDate - ReportStart;
    if (x <= 0.0)
    {
        *days = 0;
        *hrs = 0;
        *mins = 0;
    }
    else
    {
        *days = (int)x;
        datetime_decodeTime(x, hrs, mins, &secs);
    }
}

/*!
 * \copydoc getDateTime
 */
DateTime getDateTime(double elapsedMsec)
{
    return datetime_addSeconds(StartDateTime, (elapsedMsec + 1) / 1000.0);
}

/*!
 * \copydoc isRelativePath
 */
static int isRelativePath(const char *fname)
{
    if (strchr(fname, ':'))
        return 0;
    if (fname[0] == '\\')
        return 0;
    if (fname[0] == '/')
        return 0;
    return 1;
}

/*!
 * \copydoc getAbsolutePath
 */
void getAbsolutePath(const char *fname, char *absPath, size_t size)
{
    char *endOfDir;

    // --- case of empty file anme
    if (fname == NULL || strlen(fname) == 0)
        return;

    // --- if fname has a relative path then retrieve its full path
    if (isRelativePath(fname))
    {
#ifdef OS_WINDOWS
        GetFullPathName((LPCSTR)fname, (DWORD)size, (LPSTR)absPath, NULL);
#else
        realpath(fname, absPath);
#endif
    }

    // --- otherwise copy fname to absPath
    else
    {
        sstrncpy(absPath, fname, strlen(fname));
    }

// --- trim file name portion of absPath
#ifdef OS_WINDOWS
    endOfDir = strrchr(absPath, '\\');
#else
    endOfDir = strrchr(absPath, '/');
#endif
    if (endOfDir)
    {
        *(endOfDir + 1) = '\0';
    }
}

/*!
 * \copydoc addAbsolutePath
 */
char *addAbsolutePath(char *fname)
{
    size_t n;
    char buffer[MAXFNAME];
    if (isRelativePath(fname))
    {
        n = snprintf(buffer, MAXFNAME, "%s%s", InpDir, fname);
        if (n > 0)
            sstrncpy(fname, buffer, MAXFNAME);
    }
    return fname;
}

/*!
 * \copydoc writecon
 */
void writecon(const char *s)
{
    fprintf(stdout, "%s", s);
    fflush(stdout);
}

#ifdef EXH

/*!
 * \copydoc xfilter
 */
int xfilter(int xc, char *module, double elapsedTime, long step)
{
    int rc;         // result code
    long hour;      // current hour of simulation
    char msg[40];   // exception type text
    char xmsg[240]; // error message text
    switch (xc)
    {
    case EXCEPTION_ACCESS_VIOLATION:
        sprintf(msg, "\n  Access violation ");
        rc = EXCEPTION_EXECUTE_HANDLER;
        break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        sprintf(msg, "\n  Illegal floating point operand ");
        rc = EXCEPTION_CONTINUE_EXECUTION;
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        sprintf(msg, "\n  Floating point divide by zero ");
        rc = EXCEPTION_CONTINUE_EXECUTION;
        break;
    case EXCEPTION_FLT_INVALID_OPERATION:
        sprintf(msg, "\n  Illegal floating point operation ");
        rc = EXCEPTION_CONTINUE_EXECUTION;
        break;
    case EXCEPTION_FLT_OVERFLOW:
        sprintf(msg, "\n  Floating point overflow ");
        rc = EXCEPTION_CONTINUE_EXECUTION;
        break;
    case EXCEPTION_FLT_STACK_CHECK:
        sprintf(msg, "\n  Floating point stack violation ");
        rc = EXCEPTION_EXECUTE_HANDLER;
        break;
    case EXCEPTION_FLT_UNDERFLOW:
        sprintf(msg, "\n  Floating point underflow ");
        rc = EXCEPTION_CONTINUE_EXECUTION;
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        sprintf(msg, "\n  Integer divide by zero ");
        rc = EXCEPTION_CONTINUE_EXECUTION;
        break;
    case EXCEPTION_INT_OVERFLOW:
        sprintf(msg, "\n  Integer overflow ");
        rc = EXCEPTION_CONTINUE_EXECUTION;
        break;
    default:
        sprintf(msg, "\n  Exception %d ", xc);
        rc = EXCEPTION_EXECUTE_HANDLER;
    }
    hour = (long)(elapsedTime / 1000.0 / 3600.0);
    sprintf(xmsg, "%sin module %s at step %ld, hour %ld",
            msg, module, step, hour);
    if (rc == EXCEPTION_EXECUTE_HANDLER ||
        ++ExceptionCount >= MAX_EXCEPTIONS)
    {
        strcat(xmsg, " --- execution halted.");
        rc = EXCEPTION_EXECUTE_HANDLER;
    }
    report_writeLine(xmsg);
    return rc;
}

#endif
