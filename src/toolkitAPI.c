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

int DLLEXPORT swmm_getSimulationDateTime(int timetype, char *dtimestr)
//
// Input: 	timetype = time type to return
// Output: 	DateTime String 
// Return:  API Error
// Purpose: Get the simulation start, end and report date times
{
	
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	
	char     theDate[12];
    char     theTime[9];
	char     _DTimeStr[22];
	DateTime _dtime;
	strcpy(dtimestr, "");
	
	switch(timetype)
	{
		//StartDateTime (globals.h)
		case 0: _dtime = StartDateTime; break;
		//EndDateTime (globals.h)
		case 1: _dtime = EndDateTime;  break;
		//ReportStart (globals.h)
		case 2: _dtime = ReportStart;  break;
		default: return(ERR_API_OUTBOUNDS);
	}
	datetime_dateToStr(_dtime, theDate);
	datetime_timeToStr(_dtime, theTime);

	strcpy(_DTimeStr, theDate);
	strcat(_DTimeStr, " ");
	strcat(_DTimeStr, theTime);
	
	strcpy(dtimestr, _DTimeStr);
	
	return (0);
}

int DLLEXPORT swmm_setSimulationDateTime(int timetype, char *dtimestr)
//
// Input: 	timetype = time type to return
//          DateTime String 
// Return:  API Error
// Purpose: Get the simulation start, end and report date times
{
	// Check if Open
	if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
	// Check if Simulation is Running
	if(swmm_IsStartedFlag() == TRUE) return(ERR_API_SIM_NRUNNING);	
	
	char     theDate[10];
    char     theTime[9];

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
	if (index < 0 || index >= Nobjects[type]) return(ERR_API_OUTBOUNDS);
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
	if (index < 0 || index >= Nobjects[NODE]) return(ERR_API_OUTBOUNDS);
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
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OUTBOUNDS);
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
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OUTBOUNDS);
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
	if (index < 0 || index >= Nobjects[NODE]) return(ERR_API_OUTBOUNDS);
	
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
	if (index < 0 || index >= Nobjects[NODE]) return(ERR_API_OUTBOUNDS);
	
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
	node_validate(index);// incorprate callback here
	
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
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OUTBOUNDS);
	
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
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OUTBOUNDS);
	
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
	link_validate(index);// incorprate callback here
	
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
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OUTBOUNDS);
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
	if (index < 0 || index >= Nobjects[SUBCATCH]) return(ERR_API_OUTBOUNDS);
	
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
	if (index < 0 || index >= Nobjects[SUBCATCH]) return(ERR_API_OUTBOUNDS);
	
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
	if (index < 0 || index >= Nobjects[SUBCATCH]) return(ERR_API_OUTBOUNDS);
	
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
	// Check if Simulation is Running
	if(swmm_IsStartedFlag() == FALSE) return(ERR_API_SIM_NRUNNING);
	
	//Provide Empty Character Array 
	
    char     theDate[12];
    char     theTime[9];
	char     _DTimeStr[22];
	
	DateTime currentTime;
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
	if (index < 0 || index >= Nobjects[NODE]) return(ERR_API_OUTBOUNDS);
	
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
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OUTBOUNDS);
	
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
	if (index < 0 || index >= Nobjects[SUBCATCH]) return(ERR_API_OUTBOUNDS);
	
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


int DLLEXPORT swmm_getSystemRoutingTotals(int type, double *value)
//
// Input: 	type = Result Type
// Output: 	volume = total data desired (Cu. Ft, Cu Meter)
//          routing error (decimal)
// Return: 	API Error
// Purpose: Gets System Routing Totals
{
	// Check if Open
	if (swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);

	// Check if Simulation is Running
	if (swmm_IsStartedFlag() == FALSE) return(ERR_API_SIM_NRUNNING);

	// Check if in bounds
	if (type >= 0 && type < 10)
	{
		*value = massbal_getRoutingFlowTotal(type);
		return(0);
	}
	else return(ERR_API_OUTBOUNDS);
}

int DLLEXPORT swmm_getSystemRunoffTotals(int type, double *value)
//
// Input: 	type = Result Type
// Output: 	volume = total data desired 
//          routing error (decimal)
// Return: 	API Error
// Purpose: Gets System Runoff Totals
{
	// Check if Open
	if (swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);

	// Check if Simulation is Running
	if (swmm_IsStartedFlag() == FALSE) return(ERR_API_SIM_NRUNNING);

	// Check if in bounds
	if (type >= 0 && type < 10)
	{
		*value = massbal_getRunoffTotal(type);
		return(0);
	}
	else return(ERR_API_OUTBOUNDS);
}


//-------------------------------
// Setters API
//-------------------------------

int DLLEXPORT swmm_setLinkSetting(int index, double targetSetting)
//
// Input: 	index = Index of desired ID
//			value = New Target Setting		
// Output: 	returns API Error
// Purpose: Sets Link open fraction (Weir, Orifice, Pump, and Outlet)
{
	// Check if Simulation is Running
	if(swmm_IsStartedFlag() == FALSE) return(ERR_API_SIM_NRUNNING);
	// Check if object index is within bounds	
	if (index < 0 || index >= Nobjects[LINK]) return(ERR_API_OUTBOUNDS);
	
	int l_type, errcode;
	
	// Get Link Type
	// errcode = swmm_getLinkType(index, &l_type);
	// WEIR, ORIFICES, PUMPS can have any value between [0,1]
	// CONDUIT can be only 0 or 1 * BEM 11/4/2016 investigate this...	
	
	Link[index].targetSetting  = targetSetting; 
	// Use internal function to apply the new setting
	link_setSetting(index, 0.0);
	
	// Add control action to RPT file if desired flagged
	if (RptFlags.controls)
	{
		DateTime currentTime;
		currentTime = getDateTime(NewRoutingTime);
		char _rule_[11] = "ToolkitAPI";
		report_writeControlAction(currentTime, Link[index].ID, targetSetting, _rule_);
	}
	return(0);
}


int DLLEXPORT swmm_setNodeInflow(int index, double flowrate)
//
// Input: 	index = Index of desired ID
//			value = New Inflow Rate 		
// Output: 	returns API Error
// Purpose: Sets new node inflow rate and holds until set again
{
	// Check if Simulation is Running
	if(swmm_IsStartedFlag() == FALSE) return(ERR_API_SIM_NRUNNING);
	// Check if object index is within bounds
	if (index < 0 || index >= Nobjects[NODE]) return(ERR_API_OUTBOUNDS);
	
	// Check to see if node has an assigned inflow object
	TExtInflow* inflow;
	inflow = Node[index].extInflow;
	if (!inflow)
	{
		// add inflow object then assign flow rate
		// nodeID	FLOW	""	FLOW	1.0	1	0   
		char *line[7];
		//2017-03-17: allocate memory for the 7 strings
		line[0] = (char *)malloc(100);
		line[1] = (char *)malloc(100);
		line[2] = (char *)malloc(100);
		line[3] = (char *)malloc(100);
		line[4] = (char *)malloc(100);
		line[5] = (char *)malloc(100);
		line[6] = (char *)malloc(100);

		int Ntokens = 7;
		// Get Node ID
		
		swmm_getObjectId(NODE, index, line[0]);
		// Insert Node ID into line
		line[1] = "FLOW";
		line[2] = "";
		line[3] = "FLOW";
		line[4] = "1.0";
		line[5] = "1";
		line[6] = "0";
		
		free(line[0]);
		free(line[1]);
		free(line[2]);
		free(line[3]);
		free(line[4]);
		free(line[5]);
		free(line[6]);
		
		// Add external inflow to linked list
		inflow_readExtInflow(line, Ntokens);
		
		// Get inflow
		inflow = Node[index].extInflow;
	}

	// Assign new flow rate
	inflow -> extIfaceInflow = flowrate;
	
	return(0);
}

