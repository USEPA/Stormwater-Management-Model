/** @file toolkit.c
 @see http://github.com/openwateranalytics/stormwater-management-model

 toolkit.c
 @brief Exportable Functions for Toolkit API.
 @date 08/30/2016 (First Contribution)
 @authors B. McDonnell (EmNet LLC), OpenWaterAnalytics members: see <a href="https://github.com/OpenWaterAnalytics/Stormwater-Management-Model/blob/develop/AUTHORS">AUTHORS</a>.


*/
#define _CRT_SECURE_NO_DEPRECATE


#include <math.h>
#include <time.h>

#include "headers.h"
#include "shared/cstr_helper.h"

#include "swmm5.h"
#include "toolkit.h"


// Function Declarations for API
int  massbal_getRoutingTotal(SM_RoutingTotals **routingTot);
int  massbal_getRunoffTotal(SM_RunoffTotals **runoffTot);
int  massbal_getNodeTotalInflow(int index, double *value);

int  stats_getNodeStat(int index, TNodeStats **nodeStats);
int  stats_getStorageStat(int index, TStorageStats **storageStats);
int  stats_getOutfallStat(int index, TOutfallStats **outfallStats);
int  stats_getLinkStat(int index, TLinkStats **linkStats);
int  stats_getPumpStat(int index, TPumpStats **pumpStats);
int  stats_getSubcatchStat(int index, TSubcatchStats **subcatchStats);


// Utilty Function Declarations
double* newDoubleArray(int n);



// void DLLEXPORT swmm_getSemVersion(char* semver)
// //
// //  Output: Returns Semantic Version
// //  Purpose: retrieves the current semantic version
// //
// //  NOTE: Each New Release should be updated in consts.h
// {
//     getSemVersion(semver);
// }

int DLLEXPORT swmm_getVersionInfo(char** major, char** minor, char** patch)
//
//  Output: Returns Semantic Version Info
//  Purpose: retrieves the current semantic version
//
//  NOTE: Each New Release should be updated in consts.h
{
    cstr_duplicate(major, SEMVERSION_MAJOR);
    cstr_duplicate(minor, SEMVERSION_MINOR);
    cstr_duplicate(patch, SEMVERSION_PATCH);
    return 0;
}

//-----------------------------------------------------------------------------
//  Extended API Functions
//-----------------------------------------------------------------------------

int DLLEXPORT  swmm_run_cb(const char* f1, const char* f2, const char* f3,
    void (*callback) (double *))
//
//  Input:   f1 = name of input file
//           f2 = name of report file
//           f3 = name of binary output file
//  Output:  returns error code
//  Purpose: runs a SWMM simulation.
//
{
    clock_t check = 0;
    double progress, elapsedTime = 0.0;


    // --- initialize flags
    IsOpenFlag = FALSE;
    IsStartedFlag = FALSE;
    SaveResultsFlag = TRUE;


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
            do
            {
                swmm_step(&elapsedTime);

                // --- callback with progress approximately twice a second
                if ( (callback != NULL) && (clock() - check) > CLOCKS_PER_SEC )
                {
                    progress = NewRoutingTime / TotalDuration;
                    callback(&progress);
                    check = clock();
                }

            } while ( elapsedTime > 0.0 && !ErrorCode );

            if ( callback != NULL )
            {
                progress = 1.0;
                callback(&progress);
            }
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


int DLLEXPORT swmm_getAPIError(int errorCode, char **errorMsg)
///
/// Input:   errorCode = error code
/// Output:  errmessage String
/// Return:  API Error
/// Purpose: Get an error message
{
    int errorIndex = error_getErrorIndex(errorCode);
    cstr_duplicate(errorMsg, error_getMsg(errorIndex));
    return 0;
}


int DLLEXPORT swmm_project_findObject(SM_ObjectType type, char *id, int *index)
{
    int error_code_index = 0;

    int idx = project_findObject(type, id);

    if (idx == -1) {
        index = NULL;
        error_code_index = ERR_API_OBJECT_INDEX;
    } else
        *index = idx;

    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getSimulationDateTime(SM_TimePropety type, int *year, int *month, int *day,
                                         int *hour, int *minute, int *second)
///
/// Input:   type = time type to return
/// Output:  year, month, day, hours, minutes, seconds = int
/// Return:  API Error
/// Purpose: Get the simulation start, end and report date times
{
    int error_code_index = 0;
    *year = 1900;
    *month = 1;
    *day = 1;
    *hour = 0;
    *minute = 0;
    *second = 0;

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    else
    {
        DateTime _dtime;
        switch (type)
        {
        //StartDateTime (globals.h)
        case SM_STARTDATE: _dtime = StartDateTime; break;
        //EndDateTime (globals.h)
        case SM_ENDDATE: _dtime = EndDateTime;  break;
        //ReportStart (globals.h)
        case SM_REPORTDATE: _dtime = ReportStart;  break;
        default: return error_getCode(ERR_API_OUTBOUNDS);
        }
        datetime_decodeDate(_dtime, year, month, day);
        datetime_decodeTime(_dtime, hour, minute, second);
    }

    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_setSimulationDateTime(SM_TimePropety type, int year, int month,
                                         int day, int hour, int minute,
                                         int second)
///
/// Input:   type = time type to set
///          year, month, day, hours, minutes, seconds = int
/// Return:  API Error
/// Purpose: Get the simulation start, end and report date times
{
    int error_code_index = 0;

    DateTime theDate;
    DateTime theTime;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if Simulation is Running
    else if(swmm_IsStartedFlag() == TRUE)
    {
        error_code_index = ERR_API_SIM_NRUNNING;
    }
    else
    {
        theDate = datetime_encodeDate(year, month, day);
        theTime = datetime_encodeTime(hour, minute, second);

        switch(type)
        {
            //StartDateTime (globals.h)
            case SM_STARTDATE:
                StartDate = theDate;
                StartTime = theTime;
                StartDateTime = StartDate + StartTime;
                TotalDuration = floor((EndDateTime - StartDateTime) * SECperDAY);
                // --- convert total duration to milliseconds
                TotalDuration *= 1000.0;
                break;
            //EndDateTime (globals.h)
            case SM_ENDDATE:
                EndDate = theDate;
                EndTime = theTime;
                EndDateTime = EndDate + EndTime;
                TotalDuration = floor((EndDateTime - StartDateTime) * SECperDAY);
                // --- convert total duration to milliseconds
                TotalDuration *= 1000.0;
                break;
            //ReportStart (globals.h)
            case SM_REPORTDATE:
                ReportStartDate = theDate;
                ReportStartTime = theTime;
                ReportStart = ReportStartDate + ReportStartTime;
                break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }

    return error_getCode(error_code_index);
}

int DLLEXPORT  swmm_getSimulationUnit(SM_Units type, int *value)
///
/// Input:   type = simulation unit type
/// Output:  enum representation of units
/// Returns: API Error
/// Purpose: get simulation unit types
{
    int error_code_index = 0;
    *value = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    else
    {
        switch(type)
        {
            // System Unit (enum.h UnitsType)
            case SM_SYSTEMUNIT:  *value = UnitSystem; break;
            // Flow Unit (enum.h FlowUnitsType)
            case SM_FLOWUNIT:  *value = FlowUnits; break;
            // Concentration Unit
            //case 2:  *value = UnitSystem; break;
            // Type not available
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }

    return error_getCode(error_code_index);
}

int DLLEXPORT  swmm_getSimulationAnalysisSetting(SM_SimOption type, int *value)
///
/// Input:   type = analysis type
/// Output:  setting True or False
/// Returns: API Error
/// Purpose: get simulation analysis setting
{
    int error_code_index = 0;
    *value = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    else
    {
        switch(type)
        {
            // No ponding at nodes (True or False)
            case SM_ALLOWPOND:  *value = AllowPonding; break;
            // Do flow routing in steady state periods  (True or False)
            case SM_SKIPSTEADY:  *value = SkipSteadyState; break;
            // Analyze rainfall/runoff  (True or False)
            case SM_IGNORERAIN:  *value = IgnoreRainfall; break;
            // Analyze RDII (True or False)
            case SM_IGNORERDII:  *value = IgnoreRDII; break;
            // Analyze snowmelt (True or False)
            case SM_IGNORESNOW:  *value = IgnoreSnowmelt; break;
            // Analyze groundwater (True or False)
            case SM_IGNOREGW:  *value = IgnoreGwater; break;
            // Analyze flow routing (True or False)
            case SM_IGNOREROUTE:  *value = IgnoreRouting; break;
            // Analyze water quality (True or False)
            case SM_IGNORERQUAL:  *value = IgnoreQuality; break;
            // Type not available
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT  swmm_getSimulationParam(SM_SimSetting type, double *value)
///
/// Input:   type = analysis type
/// Output:  Simulation Parameter
/// Returns: error code
/// Purpose: Get simulation analysis parameter
{
    int error_code_index = 0;
    *value = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Output  setting
    else
    {
        switch(type)
        {
            // Routing time step (sec)
            case SM_ROUTESTEP: *value = RouteStep; break;
            // Minimum variable time step (sec)
            case SM_MINROUTESTEP: *value = MinRouteStep; break;
            // Time step for lengthening (sec)
            case SM_LENGTHSTEP: *value = LengtheningStep; break;
            // Antecedent dry days
            case SM_STARTDRYDAYS: *value = StartDryDays; break;
            // Courant time step factor
            case SM_COURANTFACTOR: *value = CourantFactor; break;
            // Minimum nodal surface area
            case SM_MINSURFAREA:
                if(RouteModel == DW)
                {
                    *value = MinSurfArea * UCF(LENGTH) * UCF(LENGTH);
                }
                else
                {
                    *value = MinSurfArea;
                }
                break;
            // Minimum conduit slope
            case SM_MINSLOPE: *value = MinSlope; break;
            // Runoff continuity error
            case SM_RUNOFFERROR: *value = RunoffError; break;
            // Groundwater continuity error
            case SM_GWERROR: *value = GwaterError; break;
            // Flow routing error
            case SM_FLOWERROR: *value = FlowError; break;
            // Quality routing error
            case SM_QUALERROR: *value = QualError; break;
            // DW routing head tolerance (ft) * UCF(LENGTH)
            case SM_HEADTOL:
                if(RouteModel == DW)
                {
                    *value = HeadTol * UCF(LENGTH);
                }
                else
                {
                    *value = HeadTol;
                }
                break;
            // Tolerance for steady system flow
            case SM_SYSFLOWTOL: *value = SysFlowTol; break;
            // Tolerance for steady nodal inflow
            case SM_LATFLOWTOL: *value = LatFlowTol; break;
            // Type not available
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT  swmm_countObjects(SM_ObjectType type, int *count)
///
/// Input:   type = object type (Based on SM_ObjectType enum)
/// Output:  count = pointer to integer
/// Returns: API Error
/// Purpose: uses Object Count table to find number of elements of an object
{
    int error_code_index = 0;
    *count = 0;
    if(type >= MAX_OBJ_TYPES)
    {
        error_code_index = ERR_API_OUTBOUNDS;
    }
    else
    {
        *count = Nobjects[type];
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getObjectIndex(SM_ObjectType type, char *id, int *index)
///
/// Input:   type = object type (Based on SM_ObjectType enum)
///          char* = ID name
/// Output:  object index
/// Return:  error
/// Purpose: Gets object id index
{
    int error_code_index = 0;

    // Check if Open
    if(swmm_IsOpenFlag() == TRUE)
        *index = project_findObject(type, id);
    else
        error_code_index = ERR_API_INPUTNOTOPEN;

    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getObjectId(SM_ObjectType type, int index, char **id)
///
/// Input:   type = object type (Based on SM_ObjectType enum)
///          index = Index of desired ID
/// Output:  id = pointer to id pass by reference
/// Return:  API Error
/// Purpose: Gets ID for any object
{
    int error_code_index = 0;
    TLidProc*  lidProc;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[type])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch (type)
        {
            case SM_GAGE:
                cstr_duplicate(id, Gage[index].ID); break;
            case SM_SUBCATCH:
                cstr_duplicate(id, Subcatch[index].ID); break;
            case SM_NODE:
                cstr_duplicate(id, Node[index].ID); break;
            case SM_LINK:
                cstr_duplicate(id, Link[index].ID); break;
            case SM_POLLUT:
                cstr_duplicate(id, Pollut[index].ID); break;
            case SM_LANDUSE:
                cstr_duplicate(id, Landuse[index].ID); break;
            case SM_TIMEPATTERN:
                cstr_duplicate(id, Pattern[index].ID); break;
            case SM_CURVE:
                cstr_duplicate(id, Curve[index].ID); break;
            case SM_TSERIES:
                cstr_duplicate(id, Tseries[index].ID); break;
            //case SM_CONTROL:
                //cstr_duplicate(id, Rules[index].ID); break;
            case SM_TRANSECT:
                cstr_duplicate(id, Transect[index].ID); break;
            case SM_AQUIFER:
                cstr_duplicate(id, Aquifer[index].ID); break;
            case SM_UNITHYD:
                cstr_duplicate(id, UnitHyd[index].ID); break;
            case SM_SNOWMELT:
                cstr_duplicate(id, Snowmelt[index].ID); break;
            //case SM_SHAPE:
                //cstr_duplicate(id, Shape[index].ID); break;
            case SM_LID:
                lidProc = lid_getLidProc(index);
                if (lidProc != NULL) cstr_duplicate(id, lidProc->ID);
                else error_code_index = ERR_API_OUTBOUNDS;
                break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
   }
   return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getNodeType(int index, SM_NodeType *Ntype)
///
/// Input:   index = Index of desired ID
///          Ntype = Node type (Based on enum SM_NodeType)
/// Return:  API Error
/// Purpose: Gets Node Type
{
    int error_code_index = 0;
    *Ntype = -1;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else *Ntype = Node[index].type;

    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getLinkType(int index, SM_LinkType *Ltype)
///
/// Input:   index = Index of desired ID
///          Ltype = Link type (Based on enum SM_LinkType)
/// Return:  API Error
/// Purpose: Gets Link Type
{
    int error_code_index = 0;
    *Ltype = -1;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else *Ltype = Link[index].type;

    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getLinkConnections(int index, int *Node1, int *Node2)
///
/// Input:   index = Index of desired ID
/// Output:  Node1 and Node2 indeces
/// Return:  API Error
/// Purpose: Gets link Connection ID Indeces
{
    int error_code_index = 0;
    *Node1 = -1;
    *Node2 = -1;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        *Node1 = Link[index].node1;
        *Node2 = Link[index].node2;
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getLinkDirection(int index, signed char *value)
///
/// Input:   index = Index of desired ID
/// Output:  Link Direction (Only changes is slope < 0)
/// Return:  API Error
/// Purpose: Gets Link Direction
{
    int error_code_index = 0;
    *value = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        *value = Link[index].direction;
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getNodeParam(int index, SM_NodeProperty param, double *value)
///
/// Input:   index = Index of desired ID
///          param = Parameter desired (Based on enum SM_NodeProperty)
/// Output:  value = value to be output
/// Return:  API Error
/// Purpose: Gets Node Parameter
{
    int error_code_index = 0;
    *value = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(param)
        {
            case SM_INVERTEL:
                *value = Node[index].invertElev * UCF(LENGTH); break;
            case SM_FULLDEPTH:
                *value = Node[index].fullDepth * UCF(LENGTH); break;
            case SM_SURCHDEPTH:
                *value = Node[index].surDepth * UCF(LENGTH); break;
            case SM_PONDAREA:
                *value = Node[index].pondedArea * UCF(LENGTH) * UCF(LENGTH); break;
            case SM_INITDEPTH:
                *value = Node[index].initDepth * UCF(LENGTH); break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_setNodeParam(int index, SM_NodeProperty param, double value)
///
/// Input:   index = Index of desired ID
///          param = Parameter desired (Based on enum SM_NodeProperty)
///          value = value to be input
/// Return:  API Error
/// Purpose: Sets Node Parameter
{
    int error_code_index = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
     // Check if Simulation is Running
    else if(swmm_IsStartedFlag() == TRUE)
    {
        error_code_index = ERR_API_SIM_NRUNNING;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(param)
        {
            case SM_INVERTEL:
                Node[index].invertElev = value / UCF(LENGTH); break;
            case SM_FULLDEPTH:
                Node[index].fullDepth = value / UCF(LENGTH); break;
            case SM_SURCHDEPTH:
                Node[index].surDepth = value / UCF(LENGTH); break;
            case SM_PONDAREA:
                Node[index].pondedArea = value / ( UCF(LENGTH) * UCF(LENGTH) ); break;
            case SM_INITDEPTH:
                Node[index].initDepth = value / UCF(LENGTH); break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    // Re-validated a node BEM 1/20/2017 Probably need to re-validate connecting links
    //node_validate(index)
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getLinkParam(int index, SM_LinkProperty param, double *value)
///
/// Input:   index = Index of desired ID
///          param = Parameter desired (Based on enum SM_LinkProperty)
/// Output:  value = value to be output
/// Return:  API Error
/// Purpose: Gets Link Parameter
{
    int error_code_index = 0;
    *value = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(param)
        {
            case SM_OFFSET1:
                *value = Link[index].offset1 * UCF(LENGTH); break;
            case SM_OFFSET2:
                *value = Link[index].offset2 * UCF(LENGTH); break;
            case SM_INITFLOW:
                *value = Link[index].q0 * UCF(FLOW); break;
            case SM_FLOWLIMIT:
                *value = Link[index].qLimit * UCF(FLOW); break;
            case SM_INLETLOSS:
                *value = Link[index].cLossInlet; break;
            case SM_OUTLETLOSS:
                *value = Link[index].cLossOutlet; break;
            case SM_AVELOSS:
                *value = Link[index].cLossAvg; break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_setLinkParam(int index, SM_LinkProperty param, double value)
///
/// Input:   index = Index of desired ID
///          param = Parameter desired (Based on enum SM_LinkProperty)
///          value = value to be output
/// Return:  API Error
/// Purpose: Sets Link Parameter
{
    int error_code_index = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(param)
        {
            // offset1
            case SM_OFFSET1:
                // Check if Simulation is Running
                if(swmm_IsStartedFlag() == TRUE)
                {
                    error_code_index = ERR_API_SIM_NRUNNING; break;
                }
                Link[index].offset1 = value / UCF(LENGTH); break;
            case SM_OFFSET2:
                // Check if Simulation is Running
                if(swmm_IsStartedFlag() == TRUE)
                {
                    error_code_index = ERR_API_SIM_NRUNNING; break;
                }
                Link[index].offset2 = value / UCF(LENGTH); break;
            case SM_INITFLOW:
                Link[index].q0 = value / UCF(FLOW); break;
            case SM_FLOWLIMIT:
                Link[index].qLimit = value / UCF(FLOW); break;
            case SM_INLETLOSS:
                Link[index].cLossInlet = value; break;
            case SM_OUTLETLOSS:
                Link[index].cLossOutlet = value; break;
            case SM_AVELOSS:
                Link[index].cLossAvg = value; break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
        // re-validated link
        //link_validate(index);
    }

    return error_getCode(error_code_index);
}


int DLLEXPORT swmm_getSubcatchParam(int index, SM_SubcProperty param, double *value)
///
/// Input:   index = Index of desired ID
///          param = Parameter desired (Based on enum SM_SubcProperty)
/// Output:  value = value to be output
/// Return:  API Error
/// Purpose: Gets Subcatchment Parameter
{
    int error_code_index = 0;
    *value = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(param)
        {
            case SM_WIDTH:
                *value = Subcatch[index].width * UCF(LENGTH); break;
            case SM_AREA:
                *value = Subcatch[index].area * UCF(LANDAREA); break;
            case SM_FRACIMPERV:
                *value = Subcatch[index].fracImperv; break;
            case SM_SLOPE:
                *value = Subcatch[index].slope; break;
            case SM_CURBLEN:
                *value = Subcatch[index].curbLength * UCF(LENGTH); break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_setSubcatchParam(int index, SM_SubcProperty param, double value)
///
/// Input:   index = Index of desired ID
///          param = Parameter desired (Based on enum SM_SubcProperty)
///          value = value to be output
/// Return:  API Error
/// Purpose: Sets Subcatchment Parameter
{
    int error_code_index = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
     // Check if Simulation is Running
    else if(swmm_IsStartedFlag() == TRUE)
    {
        error_code_index = ERR_API_SIM_NRUNNING;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(param)
        {
            case SM_WIDTH:
                Subcatch[index].width = value / UCF(LENGTH); break;
            case SM_AREA:
                Subcatch[index].area = value / UCF(LANDAREA); break;
            case SM_FRACIMPERV:
                break; //Subcatch[index].fracImperv; break;
                // Cannot Open Function just yet.  Need
                // to adjust some internal functions to
                // ensure parameters are recalculated
                // = MIN(value, 100.0) / 100.0; break;
            case SM_SLOPE:
                Subcatch[index].slope = value; break;
            case SM_CURBLEN:
                Subcatch[index].curbLength = value / UCF(LENGTH); break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
        //re-validate subcatchment
        subcatch_validate(index); // incorprate callback here
    }

    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getSubcatchOutConnection(int index, SM_ObjectType *type, int *out_index)
///
/// Input:   index = Index of desired ID
///         (Subcatchments can load to Node or another Subcatchment)
/// Output:  Type of Object
///          Index of Object
/// Return:  API Error
/// Purpose: Gets Subcatchment Connection ID Indeces for either Node or Subcatchment
{
    int error_code_index = 0;
    *type = -1;
    *out_index = -1;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        if (Subcatch[index].outNode == -1 && Subcatch[index].outSubcatch == -1)
        {
            *out_index = index; // Case of self Loading subcatchment
            *type = (SM_ObjectType)SUBCATCH;
        }
        if (Subcatch[index].outNode >= 0)
        {
            *out_index = Subcatch[index].outNode;
            *type = (SM_ObjectType)NODE;
        }
        if (Subcatch[index].outSubcatch >= 0)
        {
            *out_index = Subcatch[index].outSubcatch;
            *type = (SM_ObjectType)SUBCATCH;
        }
    }
    return error_getCode(error_code_index);
}


int DLLEXPORT swmm_getLidUCount(int index, int *value)
// Input:   index = Index of desired subcatchment
// Output:  int = number of lid units for subcatchment
// Return:  number of lid units for subcatchment
// Purpose: count number of lid units for subcatchment
{
    int error_code_index = 0;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if subcatchment index is within bounds
    else if(index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        *value = lid_getLidUnitCount(index);
    }

    return error_getCode(error_code_index);
}


int DLLEXPORT swmm_getLidUParam(int index, int lidIndex, SM_LidUProperty param, double *value)
//
// Input:   index = Index of desired subcatchment
//          lidIndex = Index of desired lid unit (subcatchment allow for multiple lid units)
//          param = Parameter desired (Based on enum SM_LidUProperty)
// Output:  value = value to be output
// Return:  API Error
// Purpose: Gets Lid Unit Parameter
{
    int error_code_index = 0;
    TLidUnit* lidUnit;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if subcatchment index is within bounds
    else if(index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        lidUnit = lid_getLidUnit(index, lidIndex, &error_code_index);

        // There are no Lid Units defined for the subcatchments
        if(lidUnit)
        {
            switch(param)
            {
                case SM_UNITAREA:
                    *value = lidUnit->area * SQR(UCF(LENGTH));
                    break;
                case SM_FWIDTH:
                    *value = lidUnit->fullWidth * UCF(LENGTH); break;
                case SM_BWIDTH:
                    *value = lidUnit->botWidth * UCF(LENGTH); break;
                case SM_INITSAT:
                    *value = lidUnit->initSat * 100; break;
                case SM_FROMIMPERV:
                    *value = lidUnit->fromImperv * 100; break;
                case SM_FROMPERV:
                    *value = lidUnit->fromPerv * 100; break;
                default:
                    error_code_index = ERR_API_OUTBOUNDS; break;
            }
        }
    }

    return error_getCode(error_code_index);
}


int DLLEXPORT swmm_setLidUParam(int index, int lidIndex, SM_LidUProperty param, double value)
//
// Input:   index = Index of desired subcatchment
//          lidIndex = Index of desired lid unit (subcatchment allow for multiple lid units)
//          param = Parameter desired (Based on enum SM_LidUProperty)
//          value = replacement
// Return:  API Error
// Purpose: Gets Lid Unit Parameter
{
    int error_code_index = 0;
    TLidUnit* lidUnit;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if subcatchment index is within bounds
    else if(index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    // Check if model is running
    else if(swmm_IsStartedFlag() == TRUE)
    {
        error_code_index = ERR_API_SIM_NRUNNING;
    }
    else
    {
        lidUnit = lid_getLidUnit(index, lidIndex, &error_code_index);

        // There are no Lid Units defined for the subcatchments
        if(lidUnit)
        {
            switch(param)
            {
                case SM_UNITAREA:
                    lidUnit->area = value / SQR(UCF(LENGTH));
                    break;
                case SM_FWIDTH:
                    lidUnit->fullWidth = value / UCF(LENGTH); break;
                //case SM_BWIDTH:
                //    lidUnit->botWidth = value / UCF(LENGTH); break;
                case SM_INITSAT:
                    lidUnit->initSat = value / 100; break;
                case SM_FROMIMPERV:
                    lidUnit->fromImperv = value / 100; break;
                case SM_FROMPERV:
                    lidUnit->fromPerv = value / 100; break;
                default:
                    error_code_index = ERR_API_OUTBOUNDS; break;
            }
        }
    }

    if(error_code_index == ERR_NONE)
    {
        lid_validateLidGroup(index);
        lid_updateLidGroup(index);
    }

    return error_getCode(error_code_index);
}


int DLLEXPORT swmm_getLidUOption(int index, int lidIndex, SM_LidUOptions param, int *value)
//
// Input:   index = Index of desired subcatchment
//          lidIndex = Index of desired lid unit (subcatchment allow for multiple lid units)
//          param = Parameter desired (Based on enum SM_LidUOption)
// Output:  value = value to be output
// Return:  API Error
// Purpose: Gets Lid Unit Option Parameter
{
    int error_code_index = 0;
    TLidUnit* lidUnit;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if subcatchment index is within bounds
    else if(index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        lidUnit = lid_getLidUnit(index, lidIndex, &error_code_index);

        // There are no Lid Units defined for the subcatchments
        if(lidUnit)
        {

            switch(param)
            {
                case SM_INDEX:
                    *value = lidUnit->lidIndex; break;
                case SM_NUMBER:
                    *value = lidUnit->number; break;
                case SM_TOPERV:
                    *value = lidUnit->toPerv; break;
                case SM_DRAINSUB:
                    *value = lidUnit->drainSubcatch; break;
                case SM_DRAINNODE:
                    *value = lidUnit->drainNode; break;
                default:
                    error_code_index = ERR_API_OUTBOUNDS; break;
            }
        }
    }

    return error_getCode(error_code_index);
}


int DLLEXPORT swmm_setLidUOption(int index, int lidIndex, SM_LidUOptions param, int value)
//
// Input:   index = Index of desired subcatchment
//          lidIndex = Index of desired lid unit (subcatchment allow for multiple lid units)
//          param = Parameter desired (Based on enum SM_LidUOption)
//          value = replacement value
// Return:  API Error
// Purpose: Sets Lid Unit Option
{
    int error_code_index = 0;
    TLidUnit* lidUnit;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if subcatchment index is within bounds
    else if(index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    // Check if model is running
    else if(swmm_IsStartedFlag() == TRUE)
    {
        lidUnit = lid_getLidUnit(index, lidIndex, &error_code_index);

        // There are no Lid Units defined for the subcatchments
        if (lidUnit)
        {
            switch (param)
            {
            case SM_INDEX:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_NUMBER:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_TOPERV:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_DRAINSUB:
                lidUnit->drainSubcatch = value;
                lidUnit->drainNode = -1;
                break;
            case SM_DRAINNODE:
                lidUnit->drainNode = value;
                lidUnit->drainSubcatch = -1;
                break;
            default:
                error_code_index = ERR_API_OUTBOUNDS; break;
            }
        }
    }
    else if(swmm_IsStartedFlag() == FALSE)
    {
        lidUnit = lid_getLidUnit(index, lidIndex, &error_code_index);

        // There are no Lid Units defined for the subcatchments
        if(lidUnit)
        {
            switch(param)
            {
                case SM_INDEX:
                    lidUnit->lidIndex = value; break;
                case SM_NUMBER:
                    lidUnit->number = value;
                    break;
                case SM_TOPERV:
                    lidUnit->toPerv = (value > 0.0); break;
                case SM_DRAINSUB:
                    lidUnit->drainSubcatch = value;
                    lidUnit->drainNode = -1;
                    break;
                case SM_DRAINNODE:
                    lidUnit->drainNode = value;
                    lidUnit->drainSubcatch = -1;
                    break;
                default:
                    error_code_index = ERR_API_OUTBOUNDS; break;
            }
        }
    }
    else
    {
        error_code_index = ERR_API_OUTBOUNDS;
    }

    if(error_code_index == ERR_NONE)
    {
        lid_validateLidGroup(index);
        if (swmm_IsStartedFlag() == FALSE)
        {
            lid_updateLidGroup(index);
        }
    }

    return error_getCode(error_code_index);
}


int DLLEXPORT swmm_getLidCOverflow(int lidControlIndex, int *condition)
//
// Input:   lidControlIndex = Index of desired lid control
// Output:  condition = value to be output
// Return:  API Error
// Purpose: Get Lid Control Surface Immediate Overflow Condition
{
    int error_code_index = 0;
    TLidProc*  lidProc;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if subcatchment index is within bounds
    else if(lidControlIndex < 0 || lidControlIndex >= Nobjects[LID])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        lidProc = lid_getLidProc(lidControlIndex);
        if(lidProc != NULL)
        {
            *condition = (int) lidProc->surface.canOverflow;

        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getLidCParam(int lidControlIndex, SM_LidLayer layerIndex, SM_LidLayerProperty param, double *value)
//
// Input:   lidControlIndex = Index of desired lid control
//          layerIndex = Index of desired lid control layer (Based on enum SM_LidLayer)
//          param = Parameter desired (Based on enum SM_LidLayerProperty)
// Output:  value = value to be output
// Return:  API Error
// Purpose: Gets Lid Control Layer Parameter
{
    int error_code_index = 0;
    TLidProc*  lidProc;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if subcatchment index is within bounds
    else if(lidControlIndex < 0 || lidControlIndex >= Nobjects[LID])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        lidProc = lid_getLidProc(lidControlIndex);

        switch(layerIndex)
        {
        case SM_SURFACE:
            switch(param)
            {
                case SM_THICKNESS:
                    *value = lidProc->surface.thickness * UCF(RAINDEPTH); break;
                case SM_VOIDFRAC:
                    *value = 1 - lidProc->surface.voidFrac; break;
                case SM_ROUGHNESS:
                    *value = lidProc->surface.roughness; break;
                case SM_SURFSLOPE:
                    *value = lidProc->surface.surfSlope * 100; break;
                case SM_SIDESLOPE:
                    *value = lidProc->surface.sideSlope; break;
                case SM_ALPHA:
                    *value = lidProc->surface.alpha; break;
                default:
                    error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        case SM_SOIL:
            switch(param)
            {
                case SM_THICKNESS:
                    *value = lidProc->soil.thickness * UCF(RAINDEPTH); break;
                case SM_POROSITY:
                    *value = lidProc->soil.porosity; break;
                case SM_FIELDCAP:
                    *value = lidProc->soil.fieldCap; break;
                case SM_WILTPOINT:
                    *value = lidProc->soil.wiltPoint; break;
                case SM_KSAT:
                    *value = lidProc->soil.kSat * UCF(RAINFALL); break;
                case SM_KSLOPE:
                    *value = lidProc->soil.kSlope; break;
                case SM_SUCTION:
                    *value = lidProc->soil.suction * UCF(RAINDEPTH); break;
                default:
                    error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        case SM_STOR:
            switch(param)
            {
                case SM_THICKNESS:
                    *value = lidProc->storage.thickness * UCF(RAINDEPTH); break;
                case SM_VOIDFRAC:
                    if (lidProc->storage.voidFrac < 1)
                    {
                        *value = lidProc->storage.voidFrac / (1 - lidProc->storage.voidFrac);
                    }
                    else
                    {
                        *value = lidProc->storage.voidFrac;
                    }
                    break;
                case SM_KSAT:
                    *value = lidProc->storage.kSat * UCF(RAINFALL); break;
                case SM_CLOGFACTOR:
                    if (lidProc->storage.thickness > 0.0)
                    {
                        *value = lidProc->storage.clogFactor /
                                (lidProc->storage.thickness *
                                lidProc->storage.voidFrac);
                    }
                    else
                    {
                        *value = lidProc->pavement.clogFactor;
                    }
                    break;
                default:
                    error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        case SM_PAVE:
            switch(param)
            {
            case SM_THICKNESS:
                *value = lidProc->pavement.thickness * UCF(RAINDEPTH); break;
            case SM_VOIDFRAC:
                if(lidProc->pavement.voidFrac < 1)
                {
                    *value = lidProc->pavement.voidFrac / (1 - lidProc->pavement.voidFrac);
                }
                else
                {
                    *value = lidProc->pavement.voidFrac;
                }
                break;
            case SM_IMPERVFRAC:
                *value = lidProc->pavement.impervFrac; break;
            case SM_KSAT:
                *value = lidProc->pavement.kSat * UCF(RAINFALL); break;
            case SM_CLOGFACTOR:
                if(lidProc->pavement.thickness > 0.0)
                {
                    *value = lidProc->pavement.clogFactor /
                            (lidProc->pavement.thickness *
                            lidProc->pavement.voidFrac *
                            (1 - lidProc->pavement.impervFrac));
                }
                else
                {
                    *value = lidProc->pavement.clogFactor;
                }
                break;
            case SM_REGENDAYS:
                *value = lidProc->pavement.regenDays; break;
            case SM_REGENDEGREE:
                *value = lidProc->pavement.regenDegree; break;
            default:
                error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        case SM_DRAIN:
            switch(param)
            {
            case SM_COEFF:
                *value = lidProc->drain.coeff; break;
            case SM_EXPON:
                *value = lidProc->drain.expon; break;
            case SM_OFFSET:
                *value = lidProc->drain.offset * UCF(RAINDEPTH); break;
            case SM_DELAY:
                *value = lidProc->drain.delay / 3600; break;
            case SM_HOPEN:
                *value = lidProc->drain.hOpen * UCF(RAINDEPTH); break;
            case SM_HCLOSE:
                *value = lidProc->drain.hClose * UCF(RAINDEPTH); break;
            default:
                error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        case SM_DRAINMAT:
            switch(param)
            {
            case SM_THICKNESS:
                *value = lidProc->drainMat.thickness * UCF(RAINDEPTH); break;
            case SM_VOIDFRAC:
                *value = lidProc->drainMat.voidFrac; break;
            case SM_ROUGHNESS:
                *value = lidProc->drainMat.roughness; break;
            case SM_ALPHA:
                *value = lidProc->drainMat.alpha; break;
            default:
                error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        default:
            error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}


int DLLEXPORT swmm_setLidCParam(int lidControlIndex, SM_LidLayer layerIndex, SM_LidLayerProperty param, double value)
//
// Input:   lidControlIndex = Index of desired lid control
//          layerIndex = Index of desired lid control layer (Based on enum SM_LidLayers)
//          param = Parameter desired (Based on enum SM_LidLayersProperty)
//          value = replacement value
// Return:  API Error
// Purpose: Sets Lid Control Layer Parameter
{
    int error_code_index = 0;
    TLidProc*  lidProc;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if subcatchment index is within bounds
    else if(lidControlIndex < 0 || lidControlIndex >= Nobjects[LID])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else if (swmm_IsStartedFlag() == TRUE)
    {
        lidProc = lid_getLidProc(lidControlIndex);

        switch (layerIndex)
        {
        case SM_SURFACE:
            switch (param)
            {
            case SM_THICKNESS:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_VOIDFRAC:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_ROUGHNESS:
                lidProc->surface.roughness = value; break;
            case SM_SURFSLOPE:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_SIDESLOPE:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            default:
                error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        case SM_SOIL:
            switch (param)
            {
            case SM_THICKNESS:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_POROSITY:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_FIELDCAP:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_WILTPOINT:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_KSAT:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_KSLOPE:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_SUCTION:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            default:
                error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        case SM_STOR:
            switch (param)
            {
            case SM_THICKNESS:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_VOIDFRAC:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_KSAT:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_CLOGFACTOR:
                lidProc->storage.clogFactor = value *
                    lidProc->storage.thickness *
                    lidProc->storage.voidFrac;
                break;
            default:
                error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        case SM_PAVE:
            switch (param)
            {
            case SM_THICKNESS:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_VOIDFRAC:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_IMPERVFRAC:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_KSAT:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_CLOGFACTOR:
                lidProc->pavement.clogFactor = value *
                    lidProc->pavement.thickness *
                    lidProc->pavement.voidFrac *
                    (1.0 - lidProc->pavement.impervFrac);
                break;
            case SM_REGENDAYS:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_REGENDEGREE:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            default:
                error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        case SM_DRAIN:
            switch (param)
            {
            case SM_COEFF:
                lidProc->drain.coeff = value; break;
            case SM_EXPON:
                lidProc->drain.expon = value; break;
            case SM_OFFSET:
                lidProc->drain.offset = value / UCF(RAINDEPTH); break;
            case SM_DELAY:
                lidProc->drain.delay = value * 3600; break;
            case SM_HOPEN:
                lidProc->drain.hOpen = value / UCF(RAINDEPTH); break;
            case SM_HCLOSE:
                lidProc->drain.hClose = value / UCF(RAINDEPTH); break;
            default:
                error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        case SM_DRAINMAT:
            switch (param)
            {
            case SM_THICKNESS:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_VOIDFRAC:
                error_code_index = ERR_API_SIM_NRUNNING; break;
            case SM_ROUGHNESS:
                lidProc->drainMat.roughness = value; break;
           //case SM_ALPHA:
           //    lidProc->drainMat.alpha = value; break;
            default:
                error_code_index = ERR_API_OUTBOUNDS; break;
            }
            break;
        default:
            error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    else if(swmm_IsStartedFlag() == FALSE)
    {
        lidProc = lid_getLidProc(lidControlIndex);

        switch(layerIndex)
        {
            case SM_SURFACE:
                switch(param)
                {
                    case SM_THICKNESS:
                        lidProc->surface.thickness = value / UCF(RAINDEPTH); break;
                    case SM_VOIDFRAC:
                        lidProc->surface.voidFrac = 1 - value; break;
                    case SM_ROUGHNESS:
                        lidProc->surface.roughness = value; break;
                    case SM_SURFSLOPE:
                        lidProc->surface.surfSlope = value / 100; break;
                    case SM_SIDESLOPE:
                        lidProc->surface.sideSlope = value; break;
                    //case SM_ALPHA:
                    //    lidProc->surface.alpha = value; break;
                    default:
                        error_code_index = ERR_API_OUTBOUNDS; break;
                }
                break;
            case SM_SOIL:
                switch(param)
                {
                    case SM_THICKNESS:
                        lidProc->soil.thickness = value / UCF(RAINDEPTH); break;
                    case SM_POROSITY:
                        lidProc->soil.porosity = value; break;
                    case SM_FIELDCAP:
                        lidProc->soil.fieldCap = value; break;
                    case SM_WILTPOINT:
                        lidProc->soil.wiltPoint = value; break;
                    case SM_KSAT:
                        lidProc->soil.kSat = value / UCF(RAINFALL); break;
                    case SM_KSLOPE:
                        lidProc->soil.kSlope = value; break;
                    case SM_SUCTION:
                        lidProc->soil.suction = value / UCF(RAINDEPTH); break;
                    default:
                        error_code_index = ERR_API_OUTBOUNDS; break;
                }
                break;
            case SM_STOR:
                switch(param)
                {
                    case SM_THICKNESS:
                        if (lidProc->storage.thickness > 0)
                        {
                            lidProc->storage.clogFactor /= lidProc->storage.thickness;
                            lidProc->storage.thickness = value / UCF(RAINDEPTH);
                            lidProc->storage.clogFactor *= lidProc->storage.thickness;
                        }
                        else
                        {
                            lidProc->storage.thickness = value / UCF(RAINDEPTH);
                        }
                        break;
                    case SM_VOIDFRAC:
                        if (lidProc->storage.voidFrac > 0)
                        {
                            lidProc->storage.clogFactor /= lidProc->storage.voidFrac;
                            lidProc->storage.voidFrac = value / (value + 1);
                            lidProc->storage.clogFactor *= lidProc->storage.voidFrac;
                        }
                        else
                        {
                            lidProc->storage.voidFrac = value / (value + 1);
                        }

                        break;
                    case SM_KSAT:
                        lidProc->storage.kSat = value / UCF(RAINFALL); break;
                    case SM_CLOGFACTOR:
                        lidProc->storage.clogFactor = value *
                        lidProc->storage.thickness *
                        lidProc->storage.voidFrac;
                        break;
                    default:
                        error_code_index = ERR_API_OUTBOUNDS; break;
                }
                break;
            case SM_PAVE:
                switch(param)
                {
                    case SM_THICKNESS:
                        if (lidProc->pavement.thickness > 0)
                        {
                            lidProc->pavement.clogFactor /= lidProc->pavement.thickness;
                            lidProc->pavement.thickness = value / UCF(RAINDEPTH);
                            lidProc->pavement.clogFactor *= lidProc->pavement.thickness;
                        }
                        else
                        {
                            lidProc->pavement.thickness = value / UCF(RAINDEPTH);
                        }
                        break;
                    case SM_VOIDFRAC:
                        if (lidProc->pavement.voidFrac > 0)
                        {
                            lidProc->pavement.clogFactor /= lidProc->pavement.voidFrac;
                            lidProc->pavement.voidFrac = value / (value + 1);
                            lidProc->pavement.clogFactor *= lidProc->pavement.voidFrac;
                        }
                        else
                        {
                            lidProc->pavement.voidFrac = value / (value + 1);
                        }
                        break;
                    case SM_IMPERVFRAC:
                        if (lidProc->pavement.impervFrac > 0)
                        {
                            lidProc->pavement.clogFactor /= (1 - lidProc->pavement.impervFrac);
                            lidProc->pavement.impervFrac = value;
                            lidProc->pavement.clogFactor *= (1 - lidProc->pavement.impervFrac);
                        }
                        else
                        {
                            lidProc->pavement.impervFrac = value;
                        }
                        break;
                    case SM_KSAT:
                        lidProc->pavement.kSat = value / UCF(RAINFALL); break;
                    case SM_CLOGFACTOR:
                        lidProc->pavement.clogFactor = value *
                        lidProc->pavement.thickness *
                        lidProc->pavement.voidFrac *
                        (1.0 - lidProc->pavement.impervFrac);
                        break;
                    case SM_REGENDAYS:
                        lidProc->pavement.regenDays = value; break;
                    case SM_REGENDEGREE:
                        lidProc->pavement.regenDegree = value; break;
                    default:
                        error_code_index = ERR_API_OUTBOUNDS; break;
                }
                break;
            case SM_DRAIN:
                switch(param)
                {
                    case SM_COEFF:
                        lidProc->drain.coeff = value; break;
                    case SM_EXPON:
                        lidProc->drain.expon = value; break;
                    case SM_OFFSET:
                        lidProc->drain.offset = value / UCF(RAINDEPTH); break;
                    case SM_DELAY:
                        lidProc->drain.delay = value * 3600; break;
                    case SM_HOPEN:
                        lidProc->drain.hOpen = value / UCF(RAINDEPTH); break;
                    case SM_HCLOSE:
                        lidProc->drain.hClose = value / UCF(RAINDEPTH); break;
                    default:
                        error_code_index = ERR_API_OUTBOUNDS; break;
                }
                break;
            case SM_DRAINMAT:
                switch(param)
                {
                    case SM_THICKNESS:
                        lidProc->drainMat.thickness = value / UCF(RAINDEPTH); break;
                    case SM_VOIDFRAC:
                        lidProc->drainMat.voidFrac = value; break;
                    case SM_ROUGHNESS:
                        lidProc->drainMat.roughness = value; break;
                    //case SM_ALPHA:
                    //    lidProc->drainMat.alpha = value; break;
                    default:
                        error_code_index = ERR_API_OUTBOUNDS; break;
                }
                break;
            default:
                error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    else
    {
        error_code_index = ERR_API_OUTBOUNDS;
    }

    if(error_code_index == ERR_NONE)
    {
        lid_validateLidProc(lidControlIndex);
        if (swmm_IsStartedFlag() == FALSE)
        {
            lid_updateAllLidUnit(lidControlIndex);
        }
    }
    return error_getCode(error_code_index);
}

//-------------------------------
// Active Simulation Results API
//-------------------------------

int DLLEXPORT swmm_getCurrentDateTime(int *year, int *month, int *day,
                                      int *hour, int *minute, int *second)
///
/// Input:   timetype = time type to return
/// Output:  year, month, day, hours, minutes, seconds = int
/// Return:  API Error
/// Purpose: Get the simulation start, end and report date times
{
    DateTime currentTime;
    int error_code_index = 0;
    *year = 1900;
    *month = 1;
    *day = 1;
    *hour = 0;
    *minute = 0;
    *second = 0;

    // Check if Simulation is Running
    if(swmm_IsStartedFlag() == FALSE)
    {
        error_code_index = ERR_API_SIM_NRUNNING;
    }
    else
    {
        // Fetch Current Time
        currentTime = getDateTime(NewRoutingTime);

        datetime_decodeDate(currentTime, year, month, day);
        datetime_decodeTime(currentTime, hour, minute, second);
    }

    return error_getCode(error_code_index);
}


int DLLEXPORT swmm_getNodeResult(int index, SM_NodeResult type, double *result)
///
/// Input:   index = Index of desired ID
///          type = Result Type (SM_NodeResult)
/// Output:  result = result data desired (byref)
/// Return:  API Error
/// Purpose: Gets Node Simulated Value at Current Time
{
    int error_code_index = 0;
    *result = 0;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch (type)
        {
            case SM_TOTALINFLOW:
                *result = Node[index].inflow * UCF(FLOW); break;
            case SM_TOTALOUTFLOW:
                *result = Node[index].outflow * UCF(FLOW); break;
            case SM_LOSSES:
                *result = Node[index].losses * UCF(FLOW); break;
            case SM_NODEVOL:
                *result = Node[index].newVolume * UCF(VOLUME); break;
            case SM_NODEFLOOD:
                *result = Node[index].overflow * UCF(FLOW); break;
            case SM_NODEDEPTH:
                *result = Node[index].newDepth * UCF(LENGTH); break;
            case SM_NODEHEAD:
                *result = (Node[index].newDepth
                            + Node[index].invertElev) * UCF(LENGTH); break;
            case SM_LATINFLOW:
                *result = Node[index].newLatFlow * UCF(FLOW); break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getNodePollut(int index, SM_NodePollut type, double **pollutArray, int *length)
///
/// Input:   index = Index of desired ID
///          type = Result Type (SM_NodePollut)
/// Output:  PollutArray pointer (pollutant data desired, byref)
/// Return:  API Error
/// Purpose: Gets Node Simulated Water Quality Value at Current Time
{
    int p;
    int error_code_index = 0;
    double* result;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else if (MEMCHECK(result = newDoubleArray(Nobjects[POLLUT])))
    {
        error_code_index = ERR_MEMORY;
    }

    else
    {
        switch (type)
        {
            case SM_NODEQUAL:
            {
                for (p = 0; p < Nobjects[POLLUT]; p++)
                {
                    result[p] = Node[index].newQual[p];
                } 
                *pollutArray = result;
                *length = Nobjects[POLLUT];
            } break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getLinkResult(int index, SM_LinkResult type, double *result)
///
/// Input:   index = Index of desired ID
///          type = Result Type (SM_LinkResult)
/// Output:  result = result data desired (byref)
/// Return:  API Error
/// Purpose: Gets Link Simulated Value at Current Time
{
    int error_code_index = 0;
    *result = 0;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch (type)
        {
            case SM_LINKFLOW:
                *result = Link[index].newFlow * UCF(FLOW) ; break;
            case SM_LINKDEPTH:
                *result = Link[index].newDepth * UCF(LENGTH); break;
            case SM_LINKVOL:
                *result = Link[index].newVolume * UCF(VOLUME); break;
            case SM_USSURFAREA:
                *result = Link[index].surfArea1 * UCF(LENGTH) * UCF(LENGTH); break;
            case SM_DSSURFAREA:
                *result = Link[index].surfArea2 * UCF(LENGTH) * UCF(LENGTH); break;
            case SM_SETTING:
                *result = Link[index].setting; break;
            case SM_TARGETSETTING:
                *result = Link[index].targetSetting; break;
            case SM_FROUDE:
                *result = Link[index].froude; break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getLinkPollut(int index, SM_LinkPollut type, double **pollutArray, int *length)
///
/// Input:   index = Index of desired ID
///          type = Result Type (SM_LinkPollut)
/// Output:  PollutArray pointer (pollutant data desired, byref)
/// Return:  API Error
/// Purpose: Gets Link Simulated Water Quality Value at Current Time
{
    int p;
    int error_code_index = 0;
    double* result;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else if (MEMCHECK(result = newDoubleArray(Nobjects[POLLUT])))
    {
        error_code_index = ERR_MEMORY;
    }

    else
    {
        switch (type)
        {
            case SM_LINKQUAL:
            {
                for (p = 0; p < Nobjects[POLLUT]; p++)
                {
                    result[p] = Link[index].newQual[p];
                } 
                *pollutArray = result;
                *length = Nobjects[POLLUT];
            } break;
            case SM_TOTALLOAD:
            {
                for (p = 0; p < Nobjects[POLLUT]; p++)
                {
                    result[p] = Link[index].totalLoad[p] * (LperFT3 * Pollut[p].mcf);
                    if (Pollut[p].units == COUNT)
                    {
                        result[p] = LOG10(result[p]);
                    }
                } 
                *pollutArray = result;
                *length = Nobjects[POLLUT];
            } break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        } 
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getSubcatchResult(int index, SM_SubcResult type, double* result)
///
/// Input:   index = Index of desired ID
///          type = Result Type (SM_SubcResult)
/// Output:  result = result data desired (byref)
/// Return:  API Error
/// Purpose: Gets Subcatchment Simulated Value at Current Time
{
    int error_code_index = 0;
    *result = 0;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch (type)
        {
            case SM_SUBCRAIN:
                *result = Subcatch[index].rainfall * UCF(RAINFALL); break;
            case SM_SUBCEVAP:
                *result = Subcatch[index].evapLoss * UCF(EVAPRATE); break;
            case SM_SUBCINFIL:
                *result = Subcatch[index].infilLoss * UCF(RAINFALL); break;
            case SM_SUBCRUNON:
                *result = Subcatch[index].runon * UCF(FLOW); break;
            case SM_SUBCRUNOFF:
                *result = Subcatch[index].newRunoff * UCF(FLOW); break;
            case SM_SUBCSNOW:
                *result = Subcatch[index].newSnowDepth * UCF(RAINDEPTH); break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getSubcatchPollut(int index, SM_SubcPollut type, double **pollutArray, int *length)
///
/// Input:   index = Index of desired ID
///          type = Result Type (SM_SubcPollut)
/// Output:  PollutArray pointer (pollutant data desired, byref)
/// Return:  API Error
/// Purpose: Gets Subcatchment Simulated Pollutant Value at Current Time
{
    int p;
    int error_code_index = 0;
    double a;
    double* result;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else if (MEMCHECK(result = newDoubleArray(Nobjects[POLLUT])))
    {
        error_code_index = ERR_MEMORY;
    }

    else
    {
        switch (type)
        {
            case SM_BUILDUP:
            {
                a = Subcatch[index].area;
                for (p = 0; p < Nobjects[POLLUT]; p++)
                {
                    result[p] = Subcatch[index].surfaceBuildup[p] /
                        (a * UCF(LANDAREA));
                } 
                *pollutArray = result;
                *length = Nobjects[POLLUT];
            } break;
            case SM_CPONDED:
            {
                for (p = 0; p < Nobjects[POLLUT]; p++)
                {
                    result[p] = Subcatch[index].concPonded[p] / LperFT3;
                } 
                *pollutArray = result;
                *length = Nobjects[POLLUT];
            } break;
            case SM_SUBCQUAL:
            {
                for (p = 0; p < Nobjects[POLLUT]; p++)
                {
                    result[p] = Subcatch[index].newQual[p];
                } 
                *pollutArray = result;
                *length = Nobjects[POLLUT];
            } break;
            case SM_SUBCTOTALLOAD:
            {
                for (p = 0; p < Nobjects[POLLUT]; p++)
                {
                    result[p] = Subcatch[index].totalLoad[p] * (LperFT3 * Pollut[p].mcf);
                    if (Pollut[p].units == COUNT)
                    {
                        result[p] = LOG10(result[p]);
                    }
                } 
                *pollutArray = result;
                *length = Nobjects[POLLUT];
            } break;

            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getGagePrecip(int index, SM_GagePrecip type, double* result)
///
/// Input:   index = Index of desired ID
///          type = Result type
/// Output:  result = Result data (byref)
/// Return:  API Error
/// Purpose: Gets the precipitation value in the gage.
{
    int error_code_index = 0;
    double rainfall = 0;
    double snowfall = 0;
    double total = 0;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[GAGE])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    // Read the rainfall value
    else
    {
        total = gage_getPrecip(index, &rainfall, &snowfall);
        switch(type)
        {
            case SM_TOTALPRECIP:
                *result = total * UCF(RAINFALL); break;
            case SM_RAINFALL:
                *result = rainfall * UCF(RAINFALL); break;
            case SM_SNOWFALL:
                *result = snowfall * UCF(RAINFALL); break;
            default: error_code_index = ERR_API_OUTBOUNDS; break;
        }
    }
    return error_getCode(error_code_index);
}


int DLLEXPORT swmm_getNodeStats(int index, SM_NodeStats *nodeStats)
///
/// Output:  Node Stats Structure (SM_NodeStats)
/// Return:  API Error
/// Purpose: Gets Node Stats and Converts Units
{
    int error_index = 0;
    
    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
        error_index = ERR_API_INPUTNOTOPEN;
    
    // Check if Simulation is Running
    else if (swmm_IsStartedFlag() == FALSE)
        error_index = ERR_API_SIM_NRUNNING;
    
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
        error_index = ERR_API_OBJECT_INDEX;

    else if (nodeStats == NULL)
        error_index = ERR_API_MEMORY;

    else
        stats_getNodeStat(index, (TNodeStats **)&nodeStats);

    return error_getCode(error_index);
}

int DLLEXPORT swmm_getNodeTotalInflow(int index, double* value)
///
/// Input:   Node Index
/// Output:  Node Total inflow Volume.
/// Return:  API Error
/// Purpose: Get Node Total Inflow Volume.
{
    int error_index = 0;

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
        error_index = ERR_API_INPUTNOTOPEN;

	// Check if Simulation is Running
    else if (swmm_IsStartedFlag() == FALSE)
        error_index = ERR_API_SIM_NRUNNING;

    else
        massbal_getNodeTotalInflow(index, value);

    return error_getCode(error_index);
}

int DLLEXPORT swmm_getStorageStats(int index, SM_StorageStats *storageStats)
///
/// Output:  Storage Node Stats Structure (SM_StorageStats)
/// Return:  API Error
/// Purpose: Gets Storage Node Stats and Converts Units
{
    int error_index = 0;
    
    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
        error_index = ERR_API_INPUTNOTOPEN;
    
    // Check if Simulation is Running
    else if (swmm_IsStartedFlag() == FALSE)
        error_index = ERR_API_SIM_NRUNNING;
    
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
        error_index = ERR_API_OBJECT_INDEX;
    
    // Check Node Type is storage
    else if (Node[index].type != STORAGE)
        error_index = ERR_API_WRONG_TYPE;
    
    else if (storageStats == NULL)
        error_index = ERR_API_MEMORY;

    else
        stats_getStorageStat(index, (TStorageStats **)&storageStats);

    return error_getCode(error_index);
}

int DLLEXPORT swmm_getOutfallStats(int index, SM_OutfallStats *outfallStats)
{
    int error_index = 0;
    
    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
        error_index = ERR_API_INPUTNOTOPEN;
    
    // Check if Simulation is Running
    else if (swmm_IsStartedFlag() == FALSE)
        error_index = ERR_API_SIM_NRUNNING;
    
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
        error_index = ERR_API_OBJECT_INDEX;
    
    // Check Node Type is outfall
    else if (Node[index].type != OUTFALL)
        error_index = ERR_API_WRONG_TYPE;
    
    else if (outfallStats == NULL)
        error_index = ERR_API_MEMORY;

    else
        stats_getOutfallStat(index, (TOutfallStats **)&outfallStats);

    return error_getCode(error_index);
}


int DLLEXPORT swmm_getLinkStats(int index, SM_LinkStats *linkStats)
///
/// Output:  Link Stats Structure (SM_LinkStats)
/// Return:  API Error
/// Purpose: Gets Link Stats and Converts Units
{
    int error_index = 0;

	// Check if Open
	if (swmm_IsOpenFlag() == FALSE)
		error_index = ERR_API_INPUTNOTOPEN;

	// Check if Simulation is Running
	else if (swmm_IsStartedFlag() == FALSE)
		error_index = ERR_API_SIM_NRUNNING;

	// Check if object index is within bounds
	else if (index < 0 || index >= Nobjects[LINK])
		error_index = ERR_API_OBJECT_INDEX;

    else if (linkStats == NULL)
        error_index = ERR_API_MEMORY;

    else
        stats_getLinkStat(index, (TLinkStats **)&linkStats);

    return error_getCode(error_index);
}


int DLLEXPORT swmm_getPumpStats(int index, SM_PumpStats *pumpStats)
///
/// Output:  Pump Link Stats Structure (SM_PumpStats)
/// Return:  API Error
/// Purpose: Gets Pump Link Stats and Converts Units
{
    int error_index = 0;

	// Check if Open
	if (swmm_IsOpenFlag() == FALSE)
		error_index = ERR_API_INPUTNOTOPEN;

	// Check if Simulation is Running
	else if (swmm_IsStartedFlag() == FALSE)
		error_index = ERR_API_SIM_NRUNNING;

	// Check if object index is within bounds
	else if (index < 0 || index >= Nobjects[LINK])
		error_index = ERR_API_OBJECT_INDEX;

	// Check if pump
	else if (Link[index].type != PUMP)
		error_index = ERR_API_WRONG_TYPE;

	else if (pumpStats == NULL)
        error_index = ERR_API_MEMORY;

    else
        stats_getPumpStat(index, (TPumpStats **)&pumpStats);

    return error_getCode(error_index);
}


int DLLEXPORT swmm_getSubcatchStats(int index, SM_SubcatchStats *subcatchStats)
///
/// Output:  Subcatchment Stats Structure (SM_SubcatchStats)
/// Return:  API Error
/// Purpose: Gets Subcatchment Stats and Converts Units
{
    int error_index = 0;

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
        error_index = ERR_API_INPUTNOTOPEN;

    // Check if Simulation is Running
    else if (swmm_IsStartedFlag() == FALSE)
        error_index = ERR_API_SIM_NRUNNING;

    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
        error_index = ERR_API_OBJECT_INDEX;

    else if (subcatchStats == NULL)
        error_index = ERR_API_MEMORY;

    else
        stats_getSubcatchStat(index, (TSubcatchStats **)&subcatchStats);

    return error_getCode(error_index);
}


int DLLEXPORT swmm_getSystemRoutingTotals(SM_RoutingTotals *routingTotals)
///
/// Output:  System Routing Totals Structure (SM_RoutingTotals)
/// Return:  API Error
/// Purpose: Gets System Flow Routing Totals and Converts Units
{
    int error_index = 0;

	// Check if Open
	if (swmm_IsOpenFlag() == FALSE)
		error_index = ERR_API_INPUTNOTOPEN;

	// Check if Simulation is Running
	else if (swmm_IsStartedFlag() == FALSE)
		error_index = ERR_API_SIM_NRUNNING;

    else if (routingTotals == NULL)
        error_index = ERR_API_MEMORY;
    
    else
        massbal_getRoutingTotal((TRoutingTotals **)&routingTotals);

    return error_getCode(error_index);
}

int DLLEXPORT swmm_getSystemRunoffTotals(SM_RunoffTotals *runoffTotals)
///
/// Output:  System Runoff Totals Structure (SM_RunoffTotals)
/// Return:  API Error
/// Purpose: Gets System Runoff Totals and Converts Units
{
    int error_index = 0;

	// Check if Open
	if (swmm_IsOpenFlag() == FALSE)
		error_index = ERR_API_INPUTNOTOPEN;

	// Check if Simulation is Running
	else if (swmm_IsStartedFlag() == FALSE)
		error_index = ERR_API_SIM_NRUNNING;

	else if (runoffTotals == NULL)
        error_index = ERR_API_MEMORY;
    
    else
        massbal_getRunoffTotal((TRunoffTotals **)&runoffTotals);

    return error_getCode(error_index);
}

int DLLEXPORT swmm_getLidUFluxRates(int index, int lidIndex, SM_LidLayer layerIndex, double* result)
//
// Input:   index = Index of desired subcatchment
//          lidIndex = Index of desired lid control (subcatchment allow for multiple lids)
//          layerIndex = Index of desired lid layer (default is surface)
// Output:  result = result data desired (byref)
// Return:  API Error
// Purpose: Gets Lid Unit Water Balance Simulated Value at Current Time
{
    int error_code_index = 0;
    TLidUnit* lidUnit;

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        lidUnit = lid_getLidUnit(index, lidIndex, &error_code_index);

        // There are no Lid Units defined for the subcatchments
        if (lidUnit)
        {
            switch (layerIndex)
            {
                case SM_SURFACE:
                    *result = lidUnit->oldFluxRates[SM_SURFACE] * UCF(LENGTH); break;
                case SM_SOIL:
                    *result = lidUnit->oldFluxRates[SM_SOIL] * UCF(LENGTH); break;
                case SM_STORAGE:
                    *result = lidUnit->oldFluxRates[SM_STORAGE] * UCF(LENGTH); break;
                case SM_PAVE:
                    *result = lidUnit->oldFluxRates[SM_PAVE] * UCF(LENGTH); break;
                default:
                    error_code_index = ERR_API_OUTBOUNDS; break;
            }
        }
        else
        {
            error_code_index = ERR_API_UNDEFINED_LID;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getLidGResult(int index, SM_LidResult type, double* result)
//
// Input:   index = index of desired subcatchment
//          type = type of result data desired
// Output:  result = result data desired
// Return:  API Error
// Purpose: Gets Lid Group Data at Current Time
{
    int error_code_index = 0;
    TLidGroup lidGroup;

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        lidGroup = lid_getLidGroup(index);

        if (lidGroup != NULL)
        {
            switch (type)
            {
                case SM_PERVAREA:
                    *result = lidGroup->pervArea * SQR(UCF(LENGTH)); break;
                case SM_FLOWTOPERV:
                    *result = lidGroup->flowToPerv * UCF(FLOW); break;
                case SM_OLDDRAINFLOW:
                    *result = lidGroup->oldDrainFlow * UCF(FLOW); break;
                case SM_NEWDRAINFLOW:
                    *result = lidGroup->newDrainFlow * UCF(FLOW); break;
                default:
                    error_code_index = ERR_API_OUTBOUNDS; break;
            }
        }
        else
        {
            error_code_index = ERR_API_UNDEFINED_LID;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_getLidUResult(int index, int lidIndex, SM_LidResult type, double* result)
//
// Input:   index = Index of desired subcatchment
//          lidIndex = Index of desired lid control (subcatchment allow for multiple lids)
// Output:  result = result data desired (byref)
// Return:  API Error
// Purpose: Gets Lid Unit Water Balance Simulated Value at Current Time
{
    int error_code_index = 0;
    TLidUnit* lidUnit;
    double    Tstep = 0;

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        lidUnit = lid_getLidUnit(index, lidIndex, &error_code_index);

        // There are no Lid Units defined for the subcatchments
        if (lidUnit)
        {
            switch (type)
            {
                case SM_INFLOW:
                    *result = lidUnit->waterBalance.inflow * UCF(RAINDEPTH); break;
                case SM_EVAP:
                    *result = lidUnit->waterBalance.evap * UCF(RAINDEPTH); break;
                case SM_INFIL:
                    *result = lidUnit->waterBalance.infil * UCF(RAINDEPTH); break;
                case SM_SURFFLOW:
                    *result = lidUnit->waterBalance.surfFlow * UCF(RAINDEPTH); break;
                case SM_DRAINFLOW:
                    *result = lidUnit->waterBalance.drainFlow * UCF(RAINDEPTH); break;
                case SM_INITVOL:
                    *result = lidUnit->waterBalance.initVol * UCF(RAINDEPTH); break;
                case SM_FINALVOL:
                    *result = lidUnit->waterBalance.finalVol * UCF(RAINDEPTH); break;
                case SM_SURFDEPTH:
                    *result = lidUnit->surfaceDepth * UCF(RAINDEPTH); break;
                case SM_PAVEDEPTH:
                    *result = lidUnit->paveDepth * UCF(RAINDEPTH); break;
                case SM_SOILMOIST:
                    *result = lidUnit->soilMoisture; break;
                case SM_STORDEPTH:
                    *result = lidUnit->storageDepth * UCF(RAINDEPTH); break;
                case SM_DRYTIME:
                    *result = lidUnit->dryTime; break;
                case SM_OLDDRAINFLOW:
                    *result = lidUnit->oldDrainFlow * UCF(FLOW); break;
                case SM_NEWDRAINFLOW:
                    *result = lidUnit->newDrainFlow * UCF(FLOW); break;
                case SM_EVAPRATE:
                    *result = lidUnit->waterRate.evap * UCF(RAINFALL); break;
                case SM_NATIVEINFIL:
                    *result = lidUnit->waterRate.maxNativeInfil * UCF(RAINFALL); break;
                case SM_SURFINFLOW:
                    *result = lidUnit->waterRate.surfaceInflow * UCF(RAINFALL); break;
                case SM_SURFINFIL:
                    *result = lidUnit->waterRate.surfaceInfil * UCF(RAINFALL); break;
                case SM_SURFEVAP:
                    *result = lidUnit->waterRate.surfaceEvap * UCF(RAINFALL); break;
                case SM_SURFOUTFLOW:
                    *result = lidUnit->waterRate.surfaceOutflow * UCF(RAINFALL); break;
                case SM_PAVEEVAP:
                    *result = lidUnit->waterRate.paveEvap * UCF(RAINFALL); break;
                case SM_PAVEPERC:
                    *result = lidUnit->waterRate.pavePerc * UCF(RAINFALL); break;
                case SM_SOILEVAP:
                    *result = lidUnit->waterRate.soilEvap * UCF(RAINFALL); break;
                case SM_SOILPERC:
                    *result = lidUnit->waterRate.soilPerc * UCF(RAINFALL); break;
                case SM_STORAGEINFLOW:
                    *result = lidUnit->waterRate.storageInflow * UCF(RAINFALL); break;
                case SM_STORAGEEXFIL:
                    *result = lidUnit->waterRate.storageExfil * UCF(RAINFALL); break;
                case SM_STORAGEEVAP:
                    *result = lidUnit->waterRate.storageEvap * UCF(RAINFALL); break;
                case SM_STORAGEDRAIN:
                    *result = lidUnit->waterRate.storageDrain * UCF(RAINFALL); break;
                default:
                    error_code_index = ERR_API_OUTBOUNDS; break;
            }
        }
        else
        {
            error_code_index = ERR_API_UNDEFINED_LID;
        }
    }
    return error_getCode(error_code_index);
}

//-------------------------------
// Setters API
//-------------------------------

int DLLEXPORT swmm_setLinkSetting(int index, double setting)
///
/// Input:   index = Index of desired ID
///          value = New Target Setting
/// Output:  returns API Error
/// Purpose: Sets Link open fraction (Weir, Orifice, Pump, and Outlet)
{
    DateTime currentTime;
    int error_code_index = 0;
    char _rule_[11] = "ToolkitAPI";

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        // --- check that new setting lies within feasible limits
        if (setting < 0.0) setting = 0.0;
        if (Link[index].type != PUMP && setting > 1.0) setting = 1.0;

        Link[index].targetSetting = setting;

        // Use internal function to apply the new setting
        link_setSetting(index, 0.0);

        // Add control action to RPT file if desired flagged
        if (RptFlags.controls)
        {
            currentTime = getDateTime(NewRoutingTime);
            report_writeControlAction(currentTime, Link[index].ID, setting, _rule_);
        }
    }
    return error_getCode(error_code_index);
}


int DLLEXPORT swmm_setNodeInflow(int index, double flowrate)
///
/// Input:   index = Index of desired ID
///          value = New Inflow Rate
/// Output:  returns API Error
/// Purpose: Sets new node inflow rate and holds until set again
{
    int error_code_index = 0;

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else
    {
        // Check to see if node has an assigned inflow object
        TExtInflow* inflow;

        // --- check if an external inflow object for this constituent already exists
        inflow = Node[index].extInflow;
        while (inflow)
        {
            if (inflow->param == -1) break;
            inflow = inflow->next;
        }

        if (!inflow)
        {
            int param = -1;        // FLOW (-1) or Pollutant Index
            int type = FLOW_INFLOW;// Type of inflow (FLOW)
            int tSeries = -1;      // No Time Series
            int basePat = -1;      // No Base Pattern
            double cf = 1.0;       // Unit Convert (Converted during validation)
            double sf = 1.0;       // Scaling Factor
            double baseline = 0.0; // Baseline Inflow Rate

            // Initializes Inflow Object
            error_code_index = inflow_setExtInflow(index, param, type, tSeries,
                basePat, cf, baseline, sf);

            // Get The Inflow Object
            if ( error_code_index == 0 )
            {
                inflow = Node[index].extInflow;
            }
        }
        // Assign new flow rate
        if ( error_code_index == 0 )
        {
            inflow -> extIfaceInflow = flowrate;
        }
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_setOutfallStage(int index, double stage)
///
/// Input:   index = Index of desired outfall
///          stage = New outfall stage (head)
/// Output:  returns API Error
/// Purpose: Sets new outfall stage and holds until set again.
{
    int error_code_index = 0;
    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if ( index < 0 || index >= Nobjects[NODE] )
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    else if ( Node[index].type != OUTFALL )
    {
        error_code_index = ERR_API_WRONG_TYPE;
    }
    else
    {
        int k = Node[index].subIndex;
        if ( Outfall[k].type != STAGED_OUTFALL )
        {
            // Change Boundary Conditions Setting Type
            Outfall[k].type = STAGED_OUTFALL;
        }
        Outfall[k].outfallStage = stage / UCF(LENGTH);
    }
    return error_getCode(error_code_index);
}

int DLLEXPORT swmm_setGagePrecip(int index, double total_precip)
///
/// Input:   index = Index of desired ID
///          total_precip = rainfall intensity to be set
/// Return:  API Error
/// Purpose: Sets the precipitation in from the external database
{
    int error_code_index = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        error_code_index = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[GAGE])
    {
        error_code_index = ERR_API_OBJECT_INDEX;
    }
    // Set the Rainfall rate
    else
    {
        if (Gage[index].dataSource != RAIN_API)
        {
            Gage[index].dataSource = RAIN_API;
        }
        if (Gage[index].isUsed == FALSE)
        {
            Gage[index].isUsed = TRUE;
        }
        if (Gage[index].coGage != -1)
        {
            Gage[index].coGage = -1;
        }
     Gage[index].externalRain = total_precip;
    }
    return error_getCode(error_code_index);
}

//-------------------------------
// Utility Functions
//-------------------------------

double* newDoubleArray(int n)
///
///  Warning: Caller must free memory allocated by this function.
///
{
    return (double*) malloc((n)*sizeof(double));
}


//void DLLEXPORT freeArray(void** array)
///
/// Helper function used to free array allocated memory by API.
///
//{
//    FREE(*array);
//    *array = NULL;
//}


void DLLEXPORT swmm_freeMemory(void *memory)
//
//  Purpose: Frees memory allocated by API calls
//
{
    free(memory);
}
