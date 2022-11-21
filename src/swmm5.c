//-----------------------------------------------------------------------------
//   swmm5.c
//
//   Project:  EPA SWMM5
//   Version:  5.2
//   Date:     11/01/21  (Build 5.2.0)
//   Author:   L. Rossman
//
//   This is the main module of the computational engine for Version 5 of
//   the U.S. Environmental Protection Agency's Storm Water Management Model
//   (SWMM). It contains functions that control the flow of computations.
//
//   This engine should be compiled into a shared object library whose API
//   functions are listed in swmm5.h.
//
//   Update History
//   ==============
//   Build 5.1.008:
//   - Support added for the MinGW compiler.
//   - Reporting of project options moved to swmm_start. 
//   - Hot start file now read before routing system opened.
//   - Final routing step adjusted so that total duration not exceeded.
//   Build 5.1.011:
//   - Made sure that MS exception handling only used with MS C compiler.
//   - Added name of module handling an exception to error report.
//   - Elapsed simulation time now saved to new global variable ElaspedTime.
//   - Added swmm_getError() function that retrieves error code and message.
//   - Changed WarningCode to Warnings (# warnings issued).
//   - Added swmm_getWarnings() function to retrieve value of Warnings.
//   - Fixed error code returned on swmm_xxx functions.
//   Build 5.1.012:
//   - #include <direct.h> only used when compiled for Windows.
//   Build 5.1.013:
//   - Support added for saving average results within a reporting period.
//   - SWMM engine now always compiled to a shared object library.
//   Build 5.1.015:
//   - Fixes bug in summary statistics when Report Start date > Start Date.
//   Build 5.2.0:
//   - Added additional API functions.
//   - Set max. number of open files to 8192.
//   - Changed getElapsedTime function to use report start as base date/time.
//   - Prevented possible infinite loop if swmm_step() called when ErrorCode > 0.
//   - Prevented early exit from swmm_end() when ErrorCode > 0.
//   - Support added for relative file names.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

// --- define WINDOWS
#undef WINDOWS
#ifdef _WIN32
  #define WINDOWS
#endif
#ifdef __WIN32__
  #define WINDOWS
#endif

// --- define EXH (MS Windows exception handling)
#undef EXH         // indicates if exception handling included
#ifdef WINDOWS
  #ifdef _MSC_VER
  #define EXH
  #endif
#endif

// --- include Windows & exception handling headers
#ifdef WINDOWS
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

//-----------------------------------------------------------------------------
//  SWMM's header files
//
//  Note: the directives listed below are also contained in headers.h which
//        is included at the start of most of SWMM's other code modules.
//-----------------------------------------------------------------------------
#include "macros.h"                    // macros used throughout SWMM
#include "objects.h"                   // definitions of SWMM's data objects
#define  EXTERN                        // defined as 'extern' in headers.h
#include "globals.h"                   // declaration of all global variables
#include "funcs.h"                     // declaration of all global functions
#include "error.h"                     // error message codes
#include "text.h"                      // listing of all text strings 

#include "swmm5.h"                     // declaration of SWMM's API functions

#define  MAX_EXCEPTIONS 100            // max. number of exceptions handled

//-----------------------------------------------------------------------------
//  Unit conversion factors
//-----------------------------------------------------------------------------
const double Ucf[10][2] = 
      {//  US      SI
      {43200.0,   1097280.0 },         // RAINFALL (in/hr, mm/hr --> ft/sec)
      {12.0,      304.8     },         // RAINDEPTH (in, mm --> ft)
      {1036800.0, 26334720.0},         // EVAPRATE (in/day, mm/day --> ft/sec)
      {1.0,       0.3048    },         // LENGTH (ft, m --> ft)
      {2.2956e-5, 0.92903e-5},         // LANDAREA (ac, ha --> ft2)
      {1.0,       0.02832   },         // VOLUME (ft3, m3 --> ft3)
      {1.0,       1.608     },         // WINDSPEED (mph, km/hr --> mph)
      {1.0,       1.8       },         // TEMPERATURE (deg F, deg C --> deg F)
      {2.203e-6,  1.0e-6    },         // MASS (lb, kg --> mg)
      {43560.0,   3048.0    }          // GWFLOW (cfs/ac, cms/ha --> ft/sec)
      };
const double Qcf[6] =                  // Flow Conversion Factors:
    {1.0,     448.831, 0.64632,        // cfs, gpm, mgd --> cfs
     0.02832, 28.317,  2.4466 };       // cms, lps, mld --> cfs

//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
static int    IsOpenFlag;           // TRUE if a project has been opened
static int    IsStartedFlag;        // TRUE if a simulation has been started
static int    SaveResultsFlag;      // TRUE if output to be saved to binary file
static int    ExceptionCount;       // number of exceptions handled
static int    DoRunoff;             // TRUE if runoff is computed
static int    DoRouting;            // TRUE if flow routing is computed
static double RoutingDuration;      // duration of a set of routing steps (msecs)

//-----------------------------------------------------------------------------
//  External API functions (prototyped in swmm5.h)
//-----------------------------------------------------------------------------
//  swmm_run
//  swmm_open
//  swmm_start
//  swmm_step
//  swmm_end
//  swmm_report
//  swmm_close
//  swmm_getMassBalErr
//  swmm_getVersion
//  swmm_getError
//  swmm_getWarnings
//  swmm_getCount
//  swmm_getIDname
//  swmm_getIndex
//  swmm_getStartNode
//  swmm_getEndNode
//  swmm_getValue
//  swmm_setValue
//  swmm_getSavedValue
//  swmm_writeLine
//  swmm_decodeDate

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void   execRouting(void);
static void   saveResults(void);
static double getGageValue(int index, int property);
static double getSubcatchValue(int index, int property);
static double getNodeValue(int index, int property);
static double getLinkValue(int index, int property);
static double getSavedDate(int period);
static double getSavedSubcatchValue(int index, int property, int period);
static double getSavedNodeValue(int index, int property, int period);
static double getSavedLinkValue(int index, int property, int period);
static double getSystemValue(int property);
static double getMaxRouteStep();
static void   setNodeLatFlow(int index, double value);
static void   setOutfallStage(int index, double value);
static void   setLinkSetting(int index, double value);
static void   setRoutingStep(double value);
static void   getAbsolutePath(const char* fname, char* absPath, size_t size);

// Exception filtering function
#ifdef EXH
static int  xfilter(int xc, char* module, double elapsedTime, long step);
#endif

//=============================================================================

int DLLEXPORT  swmm_run(const char *f1, const char *f2, const char *f3)
//
//  Input:   f1 = name of input file
//           f2 = name of report file
//           f3 = name of binary output file
//  Output:  returns error code
//  Purpose: runs a SWMM simulation.
//
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
    swmm_open(f1, f2, f3);

    // --- run the simulation if input data OK
    if ( !ErrorCode )
    {
        // --- initialize values
        swmm_start(TRUE);

        // --- execute each time step until elapsed time is re-set to 0
        if ( !ErrorCode )
        {
            writecon("\n o  Simulating day: 0     hour:  0");
            do
            {
                swmm_step(&elapsedTime);
                newHour = (long)(elapsedTime * 24.0);
                if ( newHour > oldHour )
                {
                    theDay = (long)elapsedTime;
                    theHour = (long)((elapsedTime - floor(elapsedTime)) * 24.0);
                    writecon("\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                    snprintf(Msg, MAXMSG, "%-5ld hour: %-2ld", theDay, theHour);
                    writecon(Msg);
                    oldHour = newHour;
                }
            } while ( elapsedTime > 0.0 && !ErrorCode );
            writecon("\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
                     "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            writecon("Simulation complete           ");
        }

        // --- clean up
        swmm_end();
    }

    // --- report results
    if ( !ErrorCode && Fout.mode == SCRATCH_FILE )
    {
        writecon("\n o  Writing output report");
        swmm_report();
    }

    // --- close the system
    swmm_close();
    return ErrorCode;
}

//=============================================================================

int DLLEXPORT swmm_open(const char *f1, const char *f2, const char *f3)
//
//  Input:   f1 = name of input file
//           f2 = name of report file
//           f3 = name of binary output file
//  Output:  returns error code
//  Purpose: opens a SWMM project.
//
{
// --- to be safe, reset the state of the floating point unit
#ifdef WINDOWS
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
        project_open(f1, f2, f3);
        getAbsolutePath(f1, InpDir, sizeof(InpDir));
        if ( ErrorCode ) return ErrorCode;
        IsOpenFlag = TRUE;
        report_writeLogo();

        // --- retrieve project data from input file
        project_readInput();
        if ( ErrorCode ) return ErrorCode;

        // --- write project title to report file & validate data
        report_writeTitle();
        project_validate();
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except(xfilter(GetExceptionCode(), "swmm_open", 0.0, 0))
    {
        ErrorCode = ERR_SYSTEM;
    }
#endif
    return ErrorCode;
}

//=============================================================================

int DLLEXPORT swmm_start(int saveResults)
//
//  Input:   saveResults = TRUE if simulation results saved to binary file 
//  Output:  returns an error code
//  Purpose: starts a SWMM simulation.
//
{
    // --- check that a project is open & no run started
    if ( ErrorCode ) return ErrorCode;
    if ( !IsOpenFlag )
        return (ErrorCode = ERR_API_NOT_OPEN);
    if ( IsStartedFlag )
        return (ErrorCode = ERR_API_NOT_ENDED);

    // --- write input summary & project options to report file if requested
    if (!RptFlags.disabled)
    {
        if (RptFlags.input)
            inputrpt_writeInput();
        report_writeOptions();
    }

    // --- save saveResults flag to global variable
    SaveResultsFlag = saveResults;
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
        if ( !IgnoreRainfall ) rain_open();
        if ( ErrorCode ) return ErrorCode;

        // --- initialize state of each major system component
        project_init();

        // --- see if runoff & routing needs to be computed
        if ( Nobjects[SUBCATCH] > 0 ) DoRunoff = TRUE;
        else DoRunoff = FALSE;
        if ( Nobjects[NODE] > 0 && !IgnoreRouting ) DoRouting = TRUE;
        else DoRouting = FALSE;

        // --- open binary output file
        output_open();

        // --- open runoff processor
        if ( DoRunoff ) runoff_open();

        // --- open & read hot start file if present
        if ( !hotstart_open() ) return ErrorCode;

        // --- open routing processor
        if ( DoRouting ) routing_open();

        // --- open mass balance and statistics processors
        massbal_open();
        stats_open();

        // --- write heading for control actions listing 
	    if (!RptFlags.disabled && RptFlags.controls)
                report_writeControlActionsHeading();
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except(xfilter(GetExceptionCode(), "swmm_start", 0.0, 0))
    {
        ErrorCode = ERR_SYSTEM;
    }
#endif
    return ErrorCode;
}
//=============================================================================

int DLLEXPORT swmm_step(double *elapsedTime)
//
//  Input:   elapsedTime = current elapsed time in decimal days
//  Output:  updated value of elapsedTime,
//           returns error code
//  Purpose: advances the simulation by one routing time step.
//
{
    // --- check that simulation can proceed
    *elapsedTime = 0.0;
    if ( ErrorCode ) 
        return ErrorCode;
    if ( !IsOpenFlag )
        return (ErrorCode = ERR_API_NOT_OPEN);
    if ( !IsStartedFlag )
        return (ErrorCode = ERR_API_NOT_STARTED);

#ifdef EXH
    // --- begin exception handling loop here
    __try
#endif
    {
        // --- if routing time has not exceeded total duration
        if ( NewRoutingTime < RoutingDuration )
        {
            // --- route flow & WQ through drainage system
            //     (runoff will be calculated as needed)
            //     (NewRoutingTime is updated)
            execRouting();
        }

        // --- if saving results to the binary file
        if ( SaveResultsFlag )
            saveResults();

        // --- update elapsed time (days)
        if ( NewRoutingTime < RoutingDuration )
            ElapsedTime = NewRoutingTime / MSECperDAY;

        // --- otherwise end the simulation
        else ElapsedTime = 0.0;
        *elapsedTime = ElapsedTime;
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except(xfilter(GetExceptionCode(), "swmm_step", ElapsedTime, TotalStepCount))
    {
        ErrorCode = ERR_SYSTEM;
    }
#endif
    return ErrorCode;
}

//=============================================================================

int  DLLEXPORT swmm_stride(int strideStep, double *elapsedTime)
//
//  Input:   strideStep = number of seconds to advance the simulation
//           elapsedTime = current elapsed time in decimal days
//  Output:  updated value of elapsedTime,
//           returns error code
//  Purpose: advances the simulation by a fixed number of seconds.
{
    double realRouteStep = RouteStep;

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
    if (strideStep < RouteStep) RouteStep = strideStep;

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
    else ElapsedTime = 0.0;
    *elapsedTime = ElapsedTime;
    return ErrorCode;
}

//=============================================================================

void execRouting()
//
//  Input:   none
//  Output:  none
//  Purpose: routes flow & WQ through drainage system over a single time step.
//
{
    double   nextRoutingTime;          // updated elapsed routing time (msec)
    double   routingStep;              // routing time step (sec)

#ifdef EXH
    // --- begin exception handling loop here
    __try
#endif
    {
        // --- determine when next routing time occurs
        TotalStepCount++;
        if ( !DoRouting ) routingStep = MIN(WetStep, ReportStep);
        else routingStep = routing_getRoutingStep(RouteModel, RouteStep);
        if ( routingStep <= 0.0 )
        {
            ErrorCode = ERR_TIMESTEP;
            return;
        }
        nextRoutingTime = NewRoutingTime + 1000.0 * routingStep;

        // --- adjust routing step so that total duration not exceeded
        if ( nextRoutingTime > RoutingDuration )
        {
            routingStep = (RoutingDuration - NewRoutingTime) / 1000.0;
            routingStep = MAX(routingStep, 1. / 1000.0);
            nextRoutingTime = RoutingDuration;
        }

        // --- compute runoff until next routing time reached or exceeded
        if ( DoRunoff ) while ( NewRunoffTime < nextRoutingTime)
        {
            runoff_execute();
            if ( ErrorCode ) return;
        }

        // --- if no runoff analysis, update climate state (for evaporation)
        else climate_setState(getDateTime(NewRoutingTime));
  
        // --- route flows & pollutants through drainage system
        //     (while updating NewRoutingTime)
        if ( DoRouting )
            routing_execute(RouteModel, routingStep);
        else
            NewRoutingTime = nextRoutingTime;
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except(xfilter(GetExceptionCode(), "execRouting",
                     ElapsedTime, TotalStepCount))
    {
        ErrorCode = ERR_SYSTEM;
        return;
    }
#endif
}

//=============================================================================

void saveResults()
//
//  Input:   none
//  Output:  none
//  Purpose: saves current results to binary output file.
{
    if (NewRoutingTime >= ReportTime)
    {
        // --- if user requested that average results be saved:
        if (RptFlags.averages)
        {
            // --- include latest results in current averages
            //     if current time equals the reporting time
            if (NewRoutingTime == ReportTime) output_updateAvgResults();

            // --- save current average results to binary file
            //     (which will re-set averages to 0)
            output_saveResults(ReportTime);

            // --- if current time exceeds reporting period then
            //     start computing averages for next period
            if (NewRoutingTime > ReportTime) output_updateAvgResults();
        }

        // --- otherwise save interpolated point results
        else output_saveResults(ReportTime);

        // --- advance to next reporting period
        ReportTime = ReportTime + 1000 * (double)ReportStep;
    }

    // --- not a reporting period so update average results if applicable
    else if (RptFlags.averages) output_updateAvgResults();

}


//=============================================================================

int DLLEXPORT swmm_end(void)
//
//  Input:   none
//  Output:  none
//  Purpose: ends a SWMM simulation.
//
{
    // --- check that project opened and run started
    if ( !IsOpenFlag )
        return (ErrorCode = ERR_API_NOT_OPEN);

    if ( IsStartedFlag )
    {
        // --- write ending records to binary output file
        if ( Fout.file ) output_end();

        // --- report mass balance results and system statistics
        if ( !ErrorCode && RptFlags.disabled == 0 )
        {
            massbal_report();
            stats_report();
        }

        // --- close all computing systems
        stats_close();
        massbal_close();
        if ( !IgnoreRainfall ) rain_close();
        if ( DoRunoff ) runoff_close();
        if ( DoRouting ) routing_close(RouteModel);
        hotstart_close();
        IsStartedFlag = FALSE;
    }
    return ErrorCode;
}

//=============================================================================

int DLLEXPORT swmm_report()
//
//  Input:   none
//  Output:  returns an error code
//  Purpose: writes simulation results to the report file.
//
{
    if ( !ErrorCode )
        report_writeReport();
    return ErrorCode;
}

//=============================================================================

void  DLLEXPORT swmm_writeLine(const char *line)
//
//  Input:   line = a character string
//  Output:  returns an error code
//  Purpose: writes a line of text to the report file.
//
{
    if (IsOpenFlag)
        report_writeLine(line);
}

//=============================================================================

int DLLEXPORT swmm_close()
//
//  Input:   none
//  Output:  returns an error code
//  Purpose: closes a SWMM project.
//
{
    if ( Fout.file ) output_close();
    if ( IsOpenFlag ) project_close();
    report_writeSysTime();
    if ( Finp.file != NULL )
        fclose(Finp.file);
    if ( Frpt.file != NULL )
        fclose(Frpt.file);
    if ( Fout.file != NULL )
    {
        fclose(Fout.file);
        if ( Fout.mode == SCRATCH_FILE ) remove(Fout.name);
    }
    IsOpenFlag = FALSE;
    IsStartedFlag = FALSE;
    return 0;
}

//=============================================================================

int  DLLEXPORT swmm_getMassBalErr(float *runoffErr, float *flowErr,
                                  float *qualErr)
//
//  Input:   none
//  Output:  runoffErr = runoff mass balance error (percent)
//           flowErr   = flow routing mass balance error (percent)
//           qualErr   = quality routing mass balance error (percent)
//           returns an error code
//  Purpose: reports a simulation's mass balance errors.
//
{
    *runoffErr = 0.0;
    *flowErr   = 0.0;
    *qualErr   = 0.0;

    if ( IsOpenFlag && !IsStartedFlag)
    {
        *runoffErr = (float)RunoffError;
        *flowErr   = (float)FlowError;
        *qualErr   = (float)QualError;
    }
    return 0;
}

//=============================================================================

int  DLLEXPORT swmm_getVersion()
//
//  Input:   none
//  Output:  returns SWMM engine version number
//  Purpose: retrieves version number of current SWMM engine which
//           uses a format of xyzzz where x = major version number,
//           y = minor version number, and zzz = build number.
//
//  NOTE: Each New Release should be updated in consts.h
{
    return VERSION;
}

//=============================================================================

int DLLEXPORT swmm_getWarnings()
//
//  Input:  none
//  Output: returns number of warning messages issued.
//  Purpose: retrieves number of warning messages issued during an analysis.
{
    return Warnings;
}

//=============================================================================

int  DLLEXPORT swmm_getError(char *errMsg, int msgLen)
//
//  Input:   errMsg = character array to hold error message text
//           msgLen = maximum size of errMsg
//  Output:  returns error message code number and text of error message.
//  Purpose: retrieves the code number and text of the error condition that
//           caused SWMM to abort its analysis.
{
    // --- copy text of last error message into errMsg
    if (ErrorCode > 0 && strlen(ErrorMsg) == 0)
        error_getMsg(ErrorCode, ErrorMsg);
    sstrncpy(errMsg, ErrorMsg, msgLen);

    // --- remove leading line feed from errMsg
    if ( msgLen > 0 && errMsg[0] == '\n' ) errMsg[0] = ' ';
    return ErrorCode;
}

//=============================================================================

int  DLLEXPORT swmm_getCount(int objType)
//
//  Input:   objType = a type of SWMM object
//  Output:  returns the number of objects;
//  Purpose: retrieves the number of objects of a specific type.
{
    if (!IsOpenFlag)
        return 0;
    if (objType < swmm_GAGE || objType > swmm_LINK)
        return 0;
    return Nobjects[objType];
}

//=============================================================================

void  DLLEXPORT swmm_getName(int objType, int index, char *name, int size)
//
//  Input:   objType = a type of SWMM object
//           index = the object's index in the array of objects
//           name = a character array
//           size = size of the name array
//  Output:  name = the object's ID name;
//  Purpose: retrieves the ID name of an object.
{
    char *idName = NULL;

    name[0] = '\0';
    if (!IsOpenFlag)
        return;
    if (objType < swmm_GAGE || objType > swmm_LINK)
        return;
    if (index < 0 || index >= Nobjects[objType])
        return;
    switch (objType)
    {
        case GAGE:     idName = Gage[index].ID;     break;
        case SUBCATCH: idName = Subcatch[index].ID; break;
        case NODE:     idName = Node[index].ID;     break;
        case LINK:     idName = Link[index].ID;     break;
    }
    if (idName)
        sstrncpy(name, idName, size);
}

//=============================================================================

int  DLLEXPORT swmm_getIndex(int objType, const char *name)
//
//  Input:   objType = a type of SWMM object
//           name = the object's ID name
//  Output:  returns the object's position in the array of like objects;
//  Purpose: retrieves the index of a named object.
{
    if (!IsOpenFlag)
        return -1;
    if (objType < swmm_GAGE || objType > swmm_LINK)
        return -1;
    return project_findObject(objType, name);
}

//=============================================================================

double  DLLEXPORT swmm_getValue(int property, int index)
//
//  Input:   property = an object's property code
//           index = the object's index in the array of like objects
//           
//  Output:  returns the property's current value
//  Purpose: retrieves the value of an object's property.
{
    if (!IsOpenFlag)
        return 0;
    if (property < 100)
        return getSystemValue(property);
    if (property < 200)
        return getGageValue(property, index);
    if (property < 300)
        return getSubcatchValue(property, index);
    if (property < 400)
        return getNodeValue(property, index);
    if (property < 500)
        return getLinkValue(property, index);
    return 0;
}

//=============================================================================

void  DLLEXPORT swmm_setValue(int property, int index, double value)
//
//  Input:   property = an object's property code
//           index = the object's index in the array of like objects
//           value = the property's new value
//  Output:  none
//  Purpose: sets the value of an object's property.
{
    if (!IsOpenFlag)
        return;
    switch (property)
    {
    case swmm_GAGE_RAINFALL:
        if (index < 0 || index >= Nobjects[GAGE])
            return;
        if (value >= 0.0)
            Gage[index].apiRainfall = value;
        return;
    case swmm_SUBCATCH_RPTFLAG:
        if (!IsStartedFlag && index >= 0 && index < Nobjects[SUBCATCH])
            Subcatch[index].rptFlag = (value > 0.0);
        return;            
    case swmm_NODE_LATFLOW:
        setNodeLatFlow(index, value);
        return;
    case swmm_NODE_HEAD:
        setOutfallStage(index, value);
        return;
    case swmm_NODE_RPTFLAG:
        if (!IsStartedFlag && index >= 0 && index < Nobjects[NODE])
            Node[index].rptFlag = (value > 0.0);
        return;            
    case swmm_LINK_SETTING:
        setLinkSetting(index, value);
        return;
    case swmm_LINK_RPTFLAG:
        if (!IsStartedFlag && index >= 0 && index < Nobjects[LINK])
            Link[index].rptFlag = (value > 0.0);
        return;            
    case swmm_ROUTESTEP:
        setRoutingStep(value);
        return;
    case swmm_REPORTSTEP:
        if (!IsStartedFlag && value > 0)
            ReportStep = (int)value;                
        return;
    case swmm_NOREPORT:
        if (!IsStartedFlag)
            RptFlags.disabled = (value > 0.0);
        return;
    }
}

//=============================================================================

double  DLLEXPORT swmm_getSavedValue(int property, int index, int period)
//
//  Input:   property = an object's property code
//           index = the object's index in the array of like objects
//           period = a reporting time period (starting from 1) 
//  Output:  returns the property's saved value 
//  Purpose: retrieves an object's computed value at a specific reporting time period.
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

//=============================================================================

void  DLLEXPORT swmm_decodeDate(double date, int *year, int *month, int *day,
      int *hour, int *minute, int *second, int *dayOfWeek)
//
//  Input:  date = an encoded date in decimal days
//  Output: date's year, month of year, day of month, time of day (hour,
//           minute, second), and day of weeek
//  Purpose: retrieves the calendar date and clock time of an encoded date.
{
    datetime_decodeDate(date, year, month, day);
    datetime_decodeTime(date, hour, minute, second);
    *dayOfWeek = datetime_dayOfWeek(date);
}

//=============================================================================
//   Object property getters and setters
//=============================================================================

double getGageValue(int property, int index)
//
//  Input:   property = a rain gage property code
//           index = the index of a rain gage
//  Output:  returns current property value
//  Purpose: retrieves current value of a rain gage property.
{
    if (index < 0 || index >= Nobjects[GAGE])
        return 0;
    if (property == swmm_GAGE_RAINFALL)
        return Gage[index].reportRainfall;
    return 0;
}

//=============================================================================

double getSubcatchValue(int property, int index)
//
//  Input:   property = a subcatchment property code
//           index = the index of a subcatchment
//  Output:  returns current property value
//  Purpose: retrieves current value of a subcatchment's property.
{
    TSubcatch* subcatch;
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
            if ( subcatch->gage >= 0 )
                return Gage[subcatch->gage].reportRainfall;
            else
                return 0.0;
        case swmm_SUBCATCH_EVAP:
          return subcatch->evapLoss * UCF(EVAPRATE);
        case swmm_SUBCATCH_INFIL:
          return subcatch->infilLoss * UCF(RAINFALL);
        case swmm_SUBCATCH_RUNOFF:
          return subcatch->newRunoff * UCF(FLOW);
        case swmm_SUBCATCH_RPTFLAG:
          return (subcatch->rptFlag > 0);
        default:
          return 0;
    }
}

//=============================================================================

double getNodeValue(int property, int index)
//
//  Input:   property = a node property code
//           index = the index of a node
//  Output:  returns current property value
//  Purpose: retrieves current value of a node's property.
{
    TNode* node;
    if (index < 0 || index >= Nobjects[NODE])
        return 0;
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
        default:
          return 0;
    }
}

//=============================================================================

double getLinkValue(int property, int index)
//
//  Input:   property = a link property code
//           index = the index of a link
//  Output:  returns current property value
//  Purpose: retrieves current value of a link's property.
{
    TLink* link;
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
              return 0;
        case swmm_LINK_SLOPE:
          if (link->type == CONDUIT)
              return Conduit[link->subIndex].slope;
          else
              return 0;
          break;
        case swmm_LINK_FULLDEPTH:
          return link->xsect.yFull * UCF(LENGTH);
        case swmm_LINK_FULLFLOW:
          return link->qFull * UCF(FLOW);
        case swmm_LINK_FLOW:
          return link->newFlow * UCF(FLOW) * (double)link->direction;
        case swmm_LINK_VELOCITY:
          return link_getVelocity(index, fabs(link->newFlow), link->newDepth)
              * UCF(LENGTH);
        case swmm_LINK_DEPTH:
          return link->newDepth * UCF(LENGTH);
        case swmm_LINK_TOPWIDTH:
          if (link->type == CONDUIT)
              return xsect_getWofY(&link->xsect, link->newDepth) * UCF(LENGTH);
          else
              return 0;
        case swmm_LINK_SETTING:
          return link->setting;
        case swmm_LINK_TIMEOPEN:
          if (link->setting > 0.0)
              return (getDateTime(NewRoutingTime) - link->timeLastSet) * 24.;
          else
              return 0;
        case swmm_LINK_TIMECLOSED:
          if (link->setting == 0.0)
              return (getDateTime(NewRoutingTime) - link->timeLastSet) * 24.;
          else
              return 0;
        case swmm_LINK_RPTFLAG:
          return (link->rptFlag > 0);
        default:
          return 0;
    }
}

//=============================================================================

double getSystemValue(int property)
//
//  Input:   property = a system property code
//  Output:  returns current property value
//  Purpose: retrieves current value of a system property.
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
        default:
          return 0;
    }
}

//=============================================================================

void  setNodeLatFlow(int index, double value)
//
//  Input:   index = the index of a node
//           value = the node's external inflow value
//  Output:  none
//  Purpose: sets the value of a node's external inflow.
{
    if (index < 0 || index >= Nobjects[NODE])
        return;
    Node[index].apiExtInflow = value / UCF(FLOW);
}

//=============================================================================

void  setOutfallStage(int index, double value)
//
//  Input:   index = the index of an outfall node
//           value = the outfall's fixed stage elevation
//  Output:  none
//  Purpose: sets the value of an outfall node's fixed stage.
{
    TNode* node;
    if (index < 0 || index >= Nobjects[NODE])
        return;
    node = &Node[index];
    if (node->type != OUTFALL)
        return;
    Outfall[node->subIndex].fixedStage = value / UCF(LENGTH);
    Outfall[node->subIndex].type = FIXED_OUTFALL;
}

//=============================================================================

void  setLinkSetting(int index, double value)
//
//  Input:   index = the index of a link
//           value = the link's new setting
//  Output:  node
//  Purpose: sets the value of a link's setting.
{
    TLink* link;
    if (index < 0 || index >= Nobjects[LINK])
        return;
    link = &Link[index];
    if (value < 0.0  || link->type == CONDUIT)
        return;
    if (link->type != PUMP && value > 1.0)
        value = 1.0;
    if (link->targetSetting == value)
        return;
    link->targetSetting = value;
    if (link->targetSetting * link->setting == 0.0)
        link->timeLastSet = StartDateTime + ElapsedTime;
    link_setSetting(index, 0.0);
}

//=============================================================================

double getSavedDate(int period)
//
//  Input:   period = a reporting period (starting at 1)
//  Output:  returns the date/time of the reporting period in decimal days
//  Purpose: retrieves the date/time of a reporting period. 
{
    double days;
    output_readDateTime(period, &days);
    return days;
}

//=============================================================================

double  getSavedSubcatchValue(int property, int index, int period)
//
//  Input:   property = index of a computed property
//           index = index of a subcatchment
//           period = a reporting period (starting at 1)
//  Output:  returns the property's value at the recording period
//  Purpose: retrieves the computed value of a subcatchment property at a
//           specific reporting period.
{
    // --- SubcatchResults array is defined in output.c and contains
    //     computed results in user's units
    extern float* SubcatchResults;

    // --- order in which subcatchment was saved to output results file
    int outIndex = Subcatch[index].rptFlag - 1;
    if (outIndex < 0) return 0;

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

//=============================================================================

double  getSavedNodeValue(int property, int index, int period)
//
//  Input:   property = index of a computed property
//           index = index of a node
//           period = a reporting period (starting at 1)
//  Output:  returns the property's value at the recording period
//  Purpose: retrieves the computed value of a node property at a
//           specific reporting period.
{
    // --- NodeResults array is defined in output.c and contains
    //     computed results in user's units
    extern float* NodeResults;

    // --- order in which node was saved to output results file
    int outIndex = Node[index].rptFlag - 1;
    if (outIndex < 0) return 0;

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

//=============================================================================

double  getSavedLinkValue(int property, int index, int period)
//
//  Input:   property = index of a computed property
//           index = index of a link
//           period = a reporting period (starting at 1)
//  Output:  returns the property's value at the recording period
//  Purpose: retrieves the computed value of a link property at a
//           specific reporting period.
{
    double y, w;

    // --- LinkResults array is defined in output.c and contains
    //     computed results in user's units
    extern float* LinkResults;

    // --- order in which link was saved to output results file
    int    outIndex = Link[index].rptFlag - 1;
    if (outIndex < 0) return 0;

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

//=============================================================================

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

//=============================================================================

void  setRoutingStep(double value)
//
//  Input:   value = a routing time step (in decimal seconds)
//  Output:  none
//  Purpose: sets the value of the current flow routing time step.
{
    if (value <= 0.0)
        return;
    if (value <= MinRouteStep)
        value = MinRouteStep;
    CourantFactor = 0.0;
    RouteStep = value;
}

//=============================================================================
//   General purpose functions
//=============================================================================

double UCF(int u)
//
//  Input:   u = integer code of quantity being converted
//  Output:  returns a units conversion factor
//  Purpose: computes a conversion factor from SWMM's internal
//           units to user's units
//
{
    if ( u < FLOW ) return Ucf[u][UnitSystem];
    else            return Qcf[FlowUnits];
}

//=============================================================================

size_t sstrncpy(char *dest, const char *src, size_t n)
//
//  Input:   dest = string to be copied to
//           src = string to be copied from
//           n = number of bytes to copy
//  Output:  returns the size of dest
//  Purpose: better version of standard strncpy function
//
{
    int offset = 0;
    if (n > 0)
    {
        while (*(src + offset) != '\0')
        {
            if (offset == n)
                break;
            *(dest + offset) = *(src + offset);
            offset++;
        }
    }
    *(dest + offset) = '\0';
    return strlen(dest);
}

//=============================================================================

size_t sstrcat(char* dest, const char* src, size_t size)
//
//  Input:   dest = string to be appended
//           src = string to append to dest
//           size = allocated size of dest (including nul terminator)
//  Output:  returns new size of dest
//  Purpose: safe version of standard strcat function
//
{
    size_t dest_len, src_len, offset, src_index;

    // obtain initial sizes
    dest_len = strlen(dest);
    src_len = strlen(src);

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
        if (offset == size - 1)
            break;
    }
    *(dest + offset) = '\0';
    return strlen(dest);
}

//=============================================================================

int  strcomp(const char *s1, const char *s2)
//
//  Input:   s1 = a character string
//           s2 = a character string
//  Output:  returns 1 if s1 is same as s2, 0 otherwise
//  Purpose: does a case insensitive comparison of two strings.
//
{
    int i;
    for (i = 0; UCHAR(s1[i]) == UCHAR(s2[i]); i++)
    {
        if (!s1[i+1] && !s2[i+1]) return(1);
    }
    return(0);
}

//=============================================================================

char* getTempFileName(char* fname)
//
//  Input:   fname = file name string (with max size of MAXFNAME)
//  Output:  returns pointer to file name
//  Purpose: creates a temporary file name with path prepended to it.
//
{
// For Windows systems:
#ifdef WINDOWS

    char* name = NULL;
    char* dir = NULL;

    // --- set dir to user's choice of a temporary directory
    if (strlen(TempDir) > 0)
    {
        if (_mkdir(TempDir) == 0 || errno == EEXIST)
            dir = TempDir;
    }

    // --- use _tempnam to get a pointer to an unused file name
    name = _tempnam(dir, "swmm");
    if (name == NULL) return NULL;

    // --- copy the file name to fname
    if (strlen(name) <= MAXFNAME) sstrncpy(fname, name, MAXFNAME);
    else fname = NULL;

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

//=============================================================================

void getElapsedTime(DateTime aDate, int* days, int* hrs, int* mins)
//
//  Input:   aDate = simulation calendar date + time
//  Output:  days, hrs, mins = elapsed days, hours & minutes for aDate
//  Purpose: finds elapsed simulation time for a given calendar date
//
{
    DateTime x;
    int secs;
    x = aDate - ReportStart;
    if ( x <= 0.0 )
    {
        *days = 0;
        *hrs  = 0;
        *mins = 0;
    }
    else
    {
        *days = (int)x;
        datetime_decodeTime(x, hrs, mins, &secs);
    }
}

//=============================================================================

DateTime getDateTime(double elapsedMsec)
//
//  Input:   elapsedMsec = elapsed milliseconds
//  Output:  returns date/time value
//  Purpose: finds calendar date/time value for elapsed milliseconds of
//           simulation time.
//
{
    return datetime_addSeconds(StartDateTime, (elapsedMsec+1)/1000.0);
}

//=============================================================================

int  isRelativePath(const char* fname)
//
//  Input:   fname = a file name
//  Output:  returns 1 if fname's path is relative or 0 if absolute
//  Purpose: determines if a file name contains a relative or absolute path.
//
{
    if (strchr(fname, ':')) return 0;
    if (fname[0] == '\\') return 0;
    if (fname[0] == '/') return 0;
    return 1;
}

//=============================================================================

void getAbsolutePath(const char* fname, char* absPath, size_t size)
//
//  Input:   fname = a file name
//           absPath = string to hold the absolute path
//           size = max. size of absPath
//  Output:  absPath = string containing absolute path of fname
//                     (including ending path delimiter)
//  Purpose: finds the full path of the directory for file fname
//
{
    char* endOfDir;

    // --- case of empty file anme
    if (fname == NULL || strlen(fname) == 0)
        return;

    // --- if fname has a relative path then retrieve its full path
    if (isRelativePath(fname))
    {
        #ifdef WINDOWS
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
    #ifdef WINDOWS
        endOfDir = strrchr(absPath, '\\');
    #else
        endOfDir = strrchr(absPath, '/');
    #endif
    if (endOfDir)
    {
        *(endOfDir+1) =  '\0';
    }
}

//=============================================================================

char* addAbsolutePath(char* fname)
//
//  Input:   fname = a file name
//  Output:  returns fname with a full path prepended to it
//  Purpose: adds an absolute path name to a file name.
//  Note:    fname must have been dimensioned to accept MAXFNAME characters.
//
{
    size_t  n;
    char buffer[MAXFNAME];
    if (isRelativePath(fname))
    {
        n = snprintf(buffer, MAXFNAME, "%s%s", InpDir, fname);
        if (n > 0)
            sstrncpy(fname, buffer, MAXFNAME);
    }
    return fname;
}

//=============================================================================

void  writecon(const char *s)
//
//  Input:   s = a character string
//  Output:  none
//  Purpose: writes string of characters to the console.
//
{
    fprintf(stdout,"%s",s);
    fflush(stdout);
}

//=============================================================================

#ifdef EXH
int xfilter(int xc, char* module, double elapsedTime, long step)
//
//  Input:   xc          = exception code
//           module      = name of code module where exception was handled
//           elapsedTime = simulation time when exception occurred (days)
//           step        = step count at time when exception occurred
//  Output:  returns an exception handling code
//  Purpose: exception filtering routine for operating system exceptions
//           under Windows and the Microsoft C compiler.
//
{
    int  rc;                           // result code
    long hour;                         // current hour of simulation
    char msg[40];                      // exception type text
    char xmsg[240];                    // error message text
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
    if ( rc == EXCEPTION_EXECUTE_HANDLER ||
         ++ExceptionCount >= MAX_EXCEPTIONS )
    {
        strcat(xmsg, " --- execution halted.");
        rc = EXCEPTION_EXECUTE_HANDLER;
    }
    report_writeLine(xmsg);
    return rc;
}
#endif
