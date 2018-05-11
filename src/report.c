//-----------------------------------------------------------------------------
//   report.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/21/2014  (Build 5.1.001)
//             04/14/14    (Build 5.1.004)
//             09/15/14    (Build 5.1.007)
//             04/02/15    (Build 5.1.008)
//             08/01/16    (Build 5.1.011)
//             03/14/17    (Build 5.1.012)
//             05/10/18    (Build 5.1.013)
//   Author:   L. Rossman (EPA)
//
//   Report writing functions.
//
//   Build 5.1.004:
//   - Ignore RDII option reported.
//
//   Build 5.1.007:
//   - Total exfiltration loss reported.
//
//   Build 5.1.008:
//   - Number of threads option reported.
//   - LID drainage volume and outfall runon reported.
//   - "Internal Outflow" label changed to "Flooding Loss" in Flow Routing
//     Continuity table.
//   - Exfiltration loss added into Quality Routing Continuity table.
//
//   Build 5.1.011:
//   - Blank line added after writing project title.
//   - Text of error message saved to global variable ErrorMsg.
//   - Global variable Warnings incremented after warning message issued.
//
//   Build 5.1.012:
//   - System time step statistics adjusted for time in steady state.
//
//   Build 5.1.013:
//   - Parsing of AVERAGES report option added to report_readOptions().
//   - Name of surcharge method reported in report_writeOptions().
//   - Missing format specifier added to fprintf() in report_writeErrorCode.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "headers.h"

#define WRITE(x) (report_writeLine((x)))
#define LINE_10 "----------"
#define LINE_12 "------------"
#define LINE_51 \
"---------------------------------------------------"
#define LINE_64 \
"----------------------------------------------------------------"


//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
static time_t SysTime;

//-----------------------------------------------------------------------------
//  Imported variables
//-----------------------------------------------------------------------------
#define REAL4 float
extern REAL4* SubcatchResults;         // Results vectors defined in OUTPUT.C
extern REAL4* NodeResults;             //  "
extern REAL4* LinkResults;             //  "
extern char   ErrString[81];           // defined in ERROR.C

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void report_LoadingErrors(int p1, int p2, TLoadingTotals* totals);
static void report_QualErrors(int p1, int p2, TRoutingTotals* totals);
static void report_Subcatchments(void);
static void report_SubcatchHeader(char *id);
static void report_Nodes(void);
static void report_NodeHeader(char *id);
static void report_Links(void);
static void report_LinkHeader(char *id);


//=============================================================================

int report_readOptions(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads reporting options from a line of input
//
{
    char  k;
    int   j, m, t;
    if ( ntoks < 2 ) return error_setInpError(ERR_ITEMS, "");
    k = (char)findmatch(tok[0], ReportWords);
    if ( k < 0 ) return error_setInpError(ERR_KEYWORD, tok[0]);
    switch ( k )
    {
      case 0: // Input
        m = findmatch(tok[1], NoYesWords);
        if      ( m == YES ) RptFlags.input = TRUE;
        else if ( m == NO )  RptFlags.input = FALSE;
        else                 return error_setInpError(ERR_KEYWORD, tok[1]);
        return 0;

      case 1: // Continuity
        m = findmatch(tok[1], NoYesWords);
        if      ( m == YES ) RptFlags.continuity = TRUE;
        else if ( m == NO )  RptFlags.continuity = FALSE;
        else                 return error_setInpError(ERR_KEYWORD, tok[1]);
        return 0;

      case 2: // Flow Statistics
        m = findmatch(tok[1], NoYesWords);
        if      ( m == YES ) RptFlags.flowStats = TRUE;
        else if ( m == NO )  RptFlags.flowStats = FALSE;
        else                 return error_setInpError(ERR_KEYWORD, tok[1]);
        return 0;

      case 3: // Controls
        m = findmatch(tok[1], NoYesWords);
        if      ( m == YES ) RptFlags.controls = TRUE;
        else if ( m == NO )  RptFlags.controls = FALSE;
        else                 return error_setInpError(ERR_KEYWORD, tok[1]);
        return 0;

      case 4:  m = SUBCATCH;  break;  // Subcatchments
      case 5:  m = NODE;      break;  // Nodes
      case 6:  m = LINK;      break;  // Links

      case 7: // Node Statistics
        m = findmatch(tok[1], NoYesWords);
        if      ( m == YES ) RptFlags.nodeStats = TRUE;
        else if ( m == NO )  RptFlags.nodeStats = FALSE;
        else                 return error_setInpError(ERR_KEYWORD, tok[1]);
        return 0;

      case 8: // Averages                                                      //(5.1.013)
        m = findmatch(tok[1], NoYesWords);                                     //
        if      (m == YES) RptFlags.averages = TRUE;                           //
        else if (m == NO)  RptFlags.averages = FALSE;                          //
        else               return error_setInpError(ERR_KEYWORD, tok[1]);      //
        return 0;                                                              //

      default: return error_setInpError(ERR_KEYWORD, tok[1]);
    }
    k = (char)findmatch(tok[1], NoneAllWords);
    if ( k < 0 )
    {
        k = SOME;
        for (t = 1; t < ntoks; t++)
        {
            j = project_findObject(m, tok[t]);
            if ( j < 0 ) return error_setInpError(ERR_NAME, tok[t]);
            switch ( m )
            {
              case SUBCATCH:  Subcatch[j].rptFlag = TRUE;  break;
              case NODE:      Node[j].rptFlag = TRUE;  break;
              case LINK:      Link[j].rptFlag = TRUE;  break;
            }
        }
    }
    switch ( m )
    {
      case SUBCATCH: RptFlags.subcatchments = k;  break;
      case NODE:     RptFlags.nodes = k;  break;
      case LINK:     RptFlags.links = k;  break;
    }
    return 0;
}

//=============================================================================

void report_writeLine(char *line)
//
//  Input:   line = line of text
//  Output:  none
//  Purpose: writes line of text to report file.
//
{
    if ( Frpt.file ) fprintf(Frpt.file, "\n  %s", line);
}

//=============================================================================

void report_writeSysTime(void)
//
//  Input:   none
//  Output:  none
//  Purpose: writes starting/ending processing times to report file.
//
{
    char    theTime[9];
    double  elapsedTime;
    time_t  endTime;
    if ( Frpt.file )
    {
        fprintf(Frpt.file, FMT20, ctime(&SysTime));
        time(&endTime);
        fprintf(Frpt.file, FMT20a, ctime(&endTime));
        elapsedTime = difftime(endTime, SysTime);
        fprintf(Frpt.file, FMT21);
        if ( elapsedTime < 1.0 ) fprintf(Frpt.file, "< 1 sec");
        else
        {
            elapsedTime /= SECperDAY;
            if (elapsedTime >= 1.0)
            {
                fprintf(Frpt.file, "%d.", (int)floor(elapsedTime));
                elapsedTime -= floor(elapsedTime);
            }
            datetime_timeToStr(elapsedTime, theTime);
            fprintf(Frpt.file, "%s", theTime);
        }
    }
}


//=============================================================================
//      SIMULATION OPTIONS REPORTING
//=============================================================================

void report_writeLogo()
//
//  Input:   none
//  Output:  none
//  Purpose: writes report header lines to report file.
//
{
    fprintf(Frpt.file, FMT08);
    fprintf(Frpt.file, FMT09);
    fprintf(Frpt.file, FMT10);
    time(&SysTime);                    // Save starting wall clock time
}

//=============================================================================

void report_writeTitle()
//
//  Input:   none
//  Output:  none
//  Purpose: writes project title to report file.
//
{
    int i;
    int lineCount = 0;
    if ( ErrorCode ) return;
    for (i=0; i<MAXTITLE; i++) if ( strlen(Title[i]) > 0 )
    {
        WRITE(Title[i]);
        lineCount++;
    }
    if ( lineCount > 0 ) WRITE("");
}

//=============================================================================

void report_writeOptions()
//
//  Input:   none
//  Output:  none
//  Purpose: writes analysis options in use to report file.
//
{
    char str[80];
    WRITE("");
    WRITE("*********************************************************");
    WRITE("NOTE: The summary statistics displayed in this report are");
    WRITE("based on results found at every computational time step,  ");
    WRITE("not just on results from each reporting time step.");
    WRITE("*********************************************************");
    WRITE("");
    WRITE("****************");
    WRITE("Analysis Options");
    WRITE("****************");
    fprintf(Frpt.file, "\n  Flow Units ............... %s",
        FlowUnitWords[FlowUnits]);
    fprintf(Frpt.file, "\n  Process Models:");
    fprintf(Frpt.file, "\n    Rainfall/Runoff ........ ");
    if ( IgnoreRainfall || Nobjects[GAGE] == 0 )
        fprintf(Frpt.file, "NO");
    else fprintf(Frpt.file, "YES");

    fprintf(Frpt.file, "\n    RDII ................... ");
    if ( IgnoreRDII || Nobjects[UNITHYD] == 0 )
        fprintf(Frpt.file, "NO");
    else fprintf(Frpt.file, "YES");

    fprintf(Frpt.file, "\n    Snowmelt ............... ");
    if ( IgnoreSnowmelt || Nobjects[SNOWMELT] == 0 )
        fprintf(Frpt.file, "NO");
    else fprintf(Frpt.file, "YES");
    fprintf(Frpt.file, "\n    Groundwater ............ ");
    if ( IgnoreGwater || Nobjects[AQUIFER] == 0 )
        fprintf(Frpt.file, "NO");
    else fprintf(Frpt.file, "YES");
    fprintf(Frpt.file, "\n    Flow Routing ........... ");
    if ( IgnoreRouting || Nobjects[LINK] == 0 )
        fprintf(Frpt.file, "NO");
    else
    {
        fprintf(Frpt.file, "YES");
        fprintf(Frpt.file, "\n    Ponding Allowed ........ ");
        if ( AllowPonding ) fprintf(Frpt.file, "YES");
        else                fprintf(Frpt.file, "NO");
    }
    fprintf(Frpt.file, "\n    Water Quality .......... ");
    if ( IgnoreQuality || Nobjects[POLLUT] == 0 )
        fprintf(Frpt.file, "NO");
    else fprintf(Frpt.file, "YES");

    if ( Nobjects[SUBCATCH] > 0 )
    fprintf(Frpt.file, "\n  Infiltration Method ...... %s",
        InfilModelWords[InfilModel]);
    if ( Nobjects[LINK] > 0 )
    fprintf(Frpt.file, "\n  Flow Routing Method ...... %s",
        RouteModelWords[RouteModel]);

    if (RouteModel == DW)                                                      //(5.1.013)
    fprintf(Frpt.file, "\n  Surcharge Method ......... %s",                    //(5.1.013)
        SurchargeWords[SurchargeMethod]);                                      //(5.1.013)

    datetime_dateToStr(StartDate, str);
    fprintf(Frpt.file, "\n  Starting Date ............ %s", str);
    datetime_timeToStr(StartTime, str);
    fprintf(Frpt.file, " %s", str);
    datetime_dateToStr(EndDate, str);
    fprintf(Frpt.file, "\n  Ending Date .............. %s", str);
    datetime_timeToStr(EndTime, str);
    fprintf(Frpt.file, " %s", str);
    fprintf(Frpt.file, "\n  Antecedent Dry Days ...... %.1f", StartDryDays);
    datetime_timeToStr(datetime_encodeTime(0, 0, ReportStep), str);
    fprintf(Frpt.file, "\n  Report Time Step ......... %s", str);
    if ( Nobjects[SUBCATCH] > 0 )
    {
        datetime_timeToStr(datetime_encodeTime(0, 0, WetStep), str);
        fprintf(Frpt.file, "\n  Wet Time Step ............ %s", str);
        datetime_timeToStr(datetime_encodeTime(0, 0, DryStep), str);
        fprintf(Frpt.file, "\n  Dry Time Step ............ %s", str);
    }
    if ( Nobjects[LINK] > 0 )
    {
        fprintf(Frpt.file, "\n  Routing Time Step ........ %.2f sec", RouteStep);
		if ( RouteModel == DW )
		{
		fprintf(Frpt.file, "\n  Variable Time Step ....... ");
		if ( CourantFactor > 0.0 ) fprintf(Frpt.file, "YES");
		else                       fprintf(Frpt.file, "NO");
		fprintf(Frpt.file, "\n  Maximum Trials ........... %d", MaxTrials);
        fprintf(Frpt.file, "\n  Number of Threads ........ %d", NumThreads);
		fprintf(Frpt.file, "\n  Head Tolerance ........... %.6f ",
            HeadTol*UCF(LENGTH));
		if ( UnitSystem == US ) fprintf(Frpt.file, "ft");
		else                    fprintf(Frpt.file, "m");
		}
    }
    WRITE("");
}


//=============================================================================
//      RAINFALL FILE REPORTING
//=============================================================================

void report_writeRainStats(int i, TRainStats* r)
//
//  Input:   i = rain gage index
//           r = rain file summary statistics
//  Output:  none
//  Purpose: writes summary of rain data read from file to report file.
//
{
    char date1[] = "***********";
    char date2[] = "***********";
    if ( i < 0 )
    {
        WRITE("");
        WRITE("*********************");
        WRITE("Rainfall File Summary");
        WRITE("*********************");
        fprintf(Frpt.file,
"\n  Station    First        Last         Recording   Periods    Periods    Periods");
        fprintf(Frpt.file,
"\n  ID         Date         Date         Frequency  w/Precip    Missing    Malfunc.");
        fprintf(Frpt.file,
"\n  -------------------------------------------------------------------------------\n");
    }
    else
    {
        if ( r->startDate != NO_DATE ) datetime_dateToStr(r->startDate, date1);
        if ( r->endDate   != NO_DATE ) datetime_dateToStr(r->endDate, date2);
        fprintf(Frpt.file, "  %-10s %-11s  %-11s  %5d min    %6ld     %6ld     %6ld\n",
            Gage[i].staID, date1, date2, Gage[i].rainInterval/60,
            r->periodsRain, r->periodsMissing, r->periodsMalfunc);
    }
}


//=============================================================================
//      RDII REPORTING
//=============================================================================

void report_writeRdiiStats(double rainVol, double rdiiVol)
//
//  Input:   rainVol = total rainfall volume over sewershed
//           rdiiVol = total RDII volume produced
//  Output:  none
//  Purpose: writes summary of RDII inflow to report file.
//
{
    double ratio;
    double ucf1, ucf2;

    ucf1 = UCF(LENGTH) * UCF(LANDAREA);
    if ( UnitSystem == US) ucf2 = MGDperCFS / SECperDAY;
    else                   ucf2 = MLDperCFS / SECperDAY;

    WRITE("");
    fprintf(Frpt.file,
    "\n  **********************           Volume        Volume");
    if ( UnitSystem == US) fprintf(Frpt.file,
    "\n  Rainfall Dependent I/I        acre-feet      10^6 gal");
    else fprintf(Frpt.file,
    "\n  Rainfall Dependent I/I        hectare-m      10^6 ltr");
    fprintf(Frpt.file,
    "\n  **********************        ---------     ---------");

    fprintf(Frpt.file, "\n  Sewershed Rainfall ......%14.3f%14.3f",
            rainVol * ucf1, rainVol * ucf2);

    fprintf(Frpt.file, "\n  RDII Produced ...........%14.3f%14.3f",
            rdiiVol * ucf1, rdiiVol * ucf2);

    if ( rainVol == 0.0 ) ratio = 0.0;
    else ratio = rdiiVol / rainVol;
    fprintf(Frpt.file, "\n  RDII Ratio ..............%14.3f", ratio);
    WRITE("");
}


//=============================================================================
//      CONTROL ACTIONS REPORTING
//=============================================================================

void   report_writeControlActionsHeading()
{
    WRITE("");
    WRITE("*********************");
    WRITE("Control Actions Taken");
    WRITE("*********************");
    fprintf(Frpt.file, "\n");
}

//=============================================================================

void   report_writeControlAction(DateTime aDate, char* linkID, double value,
                                 char* ruleID)
//
//  Input:   aDate  = date/time of rule action
//           linkID = ID of link being controlled
//           value  = new status value of link
//           ruleID = ID of rule implementing the action
//  Output:  none
//  Purpose: reports action taken by a control rule.
//
{
    char     theDate[12];
    char     theTime[9];
    datetime_dateToStr(aDate, theDate);
    datetime_timeToStr(aDate, theTime);
    fprintf(Frpt.file,
            "  %11s: %8s Link %s setting changed to %6.2f by Control %s\n",
            theDate, theTime, linkID, value, ruleID);
}


//=============================================================================
//      CONTINUITY ERROR REPORTING
//=============================================================================

void report_writeRunoffError(TRunoffTotals* totals, double totalArea)
//
//  Input:  totals = accumulated runoff totals
//          totalArea = total area of all subcatchments
//  Output:  none
//  Purpose: writes runoff continuity error to report file.
//
{

    if ( Frunoff.mode == USE_FILE )
    {
        WRITE("");
        fprintf(Frpt.file,
        "\n  **************************"
        "\n  Runoff Quantity Continuity"
        "\n  **************************"
        "\n  Runoff supplied by interface file %s", Frunoff.name);
        WRITE("");
        return;
    }

    if ( totalArea == 0.0 ) return;
    WRITE("");

    fprintf(Frpt.file,
    "\n  **************************        Volume         Depth");
    if ( UnitSystem == US) fprintf(Frpt.file,
    "\n  Runoff Quantity Continuity     acre-feet        inches");
    else fprintf(Frpt.file,
    "\n  Runoff Quantity Continuity     hectare-m            mm");
    fprintf(Frpt.file,
    "\n  **************************     ---------       -------");

    if ( totals->initStorage > 0.0 )
    {
        fprintf(Frpt.file, "\n  Initial LID Storage ......%14.3f%14.3f",
            totals->initStorage * UCF(LENGTH) * UCF(LANDAREA),
            totals->initStorage / totalArea * UCF(RAINDEPTH));
    }

    if ( Nobjects[SNOWMELT] > 0 )
    {
        fprintf(Frpt.file, "\n  Initial Snow Cover .......%14.3f%14.3f",
            totals->initSnowCover * UCF(LENGTH) * UCF(LANDAREA),
            totals->initSnowCover / totalArea * UCF(RAINDEPTH));
    }

    fprintf(Frpt.file, "\n  Total Precipitation ......%14.3f%14.3f",
            totals->rainfall * UCF(LENGTH) * UCF(LANDAREA),
            totals->rainfall / totalArea * UCF(RAINDEPTH));

    if ( totals->runon > 0.0 )
    {
        fprintf(Frpt.file, "\n  Outfall Runon ............%14.3f%14.3f",
            totals->runon * UCF(LENGTH) * UCF(LANDAREA),
            totals->runon / totalArea * UCF(RAINDEPTH));
    }

    fprintf(Frpt.file, "\n  Evaporation Loss .........%14.3f%14.3f",
            totals->evap * UCF(LENGTH) * UCF(LANDAREA),
            totals->evap / totalArea * UCF(RAINDEPTH));

    fprintf(Frpt.file, "\n  Infiltration Loss ........%14.3f%14.3f",
            totals->infil * UCF(LENGTH) * UCF(LANDAREA),
            totals->infil / totalArea * UCF(RAINDEPTH));

    fprintf(Frpt.file, "\n  Surface Runoff ...........%14.3f%14.3f",
            totals->runoff * UCF(LENGTH) * UCF(LANDAREA),
            totals->runoff / totalArea * UCF(RAINDEPTH));

    if ( totals->drains > 0.0 )
    {
        fprintf(Frpt.file, "\n  LID Drainage .............%14.3f%14.3f",
            totals->drains * UCF(LENGTH) * UCF(LANDAREA),
            totals->drains / totalArea * UCF(RAINDEPTH));
    }

    if ( Nobjects[SNOWMELT] > 0 )
    {
        fprintf(Frpt.file, "\n  Snow Removed .............%14.3f%14.3f",
            totals->snowRemoved * UCF(LENGTH) * UCF(LANDAREA),
            totals->snowRemoved / totalArea * UCF(RAINDEPTH));
        fprintf(Frpt.file, "\n  Final Snow Cover .........%14.3f%14.3f",
            totals->finalSnowCover * UCF(LENGTH) * UCF(LANDAREA),
            totals->finalSnowCover / totalArea * UCF(RAINDEPTH));
    }

    fprintf(Frpt.file, "\n  Final Storage ............%14.3f%14.3f",
            totals->finalStorage * UCF(LENGTH) * UCF(LANDAREA),
            totals->finalStorage / totalArea * UCF(RAINDEPTH));

    fprintf(Frpt.file, "\n  Continuity Error (%%) .....%14.3f",
            totals->pctError);
    WRITE("");
}

//=============================================================================

void report_writeLoadingError(TLoadingTotals* totals)
//
//  Input:   totals = accumulated pollutant loading totals
//           area = total area of all subcatchments
//  Output:  none
//  Purpose: writes runoff loading continuity error to report file.
//
{
    int p1, p2;
    p1 = 1;
    p2 = MIN(5, Nobjects[POLLUT]);
    while ( p1 <= Nobjects[POLLUT] )
    {
        report_LoadingErrors(p1-1, p2-1, totals);
        p1 = p2 + 1;
        p2 = p1 + 4;
        p2 = MIN(p2, Nobjects[POLLUT]);
    }
}

//=============================================================================

void report_LoadingErrors(int p1, int p2, TLoadingTotals* totals)
//
//  Input:   p1 = index of first pollutant to report
//           p2 = index of last pollutant to report
//           totals = accumulated pollutant loading totals
//           area = total area of all subcatchments
//  Output:  none
//  Purpose: writes runoff loading continuity error to report file for
//           up to 5 pollutants at a time.
//
{
    int    i;
    int    p;
    double cf = 1.0;
    char   units[15];

    WRITE("");
    fprintf(Frpt.file, "\n  **************************");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14s", Pollut[p].ID);
    }
    fprintf(Frpt.file, "\n  Runoff Quality Continuity ");
    for (p = p1; p <= p2; p++)
    {
        i = UnitSystem;
        if ( Pollut[p].units == COUNT ) i = 2;
        strcpy(units, LoadUnitsWords[i]);
        fprintf(Frpt.file, "%14s", units);
    }
    fprintf(Frpt.file, "\n  **************************");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "    ----------");
    }

    fprintf(Frpt.file, "\n  Initial Buildup ..........");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", totals[p].initLoad*cf);
    }
    fprintf(Frpt.file, "\n  Surface Buildup ..........");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", totals[p].buildup*cf);
    }
    fprintf(Frpt.file, "\n  Wet Deposition ...........");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", totals[p].deposition*cf);
    }
    fprintf(Frpt.file, "\n  Sweeping Removal .........");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", totals[p].sweeping*cf);
    }
    fprintf(Frpt.file, "\n  Infiltration Loss ........");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", totals[p].infil*cf);
    }
    fprintf(Frpt.file, "\n  BMP Removal ..............");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", totals[p].bmpRemoval*cf);
    }
    fprintf(Frpt.file, "\n  Surface Runoff ...........");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", totals[p].runoff*cf);
    }
    fprintf(Frpt.file, "\n  Remaining Buildup ........");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", totals[p].finalLoad*cf);
    }
    fprintf(Frpt.file, "\n  Continuity Error (%%) .....");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", totals[p].pctError);
    }
    WRITE("");
}

//=============================================================================

void report_writeGwaterError(TGwaterTotals* totals, double gwArea)
//
//  Input:   totals = accumulated groundwater totals
//           gwArea = total area of all subcatchments with groundwater
//  Output:  none
//  Purpose: writes groundwater continuity error to report file.
//
{
    WRITE("");
    fprintf(Frpt.file,
    "\n  **************************        Volume         Depth");
    if ( UnitSystem == US) fprintf(Frpt.file,
    "\n  Groundwater Continuity         acre-feet        inches");
    else fprintf(Frpt.file,
    "\n  Groundwater Continuity         hectare-m            mm");
    fprintf(Frpt.file,
    "\n  **************************     ---------       -------");
    fprintf(Frpt.file, "\n  Initial Storage ..........%14.3f%14.3f",
            totals->initStorage * UCF(LENGTH) * UCF(LANDAREA),
            totals->initStorage / gwArea * UCF(RAINDEPTH));

    fprintf(Frpt.file, "\n  Infiltration .............%14.3f%14.3f",
            totals->infil * UCF(LENGTH) * UCF(LANDAREA),
            totals->infil / gwArea * UCF(RAINDEPTH));

    fprintf(Frpt.file, "\n  Upper Zone ET ............%14.3f%14.3f",
            totals->upperEvap * UCF(LENGTH) * UCF(LANDAREA),
            totals->upperEvap / gwArea * UCF(RAINDEPTH));

    fprintf(Frpt.file, "\n  Lower Zone ET ............%14.3f%14.3f",
            totals->lowerEvap * UCF(LENGTH) * UCF(LANDAREA),
            totals->lowerEvap / gwArea * UCF(RAINDEPTH));

    fprintf(Frpt.file, "\n  Deep Percolation .........%14.3f%14.3f",
            totals->lowerPerc * UCF(LENGTH) * UCF(LANDAREA),
            totals->lowerPerc / gwArea * UCF(RAINDEPTH));

    fprintf(Frpt.file, "\n  Groundwater Flow .........%14.3f%14.3f",
            totals->gwater * UCF(LENGTH) * UCF(LANDAREA),
            totals->gwater / gwArea * UCF(RAINDEPTH));

    fprintf(Frpt.file, "\n  Final Storage ............%14.3f%14.3f",
            totals->finalStorage * UCF(LENGTH) * UCF(LANDAREA),
            totals->finalStorage / gwArea * UCF(RAINDEPTH));

    fprintf(Frpt.file, "\n  Continuity Error (%%) .....%14.3f",
            totals->pctError);
    WRITE("");
}

//=============================================================================

void report_writeFlowError(TRoutingTotals *totals)
//
//  Input:  totals = accumulated flow routing totals
//  Output:  none
//  Purpose: writes flow routing continuity error to report file.
//
{
    double ucf1, ucf2;

    ucf1 = UCF(LENGTH) * UCF(LANDAREA);
    if ( UnitSystem == US) ucf2 = MGDperCFS / SECperDAY;
    else                   ucf2 = MLDperCFS / SECperDAY;

    WRITE("");
    fprintf(Frpt.file,
    "\n  **************************        Volume        Volume");
    if ( UnitSystem == US) fprintf(Frpt.file,
    "\n  Flow Routing Continuity        acre-feet      10^6 gal");
    else fprintf(Frpt.file,
    "\n  Flow Routing Continuity        hectare-m      10^6 ltr");
    fprintf(Frpt.file,
    "\n  **************************     ---------     ---------");

    fprintf(Frpt.file, "\n  Dry Weather Inflow .......%14.3f%14.3f",
            totals->dwInflow * ucf1, totals->dwInflow * ucf2);

    fprintf(Frpt.file, "\n  Wet Weather Inflow .......%14.3f%14.3f",
            totals->wwInflow * ucf1, totals->wwInflow * ucf2);

    fprintf(Frpt.file, "\n  Groundwater Inflow .......%14.3f%14.3f",
            totals->gwInflow * ucf1, totals->gwInflow * ucf2);

    fprintf(Frpt.file, "\n  RDII Inflow ..............%14.3f%14.3f",
            totals->iiInflow * ucf1, totals->iiInflow * ucf2);

    fprintf(Frpt.file, "\n  External Inflow ..........%14.3f%14.3f",
            totals->exInflow * ucf1, totals->exInflow * ucf2);

    fprintf(Frpt.file, "\n  External Outflow .........%14.3f%14.3f",
            totals->outflow * ucf1, totals->outflow * ucf2);

    fprintf(Frpt.file, "\n  Flooding Loss ............%14.3f%14.3f",
            totals->flooding * ucf1, totals->flooding * ucf2);

    fprintf(Frpt.file, "\n  Evaporation Loss .........%14.3f%14.3f",
            totals->evapLoss * ucf1, totals->evapLoss * ucf2);

    fprintf(Frpt.file, "\n  Exfiltration Loss ........%14.3f%14.3f",
            totals->seepLoss * ucf1, totals->seepLoss * ucf2);

    fprintf(Frpt.file, "\n  Initial Stored Volume ....%14.3f%14.3f",
            totals->initStorage * ucf1, totals->initStorage * ucf2);

    fprintf(Frpt.file, "\n  Final Stored Volume ......%14.3f%14.3f",
            totals->finalStorage * ucf1, totals->finalStorage * ucf2);

    fprintf(Frpt.file, "\n  Continuity Error (%%) .....%14.3f",
            totals->pctError);
    WRITE("");
}

//=============================================================================

void report_writeQualError(TRoutingTotals QualTotals[])
//
//  Input:   totals = accumulated quality routing totals for each pollutant
//  Output:  none
//  Purpose: writes quality routing continuity error to report file.
//
{
    int p1, p2;
    p1 = 1;
    p2 = MIN(5, Nobjects[POLLUT]);
    while ( p1 <= Nobjects[POLLUT] )
    {
        report_QualErrors(p1-1, p2-1, QualTotals);
        p1 = p2 + 1;
        p2 = p1 + 4;
        p2 = MIN(p2, Nobjects[POLLUT]);
    }
}

//=============================================================================

void report_QualErrors(int p1, int p2, TRoutingTotals* QualTotals)
{
    int   i;
    int   p;
    char  units[15];

    WRITE("");
    fprintf(Frpt.file, "\n  **************************");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14s", Pollut[p].ID);
    }
    fprintf(Frpt.file, "\n  Quality Routing Continuity");
    for (p = p1; p <= p2; p++)
    {
        i = UnitSystem;
        if ( Pollut[p].units == COUNT ) i = 2;
        strcpy(units, LoadUnitsWords[i]);
        fprintf(Frpt.file, "%14s", units);
    }
    fprintf(Frpt.file, "\n  **************************");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "    ----------");
    }

    fprintf(Frpt.file, "\n  Dry Weather Inflow .......");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].dwInflow);
    }

    fprintf(Frpt.file, "\n  Wet Weather Inflow .......");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].wwInflow);
    }

    fprintf(Frpt.file, "\n  Groundwater Inflow .......");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].gwInflow);
    }

    fprintf(Frpt.file, "\n  RDII Inflow ..............");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].iiInflow);
    }

    fprintf(Frpt.file, "\n  External Inflow ..........");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].exInflow);
    }

    fprintf(Frpt.file, "\n  External Outflow .........");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].outflow);
    }

    fprintf(Frpt.file, "\n  Flooding Loss ............");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].flooding);
    }

    fprintf(Frpt.file, "\n  Exfiltration Loss ........");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].seepLoss);
    }

    fprintf(Frpt.file, "\n  Mass Reacted .............");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].reacted);
    }

    fprintf(Frpt.file, "\n  Initial Stored Mass ......");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].initStorage);
    }

    fprintf(Frpt.file, "\n  Final Stored Mass ........");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].finalStorage);
    }

    fprintf(Frpt.file, "\n  Continuity Error (%%) .....");
    for (p = p1; p <= p2; p++)
    {
        fprintf(Frpt.file, "%14.3f", QualTotals[p].pctError);
    }
    WRITE("");
}

//=============================================================================

void report_writeMaxStats(TMaxStats maxMassBalErrs[], TMaxStats maxCourantCrit[],
                          int nMaxStats)
//
//  Input:   maxMassBal[] = nodes with highest mass balance errors
//           maxCourantCrit[] = nodes most often Courant time step critical
//           maxLinkTimes[] = links most often Courant time step critical
//           nMaxStats = number of most critical nodes/links saved
//  Output:  none
//  Purpose: lists nodes & links with highest mass balance errors and
//           time Courant time step critical
//
{
    int i, j, k;

    if ( RouteModel != DW || Nobjects[LINK] == 0 ) return;
    if ( nMaxStats <= 0 ) return;
    if ( maxMassBalErrs[0].index >= 0 )
    {
        WRITE("");
        WRITE("*************************");
        WRITE("Highest Continuity Errors");
        WRITE("*************************");
        for (i=0; i<nMaxStats; i++)
        {
            j = maxMassBalErrs[i].index;
            if ( j < 0 ) continue;
            fprintf(Frpt.file, "\n  Node %s (%.2f%%)",
                Node[j].ID, maxMassBalErrs[i].value);
        }
        WRITE("");
    }

    if ( CourantFactor == 0.0 ) return;
    WRITE("");
    WRITE("***************************");
    WRITE("Time-Step Critical Elements");
    WRITE("***************************");
    k = 0;
    for (i=0; i<nMaxStats; i++)
    {
        j = maxCourantCrit[i].index;
        if ( j < 0 ) continue;
        k++;
        if ( maxCourantCrit[i].objType == NODE )
             fprintf(Frpt.file, "\n  Node %s", Node[j].ID);
        else fprintf(Frpt.file, "\n  Link %s", Link[j].ID);
        fprintf(Frpt.file, " (%.2f%%)", maxCourantCrit[i].value);
    }
    if ( k == 0 ) fprintf(Frpt.file, "\n  None");
    WRITE("");
}

//=============================================================================

void report_writeMaxFlowTurns(TMaxStats flowTurns[], int nMaxStats)
//
//  Input:   flowTurns[] = links with highest number of flow turns
//           nMaxStats = number of links in flowTurns[]
//  Output:  none
//  Purpose: lists links with highest number of flow turns (i.e., fraction
//           of time periods where the flow is higher (or lower) than the
//           flows in the previous and following periods).
//
{
    int i, j;

    if ( Nobjects[LINK] == 0 ) return;
    WRITE("");
    WRITE("********************************");
    WRITE("Highest Flow Instability Indexes");
    WRITE("********************************");
    if ( nMaxStats <= 0 || flowTurns[0].index <= 0 )
        fprintf(Frpt.file, "\n  All links are stable.");
    else
    {
        for (i=0; i<nMaxStats; i++)
        {
            j = flowTurns[i].index;
            if ( j < 0 ) continue;
            fprintf(Frpt.file, "\n  Link %s (%.0f)",
                Link[j].ID, flowTurns[i].value);
        }
    }
    WRITE("");
}

//=============================================================================

void report_writeSysStats(TSysStats* sysStats)
//
//  Input:   sysStats = simulation statistics for overall system
//  Output:  none
//  Purpose: writes simulation statistics for overall system to report file.
//
{
    double x;
    double eventStepCount = (double)StepCount - sysStats->steadyStateCount;

    if ( Nobjects[LINK] == 0 || StepCount == 0
	                     || eventStepCount == 0.0 ) return; 
    WRITE("");
    WRITE("*************************");
    WRITE("Routing Time Step Summary");
    WRITE("*************************");
    fprintf(Frpt.file,
        "\n  Minimum Time Step           :  %7.2f sec",
        sysStats->minTimeStep);
    fprintf(Frpt.file,
        "\n  Average Time Step           :  %7.2f sec",
        sysStats->avgTimeStep / eventStepCount);
    fprintf(Frpt.file,
        "\n  Maximum Time Step           :  %7.2f sec",
        sysStats->maxTimeStep);
    x = (1.0 - sysStats->avgTimeStep * 1000.0 / NewRoutingTime) * 100.0;
    fprintf(Frpt.file,
        "\n  Percent in Steady State     :  %7.2f", MIN(x, 100.0));
    fprintf(Frpt.file,
        "\n  Average Iterations per Step :  %7.2f",
        sysStats->avgStepCount / eventStepCount);
    fprintf(Frpt.file,
        "\n  Percent Not Converging      :  %7.2f",
        100.0 * (double)NonConvergeCount / eventStepCount);
    WRITE("");
}


//=============================================================================
//      SIMULATION RESULTS REPORTING
//=============================================================================

void report_writeReport()
//
//  Input:   none
//  Output:  none
//  Purpose: writes simulation results to report file.
//
{
    if ( ErrorCode ) return;
    if ( Nperiods == 0 ) return;
    if ( RptFlags.subcatchments != NONE
         && ( IgnoreRainfall == FALSE ||
              IgnoreSnowmelt == FALSE ||
              IgnoreGwater == FALSE)
       ) report_Subcatchments();

    if ( IgnoreRouting == TRUE && IgnoreQuality == TRUE ) return;
    if ( RptFlags.nodes != NONE ) report_Nodes();
    if ( RptFlags.links != NONE ) report_Links();
}

//=============================================================================

void report_Subcatchments()
//
//  Input:   none
//  Output:  none
//  Purpose: writes results for selected subcatchments to report file.
//
{
    int      j, p, k;
    int      period;
    DateTime days;
    char     theDate[12];
    char     theTime[9];
    int      hasSnowmelt = (Nobjects[SNOWMELT] > 0 && !IgnoreSnowmelt);
    int      hasGwater   = (Nobjects[AQUIFER] > 0  && !IgnoreGwater);
    int      hasQuality  = (Nobjects[POLLUT] > 0 && !IgnoreQuality);

    if ( Nobjects[SUBCATCH] == 0 ) return;
    WRITE("");
    WRITE("********************");
    WRITE("Subcatchment Results");
    WRITE("********************");
    k = 0;
    for (j = 0; j < Nobjects[SUBCATCH]; j++)
    {
        if ( Subcatch[j].rptFlag == TRUE )
        {
            report_SubcatchHeader(Subcatch[j].ID);
            for ( period = 1; period <= Nperiods; period++ )
            {
                output_readDateTime(period, &days);
                datetime_dateToStr(days, theDate);
                datetime_timeToStr(days, theTime);
                output_readSubcatchResults(period, k);
                fprintf(Frpt.file, "\n  %11s %8s %10.3f%10.3f%10.4f",
                    theDate, theTime, SubcatchResults[SUBCATCH_RAINFALL],
                    SubcatchResults[SUBCATCH_EVAP]/24.0 +
                    SubcatchResults[SUBCATCH_INFIL],
                    SubcatchResults[SUBCATCH_RUNOFF]);
                if ( hasSnowmelt )
                    fprintf(Frpt.file, "  %10.3f",
                        SubcatchResults[SUBCATCH_SNOWDEPTH]);
                if ( hasGwater )
                    fprintf(Frpt.file, "%10.3f%10.4f",
                        SubcatchResults[SUBCATCH_GW_ELEV],
                        SubcatchResults[SUBCATCH_GW_FLOW]);
                if ( hasQuality )
                    for (p = 0; p < Nobjects[POLLUT]; p++)
                        fprintf(Frpt.file, "%10.3f",
                            SubcatchResults[SUBCATCH_WASHOFF+p]);
            }
            WRITE("");
            k++;
        }
    }
}

//=============================================================================

void  report_SubcatchHeader(char *id)
//
//  Input:   id = subcatchment ID name
//  Output:  none
//  Purpose: writes table headings for subcatchment results to report file.
//
{
    int i;
    int hasSnowmelt = (Nobjects[SNOWMELT] > 0 && !IgnoreSnowmelt);
    int hasGwater   = (Nobjects[AQUIFER] > 0  && !IgnoreGwater);
    int hasQuality  = (Nobjects[POLLUT] > 0 && !IgnoreQuality);

    // --- print top border of header
    WRITE("");
    fprintf(Frpt.file,"\n  <<< Subcatchment %s >>>", id);
    WRITE(LINE_51);
    if ( hasSnowmelt  > 0 ) fprintf(Frpt.file, LINE_12);
    if ( hasGwater )
    {
        fprintf(Frpt.file, LINE_10);
        fprintf(Frpt.file, LINE_10);
    }
    if ( hasQuality )
    {
        for (i = 0; i < Nobjects[POLLUT]; i++) fprintf(Frpt.file, LINE_10);
    }

    // --- print first line of column headings
    fprintf(Frpt.file,
    "\n  Date        Time        Precip.    Losses    Runoff");
    if ( hasSnowmelt ) fprintf(Frpt.file, "  Snow Depth");
    if ( hasGwater   ) fprintf(Frpt.file, "  GW Elev.   GW Flow");
    if ( hasQuality ) for (i = 0; i < Nobjects[POLLUT]; i++)
        fprintf(Frpt.file, "%10s", Pollut[i].ID);

    // --- print second line of column headings
    if ( UnitSystem == US ) fprintf(Frpt.file,
    "\n                            in/hr     in/hr %9s", FlowUnitWords[FlowUnits]);
    else fprintf(Frpt.file,
    "\n                            mm/hr     mm/hr %9s", FlowUnitWords[FlowUnits]);
    if ( hasSnowmelt )
    {
        if ( UnitSystem == US ) fprintf(Frpt.file, "      inches");
        else                    fprintf(Frpt.file, "     mmeters");
    }
    if ( hasGwater )
    {
        if ( UnitSystem == US )
            fprintf(Frpt.file, "      feet %9s", FlowUnitWords[FlowUnits]);
        else
            fprintf(Frpt.file, "    meters %9s", FlowUnitWords[FlowUnits]);
    }
    if ( hasQuality ) for (i = 0; i < Nobjects[POLLUT]; i++)
        fprintf(Frpt.file, "%10s", QualUnitsWords[Pollut[i].units]);

    // --- print lower border of header
    WRITE(LINE_51);
    if ( hasSnowmelt ) fprintf(Frpt.file, LINE_12);
    if ( hasGwater )
    {
        fprintf(Frpt.file, LINE_10);
        fprintf(Frpt.file, LINE_10);
    }
    if ( hasQuality ) for (i = 0; i < Nobjects[POLLUT]; i++)
        fprintf(Frpt.file, LINE_10);
}

//=============================================================================

void report_Nodes()
//
//  Input:   none
//  Output:  none
//  Purpose: writes results for selected nodes to report file.
//
{
    int      j, p, k;
    int      period;
    DateTime days;
    char     theDate[20];
    char     theTime[20];

    if ( Nobjects[NODE] == 0 ) return;
    WRITE("");
    WRITE("************");
    WRITE("Node Results");
    WRITE("************");
    k = 0;
    for (j = 0; j < Nobjects[NODE]; j++)
    {
        if ( Node[j].rptFlag == TRUE )
        {
            report_NodeHeader(Node[j].ID);
            for ( period = 1; period <= Nperiods; period++ )
            {
                output_readDateTime(period, &days);
                datetime_dateToStr(days, theDate);
                datetime_timeToStr(days, theTime);
                output_readNodeResults(period, k);
                fprintf(Frpt.file, "\n  %11s %8s  %9.3f %9.3f %9.3f %9.3f",
                    theDate, theTime, NodeResults[NODE_INFLOW],
                    NodeResults[NODE_OVERFLOW], NodeResults[NODE_DEPTH],
                    NodeResults[NODE_HEAD]);
                if ( !IgnoreQuality ) for (p = 0; p < Nobjects[POLLUT]; p++)
                    fprintf(Frpt.file, " %9.3f", NodeResults[NODE_QUAL + p]);
            }
            WRITE("");
            k++;
        }
    }
}

//=============================================================================

void  report_NodeHeader(char *id)
//
//  Input:   id = node ID name
//  Output:  none
//  Purpose: writes table headings for node results to report file.
//
{
    int i;
    char lengthUnits[9];
    WRITE("");
    fprintf(Frpt.file,"\n  <<< Node %s >>>", id);
    WRITE(LINE_64);
    for (i = 0; i < Nobjects[POLLUT]; i++) fprintf(Frpt.file, LINE_10);

    fprintf(Frpt.file,
    "\n                           Inflow  Flooding     Depth      Head");
    if ( !IgnoreQuality ) for (i = 0; i < Nobjects[POLLUT]; i++)
        fprintf(Frpt.file, "%10s", Pollut[i].ID);
    if ( UnitSystem == US) strcpy(lengthUnits, "feet");
    else strcpy(lengthUnits, "meters");
    fprintf(Frpt.file,
    "\n  Date        Time      %9s %9s %9s %9s",
        FlowUnitWords[FlowUnits], FlowUnitWords[FlowUnits],
        lengthUnits, lengthUnits);
    if ( !IgnoreQuality ) for (i = 0; i < Nobjects[POLLUT]; i++)
        fprintf(Frpt.file, "%10s", QualUnitsWords[Pollut[i].units]);

    WRITE(LINE_64);
    if ( !IgnoreQuality )
        for (i = 0; i < Nobjects[POLLUT]; i++) fprintf(Frpt.file, LINE_10);
}

//=============================================================================

void report_Links()
//
//  Input:   none
//  Output:  none
//  Purpose: writes results for selected links to report file.
//
{
    int      j, p, k;
    int      period;
    DateTime days;
    char     theDate[12];
    char     theTime[9];

    if ( Nobjects[LINK] == 0 ) return;
    WRITE("");
    WRITE("************");
    WRITE("Link Results");
    WRITE("************");
    k = 0;
    for (j = 0; j < Nobjects[LINK]; j++)
    {
        if ( Link[j].rptFlag == TRUE )
        {
            report_LinkHeader(Link[j].ID);
            for ( period = 1; period <= Nperiods; period++ )
            {
                output_readDateTime(period, &days);
                datetime_dateToStr(days, theDate);
                datetime_timeToStr(days, theTime);
                output_readLinkResults(period, k);
                fprintf(Frpt.file, "\n  %11s %8s  %9.3f %9.3f %9.3f %9.3f",
                    theDate, theTime, LinkResults[LINK_FLOW],
                    LinkResults[LINK_VELOCITY], LinkResults[LINK_DEPTH],
                    LinkResults[LINK_CAPACITY]);
                if ( !IgnoreQuality ) for (p = 0; p < Nobjects[POLLUT]; p++)
                    fprintf(Frpt.file, " %9.3f", LinkResults[LINK_QUAL + p]);
            }
            WRITE("");
            k++;
        }
    }
}

//=============================================================================

void  report_LinkHeader(char *id)
//
//  Input:   id = link ID name
//  Output:  none
//  Purpose: writes table headings for link results to report file.
//
{
    int i;
    WRITE("");
    fprintf(Frpt.file,"\n  <<< Link %s >>>", id);
    WRITE(LINE_64);
    for (i = 0; i < Nobjects[POLLUT]; i++) fprintf(Frpt.file, LINE_10);

    fprintf(Frpt.file,
    "\n                             Flow  Velocity     Depth  Capacity/");
    if ( !IgnoreQuality ) for (i = 0; i < Nobjects[POLLUT]; i++)
        fprintf(Frpt.file, "%10s", Pollut[i].ID);

    if ( UnitSystem == US )
        fprintf(Frpt.file,
        "\n  Date        Time     %10s    ft/sec      feet   Setting ",
        FlowUnitWords[FlowUnits]);
    else
        fprintf(Frpt.file,
        "\n  Date        Time     %10s     m/sec    meters   Setting ",
        FlowUnitWords[FlowUnits]);
    if ( !IgnoreQuality ) for (i = 0; i < Nobjects[POLLUT]; i++)
        fprintf(Frpt.file, " %9s", QualUnitsWords[Pollut[i].units]);

    WRITE(LINE_64);
    if ( !IgnoreQuality )
        for (i = 0; i < Nobjects[POLLUT]; i++) fprintf(Frpt.file, LINE_10);
}


//=============================================================================
//      ERROR REPORTING
//=============================================================================

void report_writeErrorMsg(int code, char* s)
//
//  Input:   code = error code
//           s = error message text
//  Output:  none
//  Purpose: writes error message to report file.
//
{
    if ( Frpt.file )
    {
        WRITE("");
        fprintf(Frpt.file, error_getMsg(code), s);
    }
    ErrorCode = code;

    // --- save message to ErrorMsg if it's not for a line of input data
    if ( ErrorCode <= ERR_INPUT || ErrorCode >= ERR_FILE_NAME )
    {                                                
        sprintf(ErrorMsg, error_getMsg(ErrorCode), s);
    }
}

//=============================================================================

void report_writeErrorCode()
//
//  Input:   none
//  Output:  none
//  Purpose: writes error message to report file.
//
{
    if ( Frpt.file )
    {
        if ( (ErrorCode >= ERR_MEMORY && ErrorCode <= ERR_TIMESTEP)
        ||   (ErrorCode >= ERR_FILE_NAME && ErrorCode <= ERR_OUT_FILE)
        ||   (ErrorCode == ERR_SYSTEM) )
            fprintf(Frpt.file, "%s", error_getMsg(ErrorCode));                 //(5.1.013)
    }
}

//=============================================================================

void report_writeInputErrorMsg(int k, int sect, char* line, long lineCount)
//
//  Input:   k = error code
//           sect = number of input data section where error occurred
//           line = line of data containing the error
//           lineCount = line number of data file containing the error
//  Output:  none
//  Purpose: writes input error message to report file.
//
{
    if ( Frpt.file )
    {
        report_writeErrorMsg(k, ErrString);
        if ( sect < 0 ) fprintf(Frpt.file, FMT17, lineCount);
        else            fprintf(Frpt.file, FMT18, lineCount, SectWords[sect]);
        fprintf(Frpt.file, "\n  %s", line);
    }
}

//=============================================================================

void report_writeWarningMsg(char* msg, char* id)
//
//  Input:   msg = text of warning message
//           id = ID name of object that message refers to
//  Output:  none
//  Purpose: writes a warning message to the report file.
//
{
    fprintf(Frpt.file, "\n  %s %s", msg, id);
    Warnings++;
}

//=============================================================================

void report_writeTseriesErrorMsg(int code, TTable *tseries)
//
//  Input:   tseries = pointer to a time series
//  Output:  none
//  Purpose: writes the date where a time series' data is out of order.
//
{
    char     theDate[20];
    char     theTime[20];
    DateTime x;

    if (code == ERR_CURVE_SEQUENCE)
    {
        x = tseries->x2;
        datetime_dateToStr(x, theDate);
        datetime_timeToStr(x, theTime);
        report_writeErrorMsg(ERR_TIMESERIES_SEQUENCE, tseries->ID);
        fprintf(Frpt.file, " at %s %s.", theDate, theTime);
    }
    else report_writeErrorMsg(code, tseries->ID);
}
