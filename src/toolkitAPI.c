//-----------------------------------------------------------------------------
//   toolkitAPI.c
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    08/30/2016
//   Author:  B. McDonnell (EmNet LLC)
//
//   Exportable Functions for Project Definition API.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers.h"

#include "swmm5.h"                     // declaration of exportable functions
#include "toolkitAPI.h"
#include "hash.h"
#include "funcs.h"

//-----------------------------------------------------------------------------
//  Extended API Functions
//-----------------------------------------------------------------------------
void DLLEXPORT swmm_getAPIError(int errcode, char *s)
//
// Input: 	errcode = error code
// Output: 	errmessage String 
// Return:  API Error
// Purpose: Get an error message
{
	char *errmsg = error_getMsg(errcode);
	strcpy(s, errmsg);
}

int DLLEXPORT swmm_getSimulationDateTime(int timetype, int *year, int *month, int *day,
	                                     int *hours, int *minutes, int *seconds)
//
// Input: 	timetype = time type to return
// Output: 	year, month, day, hours, minutes, seconds = int
// Return:  API Error
// Purpose: Get the simulation start, end and report date times
{
	int errcode = 0;
	// Check if Open
	if (swmm_IsOpenFlag() == FALSE)
	{
		errcode = ERR_API_INPUTNOTOPEN;
	}
	else
	{
		DateTime _dtime;
		switch (timetype)
		{
			//StartDateTime (globals.h)
		case 0: _dtime = StartDateTime; break;
			//EndDateTime (globals.h)
		case 1: _dtime = EndDateTime;  break;
			//ReportStart (globals.h)
		case 2: _dtime = ReportStart;  break;
			//Current Routing Time
		case 3: _dtime = NewRoutingTime; break;
		default: return(ERR_API_OUTBOUNDS);
		}
		datetime_decodeDate(_dtime, year, month, day);
		datetime_decodeTime(_dtime, hours, minutes, seconds);
	}

	return (errcode);
}

int DLLEXPORT swmm_setSimulationDateTime(int timetype, char *dtimestr)
//
// Input: 	timetype = time type to return
//          DateTime String 
// Return:  API Error
// Purpose: Get the simulation start, end and report date times
{
    char theDate[10];
    char theTime[9];

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);

    // Check if Simulation is Running
    if(swmm_IsStartedFlag() == TRUE) return(ERR_API_SIM_NRUNNING);

    strncpy(theDate, dtimestr, 10);
    strncpy(theTime, dtimestr+11, 9);

	switch(timetype)
	{
		//StartDateTime (globals.h)
		case 0: 
			project_readOption("START_DATE", theDate);
			project_readOption("START_TIME", theTime);
			StartDateTime = StartDate + StartTime;
			TotalDuration = floor((EndDateTime - StartDateTime) * SECperDAY);
			// --- convert total duration to milliseconds
			TotalDuration *= 1000.0;
			break;
		//EndDateTime (globals.h)
		case 1: 
			project_readOption("END_DATE", theDate);
			project_readOption("END_TIME", theTime);
			EndDateTime = EndDate + EndTime;
			TotalDuration = floor((EndDateTime - StartDateTime) * SECperDAY);
			// --- convert total duration to milliseconds
			TotalDuration *= 1000.0;
			break;		
		//ReportStart (globals.h)
		case 2: 
			project_readOption("REPORT_START_DATE", theDate);
			project_readOption("REPORT_START_TIME", theTime);
			ReportStart = ReportStartDate + ReportStartTime;
			break;			
		default: return(ERR_API_OUTBOUNDS);
	}
	
	return (0);
}

int DLLEXPORT  swmm_getSimulationUnit(int type, int *value)
//
// Input: 	type = simulation unit type
// Output: 	enum representation of units
// Returns: API Error
// Purpose: get simulation unit types
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	
	// Output unit
	switch(type)
	{
		// System Unit (enum.h UnitsType)
		case 0:  *value = UnitSystem; break;
		// Flow Unit (enum.h FlowUnitsType)
		case 1:  *value = FlowUnits; break;
		// Concentration Unit
		//case 2:  *value = UnitSystem; break;
		// Type not available
		default: return(ERR_API_OUTBOUNDS); 
	}
	
	return (0);
}

int DLLEXPORT  swmm_getSimulationAnalysisSetting(int type, int *value)
//
// Input: 	type = analysis type
// Output: 	setting True or False
// Returns: API Error
// Purpose: get simulation analysis setting
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	
	// Output  setting
	switch(type)
	{
		// No ponding at nodes (True or False)
		case 0:  *value = AllowPonding; break;
		// Do flow routing in steady state periods  (True or False)
		case 1:  *value = SkipSteadyState; break;
		// Analyze rainfall/runoff  (True or False)
		case 2:  *value = IgnoreRainfall; break;
		// Analyze RDII (True or False)
		case 3:  *value = IgnoreRDII; break;
		// Analyze snowmelt (True or False)
		case 4:  *value = IgnoreSnowmelt; break;
		// Analyze groundwater (True or False)
		case 5:  *value = IgnoreGwater; break;
		// Analyze flow routing (True or False)
		case 6:  *value = IgnoreRouting; break;
		// Analyze water quality (True or False)
		case 7:  *value = IgnoreQuality; break;	
		// Type not available
		default: return(ERR_API_OUTBOUNDS); 
	}
	
	return (0);	
}

int DLLEXPORT  swmm_getSimulationParam(int type, double *value)
//
// Input: 	type = analysis type
// Output: 	Simulation Parameter
// Returns: error code
// Purpose: Get simulation analysis parameter
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);

	// Output  setting
	switch(type)
	{
		// Routing time step (sec)
		case 0: *value = RouteStep; break;
		// Minimum variable time step (sec)
		case 1: *value = MinRouteStep; break;
		// Time step for lengthening (sec)
		case 2: *value = LengtheningStep; break;
		// Antecedent dry days 
		case 3: *value = StartDryDays; break;
		// Courant time step factor
		case 4: *value = CourantFactor; break;
		// Minimum nodal surface area
		case 5: *value = MinSurfArea; break;
		// Minimum conduit slope
		case 6: *value = MinSlope; break;
		// Runoff continuity error
		case 7: *value = RunoffError; break;
		// Groundwater continuity error	
		case 8: *value = GwaterError; break;
		// Flow routing error
		case 9: *value = FlowError; break;
		// Quality routing error
		case 10: *value = QualError; break;
		// DW routing head tolerance (ft)
		case 11: *value = HeadTol; break;
		// Tolerance for steady system flow	
		case 12: *value = SysFlowTol; break;
		// Tolerance for steady nodal inflow
		case 13: *value = LatFlowTol; break;    
		// Type not available
		default: return(ERR_API_OUTBOUNDS); 
	}
	return (0);
}

int DLLEXPORT  swmm_countObjects(int type, int *count)
//
// Input: 	type = object type (Based on ObjectType enum)
// Output: 	count = pointer to integer
// Returns: API Error
// Purpose: uses Object Count table to find number of elements of an object
{	
	if(type >= MAX_OBJ_TYPES)return ERR_API_OUTBOUNDS;
	*count = Nobjects[type];
	return (0);
}

int DLLEXPORT swmm_getObjectId(int type, int index, char *id)
//
// Input: 	type = object type (Based on ObjectType enum)
//			index = Index of desired ID
// Output: 	id = pointer to id pass by reference
// Return:  API Error
// Purpose: Gets ID for any object
{
	//Provide Empty Character Array 
	strcpy(id,"");
	
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[type]) return(ERR_API_OBJECT_INDEX);
	switch (type)
	{
		case GAGE:
			strcpy(id,Gage[index].ID); break;
		case SUBCATCH:
			strcpy(id,Subcatch[index].ID); break;
		case NODE:
			strcpy(id,Node[index].ID); break;
		case LINK:
			strcpy(id,Link[index].ID); break;
		case POLLUT:
			strcpy(id,Pollut[index].ID); break;
		case LANDUSE:
			strcpy(id,Landuse[index].ID); break;
		case TIMEPATTERN:
			strcpy(id,Pattern[index].ID); break;
		//case CURVE:
			//strcpy(id,Curve[index].ID); break;
		//case TSERIES:
			//strcpy(id,Tseries[index].ID); break;
		//case CONTROL:
			//strcpy(id,Rules[index].ID); break;
		case TRANSECT:
			strcpy(id,Transect[index].ID); break;
		case AQUIFER:
			strcpy(id,Aquifer[index].ID); break;
		case UNITHYD:
			strcpy(id,UnitHyd[index].ID); break;
		case SNOWMELT:
			strcpy(id,Snowmelt[index].ID); break;
		//case SHAPE:
		//strcpy(id,Shape[index].ID); break;
		//case LID:
		//	strcpy(id,LidProcs[index].ID); break;
		// Type not available
		default: return(ERR_API_OUTBOUNDS);
   }
   return(0);
}

int DLLEXPORT swmm_getNodeType(int index, int *Ntype)
//
// Input: 	index = Index of desired ID
//			Ntype = Node type (Based on enum NodeType)
// Return:  API Error
// Purpose: Gets Node Type
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);
	*Ntype = Node[index].type;
	return(0);
}

int DLLEXPORT swmm_getLinkType(int index, int *Ltype)
//
// Input: 	index = Index of desired ID
//			Ltype = Link type (Based on enum NodeType)
// Return:  API Error
// Purpose: Gets Link Type
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OBJECT_INDEX);
	*Ltype = Link[index].type;
	return(0);
}

int DLLEXPORT swmm_getLinkConnections(int index, int *Node1, int *Node2)
//
// Input: 	type = object type (Based on ObjectType enum)
//			index = Index of desired ID
// Output: 	Node1 and Node2
// Return:  API Error
// Purpose: Gets link Connection ID Indeces
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OBJECT_INDEX);
	*Node1 = Link[index].node1;
	*Node2 = Link[index].node2;
	return(0);
}

int DLLEXPORT swmm_getNodeParam(int index, int Param, double *value)
//
// Input: 	index = Index of desired ID
//			param = Parameter desired (Perhaps define enum )
// Output: 	value = value to be output
// Return:  API Error
// Purpose: Gets Node Parameter
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);
	
	switch(Param)
	{
		// invertElev
		case 0: *value = Node[index].invertElev * UCF(LENGTH); break;
		// fullDepth
		case 1: *value = Node[index].fullDepth * UCF(LENGTH); break;
		// surDepth
		case 2: *value = Node[index].surDepth * UCF(LENGTH); break;	
		// pondedArea
		case 3: *value = Node[index].pondedArea * UCF(LENGTH) * UCF(LENGTH); break;
		// initDepth
		case 4: *value = Node[index].initDepth * UCF(LENGTH); break;
		// Type not available
		default: return(ERR_API_OUTBOUNDS);
	}
	return(0);
}

int DLLEXPORT swmm_setNodeParam(int index, int Param, double value)
//
// Input: 	index = Index of desired ID
//			param = Parameter desired (Perhaps define enum )
//          value = value to be input
// Return:  API Error
// Purpose: Sets Node Parameter
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	// Check if Simulation is Running
	if(swmm_IsStartedFlag() == TRUE) return(ERR_API_SIM_NRUNNING);
	// Check if object index is within bounds	
	if (index < 0 || index >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);
	
	switch(Param)
	{
		// invertElev
		case 0: Node[index].invertElev = value / UCF(LENGTH); break;
		// fullDepth
		case 1: Node[index].fullDepth = value / UCF(LENGTH); break;
		// surDepth
		case 2: Node[index].surDepth = value / UCF(LENGTH); break;	
		// pondedArea
		case 3: Node[index].pondedArea = value / ( UCF(LENGTH) * UCF(LENGTH) ); break;
		// initDepth
		case 4: Node[index].initDepth = value / UCF(LENGTH); break;
		// Type not available
		default: return(ERR_API_OUTBOUNDS);
	}
	// Re-validated a node ******************** BEM 1/20/2017 Probably need to re-validate connecting links
	//node_validate(index);// incorprate callback here
	
	return(0);
}

int DLLEXPORT swmm_getLinkParam(int index, int Param, double *value)
//
// Input: 	index = Index of desired ID
//			param = Parameter desired (Perhaps define enum )
// Output: 	value = value to be output
// Return:  API Error
// Purpose: Gets Link Parameter
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OBJECT_INDEX);
	
	switch(Param)
	{
		// offset1
		case 0: *value = Link[index].offset1 * UCF(LENGTH); break;
		// offset2
		case 1: *value = Link[index].offset2 * UCF(LENGTH); break;
		// q0
		case 2: *value = Link[index].q0  * UCF(FLOW); break;
		// qLimit
		case 3: *value = Link[index].qLimit * UCF(FLOW); break;
		// cLossInlet
		case 4: *value = Link[index].cLossInlet; break;
		// cLossOutlet
		case 5: *value = Link[index].cLossOutlet; break;
		// cLossAvg
		case 6: *value = Link[index].cLossAvg; break;
		// seepRate
		//case 7: *value = Link[index].seepRate * UCF(FLOW); break;
		// Type not available
		default: return(ERR_API_OUTBOUNDS);
	}
	return(0);
}

int DLLEXPORT swmm_setLinkParam(int index, int Param, double value)
//
// Input: 	index = Index of desired ID
//			param = Parameter desired (Perhaps define enum )
//			value = value to be input
// Return:  API Error
// Purpose: Gets Link Parameter
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OBJECT_INDEX);
	
	switch(Param)
	{
		// offset1
		case 0: 
			// Check if Simulation is Running
			if(swmm_IsStartedFlag() == TRUE) return(ERR_API_SIM_NRUNNING);	
			Link[index].offset1 = value / UCF(LENGTH); break;
		// offset2
		case 1: 
			// Check if Simulation is Running
			if(swmm_IsStartedFlag() == TRUE) return(ERR_API_SIM_NRUNNING);	
			Link[index].offset2 = value / UCF(LENGTH); break;
		// q0
		case 2: 
			// Check if Simulation is Running
			if(swmm_IsStartedFlag() == TRUE) return(ERR_API_SIM_NRUNNING);	
			Link[index].q0 = value / UCF(FLOW); break;
		// qLimit
		case 3: Link[index].qLimit = value / UCF(FLOW); break;
		// cLossInlet
		case 4: Link[index].cLossInlet; break;
		// cLossOutlet
		case 5: Link[index].cLossOutlet; break;
		// cLossAvg
		case 6: Link[index].cLossAvg; break;
		// seepRate
		//case 7: *value = Link[index].seepRate * UCF(FLOW); break;
		// Type not available
		default: return(ERR_API_OUTBOUNDS);
	}
	// re-validated link
	//link_validate(index);// incorprate callback here
	
	return(0);
}

int DLLEXPORT swmm_getLinkDirection(int index, signed char *value)
//
// Input: 	index = Index of desired ID
// Output: 	Link Direction (Only changes is slope < 0)
// Return:  API Error
// Purpose: Gets Link Direction
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
		// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OBJECT_INDEX);
	*value = Link[index].direction;
	return(0);
}



int DLLEXPORT swmm_getSubcatchParam(int index, int Param, double *value)
//
// Input: 	index = Index of desired ID
//			param = Parameter desired (Perhaps define enum )
// Output: 	value = value to be output
// Return:  API Error
// Purpose: Gets Subcatchment Parameter
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
		// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[SUBCATCH]) return(ERR_API_OBJECT_INDEX);
	
	switch(Param)
	{
		// width
		case 0: *value = Subcatch[index].width * UCF(LENGTH); break;
		// area
		case 1: *value = Subcatch[index].area * UCF(LANDAREA); break;
		// fracImperv
		case 2: *value = Subcatch[index].fracImperv; break;	
		// slope
		case 3: *value = Subcatch[index].slope; break;	
		// curbLength
		case 4: *value = Subcatch[index].curbLength * UCF(LENGTH); break;
		// initBuildup
		//case 5: *value = Subcatch[index].initBuildup; break;
		// Type not available
		default: return(ERR_API_OUTBOUNDS);
	}
	return(0);
}

int DLLEXPORT swmm_setSubcatchParam(int index, int Param, double value)
//
// Input: 	index = Index of desired ID
//			param = Parameter desired (Perhaps define enum )
//          value = value to be output
// Return:  API Error
// Purpose: Sets Subcatchment Parameter
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	// Check if Simulation is Running
	if(swmm_IsStartedFlag() == TRUE) return(ERR_API_SIM_NRUNNING);
	// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[SUBCATCH]) return(ERR_API_OBJECT_INDEX);
	
	switch(Param)
	{
		// width
		case 0: Subcatch[index].width = value / UCF(LENGTH); break;
		// area
		case 1: Subcatch[index].area = value / UCF(LANDAREA); break;
		// fracImperv
		case 2: Subcatch[index].fracImperv; break;	
		// slope
		case 3: Subcatch[index].slope; break;	
		// curbLength
		case 4: Subcatch[index].curbLength = value / UCF(LENGTH); break;
		// initBuildup
		//case 5: *value = Subcatch[index].initBuildup; break;
		// Type not available
		default: return(ERR_API_OUTBOUNDS);
	}
	//re-validate subcatchment
	subcatch_validate(index); // incorprate callback here
	
	return(0);
}

int DLLEXPORT swmm_getSubcatchOutConnection(int index, int *type, int *Index )
//
// Input: 	type = object type (Based on ObjectType enum) (Subcatchments can load to Node or another Subcatchment)
//			index = Index of desired ID
// Output: 	Node1 and Node2
// Return:  API Error
// Purpose: Gets Subcatchment Connection ID Indeces for either Node or Subcatchment
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
		// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[SUBCATCH]) return(ERR_API_OBJECT_INDEX);
	
	if (Subcatch[index].outNode == -1 && Subcatch[index].outSubcatch == -1)
	{
		*Index = index; // Case of self Loading subcatchment
		*type = SUBCATCH;
	}
	if (Subcatch[index].outNode >= 0)
	{
		*Index = Subcatch[index].outNode;
		*type = NODE;
	}
	if (Subcatch[index].outSubcatch >= 0)
	{
		*Index = Subcatch[index].outSubcatch;
		*type = SUBCATCH;
	}
	
	return(0);
} 


//-------------------------------
// Active Simulation Results API
//-------------------------------

int DLLEXPORT swmm_getCurrentDateTimeStr(char *dtimestr)
//
// Output: 	DateTime String 
// Return:  API Error
// Purpose: Get the current simulation time
{
    //Provide Empty Character Array 
    char     theDate[12];
    char     theTime[9];
    char     _DTimeStr[22];
    DateTime currentTime;

    // Check if Simulation is Running
    if(swmm_IsStartedFlag() == FALSE) return(ERR_API_SIM_NRUNNING);

    // Fetch Current Time
    currentTime = getDateTime(NewRoutingTime);

    // Convert To Char
    datetime_dateToStr(currentTime, theDate);
    datetime_timeToStr(currentTime, theTime);

    strcpy(_DTimeStr, theDate);
    strcat(_DTimeStr, " ");
    strcat(_DTimeStr, theTime);

    strcpy(dtimestr, _DTimeStr);
    return(0);
}


int DLLEXPORT swmm_getNodeResult(int index, int type, double *result)
//
// Input: 	index = Index of desired ID	
//			type = Result Type
// Output: 	result = result data desired (byref)
// Return: 	API Error
// Purpose: Gets Node Simulated Value
{
	// Check if Simulation is Running
	if(swmm_IsStartedFlag() == FALSE) return(ERR_API_SIM_NRUNNING);
		// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);
	
	switch (type)
	{
		// Total Inflow
		case 0: *result = Node[index].inflow * UCF(FLOW); break; 
		// Total Outflow
		case 1: *result = Node[index].outflow * UCF(FLOW); break; 
		// Losses (evap + exfiltration loss)
		case 2: *result = Node[index].losses * UCF(FLOW); break; 
		// Current Volume
		case 3: *result = Node[index].newVolume * UCF(VOLUME); break; 
		// overflow
		case 4: *result = Node[index].overflow * UCF(FLOW); break; 
		// Current water depth
		case 5: *result = Node[index].newDepth * UCF(LENGTH); break; 
		// Current water head
		case 6: *result = (Node[index].newDepth + Node[index].invertElev) * UCF(LENGTH); break;
		// Current Lateral Inflow
		case 7: *result = Node[index].newLatFlow * UCF(FLOW); break;
		// Type not available
		default: return(ERR_API_OUTBOUNDS);
	}
	return(0);
}

int DLLEXPORT swmm_getLinkResult(int index, int type, double *result)
//
// Input: 	index = Index of desired ID	
//			type = Result Type
// Output: 	result = result data desired (byref)
// Return: 	API Error
// Purpose: Gets Node Simulated Value
{
	// Check if Simulation is Running
	if(swmm_IsStartedFlag() == FALSE) return(ERR_API_SIM_NRUNNING);
		// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OBJECT_INDEX);
	
	switch (type)
	{
		// Current Flow
		case 0: *result = Link[index].newFlow * UCF(FLOW) ; break; 
		// Current Depth
		case 1: *result = Link[index].newDepth * UCF(LENGTH); break; 
		// Current Volume
		case 2: *result = Link[index].newVolume * UCF(VOLUME); break; 
		// Upstream Surface Area
		case 3: *result = Link[index].surfArea1 * UCF(LENGTH) * UCF(LENGTH); break; 
		// Downstream Surface Area
		case 4: *result = Link[index].surfArea2 * UCF(LENGTH) * UCF(LENGTH); break; 
		// Current Setting
		case 5: *result = Link[index].setting; break; 
		// Target Setting
		case 6: *result = Link[index].targetSetting; break;
		// Froude number
		case 7: *result = Link[index].froude; break;
		// Type not available
		default: return(ERR_API_OUTBOUNDS);
	}
	return(0);
}

int DLLEXPORT swmm_getSubcatchResult(int index, int type, double *result)
//
// Input: 	index = Index of desired ID	
//			type = Result Type
// Output: 	result = result data desired (byref)
// Return: 	API Error
// Purpose: Gets Node Simulated Value
{
	// Check if Simulation is Running
	if(swmm_IsStartedFlag() == FALSE) return(ERR_API_SIM_NRUNNING);
	// Check if object index is within bounds	
	if (index < 0 || index >= Nobjects[SUBCATCH]) return(ERR_API_OBJECT_INDEX);
	
	switch (type)
	{
		// Current Rainfall
		case 0: *result = Subcatch[index].rainfall * UCF(RAINFALL); break; 
		// Current Evaporation Loss
		case 1: *result = Subcatch[index].evapLoss * UCF(EVAPRATE); break; 
		// Current Infiltration Loss
		case 2: *result = Subcatch[index].infilLoss * UCF(RAINFALL); break; 
		// Subcatchment Runon
		case 3: *result = Subcatch[index].runon * UCF(FLOW); break; 
		// Current Runoff
		case 4: *result = Subcatch[index].newRunoff * UCF(FLOW); break; 
		// Current Snow Depth
		case 5: *result = Subcatch[index].newSnowDepth * UCF(RAINDEPTH); break;
		// Type not available
		default: return(ERR_API_OUTBOUNDS);
	}
	return(0);
}


int DLLEXPORT swmm_getNodeStats(int index, TNodeStats *nodeStats)
//
// Output: 	Node Stats Structure (TNodeStats)
// Return: 	API Error
// Purpose: Gets Node Stats and Converts Units 
{
	int errorcode = stats_getNodeStat(index, nodeStats);
	
	if (errorcode == 0)
	{
		// Current Average Depth
		nodeStats->avgDepth *= (UCF(LENGTH) / (double)StepCount);
		// Current Maximum Depth
		nodeStats->maxDepth *= UCF(LENGTH);
		// Current Maximum Lateral Inflow
		nodeStats->maxLatFlow *= UCF(FLOW);
		// Current Maximum Inflow
		nodeStats->maxInflow *= UCF(FLOW);
		// Cumulative Lateral Inflow
		nodeStats->totLatFlow *= UCF(VOLUME);
		// Time Courant Critical (hrs)
		nodeStats->timeCourantCritical /= 3600.0;
		// Cumulative Flooded Volume
		nodeStats->volFlooded *= UCF(VOLUME);
		// Time Flooded (hrs)
		nodeStats->timeFlooded /= 3600.0;
		// Current Maximum Overflow
		nodeStats->maxOverflow *= UCF(FLOW);
		// Current Maximum Ponding Volume
		nodeStats->maxPondedVol *= UCF(VOLUME);
		// Time Surcharged 
		nodeStats->timeSurcharged /= 3600.0;
	}
	
	return (errorcode);
}

int DLLEXPORT swmm_getNodeTotalInflow(int index, double *value)
//
// Input:   Node Index
// Output: 	Node Total inflow Volume.
// Return: 	API Error
// Purpose: Get Node Total Inflow Volume.
{

	int errorcode = massbal_getNodeTotalInflow(index, value);

	if (errorcode == 0)
	{
		*value *= UCF(VOLUME);
	}

	return(errorcode);
}

int DLLEXPORT swmm_getStorageStats(int index, TStorageStats *storageStats)
//
// Output: 	Storage Node Stats Structure (TStorageStats)
// Return: 	API Error
// Purpose: Gets Storage Node Stats and Converts Units 
{
	int errorcode = stats_getStorageStat(index, storageStats);
	
	if (errorcode == 0)
	{
		// Initial Volume
		storageStats->initVol *= UCF(VOLUME);
		// Current Average Volume
		storageStats->avgVol *= (UCF(VOLUME) / (double)StepCount);
		// Current Maximum Volume
		storageStats->maxVol *= UCF(VOLUME);
		// Current Maximum Flow
		storageStats->maxFlow *= UCF(FLOW);
		// Current Evaporation Volume
		storageStats->evapLosses *= UCF(VOLUME);
		// Current Exfiltration Volume
		storageStats->exfilLosses *= UCF(VOLUME);
	}
	
	return (errorcode);	
}

int DLLEXPORT swmm_getOutfallStats(int index, TOutfallStats *outfallStats)
//
// Output: 	Outfall Stats Structure (TOutfallStats)
// Return: 	API Error
// Purpose: Gets Outfall Node Stats and Converts Units 
// Note:    Caller is responsible for calling swmm_freeOutfallStats
//          to free the pollutants array.
{
	int p;
	int errorcode = stats_getOutfallStat(index, outfallStats);

	if (errorcode == 0)
	{
		// Current Average Flow
        if ( outfallStats->totalPeriods > 0 )
		{
			outfallStats->avgFlow *= (UCF(FLOW) / (double)outfallStats->totalPeriods);
		}
        else
		{
			outfallStats->avgFlow *= 0.0;
		}
		// Current Maximum Flow
		outfallStats->maxFlow *= UCF(FLOW);
		// Convert Mass Units
		if (Nobjects[POLLUT] > 0)
		{
			for (p = 0; p < Nobjects[POLLUT]; p++)
				outfallStats->totalLoad[p] *= (LperFT3 * Pollut[p].mcf);
				if (Pollut[p].units == COUNT)
				{
					outfallStats->totalLoad[p] = LOG10(outfallStats->totalLoad[p]);
				}
		}
	}
	
	return (errorcode);	
}

void DLLEXPORT swmm_freeOutfallStats(TOutfallStats *outfallStats)
//
// Return: 	API Error
// Purpose: Frees Outfall Node Stats and Converts Units 
// Note:    API user is responsible for calling swmm_freeOutfallStats
//          since this function performs a memory allocation.
{
	FREE(outfallStats->totalLoad);
}



int DLLEXPORT swmm_getLinkStats(int index, TLinkStats *linkStats)
//
// Output: 	Link Stats Structure (TLinkStats)
// Return: 	API Error
// Purpose: Gets Link Stats and Converts Units 
{
	int errorcode = stats_getLinkStat(index, linkStats);
	
	if (errorcode == 0)
	{
		// Cumulative Maximum Flowrate
		linkStats->maxFlow *= UCF(FLOW);
		// Cumulative Maximum Velocity
		linkStats->maxVeloc *= UCF(LENGTH);
		// Cumulative Maximum Depth
		linkStats->maxDepth *= UCF(LENGTH);
		// Cumulative Time Normal Flow
		linkStats->timeNormalFlow /= 3600.0;
		// Cumulative Time Inlet Control
		linkStats->timeInletControl /= 3600.0;
		// Cumulative Time Surcharged
		linkStats->timeSurcharged /= 3600.0;
		// Cumulative Time Upstream Full
		linkStats->timeFullUpstream /= 3600.0;
		// Cumulative Time Downstream Full
		linkStats->timeFullDnstream /= 3600.0;
		// Cumulative Time Full Flow
		linkStats->timeFullFlow /= 3600.0;
		// Cumulative Time Capacity limited
		linkStats->timeCapacityLimited /= 3600.0;
		// Cumulative Time Courant Critical Flow
		linkStats->timeCourantCritical /= 3600.0;
	}
	
	return (errorcode);	
}


int DLLEXPORT swmm_getPumpStats(int index, TPumpStats *pumpStats)
//
// Output: 	Pump Link Stats Structure (TPumpStats)
// Return: 	API Error
// Purpose: Gets Pump Link Stats and Converts Units 
{
	int errorcode = stats_getPumpStat(index, pumpStats);

	if (errorcode == 0)
	{
		// Cumulative Minimum Flow
		pumpStats->minFlow *= UCF(FLOW);
		// Cumulative Average Flow
		if (pumpStats->totalPeriods > 0)
		{
			pumpStats->avgFlow *= (UCF(FLOW) / (double)pumpStats->totalPeriods);
		}
		else
		{
			pumpStats->avgFlow *= 0.0;
		}
		// Cumulative Maximum Flow
		pumpStats->maxFlow *= UCF(FLOW);
		// Cumulative Pumping Volume
		pumpStats->volume *= UCF(VOLUME);
	}
	
	return (errorcode);	
}



int DLLEXPORT swmm_getSubcatchStats(int index, TSubcatchStats *subcatchStats)
//
// Output: 	Subcatchment Stats Structure (TSubcatchStats)
// Return: 	API Error
// Purpose: Gets Subcatchment Stats and Converts Units 
{
	int errorcode = stats_getSubcatchStat(index, subcatchStats);
	
	if (errorcode == 0)
	{
		double a = Subcatch[index].area;

		// Cumulative Runon Volume
		subcatchStats->runon *= (UCF(RAINDEPTH) / a);
		// Cumulative Infiltration Volume
		subcatchStats->infil *= (UCF(RAINDEPTH) / a);
		// Cumulative Runoff Volume
		subcatchStats->runoff *= (UCF(RAINDEPTH) / a);
		// Maximum Runoff Rate
		subcatchStats->maxFlow *= UCF(FLOW);
		// Cumulative Rainfall Depth
		subcatchStats->precip *= (UCF(RAINDEPTH) / a);
		// Cumulative Evaporation Volume
		subcatchStats->evap *= (UCF(RAINDEPTH) / a);
	}
	
	return (errorcode);		
}


int DLLEXPORT swmm_getSystemRoutingStats(TRoutingTotals *routingTot)
//
// Output: 	System Routing Totals Structure (TRoutingTotals)
// Return: 	API Error
// Purpose: Gets System Flow Routing Totals and Converts Units 
{
	int errorcode = massbal_getRoutingFlowTotal(routingTot);

	if (errorcode == 0)
	{
		// Cumulative Dry Weather Inflow Volume
		routingTot->dwInflow *= UCF(VOLUME);
		// Cumulative Wet Weather Inflow Volume
		routingTot->wwInflow *= UCF(VOLUME);
		// Cumulative Groundwater Inflow Volume
		routingTot->gwInflow *= UCF(VOLUME);
		// Cumulative I&I Inflow Volume
		routingTot->iiInflow *= UCF(VOLUME);
		// Cumulative External Inflow Volume
		routingTot->exInflow *= UCF(VOLUME);
		// Cumulative Flooding Volume
		routingTot->flooding *= UCF(VOLUME);
		// Cumulative Outflow Volume
		routingTot->outflow  *= UCF(VOLUME);
		// Cumulative Evaporation Loss
		routingTot->evapLoss *= UCF(VOLUME);
		// Cumulative Seepage Loss
		routingTot->seepLoss *= UCF(VOLUME);
		// Continuity Error
		routingTot->pctError *= 100;
	}
	
	return(errorcode);
}

int DLLEXPORT swmm_getSystemRunoffStats(TRunoffTotals *runoffTot)
//
// Output: 	System Runoff Totals Structure (TRunoffTotals)
// Return: 	API Error
// Purpose: Gets System Runoff Totals and Converts Units
{
	int errorcode =  massbal_getRunoffTotal(runoffTot);

	if (errorcode == 0)
	{
		double TotalArea = massbal_getTotalArea();
		// Cumulative Rainfall Volume
		runoffTot->rainfall *= (UCF(RAINDEPTH) / TotalArea);
		// Cumulative Evaporation Volume
		runoffTot->evap *= (UCF(RAINDEPTH) / TotalArea);
		// Cumulative Infiltration Volume
		runoffTot->infil *= (UCF(RAINDEPTH) / TotalArea);
		// Cumulative Runoff Volume
		runoffTot->runoff *= (UCF(RAINDEPTH) / TotalArea);
		// Cumulative Runon Volume
		runoffTot->runon *= (UCF(RAINDEPTH) / TotalArea);
		// Cumulative Drain Volume
		runoffTot->drains *= (UCF(RAINDEPTH) / TotalArea);
		// Cumulative Snow Removed Volume
		runoffTot->snowRemoved *= (UCF(RAINDEPTH) / TotalArea);
		// Initial Storage Volume
		runoffTot->initStorage *= (UCF(RAINDEPTH) / TotalArea);
		// Initial Snow Cover Volume
		runoffTot->initSnowCover *= (UCF(RAINDEPTH) / TotalArea);
		// Continuity Error
		runoffTot->pctError *= 100;
	}

	return(errorcode);
}


//-------------------------------
// Setters API
//-------------------------------

int DLLEXPORT swmm_setLinkSetting(int index, double targetSetting)
//
// Input:   index = Index of desired ID
//          value = New Target Setting
// Output:  returns API Error
// Purpose: Sets Link open fraction (Weir, Orifice, Pump, and Outlet)
{
    DateTime currentTime;
    int errcode = 0;
    char _rule_[11] = "ToolkitAPI";

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        // --- check that new setting lies within feasible limits
        if (targetSetting < 0.0) targetSetting = 0.0;
        if (Link[index].type != PUMP && targetSetting > 1.0) targetSetting = 1.0;

        Link[index].targetSetting = targetSetting;

        // Use internal function to apply the new setting
        link_setSetting(index, 0.0);

        // Add control action to RPT file if desired flagged
        if (RptFlags.controls)
        {
            currentTime = getDateTime(NewRoutingTime);
            report_writeControlAction(currentTime, Link[index].ID, targetSetting, _rule_);
        }
    }
    return(errcode);
}


int DLLEXPORT swmm_setNodeInflow(int index, double flowrate)
//
// Input:   index = Index of desired ID
//          value = New Inflow Rate
// Output:  returns API Error
// Purpose: Sets new node inflow rate and holds until set again
{
    int errcode = 0;

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        errcode = ERR_API_OBJECT_INDEX;
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
            errcode = inflow_setExtInflow(index, param, type, tSeries,
                basePat, cf, baseline, sf);
        
            // Get The Inflow Object
            if ( errcode == 0 )
            {
                inflow = Node[index].extInflow;
            }
        }
        // Assign new flow rate
        if ( errcode == 0 )
        {
            inflow -> extIfaceInflow = flowrate;
        }
    }
    return(errcode);
}

int DLLEXPORT swmm_setOutfallStage(int index, double stage)
//
// Input:   index = Index of desired outfall
//          stage = New outfall stage (head)
// Output:  returns API Error
// Purpose: Sets new outfall stage and holds until set again.
{
    int errcode = 0;
    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if ( index < 0 || index >= Nobjects[NODE] )
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else if ( Node[index].type != OUTFALL )
    {
        errcode = ERR_API_WRONG_TYPE;
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
    return(errcode);
}