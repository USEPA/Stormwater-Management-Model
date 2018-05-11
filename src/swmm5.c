//-----------------------------------------------------------------------------
//   swmm5.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/19/14  (Build 5.1.001)
//             03/19/15  (Build 5.1.008)
//             08/01/16  (Build 5.1.011)
//             03/14/17  (Build 5.1.012)
//             05/10/18  (Build 5.1.013)
//   Author:   L. Rossman
//
//   This is the main module of the computational engine for Version 5 of
//   the U.S. Environmental Protection Agency's Storm Water Management Model
//   (SWMM). It contains functions that control the flow of computations.
//
//   This engine should be compiled into a shared object library whose API
//   functions are listed in swmm5.h.
//
//   Build 5.1.008:
//   - Support added for the MinGW compiler.
//   - Reporting of project options moved to swmm_start. 
//   - Hot start file now read before routing system opened.
//   - Final routing step adjusted so that total duration not exceeded.
//
//   Build 5.1.011:
//   - Made sure that MS exception handling only used with MS C compiler.
//   - Added name of module handling an exception to error report.
//   - Elapsed simulation time now saved to new global variable ElaspedTime.
//   - Added swmm_getError() function that retrieves error code and message.
//   - Changed WarningCode to Warnings (# warnings issued).
//   - Added swmm_getWarnings() function to retrieve value of Warnings.
//   - Fixed error code returned on swmm_xxx functions.
//
//   Build 5.1.012:
//   - #include <direct.h> only used when compiled for Windows.
//
//   Build 5.1.013:
//   - Support added for saving average results within a reporting period.
//   - SWMM engine now always compiled to a shared object library.
//     
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
#include "consts.h"                    // defined constants
#include "macros.h"                    // macros used throughout SWMM
#include "enums.h"                     // enumerated variables
#include "error.h"                     // error message codes
#include "datetime.h"                  // date/time functions
#include "objects.h"                   // definitions of SWMM's data objects
#include "funcs.h"                     // declaration of all global functions
#include "text.h"                      // listing of all text strings 
#define  EXTERN                        // defined as 'extern' in headers.h
#include "globals.h"                   // declaration of all global variables

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
static int  IsOpenFlag;           // TRUE if a project has been opened
static int  IsStartedFlag;        // TRUE if a simulation has been started
static int  SaveResultsFlag;      // TRUE if output to be saved to binary file
static int  ExceptionCount;       // number of exceptions handled
static int  DoRunoff;             // TRUE if runoff is computed
static int  DoRouting;            // TRUE if flow routing is computed

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

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void execRouting(void);

// Exception filtering function
#ifdef EXH
static int  xfilter(int xc, char* module, double elapsedTime, long step);
#endif

//=============================================================================

int DLLEXPORT  swmm_run(char* f1, char* f2, char* f3)
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

    // --- initialize flags                                                    //(5.1.013)
    IsOpenFlag = FALSE;                                                        //
    IsStartedFlag = FALSE;                                                     //
    SaveResultsFlag = TRUE;                                                    //

    // --- open the files & read input data
    ErrorCode = 0;
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
                    sprintf(Msg, "%-5ld hour: %-2ld", theDay, theHour);        //(5.1.013)
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
    if ( Fout.mode == SCRATCH_FILE ) swmm_report();

    // --- close the system
    swmm_close();
    return error_getCode(ErrorCode);
}

//=============================================================================

int DLLEXPORT swmm_open(char* f1, char* f2, char* f3)
//
//  Input:   f1 = name of input file
//           f2 = name of report file
//           f3 = name of binary output file
//  Output:  returns error code
//  Purpose: opens a SWMM project.
//
{
// --- to be safe, reset the state of the floating point unit                  //(5.1.013)
#ifdef WINDOWS                                                                 //(5.1.013)
    _fpreset();
#endif

#ifdef EXH
    // --- begin exception handling here
    __try
#endif
    {
        // --- initialize error & warning codes
        datetime_setDateFormat(M_D_Y);
        ErrorCode = 0;
        strcpy(ErrorMsg, "");
        Warnings = 0;
        IsOpenFlag = FALSE;
        IsStartedFlag = FALSE;
        ExceptionCount = 0;

        // --- open a SWMM project
        project_open(f1, f2, f3);
        if ( ErrorCode ) return error_getCode(ErrorCode);
        IsOpenFlag = TRUE;
        report_writeLogo();
        writecon(FMT06);

        // --- retrieve project data from input file
        project_readInput();
        if ( ErrorCode ) return error_getCode(ErrorCode);

        // --- write project title to report file & validate data
        report_writeTitle();
        project_validate();

        // --- write input summary to report file if requested
        if ( RptFlags.input ) inputrpt_writeInput();
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except(xfilter(GetExceptionCode(), "swmm_open", 0.0, 0))
    {
        ErrorCode = ERR_SYSTEM;
    }
#endif
    return error_getCode(ErrorCode);
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
    if ( ErrorCode ) return error_getCode(ErrorCode);
    if ( !IsOpenFlag || IsStartedFlag )
    {
        report_writeErrorMsg(ERR_NOT_OPEN, "");
        return error_getCode(ErrorCode);
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

        // --- initialize runoff, routing & reporting time (in milliseconds)
        NewRunoffTime = 0.0;
        NewRoutingTime = 0.0;
        ReportTime =   (double)(1000 * ReportStep);
        StepCount = 0;
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
        if ( ErrorCode ) return error_getCode(ErrorCode);

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

        // --- write project options to report file 
	    report_writeOptions();
        if ( RptFlags.controls ) report_writeControlActionsHeading();
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except(xfilter(GetExceptionCode(), "swmm_start", 0.0, 0))
    {
        ErrorCode = ERR_SYSTEM;
    }
#endif
    return error_getCode(ErrorCode);
}
//=============================================================================

int DLLEXPORT swmm_step(double* elapsedTime)
//
//  Input:   elapsedTime = current elapsed time in decimal days
//  Output:  updated value of elapsedTime,
//           returns error code
//  Purpose: advances the simulation by one routing time step.
//
{
    // --- check that simulation can proceed
    if ( ErrorCode ) return error_getCode(ErrorCode);
    if ( !IsOpenFlag || !IsStartedFlag  )
    {
        report_writeErrorMsg(ERR_NOT_OPEN, "");
        return error_getCode(ErrorCode);
    }

#ifdef EXH
    // --- begin exception handling loop here
    __try
#endif
    {
        // --- if routing time has not exceeded total duration
        if ( NewRoutingTime < TotalDuration )
        {
            // --- route flow & WQ through drainage system
            //     (runoff will be calculated as needed)
            //     (NewRoutingTime is updated)
            execRouting();
        }

////  Following code segment modified for release 5.1.013.  ////               //(5.1.013)
        // --- if saving results to the binary file
        if ( SaveResultsFlag )
        {
            // --- and it's time to save results
            if ( NewRoutingTime >= ReportTime )
            {
                // --- if user requested that average results be saved:
                if ( RptFlags.averages )
                {
                    // --- include latest results in current averages
                    //     if current time equals the reporting time
                    if ( NewRoutingTime == ReportTime ) output_updateAvgResults();

                    // --- save current average results to binary file
                    //     (which will re-set averages to 0)
                    output_saveResults(ReportTime);

                    // --- if current time exceeds reporting period then
                    //     start computing averages for next period
                    if ( NewRoutingTime > ReportTime ) output_updateAvgResults();
                }

                // --- otherwise save interpolated point results
                else output_saveResults(ReportTime);

                // --- advance to next reporting period
                ReportTime = ReportTime + (double)(1000 * ReportStep);
            }

            // --- not a reporting period so update average results if applicable
            else if ( RptFlags.averages ) output_updateAvgResults();
        }
////

        // --- update elapsed time (days)
        if ( NewRoutingTime < TotalDuration )
        {
            ElapsedTime = NewRoutingTime / MSECperDAY;
        }

        // --- otherwise end the simulation
        else ElapsedTime = 0.0;
        *elapsedTime = ElapsedTime;
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except(xfilter(GetExceptionCode(), "swmm_step", ElapsedTime, StepCount))
    {
        ErrorCode = ERR_SYSTEM;
    }
#endif
    return error_getCode(ErrorCode);
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
        StepCount++;
        if ( !DoRouting ) routingStep = MIN(WetStep, ReportStep);
        else routingStep = routing_getRoutingStep(RouteModel, RouteStep);
        if ( routingStep <= 0.0 )
        {
            ErrorCode = ERR_TIMESTEP;
            return;
        }
        nextRoutingTime = NewRoutingTime + 1000.0 * routingStep;

        // --- adjust routing step so that total duration not exceeded
        if ( nextRoutingTime > TotalDuration )
        {
            routingStep = (TotalDuration - NewRoutingTime) / 1000.0;
            routingStep = MAX(routingStep, 1./1000.0);
            nextRoutingTime = TotalDuration;
        }

        // --- compute runoff until next routing time reached or exceeded
        if ( DoRunoff ) while ( NewRunoffTime < nextRoutingTime )
        {
            runoff_execute();
            if ( ErrorCode ) return;
        }

        // --- if no runoff analysis, update climate state (for evaporation)
        else climate_setState(getDateTime(NewRoutingTime));
  
        // --- route flows & pollutants through drainage system
        //     (while updating NewRoutingTime)
        if ( DoRouting ) routing_execute(RouteModel, routingStep);
        else
        NewRoutingTime = nextRoutingTime;
    }

#ifdef EXH
    // --- end of try loop; handle exception here
    __except(xfilter(GetExceptionCode(), "execRouting",
                     ElapsedTime, StepCount))
    {
        ErrorCode = ERR_SYSTEM;
        return;
    }
#endif
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
    {
        report_writeErrorMsg(ERR_NOT_OPEN, "");
        return error_getCode(ErrorCode);
    }

    if ( IsStartedFlag )
    {
        // --- write ending records to binary output file
        if ( Fout.file ) output_end();

        // --- report mass balance results and system statistics
        if ( !ErrorCode )
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
    return error_getCode(ErrorCode);
}

//=============================================================================

int DLLEXPORT swmm_report()
//
//  Input:   none
//  Output:  returns an error code
//  Purpose: writes simulation results to report file.
//
{
    if ( Fout.mode == SCRATCH_FILE ) output_checkFileSize();
    if ( ErrorCode ) report_writeErrorCode();
    else
    {
        writecon(FMT07);
        report_writeReport();
    }
    return error_getCode(ErrorCode);
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
    if ( Finp.file != NULL ) fclose(Finp.file);
    if ( Frpt.file != NULL ) fclose(Frpt.file);
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

int  DLLEXPORT swmm_getMassBalErr(float* runoffErr, float* flowErr,
                                  float* qualErr)
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

int  DLLEXPORT swmm_getVersion(void)
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

int DLLEXPORT swmm_getWarnings(void)
//
//  Input:  none
//  Output: returns number of warning messages issued.
//  Purpose: retireves number of warning messages issued during an analysis.
{
    return Warnings;
}

//=============================================================================

int  DLLEXPORT swmm_getError(char* errMsg, int msgLen)
//
//  Input:   errMsg = character array to hold error message text
//           msgLen = maximum size of errMsg
//  Output:  returns error message code number and text of error message.
//  Purpose: retrieves the code number and text of the error condition that
//           caused SWMM to abort its analysis.
{
    size_t errMsgLen = msgLen;

    // --- copy text of last error message into errMsg
    if ( ErrorCode > 0 && strlen(ErrorMsg) == 0 ) sstrncpy(errMsg, "", 1);
    else
    {
        errMsgLen = MIN(errMsgLen, strlen(ErrorMsg));
        errMsg = sstrncpy(errMsg, ErrorMsg, errMsgLen);
    }

    // --- remove leading line feed from errMsg
    if ( errMsgLen > 0 && errMsg[0] == '\n' ) errMsg[0] = ' ';
    return error_getCode(ErrorCode);
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

char* sstrncpy(char *dest, const char *src, size_t maxlen)
//
//  Input:   dest = string to be copied to
//           src = string to be copied from
//           maxlen = number of characters to copy
//  Output:  returns a pointer to dest
//  Purpose: safe version of standard strncpy function
//
{
     strncpy(dest, src, maxlen);
     dest[maxlen] = '\0';
     return dest;
}

//=============================================================================

int  strcomp(char *s1, char *s2)
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
        _mkdir(TempDir);
        dir = TempDir;
    }

    // --- use _tempnam to get a pointer to an unused file name
    name = _tempnam(dir, "swmm");
    if (name == NULL) return NULL;

    // --- copy the file name to fname
    if (strlen(name) < MAXFNAME) strncpy(fname, name, MAXFNAME);
    else fname = NULL;

    // --- free the pointer returned by _tempnam
    free(name);

    // --- return the new contents of fname
    return fname;

// For non-Windows systems:
#else

    // --- use system function mkstemp() to create a temporary file name
    strcpy(fname, "swmmXXXXXX");
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
    x = aDate - StartDateTime;
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

void  writecon(char *s)
//
//  Input:   s = a character string
//  Output:  none
//  Purpose: writes string of characters to the console.
//
{
    fprintf(stdout,s);
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
    char xmsg[120];                    // error message text
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
    sprintf(xmsg, "%sin module %s at step %d, hour %d",
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
