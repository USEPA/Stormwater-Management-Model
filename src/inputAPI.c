//-----------------------------------------------------------------------------
//   inputAPI.c
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

#include "swmm5.h"                     // declaration of exportable functions
#include "inputAPI.h"

int DLLEXPORT  swmm_countObjects(int type, int *count)
//
// Input: 	type = object type (Based on ObjectType enum)
//			count = pointer to integer
// Output: 	returns API Error
// Purpose: uses Object Count table to find number of elements of an object
{	
	int errorcode = 0;
	
	if(type >= MAX_OBJ_TYPES)
	{
		errorcode = 901;
		return errorcode;
	}
	*count = Nobjects[type];
	
	return errorcode;
}

int DLLEXPORT swmm_getObjectId(int type, int index, char *id)
//
// Input: 	type = object type (Based on ObjectType enum)
//			index = Index of desired ID
//			id = pointer to id pass by reference
// Output: 	returns API Error
// Purpose: Gets ID for any object
{
	//Provide Empty Character Array 
	strcpy(id,"");
	
	//Is *.inp file open?
	//if (!IsOpenFlag) return(ERR_NOT_OPEN);
	
	int errorcode = 0;
	if(type >= MAX_OBJ_TYPES)
	{
		errorcode = 901;
		return(errorcode);
	}
	
	if (index < 0 || index >= Nobjects[type]) return(99922);
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
   }

   return(errorcode);
}


int DLLEXPORT swmm_getNodeParam(int index, int Param, float *value)
//
// Input: 	type = object type (Based on ObjectType enum)
//			index = Index of desired ID
//			id = pointer to id pass by reference
// Output: 	returns API Error
// Purpose: Gets ID for any object
{
	// Add Checks later on (08/31/2016 ** BEM)
	// Add Enum later for values
	if (index < 0 || index >= Nobjects[NODE]) return(999232);
	
	switch(Param)
	{
		// Type
		case 0: *value = Node[index].type; break;
		// invertElev
		case 1: *value = Node[index].invertElev; break;			
		// fullDepth
		case 2: *value = Node[index].fullDepth; break;		
		// surDepth
		case 3: *value = Node[index].surDepth; break;	
		// pondedArea
		case 4: *value = Node[index].pondedArea; break;	
		// initDepth
		case 5: *value = Node[index].initDepth; break;					
	}
	return(0);
}


























