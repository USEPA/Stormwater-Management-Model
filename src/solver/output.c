//-----------------------------------------------------------------------------
//   output.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14  (Build 5.1.001)
//             03/19/15  (Build 5.1.008)
//             08/05/15  (Build 5.1.010)
//             05/10/18  (Build 5.1.013)
//   Author:   L. Rossman (EPA)
//
//   Binary output file access functions.
//
//   Build 5.1.008:
//   - Possible divide by zero for reported system wide variables avoided.
//   - Updating of maximum node depth at reporting times added.
//
//   Build 5.1.010:
//   - Potentional ET added to list of system-wide variables saved to file.
//
//   Build 5.1.013:
//   - Names NsubcatchVars, NnodeVars & NlinkVars replaced with
//     NumSubcatchVars, NumNodeVars & NumLinkVars 
//   - Support added for saving average node & link routing results to
//     binary file in each reporting period.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "headers.h"


// Definition of 4-byte integer, 4-byte real and 8-byte real types
#define INT4  int
#define REAL4 float
#define REAL8 double

enum InputDataType {INPUT_TYPE_CODE, INPUT_AREA, INPUT_INVERT, INPUT_MAX_DEPTH,
                    INPUT_OFFSET, INPUT_LENGTH};

typedef struct                                                                 //(5.1.013)
{                                                                              //
    REAL4* xAvg;                                                               //
}   TAvgResults;                                                               //

//-----------------------------------------------------------------------------
//  Shared variables    
//-----------------------------------------------------------------------------
static INT4      IDStartPos;           // starting file position of ID names
static INT4      InputStartPos;        // starting file position of input data
static INT4      OutputStartPos;       // starting file position of output data
static INT4      BytesPerPeriod;       // bytes saved per simulation time period
static INT4      NumSubcatchVars;      // number of subcatchment output variables
static INT4      NumNodeVars;          // number of node output variables
static INT4      NumLinkVars;          // number of link output variables
static INT4      NumSubcatch;          // number of subcatchments reported on
static INT4      NumNodes;             // number of nodes reported on
static INT4      NumLinks;             // number of links reported on
static INT4      NumPolluts;           // number of pollutants reported on
static REAL4     SysResults[MAX_SYS_RESULTS];    // values of system output vars.

static TAvgResults* AvgLinkResults;                                            //(5.1.013)
static TAvgResults* AvgNodeResults;                                            //
static int          Nsteps;                                                    //

//-----------------------------------------------------------------------------
//  Exportable variables (shared with report.c)
//-----------------------------------------------------------------------------
REAL4*           SubcatchResults;
REAL4*           NodeResults;
REAL4*           LinkResults;


//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void output_openOutFile(void);
static void output_saveID(char* id, FILE* file);
static void output_saveSubcatchResults(double reportTime, FILE* file);
static void output_saveNodeResults(double reportTime, FILE* file);
static void output_saveLinkResults(double reportTime, FILE* file);

static int  output_openAvgResults(void);                                       //(5.1.013)
static void output_closeAvgResults(void);                                      //
static void output_initAvgResults(void);                                       //
static void output_saveAvgResults(FILE* file);                                 //


//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  output_open                   (called by swmm_start in swmm5.c)
//  output_end                    (called by swmm_end in swmm5.c)
//  output_close                  (called by swmm_close in swmm5.c)
//  output_updateAvgResults       (called by swmm_step in swmm5.c)             //(5.1.013)
//  output_saveResults            (called by swmm_step in swmm5.c)
//  output_checkFileSize          (called by swmm_report)
//  output_readDateTime           (called by routines in report.c)
//  output_readSubcatchResults    (called by report_Subcatchments)
//  output_readNodeResults        (called by report_Nodes)
//  output_readLinkResults        (called by report_Links)


//=============================================================================

int output_open()
//
//  Input:   none
//  Output:  returns an error code
//  Purpose: writes basic project data to binary output file.
//
{
    int   j;
    int   m;
    INT4  k;
    REAL4 x;
    REAL8 z;

    // --- open binary output file
    output_openOutFile();
    if ( ErrorCode ) return ErrorCode;

    // --- ignore pollutants if no water quality analsis performed
    if ( IgnoreQuality ) NumPolluts = 0;
    else NumPolluts = Nobjects[POLLUT];

    // --- subcatchment results consist of Rainfall, Snowdepth, Evap, 
    //     Infil, Runoff, GW Flow, GW Elev, GW Sat, and Washoff
    NumSubcatchVars = MAX_SUBCATCH_RESULTS - 1 + NumPolluts;

    // --- node results consist of Depth, Head, Volume, Lateral Inflow,
    //     Total Inflow, Overflow and Quality
    NumNodeVars = MAX_NODE_RESULTS - 1 + NumPolluts;

    // --- link results consist of Depth, Flow, Velocity, Volume,              //(5.1.013)
    //     Capacity and Quality
    NumLinkVars = MAX_LINK_RESULTS - 1 + NumPolluts;

    // --- get number of objects reported on
    NumSubcatch = 0;
    NumNodes = 0;
    NumLinks = 0;
    for (j=0; j<Nobjects[SUBCATCH]; j++) if (Subcatch[j].rptFlag) NumSubcatch++;
    for (j=0; j<Nobjects[NODE]; j++) if (Node[j].rptFlag) NumNodes++;
    for (j=0; j<Nobjects[LINK]; j++) if (Link[j].rptFlag) NumLinks++;

    BytesPerPeriod = sizeof(REAL8)
        + NumSubcatch * NumSubcatchVars * sizeof(REAL4)
        + NumNodes * NumNodeVars * sizeof(REAL4)
        + NumLinks * NumLinkVars * sizeof(REAL4)
        + MAX_SYS_RESULTS * sizeof(REAL4);
    Nperiods = 0;

    SubcatchResults = NULL;
    NodeResults = NULL;
    LinkResults = NULL;
    SubcatchResults = (REAL4 *) calloc(NumSubcatchVars, sizeof(REAL4));
    NodeResults = (REAL4 *) calloc(NumNodeVars, sizeof(REAL4));
    LinkResults = (REAL4 *) calloc(NumLinkVars, sizeof(REAL4));
    if ( !SubcatchResults || !NodeResults || !LinkResults )
    {
        report_writeErrorMsg(ERR_MEMORY, "");
        return ErrorCode;
    }

    // --- allocate memory to store average node & link results per period     //(5.1.013)
    AvgNodeResults = NULL;                                                     //
    AvgLinkResults = NULL;                                                     //
    if ( RptFlags.averages && !output_openAvgResults() )                       //
    {                                                                          //
        report_writeErrorMsg(ERR_MEMORY, "");                                  //
        return ErrorCode;                                                      //
    }                                                                          //

    fseek(Fout.file, 0, SEEK_SET);
    k = MAGICNUMBER;
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // Magic number
    k = VERSION;
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // Version number
    k = FlowUnits;
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // Flow units
    k = NumSubcatch;
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // # subcatchments
    k = NumNodes;
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // # nodes
    k = NumLinks;
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // # links
    k = NumPolluts;
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // # pollutants

    // --- save ID names of subcatchments, nodes, links, & pollutants 
    IDStartPos = ftell(Fout.file);
    for (j=0; j<Nobjects[SUBCATCH]; j++)
    {
        if ( Subcatch[j].rptFlag ) output_saveID(Subcatch[j].ID, Fout.file);
    }
    for (j=0; j<Nobjects[NODE];     j++)
    {
        if ( Node[j].rptFlag ) output_saveID(Node[j].ID, Fout.file);
    }
    for (j=0; j<Nobjects[LINK];     j++)
    {
        if ( Link[j].rptFlag ) output_saveID(Link[j].ID, Fout.file);
    }
    for (j=0; j<NumPolluts; j++) output_saveID(Pollut[j].ID, Fout.file);

    // --- save codes of pollutant concentration units
    for (j=0; j<NumPolluts; j++)
    {
        k = Pollut[j].units;
        fwrite(&k, sizeof(INT4), 1, Fout.file);
    }

    InputStartPos = ftell(Fout.file);

    // --- save subcatchment area
    k = 1;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_AREA;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    for (j=0; j<Nobjects[SUBCATCH]; j++)
    {
         if ( !Subcatch[j].rptFlag ) continue;
         SubcatchResults[0] = (REAL4)(Subcatch[j].area * UCF(LANDAREA));
         fwrite(&SubcatchResults[0], sizeof(REAL4), 1, Fout.file);
    }

    // --- save node type, invert, & max. depth
    k = 3;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_TYPE_CODE;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_INVERT;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_MAX_DEPTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    for (j=0; j<Nobjects[NODE]; j++)
    {
        if ( !Node[j].rptFlag ) continue;
        k = Node[j].type;
        NodeResults[0] = (REAL4)(Node[j].invertElev * UCF(LENGTH));
        NodeResults[1] = (REAL4)(Node[j].fullDepth * UCF(LENGTH));
        fwrite(&k, sizeof(INT4), 1, Fout.file);
        fwrite(NodeResults, sizeof(REAL4), 2, Fout.file);
    }

    // --- save link type, offsets, max. depth, & length
    k = 5;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_TYPE_CODE;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_OFFSET;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_OFFSET;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_MAX_DEPTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_LENGTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);

    for (j=0; j<Nobjects[LINK]; j++)
    {
        if ( !Link[j].rptFlag ) continue;
        k = Link[j].type;
        if ( k == PUMP )
        {
            for (m=0; m<4; m++) LinkResults[m] = 0.0f;
        }
        else
        {
            LinkResults[0] = (REAL4)(Link[j].offset1 * UCF(LENGTH));
            LinkResults[1] = (REAL4)(Link[j].offset2 * UCF(LENGTH));
            if ( Link[j].direction < 0 )
            {
                x = LinkResults[0];
                LinkResults[0] = LinkResults[1];
                LinkResults[1] = x;
            }
            if ( k == OUTLET ) LinkResults[2] = 0.0f;
            else LinkResults[2] = (REAL4)(Link[j].xsect.yFull * UCF(LENGTH));
            if ( k == CONDUIT )
            {
                m = Link[j].subIndex;
                LinkResults[3] = (REAL4)(Conduit[m].length * UCF(LENGTH));
            }
            else LinkResults[3] = 0.0f;
        }
        fwrite(&k, sizeof(INT4), 1, Fout.file);
        fwrite(LinkResults, sizeof(REAL4), 4, Fout.file);
    }

    // --- save number & codes of subcatchment result variables
    k = NumSubcatchVars;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_RAINFALL;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_SNOWDEPTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_EVAP;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_INFIL;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_RUNOFF;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_GW_FLOW;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_GW_ELEV;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_SOIL_MOIST;
    fwrite(&k, sizeof(INT4), 1, Fout.file);

    for (j=0; j<NumPolluts; j++) 
    {
        k = SUBCATCH_WASHOFF + j;
        fwrite(&k, sizeof(INT4), 1, Fout.file);
    }

    // --- save number & codes of node result variables
    k = NumNodeVars;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_DEPTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_HEAD;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_VOLUME;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_LATFLOW;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_INFLOW;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_OVERFLOW;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    for (j=0; j<NumPolluts; j++)
    {
        k = NODE_QUAL + j;
        fwrite(&k, sizeof(INT4), 1, Fout.file);
    }

    // --- save number & codes of link result variables
    k = NumLinkVars;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = LINK_FLOW;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = LINK_DEPTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = LINK_VELOCITY;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = LINK_VOLUME;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = LINK_CAPACITY;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    for (j=0; j<NumPolluts; j++)
    {
        k = LINK_QUAL + j;
        fwrite(&k, sizeof(INT4), 1, Fout.file);
    }

    // --- save number & codes of system result variables
    k = MAX_SYS_RESULTS;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    for (k=0; k<MAX_SYS_RESULTS; k++) fwrite(&k, sizeof(INT4), 1, Fout.file);

    // --- save starting report date & report step
    //     (if reporting start date > simulation start date then
    //      make saved starting report date one reporting period
    //      prior to the date of the first reported result)
    z = (double)ReportStep/86400.0;
    if ( StartDateTime + z > ReportStart ) z = StartDateTime;
    else
    {
        z = floor((ReportStart - StartDateTime)/z) - 1.0;
        z = StartDateTime + z*(double)ReportStep/86400.0;
    }
    fwrite(&z, sizeof(REAL8), 1, Fout.file);
    k = ReportStep;
    if ( fwrite(&k, sizeof(INT4), 1, Fout.file) < 1)
    {
        report_writeErrorMsg(ERR_OUT_WRITE, "");
        return ErrorCode;
    }
    OutputStartPos = ftell(Fout.file);
    if ( Fout.mode == SCRATCH_FILE ) output_checkFileSize();
    return ErrorCode;
}

//=============================================================================

void  output_checkFileSize()
//
//  Input:   none
//  Output:  none
//  Purpose: checks if the size of the binary output file will be too big
//           to access using an integer file pointer variable.
//
{
    if ( RptFlags.subcatchments != NONE ||
         RptFlags.nodes != NONE ||
         RptFlags.links != NONE )
    {
        if ( (double)OutputStartPos + (double)BytesPerPeriod * TotalDuration
             / 1000.0 / (double)ReportStep >= (double)MAXFILESIZE )
        {
            report_writeErrorMsg(ERR_FILE_SIZE, "");
        }
    }
}


//=============================================================================

void output_openOutFile()
//
//  Input:   none
//  Output:  none
//  Purpose: opens a project's binary output file.
//
{
    // --- close output file if already opened
    if (Fout.file != NULL) fclose(Fout.file); 

    // --- else if file name supplied then set file mode to SAVE
    else if (strlen(Fout.name) != 0) Fout.mode = SAVE_FILE;

    // --- otherwise set file mode to SCRATCH & generate a name
    else
    {
        Fout.mode = SCRATCH_FILE;
        getTempFileName(Fout.name);
    }

    // --- try to open the file
    if ( (Fout.file = fopen(Fout.name, "w+b")) == NULL)
    {
        writecon(FMT14);
        ErrorCode = ERR_OUT_FILE;
    }
}

//=============================================================================

void output_saveResults(double reportTime)
//
//  Input:   reportTime = elapsed simulation time (millisec)
//  Output:  none
//  Purpose: writes computed results for current report time to binary file.
//
{
    int i;
    extern TRoutingTotals StepFlowTotals;  // defined in massbal.c             //(5.1.013)
    DateTime reportDate = getDateTime(reportTime);
    REAL8 date;

    // --- initialize system-wide results
    if ( reportDate < ReportStart ) return;
    for (i=0; i<MAX_SYS_RESULTS; i++) SysResults[i] = 0.0f;

    // --- save date corresponding to this elapsed reporting time
    date = reportDate;
    fwrite(&date, sizeof(REAL8), 1, Fout.file);

    // --- save subcatchment results
    if (Nobjects[SUBCATCH] > 0)
        output_saveSubcatchResults(reportTime, Fout.file);

    // --- save average routing results over reporting period if called for    //(5.1.013)
    if ( RptFlags.averages ) output_saveAvgResults(Fout.file);                 //

    // --- otherwise save interpolated point routing results                   //(5.1.013)
    else                                                                       //
    {
        if (Nobjects[NODE] > 0)
            output_saveNodeResults(reportTime, Fout.file);
        if (Nobjects[LINK] > 0)
            output_saveLinkResults(reportTime, Fout.file);
    }

    // --- update & save system-wide flows 
    SysResults[SYS_FLOODING] = (REAL4)(StepFlowTotals.flooding * UCF(FLOW));
    SysResults[SYS_OUTFLOW] = (REAL4)(StepFlowTotals.outflow * UCF(FLOW));
    SysResults[SYS_DWFLOW] = (REAL4)(StepFlowTotals.dwInflow * UCF(FLOW));
    SysResults[SYS_GWFLOW] = (REAL4)(StepFlowTotals.gwInflow * UCF(FLOW));
    SysResults[SYS_IIFLOW] = (REAL4)(StepFlowTotals.iiInflow * UCF(FLOW));
    SysResults[SYS_EXFLOW] = (REAL4)(StepFlowTotals.exInflow * UCF(FLOW));
    SysResults[SYS_INFLOW] = SysResults[SYS_RUNOFF] +
                             SysResults[SYS_DWFLOW] +
                             SysResults[SYS_GWFLOW] +
                             SysResults[SYS_IIFLOW] +
                             SysResults[SYS_EXFLOW];
    fwrite(SysResults, sizeof(REAL4), MAX_SYS_RESULTS, Fout.file);

    // --- save outfall flows to interface file if called for
    if ( Foutflows.mode == SAVE_FILE && !IgnoreRouting ) 
        iface_saveOutletResults(reportDate, Foutflows.file);
    Nperiods++;
}

//=============================================================================

void output_end()
//
//  Input:   none
//  Output:  none
//  Purpose: writes closing records to binary file.
//
{
    INT4 k;
    fwrite(&IDStartPos, sizeof(INT4), 1, Fout.file);
    fwrite(&InputStartPos, sizeof(INT4), 1, Fout.file);
    fwrite(&OutputStartPos, sizeof(INT4), 1, Fout.file);
    k = Nperiods;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = (INT4)error_getCode(ErrorCode);
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = MAGICNUMBER;
    if (fwrite(&k, sizeof(INT4), 1, Fout.file) < 1)
    {
        report_writeErrorMsg(ERR_OUT_WRITE, "");
    }
}

//=============================================================================

void output_close()
//
//  Input:   none
//  Output:  none
//  Purpose: frees memory used for accessing the binary file.
//
{
    FREE(SubcatchResults);
    FREE(NodeResults);
    FREE(LinkResults);
    output_closeAvgResults();                                                  //(5.1.013)
}

//=============================================================================

void output_saveID(char* id, FILE* file)
//
//  Input:   id = name of an object
//           file = ptr. to binary output file
//  Output:  none
//  Purpose: writes an object's name to the binary output file.
//
{
    INT4 n = strlen(id);
    fwrite(&n, sizeof(INT4), 1, file);
    fwrite(id, sizeof(char), n, file);
}

//=============================================================================

void output_saveSubcatchResults(double reportTime, FILE* file)
//
//  Input:   reportTime = elapsed simulation time (millisec)
//           file = ptr. to binary output file
//  Output:  none
//  Purpose: writes computed subcatchment results to binary file.
//
{
    int      j;
    double   f;
    double   area;
    REAL4    totalArea = 0.0f; 
    DateTime reportDate = getDateTime(reportTime);

    // --- update reported rainfall at each rain gage
    for ( j=0; j<Nobjects[GAGE]; j++ )
    {
        gage_setReportRainfall(j, reportDate);
    }

    // --- find where current reporting time lies between latest runoff times
    f = (reportTime - OldRunoffTime) / (NewRunoffTime - OldRunoffTime);

    // --- write subcatchment results to file
    for ( j=0; j<Nobjects[SUBCATCH]; j++)
    {
        // --- retrieve interpolated results for reporting time & write to file
        subcatch_getResults(j, f, SubcatchResults);
        if ( Subcatch[j].rptFlag )
            fwrite(SubcatchResults, sizeof(REAL4), NumSubcatchVars, file);

        // --- update system-wide results
        area = Subcatch[j].area * UCF(LANDAREA);
        totalArea += (REAL4)area;
        SysResults[SYS_RAINFALL] +=
            (REAL4)(SubcatchResults[SUBCATCH_RAINFALL] * area);
        SysResults[SYS_SNOWDEPTH] +=
            (REAL4)(SubcatchResults[SUBCATCH_SNOWDEPTH] * area);
        SysResults[SYS_EVAP] +=
            (REAL4)(SubcatchResults[SUBCATCH_EVAP] * area);
        if ( Subcatch[j].groundwater ) SysResults[SYS_EVAP] += 
            (REAL4)(Subcatch[j].groundwater->evapLoss * UCF(EVAPRATE) * area);
        SysResults[SYS_INFIL] +=
            (REAL4)(SubcatchResults[SUBCATCH_INFIL] * area);
        SysResults[SYS_RUNOFF] += (REAL4)SubcatchResults[SUBCATCH_RUNOFF];
    }

    // --- normalize system-wide results to catchment area
    if ( totalArea > 0.0 )
    {
        SysResults[SYS_EVAP]      /= totalArea;
        SysResults[SYS_RAINFALL]  /= totalArea;
        SysResults[SYS_SNOWDEPTH] /= totalArea;
        SysResults[SYS_INFIL]     /= totalArea;
    }

    // --- update system temperature and PET
    if ( UnitSystem == SI ) f = (5./9.) * (Temp.ta - 32.0);
    else f = Temp.ta;
    SysResults[SYS_TEMPERATURE] = (REAL4)f;
    f = Evap.rate * UCF(EVAPRATE);
    SysResults[SYS_PET] = (REAL4)f;

}

//=============================================================================

////  This function was re-written for release 5.1.013.  ////                  //(5.1.013)

void output_saveNodeResults(double reportTime, FILE* file)
//
//  Input:   reportTime = elapsed simulation time (millisec)
//           file = ptr. to binary output file
//  Output:  none
//  Purpose: writes computed node results to binary file.
//
{
    int j;

    // --- find where current reporting time lies between latest routing times
    double f = (reportTime - OldRoutingTime) /
               (NewRoutingTime - OldRoutingTime);

    // --- write node results to file
    for (j=0; j<Nobjects[NODE]; j++)
    {
        // --- retrieve interpolated results for reporting time & write to file
        node_getResults(j, f, NodeResults);
        if ( Node[j].rptFlag )
            fwrite(NodeResults, sizeof(REAL4), NumNodeVars, file);
        stats_updateMaxNodeDepth(j, NodeResults[NODE_DEPTH]);

        // --- update system-wide storage volume 
        SysResults[SYS_STORAGE] += NodeResults[NODE_VOLUME];
    }
}

//=============================================================================

void output_saveLinkResults(double reportTime, FILE* file)
//
//  Input:   reportTime = elapsed simulation time (millisec)
//           file = ptr. to binary output file
//  Output:  none
//  Purpose: writes computed link results to binary file.
//
{
    int j;
    double f;
    double z;

    // --- find where current reporting time lies between latest routing times
    f = (reportTime - OldRoutingTime) / (NewRoutingTime - OldRoutingTime);

    // --- write link results to file
    for (j=0; j<Nobjects[LINK]; j++)
    {
        // --- retrieve interpolated results for reporting time & write to file
        if (Link[j].rptFlag)
        {
            link_getResults(j, f, LinkResults);
            fwrite(LinkResults, sizeof(REAL4), NumLinkVars, file);
        }

        // --- update system-wide results
        z = ((1.0-f)*Link[j].oldVolume + f*Link[j].newVolume) * UCF(VOLUME);
        SysResults[SYS_STORAGE] += (REAL4)z;
    }
}

//=============================================================================

void output_readDateTime(int period, DateTime* days)
//
//  Input:   period = index of reporting time period
//  Output:  days = date/time value
//  Purpose: retrieves the date/time for a specific reporting period
//           from the binary output file.
//
{
    INT4 bytePos = OutputStartPos + (period-1)*BytesPerPeriod;
    fseek(Fout.file, bytePos, SEEK_SET);
    *days = NO_DATE;
    fread(days, sizeof(REAL8), 1, Fout.file);
}

//=============================================================================

void output_readSubcatchResults(int period, int index)
//
//  Input:   period = index of reporting time period
//           index = subcatchment index
//  Output:  none
//  Purpose: reads computed results for a subcatchment at a specific time
//           period.
//
{
    INT4 bytePos = OutputStartPos + (period-1)*BytesPerPeriod;
    bytePos += sizeof(REAL8) + index*NumSubcatchVars*sizeof(REAL4);
    fseek(Fout.file, bytePos, SEEK_SET);
    fread(SubcatchResults, sizeof(REAL4), NumSubcatchVars, Fout.file);
}

//=============================================================================

void output_readNodeResults(int period, int index)
//
//  Input:   period = index of reporting time period
//           index = node index
//  Output:  none
//  Purpose: reads computed results for a node at a specific time period.
//
{
    INT4 bytePos = OutputStartPos + (period-1)*BytesPerPeriod;
    bytePos += sizeof(REAL8) + NumSubcatch*NumSubcatchVars*sizeof(REAL4);
    bytePos += index*NumNodeVars*sizeof(REAL4);
    fseek(Fout.file, bytePos, SEEK_SET);
    fread(NodeResults, sizeof(REAL4), NumNodeVars, Fout.file);
}

//=============================================================================

void output_readLinkResults(int period, int index)
//
//  Input:   period = index of reporting time period
//           index = link index
//  Output:  none
//  Purpose: reads computed results for a link at a specific time period.
//
{
    INT4 bytePos = OutputStartPos + (period-1)*BytesPerPeriod;
    bytePos += sizeof(REAL8) + NumSubcatch*NumSubcatchVars*sizeof(REAL4);
    bytePos += NumNodes*NumNodeVars*sizeof(REAL4);
    bytePos += index*NumLinkVars*sizeof(REAL4);
    fseek(Fout.file, bytePos, SEEK_SET);
    fread(LinkResults, sizeof(REAL4), NumLinkVars, Fout.file);
    fread(SysResults, sizeof(REAL4), MAX_SYS_RESULTS, Fout.file);
}

////  The following functions were added for release 5.1.013.  ////            //(5.1.013)

//=============================================================================
//  Functions for saving average results within a reporting period to file.
//=============================================================================

int output_openAvgResults()
{
    int i;
    
    // --- allocate memory for averages at reportable nodes
    AvgNodeResults = (TAvgResults *)calloc(NumNodes, sizeof(TAvgResults));
    if ( AvgNodeResults == NULL ) return FALSE;
    for (i = 0; i < NumNodes; i++ ) AvgNodeResults[i].xAvg = NULL;

    // --- allocate memory for averages at reportable links
    AvgLinkResults = (TAvgResults *)calloc(NumLinks, sizeof(TAvgResults));
    if (AvgLinkResults == NULL)
    {
        output_closeAvgResults();
        return FALSE;
    }
    for (i = 0; i < NumLinks; i++) AvgLinkResults[i].xAvg = NULL;

    // --- allocate memory for each reportable variable for each reportable node
    for (i = 0; i < NumNodes; i++)
    {
        AvgNodeResults[i].xAvg = (REAL4*) calloc(NumNodeVars, sizeof(REAL4));
        if (AvgNodeResults[i].xAvg == NULL)
        {
            output_closeAvgResults();
            return FALSE;
        }
    }

    // --- allocate memory for each reportable variable for each reportable link
    for (i = 0; i < NumLinks; i++)
    {
        AvgLinkResults[i].xAvg = (REAL4*)calloc(NumLinkVars, sizeof(REAL4));
        if (AvgLinkResults[i].xAvg == NULL)
        {
            output_closeAvgResults();
            return FALSE;
        }
    }
    return TRUE;
}

//=============================================================================

void output_closeAvgResults()
{
    int i;
    if (AvgNodeResults)
    {
        for (i = 0; i < NumNodes; i++)  FREE(AvgNodeResults[i].xAvg); 
        FREE(AvgNodeResults);
    }
    if (AvgLinkResults)
    {
        for (i = 0; i < NumLinks; i++)  FREE(AvgLinkResults[i].xAvg);
        FREE(AvgLinkResults);
    }
}

//=============================================================================

void output_initAvgResults()
{
    int i, j;
    Nsteps = 0;
    for (i = 0; i < NumNodes; i++)
    {
        for (j = 0; j < NumNodeVars; j++) AvgNodeResults[i].xAvg[j] = 0.0;
    }
    for (i = 0; i < NumLinks; i++)
    {
        for (j = 0; j < NumLinkVars; j++) AvgLinkResults[i].xAvg[j] = 0.0;
    }
}

//=============================================================================

void output_updateAvgResults()
{
    int i, j, k, sign;

    // --- update average accumulations for nodes
    k = 0;
    for (i = 0; i < Nobjects[NODE]; i++)
    {
        if ( !Node[i].rptFlag ) continue;
        node_getResults(i, 1.0, NodeResults);
        for (j = 0; j < NumNodeVars; j++)
        {
            AvgNodeResults[k].xAvg[j] += NodeResults[j];
        }
        k++;
    }

    // --- update average accumulations for links
    k = 0;
    for (i = 0; i < Nobjects[LINK]; i++)
    {
        if ( !Link[i].rptFlag ) continue;
        link_getResults(i, 1.0, LinkResults);

        // --- save sign of current flow rate
        sign = SGN(LinkResults[LINK_FLOW]);

        // --- add current results to average accumulation
        for (j = 0; j < NumLinkVars; j++)
        {
            // --- accumulate flow so its sign (+/-) will equal that of most
            //     recent flow result
            if ( j == LINK_FLOW )
            {
                AvgLinkResults[k].xAvg[j] =
                    sign * (ABS(AvgLinkResults[k].xAvg[j]) + ABS(LinkResults[j]));
            }

            // --- link capacity is another special case
            else if (j == LINK_CAPACITY)
            {
                // --- accumulate capacity (fraction full) for conduits 
                if ( Link[i].type == CONDUIT )
                    AvgLinkResults[k].xAvg[j] += LinkResults[j];

                // --- for other links capacity is pump speed or regulator
                //     opening fraction which shouldn't be averaged
                //     (multiplying by Nsteps+1 will preserve last value
                //     when average results are taken in saveAvgResults())
                else  
                    AvgLinkResults[k].xAvg[j] = LinkResults[j] * (Nsteps+1);
            }

            // --- accumulation for all other reported results
            else AvgLinkResults[k].xAvg[j] += LinkResults[j];
        }
        k++;
    }
    Nsteps++;
}

//=============================================================================

void output_saveAvgResults(FILE* file)
{
    int i, j;

    // --- examine each reportable node
    for (i = 0; i < NumNodes; i++)
    {
        // --- determine the node's average results
        for (j = 0; j < NumNodeVars; j++)
        {
            NodeResults[j] = AvgNodeResults[i].xAvg[j] / Nsteps;
        }

        // --- save average results to file
        fwrite(NodeResults, sizeof(REAL4), NumNodeVars, file);
    }

    // --- update each node's max depth and contribution to system storage
    for (i = 0; i < Nobjects[NODE]; i++)
    {
        stats_updateMaxNodeDepth(i, Node[i].newDepth * UCF(LENGTH));
        SysResults[SYS_STORAGE] += (REAL4)(Node[i].newVolume * UCF(VOLUME));
    }

    // --- examine each reportable link
    for (i = 0; i < NumLinks; i++)
    {
        // --- determine the link's average results
        for (j = 0; j < NumLinkVars; j++)
        {
            LinkResults[j] = AvgLinkResults[i].xAvg[j] / Nsteps;
        }

        // --- save average results to file
        fwrite(LinkResults, sizeof(REAL4), NumLinkVars, file);
    }
 
    // --- add each link's volume to total system storage
    for (i = 0; i < Nobjects[NODE]; i++)
    {
        SysResults[SYS_STORAGE] += (REAL4)(Link[i].newVolume * UCF(VOLUME));
    }

    // --- re-initialize average results for all nodes and links
    output_initAvgResults();
}
