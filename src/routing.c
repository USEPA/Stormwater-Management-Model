//-----------------------------------------------------------------------------
//   routing.c
//
//   Project:  EPA SWMM5
//   Version:  5.0
//   Date:     6/19/07   (Build 5.0.010)
//             2/4/08    (Build 5.0.012)
//             3/11/08   (Build 5.0.013)
//             1/21/09   (Build 5.0.014)
//             4/10/09   (Build 5.0.015)
//             10/7/09   (Build 5.0.017)
//             11/18/09  (Build 5.0.018)
//             07/30/10  (Build 5.0.019)
//             04/20/11  (Build 5.0.022)
//   Author:   L. Rossman
//
//   Conveyance system routing functions.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//     Constants 
//-----------------------------------------------------------------------------
static const double LATERAL_FLOW_TOL = 0.5;  // for steady state (cfs)         //(5.0.012 - LR)
static const double FLOW_ERR_TOL = 0.05;     // for steady state               //(5.0.012 - LR)

//-----------------------------------------------------------------------------
// Shared variables
//-----------------------------------------------------------------------------
static int* SortedLinks;
static int  InSteadyState;

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
// routing_open            (called by swmm_start in swmm5.c)
// routing_getRoutingStep  (called by swmm_step in swmm5.c)
// routing_execute         (called by swmm_step in swmm5.c)
// routing_close           (called by swmm_end in swmm5.c)

//-----------------------------------------------------------------------------
// Function declarations
//-----------------------------------------------------------------------------
static int  openHotstartFile1(void);
static int  openHotstartFile2(void);
static void saveHotstartFile(void);
static void readHotstartFile(int fileVersion);                                 //(5.0.018 - LR)
static void addExternalInflows(DateTime currentDate);
static void addDryWeatherInflows(DateTime currentDate);
static void addWetWeatherInflows(double routingTime);
static void addGroundwaterInflows(double routingTime);
static void addRdiiInflows(DateTime currentDate);
static void addIfaceInflows(DateTime currentDate);
static void removeStorageLosses(void);                                         //(5.0.019 - LR)
static void removeOutflows(void);
static int  systemHasChanged(int routingModel);
static int  readFloat(float *x);                                               //(5.0.013 - LR)

//=============================================================================

int routing_open(int routingModel)
//
//  Input:   routingModel = routing method code
//  Output:  returns an error code
//  Purpose: initializes the routing analyzer.
//
{
    //  --- initialize steady state indicator
    InSteadyState = FALSE;

    // --- open treatment system
    if ( !treatmnt_open() ) return ErrorCode;

    // --- topologically sort the links
    SortedLinks = NULL;
    if ( Nobjects[LINK] > 0 )
    {
        SortedLinks = (int *) calloc(Nobjects[LINK], sizeof(int));
        if ( !SortedLinks )
        {
            report_writeErrorMsg(ERR_MEMORY, "");
            return ErrorCode;
        }
        toposort_sortLinks(SortedLinks);
        if ( ErrorCode ) return ErrorCode;
    }

    // --- open any routing interface files
    iface_openRoutingFiles();
    if ( ErrorCode ) return ErrorCode;

    // --- open hot start files
    if ( !openHotstartFile1() ) return ErrorCode;
    if ( !openHotstartFile2() ) return ErrorCode;

    // --- initialize the flow routing model
    flowrout_init(routingModel);
    return ErrorCode;
}

//=============================================================================

void routing_close(int routingModel)
//
//  Input:   routingModel = routing method code
//  Output:  none
//  Purpose: closes down the routing analyzer.
//
{
    // --- close hotstart file if in use
    if ( Fhotstart2.file )
    {
        // --- save latest results if called for
        if ( Fhotstart2.mode == SAVE_FILE ) saveHotstartFile();
        fclose(Fhotstart2.file);
    }

    // --- close any routing interface files
    iface_closeRoutingFiles();

    // --- free allocated memory
    flowrout_close(routingModel);
    treatmnt_close();
    FREE(SortedLinks);
}

//=============================================================================

double routing_getRoutingStep(int routingModel, double fixedStep)
//
//  Input:   routingModel = routing method code
//           fixedStep = user-supplied time step (sec)
//  Output:  returns a routing time step (sec)
//  Purpose: determines time step used for flow routing at current time period.
//
{
    if ( Nobjects[LINK] == 0 ) return fixedStep;
    else return flowrout_getRoutingStep(routingModel, fixedStep);
}

//=============================================================================

void routing_execute(int routingModel, double routingStep)
//
//  Input:   routingModel = routing method code
//           routingStep = routing time step (sec)
//  Output:  none
//  Purpose: executes the routing process at the current time period.
//
{
    int      j;
    int      stepCount = 1;
    int      actionCount = 0;                                                  //(5.0.010 - LR)
    DateTime currentDate;
    double   stepFlowError;                                                    //(5.0.012 - LR)

    // --- update continuity with current state
    //     applied over 1/2 of time step
    if ( ErrorCode ) return;
    massbal_updateRoutingTotals(routingStep/2.);

    // --- evaluate control rules at current date and elapsed time
    currentDate = getDateTime(NewRoutingTime);
    for (j=0; j<Nobjects[LINK]; j++) link_setTargetSetting(j);                 //(5.0.010 - LR)
    controls_evaluate(currentDate, currentDate - StartDateTime,                //(5.0.010 - LR)
                      routingStep/SECperDAY);                                  //(5.0.010 - LR)
    for (j=0; j<Nobjects[LINK]; j++)                                           //(5.0.010 - LR)
    {                                                                          //(5.0.010 - LR)
        if ( Link[j].targetSetting != Link[j].setting )                        //(5.0.010 - LR)
        {                                                                      //(5.0.010 - LR)
            link_setSetting(j, routingStep);                                   //(5.0.010 - LR)
            actionCount++;                                                     //(5.0.010 - LR)
        }                                                                      //(5.0.010 - LR)
    }                                                                          //(5.0.010 - LR)

    // --- update value of elapsed routing time (in milliseconds)
    OldRoutingTime = NewRoutingTime;
    NewRoutingTime = NewRoutingTime + 1000.0 * routingStep;
    currentDate = getDateTime(NewRoutingTime);

    // --- initialize mass balance totals for time step
    stepFlowError = massbal_getStepFlowError();                                //(5.0.012 - LR)
    massbal_initTimeStepTotals();

    // --- replace old water quality state with new state
    if ( Nobjects[POLLUT] > 0 )
    {
        for (j=0; j<Nobjects[NODE]; j++) node_setOldQualState(j);
        for (j=0; j<Nobjects[LINK]; j++) link_setOldQualState(j);
    }

    // --- add lateral inflows to nodes
    for (j = 0; j < Nobjects[NODE]; j++)
    {
        Node[j].oldLatFlow  = Node[j].newLatFlow;
        Node[j].newLatFlow  = 0.0;
    }
    addExternalInflows(currentDate);
    addDryWeatherInflows(currentDate);
    addWetWeatherInflows(NewRoutingTime);
    addGroundwaterInflows(NewRoutingTime);
    addRdiiInflows(currentDate);
    addIfaceInflows(currentDate);

    // --- check if can skip steady state periods
    if ( SkipSteadyState )
    {
        if ( OldRoutingTime == 0.0
        ||   actionCount > 0
        ||   fabs(stepFlowError) > FLOW_ERR_TOL                                //(5.0.012 - LR)
        ||   systemHasChanged(routingModel) ) InSteadyState = FALSE;
        else InSteadyState = TRUE;
    }

    // --- find new hydraulic state if system has changed
    if ( InSteadyState == FALSE )
    {
        // --- replace old hydraulic state values with current ones
        for (j = 0; j < Nobjects[LINK]; j++) link_setOldHydState(j);
        for (j = 0; j < Nobjects[NODE]; j++)
        {
            node_setOldHydState(j);
            node_initInflow(j, routingStep);
        }

        // --- route flow through the drainage network
        if ( Nobjects[LINK] > 0 )
        {
            stepCount = flowrout_execute(SortedLinks, routingModel, routingStep);
        }
    }

    // --- route quality through the drainage network
    if ( Nobjects[POLLUT] > 0 && !IgnoreQuality )                              //(5.0.014 - LR)
    {
        qualrout_execute(routingStep);
    }

    // --- remove evaporation, infiltration & system outflows from nodes       //(5.0.015 - LR)
    removeStorageLosses();                                                     //(5.0.019 - LR)
    removeOutflows();
	
    // --- update continuity with new totals
    //     applied over 1/2 of routing step
    massbal_updateRoutingTotals(routingStep/2.);

    // --- update summary statistics
    if ( RptFlags.flowStats && Nobjects[LINK] > 0 )
    {
        stats_updateFlowStats(routingStep, currentDate, stepCount, InSteadyState);
    }
}

//=============================================================================

void addExternalInflows(DateTime currentDate)
//
//  Input:   currentDate = current date/time
//  Output:  none
//  Purpose: adds direct external inflows to nodes at current date.
//
{
    int     j, p;
    double  q, w;
    TExtInflow* inflow;

    // --- for each node with a defined external inflow
    for (j = 0; j < Nobjects[NODE]; j++)
    {
        inflow = Node[j].extInflow;
        if ( !inflow ) continue;

        // --- get flow inflow
        q = 0.0;
        while ( inflow )
        {
            if ( inflow->type == FLOW_INFLOW )
            {
                q = inflow_getExtInflow(inflow, currentDate);
                break;
            }
            else inflow = inflow->next;
        }
        if ( fabs(q) < FLOW_TOL ) q = 0.0;

        // --- add flow inflow to node's lateral inflow
        Node[j].newLatFlow += q;
        massbal_addInflowFlow(EXTERNAL_INFLOW, q);

        // --- add on any inflow (i.e., reverse flow) through an outfall       //(5.0.014 - LR)
        if ( Node[j].type == OUTFALL && Node[j].oldNetInflow < 0.0 )           //(5.0.014 - LR)
        {                                                                      //(5.0.014 - LR)
            q = q - Node[j].oldNetInflow;                                      //(5.0.014 - LR)
        }                                                                      //(5.0.014 - LR)

        // --- get pollutant mass inflows
        inflow = Node[j].extInflow;
        while ( inflow )
        {
            if ( inflow->type != FLOW_INFLOW )
            {
                p = inflow->param;
                w = inflow_getExtInflow(inflow, currentDate);
                if ( inflow->type == CONCEN_INFLOW ) w *= q;
                Node[j].newQual[p] += w;
                massbal_addInflowQual(EXTERNAL_INFLOW, p, w);
            }
            inflow = inflow->next;
        }
    }
}

//=============================================================================

void addDryWeatherInflows(DateTime currentDate)
//
//  Input:   currentDate = current date/time
//  Output:  none
//  Purpose: adds dry weather inflows to nodes at current date.
//
{
    int      j, p;
    int      month, day, hour;
    double   q, w;
    TDwfInflow* inflow;

    // --- get month (zero-based), day-of-week (zero-based),
    //     & hour-of-day for routing date/time
    month = datetime_monthOfYear(currentDate) - 1;
    day   = datetime_dayOfWeek(currentDate) - 1;
    hour  = datetime_hourOfDay(currentDate);

    // --- for each node with a defined dry weather inflow
    for (j = 0; j < Nobjects[NODE]; j++)
    {
        inflow = Node[j].dwfInflow;
        if ( !inflow ) continue;

        // --- get flow inflow (i.e., the inflow whose param code is -1)
        q = 0.0;
        while ( inflow )
        {
            if ( inflow->param < 0 )
            {
                q = inflow_getDwfInflow(inflow, month, day, hour);
                break;
            }
            inflow = inflow->next;
        }
        if ( fabs(q) < FLOW_TOL ) q = 0.0;

        // --- add flow inflow to node's lateral inflow
        Node[j].newLatFlow += q;
        massbal_addInflowFlow(DRY_WEATHER_INFLOW, q);

////  Following code segment added to release 5.0.017.  ////                   //(5.0.017 - LR)
        // --- add default DWF pollutant inflows
        for ( p = 0; p < Nobjects[POLLUT]; p++)
        {
            if ( Pollut[p].dwfConcen > 0.0 )
            {
                w = q * Pollut[p].dwfConcen;
                Node[j].newQual[p] += w;
                massbal_addInflowQual(DRY_WEATHER_INFLOW, p, w);
            }
        }
////  End of new code segment  ////                                            //(5.0.017 - LR)

        // --- get pollutant mass inflows
        inflow = Node[j].dwfInflow;
        while ( inflow )
        {
            if ( inflow->param >= 0 )
            {
                p = inflow->param;
                w = q * inflow_getDwfInflow(inflow, month, day, hour);
                Node[j].newQual[p] += w;
                massbal_addInflowQual(DRY_WEATHER_INFLOW, p, w);

////  Following code segment added to release 5.0.017.  ////                   //(5.0.017 - LR)
                // --- subtract off any default inflow
                if ( Pollut[p].dwfConcen > 0.0 )
                {
                    w = q * Pollut[p].dwfConcen;
                    Node[j].newQual[p] -= w;
                    massbal_addInflowQual(DRY_WEATHER_INFLOW, p, -w);
                }
////  End of new code segment  ////                                            //(5.0.017 - LR)
            }
            inflow = inflow->next;
        }
    }
}

//=============================================================================

void addWetWeatherInflows(double routingTime)
//
//  Input:   routingTime = elasped time (millisec)
//  Output:  none
//  Purpose: adds runoff inflows to nodes at current elapsed time.
//
{
    int    i, j, p;
    double q, w;
    double f;

    // --- find where current routing time lies between latest runoff times
    if ( Nobjects[SUBCATCH] == 0 ) return;
    f = (routingTime - OldRunoffTime) / (NewRunoffTime - OldRunoffTime);
    if ( f < 0.0 ) f = 0.0;
    if ( f > 1.0 ) f = 1.0;

    // for each subcatchment outlet node,
    // add interpolated runoff flow & pollutant load to node's inflow
    for (i = 0; i < Nobjects[SUBCATCH]; i++)
    {
        j = Subcatch[i].outNode;
        if ( j >= 0)
        {
            // add runoff flow to lateral inflow
            q = subcatch_getWtdOutflow(i, f);     // current runoff flow
//          if ( fabs(q) < FLOW_TOL ) q = 0.0;                                 //(5.0.014 - LR)
            Node[j].newLatFlow += q;
            massbal_addInflowFlow(WET_WEATHER_INFLOW, q);

            // add pollutant load
            for (p = 0; p < Nobjects[POLLUT]; p++)
            {
                w = q * subcatch_getWtdWashoff(i, p, f);
                Node[j].newQual[p] += w;
                massbal_addInflowQual(WET_WEATHER_INFLOW, p, w);
            }
        }
    }
}

//=============================================================================

void addGroundwaterInflows(double routingTime)
//
//  Input:   routingTime = elasped time (millisec)
//  Output:  none
//  Purpose: adds groundwater inflows to nodes at current elapsed time.
//
{
    int    i, j, p;
    double q, w;
    double f;
    TGroundwater* gw;

    // --- find where current routing time lies between latest runoff times
    if ( Nobjects[SUBCATCH] == 0 ) return;
    f = (routingTime - OldRunoffTime) / (NewRunoffTime - OldRunoffTime);
    if ( f < 0.0 ) f = 0.0;
    if ( f > 1.0 ) f = 1.0;

    // --- for each subcatchment
    for (i = 0; i < Nobjects[SUBCATCH]; i++)
    {
        // --- see if subcatch contains groundwater
        gw = Subcatch[i].groundwater;
        if ( gw )
        {
            // --- identify node receiving groundwater flow
            j = gw->node;
            if ( j >= 0 )
            {
                // add groundwater flow to lateral inflow
                q = ( (1.0 - f)*(gw->oldFlow) + f*(gw->newFlow) )
                    * Subcatch[i].area;
                if ( fabs(q) < FLOW_TOL ) continue;
                Node[j].newLatFlow += q;
                massbal_addInflowFlow(GROUNDWATER_INFLOW, q);

                // add pollutant load (for positive inflow)
                if ( q > 0.0 )
                {
                    for (p = 0; p < Nobjects[POLLUT]; p++)
                    {
                        w = q * Pollut[p].gwConcen;
                        Node[j].newQual[p] += w;
                        massbal_addInflowQual(GROUNDWATER_INFLOW, p, w);
                    }
                }
            }
        }
    }
}

//=============================================================================

void addRdiiInflows(DateTime currentDate)
//
//  Input:   currentDate = current date/time
//  Output:  none
//  Purpose: adds RDII inflows to nodes at current date.
//
{
    int    i, j, p;
    double q, w;
    int    numRdiiNodes;

    // --- see if any nodes have RDII at current date
    numRdiiNodes = rdii_getNumRdiiFlows(currentDate);

    // --- add RDII flow to each node's lateral inflow
    for (i=0; i<numRdiiNodes; i++)
    {
        rdii_getRdiiFlow(i, &j, &q);
        if ( j < 0 ) continue;
        if ( fabs(q) < FLOW_TOL ) continue;
        Node[j].newLatFlow += q;
        massbal_addInflowFlow(RDII_INFLOW, q);

        // add pollutant load (for positive inflow)
        if ( q > 0.0 )
        {
            for (p = 0; p < Nobjects[POLLUT]; p++)
            {
                w = q * Pollut[p].rdiiConcen;                                  //(5.0.012 - LR)
                Node[j].newQual[p] += w;
                massbal_addInflowQual(RDII_INFLOW, p, w);
            }
        }
    }
}

//=============================================================================

void addIfaceInflows(DateTime currentDate)
//
//  Input:   currentDate = current date/time
//  Output:  none
//  Purpose: adds inflows from routing interface file to nodes at current date.
//
{
    int    i, j, p;
    double q, w;
    int    numIfaceNodes;

    // --- see if any nodes have interface inflows at current date
    if ( Finflows.mode != USE_FILE ) return;
    numIfaceNodes = iface_getNumIfaceNodes(currentDate);

    // --- add interface flow to each node's lateral inflow
    for (i=0; i<numIfaceNodes; i++)
    {
        j = iface_getIfaceNode(i);
        if ( j < 0 ) continue;
        q = iface_getIfaceFlow(i);
        if ( fabs(q) < FLOW_TOL ) continue;
        Node[j].newLatFlow += q;
        massbal_addInflowFlow(EXTERNAL_INFLOW, q);

        // add pollutant load (for positive inflow)
        if ( q > 0.0 )
        {
            for (p = 0; p < Nobjects[POLLUT]; p++)
            {
                w = q * iface_getIfaceQual(i, p);
                Node[j].newQual[p] += w;
                massbal_addInflowQual(EXTERNAL_INFLOW, p, w);
            }
        }
    }
}

//=============================================================================

int  systemHasChanged(int routingModel)
//
//  Input:   none
//  Output:  returns TRUE if external inflows or hydraulics have changed
//           from the previous time step
//  Purpose: checks if the hydraulic state of the system has changed from
//           the previous time step.
//
{
    int    j;                                                                  //(5.0.012 - LR)
    double diff;

    // --- check if external inflows or outflows have changed                  //(5.0.012 - LR)
    for (j=0; j<Nobjects[NODE]; j++)
    {
        diff = Node[j].oldLatFlow - Node[j].newLatFlow;
        if ( fabs(diff) > LATERAL_FLOW_TOL ) return TRUE;                      //(5.0.012 - LR)
        if ( Node[j].type == OUTFALL || Node[j].degree == 0 )                  //(5.0.012 - LR)
        {                                                                      //(5.0.012 - LR)
            diff = Node[j].oldFlowInflow - Node[j].inflow;                     //(5.0.012 - LR)
            if ( fabs(diff) > LATERAL_FLOW_TOL ) return TRUE;                  //(5.0.012 - LR)
        }                                                                      //(5.0.012 - LR)
    }

//// Start of deprecated code block.  ////                                     //(5.0.012 - LR)
/*
    // --- if system was already in steady state & there are no changes
    //     in inflows, then system must remain in steady state
    if ( InSteadyState ) return FALSE;

    // --- check for changes in node volume
    for (j=0; j<Nobjects[NODE]; j++)
    {
        diff = Node[j].newVolume - Node[j].oldVolume;
        if ( fabs(diff) > VOLUME_TOL ) return TRUE;
    }

    // --- check for other routing changes
    switch (routingModel)
    {
    // --- for dynamic wave routing, check if node depths have changed
    case DW:
        for (j=0; j<Nobjects[NODE]; j++)
        {
            diff = Node[j].oldDepth - Node[j].newDepth;
            if ( fabs(diff) > DEPTH_TOL ) return TRUE;
        }
        break;

    // --- for other routing methods, check if flows have changed
    case SF:
    case KW:
        for (j=0; j<Nobjects[LINK]; j++)
        {
            if ( Link[j].type == CONDUIT )
            {
                k = Link[j].subIndex;
                diff = Conduit[k].q1Old - Conduit[k].q1;
                if ( fabs(diff) > FLOW_TOL ) return TRUE;
                diff = Conduit[k].q2Old - Conduit[k].q2;
                if ( fabs(diff) > FLOW_TOL ) return TRUE;
            }
            else
            {
                diff = Link[j].oldFlow - Link[j].newFlow;
                if ( fabs(diff) > FLOW_TOL ) return TRUE;
            }
        }
        break;
    default: return TRUE;
    }
*/
////  End of deprecated code block.  ////                                      //(5.0.012 - LR)
    return FALSE;
}

//=============================================================================

////  This function replaces removeLosses from earlier releases.  ////         //(5.0.019 - LR)

void removeStorageLosses()
//
//  Input:   routingStep = routing time step (sec)
//  Output:  none
//  Purpose: adds mass lost from storage nodes to evaporation & infiltration
//           over current time step to overall mass balance.
//
{
    int i, j, p;
    double vRatio;
    double losses = 0.0;

    // --- check each storage node
    for ( i = 0; i < Nobjects[NODE]; i++ )
    {
        if (Node[i].type == STORAGE)
        {

            // --- update total system storage losses
            losses += Storage[Node[i].subIndex].losses;

            // --- adjust storage concentrations for any evaporation loss
            if ( Nobjects[POLLUT] > 0 && Node[i].newVolume > FUDGE )
            {
                j = Node[i].subIndex;
                vRatio = 1.0 + (Storage[j].evapLoss / Node[i].newVolume);
                for ( p = 0; p < Nobjects[POLLUT]; p++ )
                {
                    Node[i].newQual[p] *= vRatio;
                }
            }
        }
    }
    massbal_addNodeLosses(losses);
}

//=============================================================================

void removeOutflows()
//
//  Input:   none
//  Output:  none
//  Purpose: finds flows that leave the system and adds these to mass
//           balance totals.
//
{
    int    i, p;
    int    isFlooded;
    double q, w;

    for ( i = 0; i < Nobjects[NODE]; i++ )
    {
        // --- determine flows leaving the system
        q = node_getSystemOutflow(i, &isFlooded);
        if ( q != 0.0 )
        {
            massbal_addOutflowFlow(q, isFlooded);
            for ( p = 0; p < Nobjects[POLLUT]; p++ )
            {
                w = q * Node[i].newQual[p];
                massbal_addOutflowQual(p, w, isFlooded);
            }
        }
    }
}

//=============================================================================

////  Function re-written to read groundwater states.                ////      //(5.0.018 - LR)

int openHotstartFile1()
//
//  Input:   none
//  Output:  none
//  Purpose: opens a previously saved hotstart file.
//
{
    int   nSubcatch;
    int   nNodes;
    int   nLinks;
    int   nPollut;
    int   flowUnits;
    char  fileStamp[] = "SWMM5-HOTSTART";
    char  fStamp[] = "SWMM5-HOTSTART";
    char  fileStamp2[] = "SWMM5-HOTSTART2";
    char  fStamp2[] = "SWMM5-HOTSTART2";
    int   fileVersion;

    // --- try to open the file
    if ( Fhotstart1.mode != USE_FILE ) return TRUE;
    if ( (Fhotstart1.file = fopen(Fhotstart1.name, "r+b")) == NULL)
    {
        report_writeErrorMsg(ERR_HOTSTART_FILE_OPEN, Fhotstart1.name);
        return FALSE;
    }

    // --- check that file contains proper header records
    fread(fStamp2, sizeof(char), strlen(fileStamp2), Fhotstart1.file);
    if ( strcmp(fStamp2, fileStamp2) == 0 ) fileVersion = 2;
    else 
    {
        rewind(Fhotstart1.file);
        fread(fStamp, sizeof(char), strlen(fileStamp), Fhotstart1.file);
        if ( strcmp(fStamp, fileStamp) != 0 )
        {
            report_writeErrorMsg(ERR_HOTSTART_FILE_FORMAT, "");
            return FALSE;
        }
        fileVersion = 1;
    }
    nSubcatch = -1;
    nNodes = -1;
    nLinks = -1;
    nPollut = -1;
    flowUnits = -1;
    if ( fileVersion == 2 )
    {    
        fread(&nSubcatch, sizeof(int), 1, Fhotstart1.file);
    }
    else nSubcatch = Nobjects[SUBCATCH];
    fread(&nNodes, sizeof(int), 1, Fhotstart1.file);
    fread(&nLinks, sizeof(int), 1, Fhotstart1.file);
    fread(&nPollut, sizeof(int), 1, Fhotstart1.file);
    fread(&flowUnits, sizeof(int), 1, Fhotstart1.file);
    if ( nSubcatch != Nobjects[SUBCATCH] 
    ||   nNodes != Nobjects[NODE]
    ||   nLinks != Nobjects[LINK]
    ||   nPollut   != Nobjects[POLLUT]
    ||   flowUnits != FlowUnits )
    {
         report_writeErrorMsg(ERR_HOTSTART_FILE_FORMAT, "");
         return FALSE;
    }

    // --- read contents of the file and close it
    readHotstartFile(fileVersion);
    fclose(Fhotstart1.file);
    if ( ErrorCode ) return FALSE;
    else return TRUE;
}

//=============================================================================

////  Function re-written to save groundwater states.                ////      //(5.0.018 - LR)

int openHotstartFile2()
//
//  Input:   none
//  Output:  none
//  Purpose: opens a new hotstart file to save results to.
//
{
    int   nSubcatch;
    int   nNodes;
    int   nLinks;
    int   nPollut;
    int   flowUnits;
    char  fileStamp[] = "SWMM5-HOTSTART2";

    // --- try to open file
    if ( Fhotstart2.mode != SAVE_FILE ) return TRUE;
    if ( (Fhotstart2.file = fopen(Fhotstart2.name, "w+b")) == NULL)
    {
        report_writeErrorMsg(ERR_HOTSTART_FILE_OPEN, Fhotstart2.name);
        return FALSE;
    }

    // --- write file stamp & number of objects to file
    nSubcatch = Nobjects[SUBCATCH];
    nNodes = Nobjects[NODE];
    nLinks = Nobjects[LINK];
    nPollut = Nobjects[POLLUT];
    flowUnits = FlowUnits;
    fwrite(fileStamp, sizeof(char), strlen(fileStamp), Fhotstart2.file);
    fwrite(&nSubcatch, sizeof(int), 1, Fhotstart2.file);
    fwrite(&nNodes, sizeof(int), 1, Fhotstart2.file);
    fwrite(&nLinks, sizeof(int), 1, Fhotstart2.file);
    fwrite(&nPollut, sizeof(int), 1, Fhotstart2.file);
    fwrite(&flowUnits, sizeof(int), 1, Fhotstart2.file);
    return TRUE;
}

//=============================================================================

////  Function re-written to save groundwater states.                ////      //(5.0.018 - LR)

void  saveHotstartFile(void)
//
//  Input:   none
//  Output:  none
//  Purpose: saves current state of all nodes and links to hotstart file.
//
{
    int   i, j;
    float zero = 0.0f;
    float x[3];
    TGroundwater* gw;

    for (i = 0; i < Nobjects[SUBCATCH]; i++)
    {
        gw = Subcatch[i].groundwater;
        if ( gw == NULL )
        {
            x[0] = -1.0f;
            x[1] = -1.0f;
        }
        else
        {
            x[0] = (float)Subcatch[i].groundwater->theta;
            x[1] = (float)(Aquifer[gw->aquifer].bottomElev + gw->lowerDepth);
        }
        fwrite(x, sizeof(float), 2, Fhotstart2.file);
    }

    for (i = 0; i < Nobjects[NODE]; i++)
    {
        x[0] = (float)Node[i].newDepth;
        x[1] = (float)Node[i].newLatFlow;
        fwrite(x, sizeof(float), 2, Fhotstart2.file);
        for (j = 0; j < Nobjects[POLLUT]; j++)
        {
            x[0] = (float)Node[i].newQual[j];
            fwrite(&x[0], sizeof(float), 1, Fhotstart2.file);
        }

        // --- write out 0 here for back compatibility
        for (j = 0; j < Nobjects[POLLUT]; j++ )
            fwrite(&zero, sizeof(float), 1, Fhotstart2.file);
    }
    for (i = 0; i < Nobjects[LINK]; i++)
    {
        x[0] = (float)Link[i].newFlow;
        x[1] = (float)Link[i].newDepth;
        x[2] = (float)Link[i].setting;
        fwrite(x, sizeof(float), 3, Fhotstart2.file);
        for (j = 0; j < Nobjects[POLLUT]; j++)
        {
            x[0] = (float)Link[i].newQual[j];
            fwrite(&x[0], sizeof(float), 1, Fhotstart2.file);
        }
    }
}

//=============================================================================

////  Function re-written to trap invalid numbers in hotstart file.  ////      //(5.0.013 - LR)
////  Function re-written to read groundwater states.                ////      //(5.0.018 - LR)

void readHotstartFile(int fileVersion)
//
//  Input:   fileVersion = hot start file version 
//  Output:  none
//  Purpose: reads initial state of all nodes, links and groundwater objects
//           from hotstart file.
//
{
    int   i, j;
    long  pos, size, size1;
    float x;
    float theta, elev;
    TGroundwater* gw;

    // --- check that file has correct size
    pos = ftell(Fhotstart1.file);
    fseek(Fhotstart1.file, 0L, SEEK_END);
    size = ( ftell(Fhotstart1.file) - pos ) / sizeof(float);
    size1 = Nobjects[NODE] * (2 + 2*Nobjects[POLLUT]) +
                Nobjects[LINK] * (3 + Nobjects[POLLUT]);
    if ( fileVersion == 2 ) size1 += Nobjects[SUBCATCH] * 2; 
    if ( size < size1 )
    {
         report_writeErrorMsg(ERR_HOTSTART_FILE_READ, "");
         return;
    }
    fseek(Fhotstart1.file, pos, SEEK_SET);

    // --- read in subcatchment groundwater states
    if ( fileVersion == 2 )
    {
        for ( i = 0; i < Nobjects[SUBCATCH]; i++)
        {
            if ( !readFloat(&theta) ) return;
            if ( !readFloat(&elev) ) return;
            gw = Subcatch[i].groundwater;
            if ( gw == NULL ) continue;
            if ( theta >= 0.0 ) gw->theta = theta;                             //(5.0.022 - LR)
            if ( elev != MISSING )                                             //(5.0.022 - LR)
                gw->lowerDepth = elev - Aquifer[gw->aquifer].bottomElev;
        }
    }

    // --- read node states
    for (i = 0; i < Nobjects[NODE]; i++)
    {
        if ( !readFloat(&x) ) return;
        Node[i].newDepth = x;
        if ( !readFloat(&x) ) return;
        Node[i].newLatFlow = x;
        for (j = 0; j < Nobjects[POLLUT]; j++)
        {
            if ( !readFloat(&x) ) return;
            Node[i].newQual[j] = x;
        }

        // --- read in zero here for back compatibility
        for (j = 0; j < Nobjects[POLLUT]; j++)
        {
            if ( !readFloat(&x) ) return;
        }
    }

    // --- read link states
    for (i = 0; i < Nobjects[LINK]; i++)
    {
        if ( !readFloat(&x) ) return;
        Link[i].newFlow = x;
        if ( !readFloat(&x) ) return;
        Link[i].newDepth = x;
        if ( !readFloat(&x) ) return;
        Link[i].setting = x;
        for (j = 0; j < Nobjects[POLLUT]; j++)
        {
            if ( !readFloat(&x) ) return;
            Link[i].newQual[j] = x;
        }
    }
}

//=============================================================================

////  New function added.  ////                                                //(5.0.013 - LR)

int  readFloat(float *x)
//
//  Input:   none
//  Output:  x  = pointer to a float variable
//  Purpose: reads a floating point value from the hotstart file
//
{
    // --- read a value from the file
    fread(x, sizeof(float), 1, Fhotstart1.file);

    // --- test if the value is NaN (not a number)
    if ( *(x) != *(x) )
    {
        report_writeErrorMsg(ERR_HOTSTART_FILE_READ, "");
        *(x) = 0.0;
        return FALSE;
    }
    return TRUE;
}

//=============================================================================
