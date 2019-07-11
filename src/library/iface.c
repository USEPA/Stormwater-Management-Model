//-----------------------------------------------------------------------------
//   iface.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//   Author:   L. Rossman
//
//   Routing interface file functions.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  Imported variables
//-----------------------------------------------------------------------------
extern double Qcf[];                   // flow units conversion factors
                                       // (see swmm5.c)

//-----------------------------------------------------------------------------                  
//  Shared variables
//-----------------------------------------------------------------------------                  
static int      IfaceFlowUnits;        // flow units for routing interface file
static int      IfaceStep;             // interface file time step (sec)
static int      NumIfacePolluts;       // number of pollutants in interface file
static int*     IfacePolluts;          // indexes of interface file pollutants
static int      NumIfaceNodes;         // number of nodes on interface file
static int*     IfaceNodes;            // indexes of nodes on interface file
static double** OldIfaceValues;        // interface flows & WQ at previous time
static double** NewIfaceValues;        // interface flows & WQ at next time
static double   IfaceFrac;             // fraction of interface file time step
static DateTime OldIfaceDate;          // previous date of interface values
static DateTime NewIfaceDate;          // next date of interface values

//-----------------------------------------------------------------------------
//  External Functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  iface_readFileParams     (called by input_readLine)
//  iface_openRoutingFiles   (called by routing_open)
//  iface_closeRoutingFiles  (called by routing_close)
//  iface_getNumIfaceNodes   (called by addIfaceInflows in routing.c)
//  iface_getIfaceNode       (called by addIfaceInflows in routing.c)
//  iface_getIfaceFlow       (called by addIfaceInflows in routing.c)
//  iface_getIfaceQual       (called by addIfaceInflows in routing.c)
//  iface_saveOutletResults  (called by output_saveResults)

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void  openFileForOutput(void);
static void  openFileForInput(void);
static int   getIfaceFilePolluts(void);
static int   getIfaceFileNodes(void);
static void  setOldIfaceValues(void);
static void  readNewIfaceValues(void);
static int   isOutletNode(int node);


//=============================================================================

int iface_readFileParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads interface file information from a line of input data.
//
//  Data format is:
//  USE/SAVE  FileType  FileName
//
{
    char  k;
    int   j;

    // --- determine file disposition and type
    if ( ntoks < 2 ) return error_setInpError(ERR_ITEMS, "");
    k = (char)findmatch(tok[0], FileModeWords);
    if ( k < 0 ) return error_setInpError(ERR_KEYWORD, tok[0]);
    j = findmatch(tok[1], FileTypeWords);
    if ( j < 0 ) return error_setInpError(ERR_KEYWORD, tok[1]);
    if ( ntoks < 3 ) return 0;

    // --- process file name
    switch ( j )
    {
      case RAINFALL_FILE:
        Frain.mode = k;
        sstrncpy(Frain.name, tok[2], MAXFNAME);
        break;

      case RUNOFF_FILE:
        Frunoff.mode = k;
        sstrncpy(Frunoff.name, tok[2], MAXFNAME);
        break;

      case HOTSTART_FILE:
        if ( k == USE_FILE )
        {
            Fhotstart1.mode = k;
            sstrncpy(Fhotstart1.name, tok[2], MAXFNAME);
        }
        else if ( k == SAVE_FILE )
        {
            Fhotstart2.mode = k;
            sstrncpy(Fhotstart2.name, tok[2], MAXFNAME);
        }
        break;

      case RDII_FILE:
        Frdii.mode = k;
        sstrncpy(Frdii.name, tok[2], MAXFNAME);
        break;

      case INFLOWS_FILE:
        if ( k != USE_FILE ) return error_setInpError(ERR_ITEMS, "");
        Finflows.mode = k;
        sstrncpy(Finflows.name, tok[2], MAXFNAME);
        break;

      case OUTFLOWS_FILE:
        if ( k != SAVE_FILE ) return error_setInpError(ERR_ITEMS, "");
        Foutflows.mode = k;
        sstrncpy(Foutflows.name, tok[2], MAXFNAME);
        break;
    }
    return 0;
}

//=============================================================================

void iface_openRoutingFiles()
//
//  Input:   none
//  Output:  none
//  Purpose: opens routing interface files.
//
{
    // --- initialize shared variables
    NumIfacePolluts = 0;
    IfacePolluts = NULL;
    NumIfaceNodes = 0;
    IfaceNodes = NULL;
    OldIfaceValues = NULL;
    NewIfaceValues = NULL;

    // --- check that inflows & outflows files are not the same
    if ( Foutflows.mode != NO_FILE && Finflows.mode != NO_FILE )
    {
        if ( strcomp(Foutflows.name, Finflows.name) )
        {
            report_writeErrorMsg(ERR_ROUTING_FILE_NAMES, "");
            return;
        }
    }

    // --- open the file for reading or writing
    if ( Foutflows.mode == SAVE_FILE ) openFileForOutput();
    if ( Finflows.mode == USE_FILE ) openFileForInput();
}

//=============================================================================

void iface_closeRoutingFiles()
//
//  Input:   none
//  Output:  none
//  Purpose: closes routing interface files.
//
{
    FREE(IfacePolluts);
    FREE(IfaceNodes);
    if ( OldIfaceValues != NULL ) project_freeMatrix(OldIfaceValues);
    if ( NewIfaceValues != NULL ) project_freeMatrix(NewIfaceValues);
    if ( Finflows.file )  fclose(Finflows.file);
    if ( Foutflows.file ) fclose(Foutflows.file);
}

//=============================================================================

int iface_getNumIfaceNodes(DateTime currentDate)
//
//  Input:   currentDate = current date/time
//  Output:  returns number of interface nodes if data exists or
//           0 otherwise
//  Purpose: reads inflow data from interface file at current date.
//
{
    // --- return 0 if file begins after current date
    if ( OldIfaceDate > currentDate ) return 0;

    // --- keep updating new interface values until current date bracketed
    while ( NewIfaceDate < currentDate && NewIfaceDate != NO_DATE )
    {
        setOldIfaceValues();
        readNewIfaceValues();
    }

    // --- return 0 if no data available
    if ( NewIfaceDate == NO_DATE ) return 0;

    // --- find fraction current date is bewteen old & new interface dates
    IfaceFrac = (currentDate - OldIfaceDate) / (NewIfaceDate - OldIfaceDate);
    IfaceFrac = MAX(0.0, IfaceFrac);
    IfaceFrac = MIN(IfaceFrac, 1.0);

    // --- return number of interface nodes
    return NumIfaceNodes;
}

//=============================================================================

int iface_getIfaceNode(int index)
//
//  Input:   index = interface file node index
//  Output:  returns project node index
//  Purpose: finds index of project node associated with interface node index
//
{
    if ( index >= 0 && index < NumIfaceNodes ) return IfaceNodes[index];
    else return -1;
}

//=============================================================================

double iface_getIfaceFlow(int index)
//
//  Input:   index = interface file node index
//  Output:  returns inflow to node
//  Purpose: finds interface flow for particular node index.
//
{
    double q1, q2;

    if ( index >= 0 && index < NumIfaceNodes )
    {
        // --- interpolate flow between old and new values
        q1 = OldIfaceValues[index][0];
        q2 = NewIfaceValues[index][0];
        return (1.0 - IfaceFrac)*q1 + IfaceFrac*q2;
    }
    else return 0.0;
}

//=============================================================================

double iface_getIfaceQual(int index, int pollut)
//
//  Input:   index = index of node on interface file
//           pollut = index of pollutant on interface file
//  Output:  returns inflow pollutant concentration
//  Purpose: finds interface concentration for particular node index & pollutant.
//
{
    int    j;
    double c1, c2;

    if ( index >= 0 && index < NumIfaceNodes )
    {
        // --- find index of pollut on interface file
        j = IfacePolluts[pollut];
        if ( j < 0 ) return 0.0;

        // --- interpolate flow between old and new values
        //     (remember that 1st col. of values matrix is for flow)
        c1 = OldIfaceValues[index][j+1];
        c2 = NewIfaceValues[index][j+1];
        return (1.0 - IfaceFrac)*c1 + IfaceFrac*c2;
    }
    else return 0.0;
}

//=============================================================================

void iface_saveOutletResults(DateTime reportDate, FILE* file)
//
//  Input:   reportDate = reporting date/time
//           file = ptr. to interface file
//  Output:  none
//  Purpose: saves system outflows to routing interface file.
//
{
    int i, p, yr, mon, day, hr, min, sec;
    char theDate[25];
    datetime_decodeDate(reportDate, &yr, &mon, &day);
    datetime_decodeTime(reportDate, &hr, &min, &sec);
    sprintf(theDate, " %04d %02d  %02d  %02d  %02d  %02d ",
            yr, mon, day, hr, min, sec);
    for (i=0; i<Nobjects[NODE]; i++)
    {
        // --- check that node is an outlet node
        if ( !isOutletNode(i) ) continue;

        // --- write node ID, date, flow, and quality to file
        fprintf(file, "\n%-16s", Node[i].ID);
        fprintf(file, "%s", theDate);
        fprintf(file, " %-10f", Node[i].inflow * UCF(FLOW));
        for ( p = 0; p < Nobjects[POLLUT]; p++ )
        {
            fprintf(file, " %-10f", Node[i].newQual[p]);
        }
    }
}

//=============================================================================

void openFileForOutput()
//
//  Input:   none
//  Output:  none
//  Purpose: opens a routing interface file for writing.
//
{
    int i, n;

    // --- open the routing file for writing text
    Foutflows.file = fopen(Foutflows.name, "wt");
    if ( Foutflows.file == NULL )
    {
        report_writeErrorMsg(ERR_ROUTING_FILE_OPEN, Foutflows.name);
        return;
    }

    // --- write title & reporting time step to file
    fprintf(Foutflows.file, "SWMM5 Interface File");
    fprintf(Foutflows.file, "\n%s", Title[0]);
    fprintf(Foutflows.file, "\n%-4d - reporting time step in sec", ReportStep);

    // --- write number & names of each constituent (including flow) to file
    fprintf(Foutflows.file, "\n%-4d - number of constituents as listed below:",
            Nobjects[POLLUT] + 1);
    fprintf(Foutflows.file, "\nFLOW %s", FlowUnitWords[FlowUnits]);
    for (i=0; i<Nobjects[POLLUT]; i++)
    {
        fprintf(Foutflows.file, "\n%s %s", Pollut[i].ID,
            QualUnitsWords[Pollut[i].units]);
    }

    // --- count number of outlet nodes
    n = 0;
    for (i=0; i<Nobjects[NODE]; i++)
    {
        if ( isOutletNode(i) ) n++;
    }

    // --- write number and names of outlet nodes to file
    fprintf(Foutflows.file, "\n%-4d - number of nodes as listed below:", n);
    for (i=0; i<Nobjects[NODE]; i++)
    {
          if ( isOutletNode(i) )
            fprintf(Foutflows.file, "\n%s", Node[i].ID);
    }

    // --- write column headings
    fprintf(Foutflows.file,
        "\nNode             Year Mon Day Hr  Min Sec FLOW      ");
    for (i=0; i<Nobjects[POLLUT]; i++)
    {
        fprintf(Foutflows.file, " %-10s", Pollut[i].ID);
    }

    // --- if reporting starts immediately, save initial outlet values
    if ( ReportStart == StartDateTime )
    {
        iface_saveOutletResults(ReportStart, Foutflows.file);
    }
}

//=============================================================================

void openFileForInput()
//
//  Input:   none
//  Output:  none
//  Purpose: opens a routing interface file for reading.
//
{
    int   err;                         // error code
    char  line[MAXLINE+1];             // line from Routing interface file
    char  s[MAXLINE+1];                // general string variable

    // --- open the routing interface file for reading text
    Finflows.file = fopen(Finflows.name, "rt");
    if ( Finflows.file == NULL )
    {
        report_writeErrorMsg(ERR_ROUTING_FILE_OPEN, Finflows.name);
        return;
    }

    // --- check for correct file type
    fgets(line, MAXLINE, Finflows.file);
    sscanf(line, "%s", s);
    if ( !strcomp(s, "SWMM5") )
    {
        report_writeErrorMsg(ERR_ROUTING_FILE_FORMAT, Finflows.name);
        return;
    }

    // --- skip title line
    fgets(line, MAXLINE, Finflows.file);

    // --- read reporting time step (sec)
    IfaceStep = 0;
    fgets(line, MAXLINE, Finflows.file);
    sscanf(line, "%d", &IfaceStep);
    if ( IfaceStep <= 0 )
    {
        report_writeErrorMsg(ERR_ROUTING_FILE_FORMAT, Finflows.name);
        return;
    }

    // --- match constituents in file with those in project
    err = getIfaceFilePolluts();
    if ( err > 0 )
    {
        report_writeErrorMsg(err, Finflows.name);
        return;
    }

    // --- match nodes in file with those in project
    err = getIfaceFileNodes();
    if ( err > 0 )
    {
        report_writeErrorMsg(err, Finflows.name);
        return;
    }

    // --- create matrices for old & new interface flows & WQ values
    OldIfaceValues = project_createMatrix(NumIfaceNodes,
                                         1+NumIfacePolluts);
    NewIfaceValues = project_createMatrix(NumIfaceNodes,
                                         1+NumIfacePolluts);
    if ( OldIfaceValues == NULL || NewIfaceValues == NULL )
    {
        report_writeErrorMsg(ERR_MEMORY, "");
        return;
    }

    // --- read in new interface flows & WQ values
    readNewIfaceValues();
    OldIfaceDate = NewIfaceDate;
}

//=============================================================================

int  getIfaceFilePolluts()
//
//  Input:   none
//  Output:  returns an error code
//  Purpose: reads names of pollutants saved on the inflows interface file.
//
{
    int   i, j;
    char  line[MAXLINE+1];             // line from inflows interface file
    char  s1[MAXLINE+1];               // general string variable
    char  s2[MAXLINE+1];         

    // --- read number of pollutants (minus FLOW)
    fgets(line, MAXLINE, Finflows.file);
    sscanf(line, "%d", &NumIfacePolluts);
    NumIfacePolluts--;
    if ( NumIfacePolluts < 0 ) return ERR_ROUTING_FILE_FORMAT;

    // --- read flow units
    fgets(line, MAXLINE, Finflows.file);
    sscanf(line, "%s %s", s1, s2);
    if ( !strcomp(s1, "FLOW") )  return ERR_ROUTING_FILE_FORMAT;
    IfaceFlowUnits = findmatch(s2, FlowUnitWords);
    if ( IfaceFlowUnits < 0 ) return ERR_ROUTING_FILE_FORMAT;

    // --- allocate memory for pollutant index array
    if ( Nobjects[POLLUT] > 0 )
    {
        IfacePolluts = (int *) calloc(Nobjects[POLLUT], sizeof(int));
        if ( !IfacePolluts ) return ERR_MEMORY;
        for (i=0; i<Nobjects[POLLUT]; i++) IfacePolluts[i] = -1;
    }

    // --- read pollutant names & units
    if ( NumIfacePolluts > 0 )
    {
        // --- check each pollutant name on file with project's pollutants
        for (i=0; i<NumIfacePolluts; i++)
        {
            if ( feof(Finflows.file) ) return ERR_ROUTING_FILE_FORMAT;
            fgets(line, MAXLINE, Finflows.file);
            sscanf(line, "%s %s", s1, s2);
            if ( Nobjects[POLLUT] > 0 )
            {
                j = project_findObject(POLLUT, s1);
                if ( j < 0 ) continue;
                if ( !strcomp(s2, QualUnitsWords[Pollut[j].units]) )
                    return ERR_ROUTING_FILE_NOMATCH;
                IfacePolluts[j] = i;
            }
        }
    }
    return 0;
}

//=============================================================================

int getIfaceFileNodes()
//
//  Input:   none
//  Output:  returns an error code
//  Purpose: reads names of nodes contained on inflows interface file.
//
{
    int   i;
    char  line[MAXLINE+1];             // line from inflows interface file
    char  s[MAXLINE+1];                // general string variable

    // --- read number of interface nodes
    fgets(line, MAXLINE, Finflows.file);
    sscanf(line, "%d", &NumIfaceNodes);
    if ( NumIfaceNodes <= 0 ) return ERR_ROUTING_FILE_FORMAT;

    // --- allocate memory for interface nodes index array
    IfaceNodes = (int *) calloc(NumIfaceNodes, sizeof(int));
    if ( !IfaceNodes ) return ERR_MEMORY;

    // --- read names of interface nodes from file & save their indexes
    for ( i=0; i<NumIfaceNodes; i++ )
    {
        if ( feof(Finflows.file) ) return ERR_ROUTING_FILE_FORMAT;
        fgets(line, MAXLINE, Finflows.file);
        sscanf(line, "%s", s);
        IfaceNodes[i] = project_findObject(NODE, s);
    }

    // --- skip over column headings line
    if ( feof(Finflows.file) ) return ERR_ROUTING_FILE_FORMAT;
    fgets(line, MAXLINE, Finflows.file);
    return 0;
}

//=============================================================================

void readNewIfaceValues()
//
//  Input:   none
//  Output:  none
//  Purpose: reads data from inflows interface file for next date.
//
{
    int    i, j;
    char*  s;
    int    yr = 0, mon = 0, day = 0,
		   hr = 0, min = 0, sec = 0;   // year, month, day, hour, minute, second
    char   line[MAXLINE+1];            // line from interface file

    // --- read a line for each interface node
    NewIfaceDate = NO_DATE;
    for (i=0; i<NumIfaceNodes; i++)
    {
        if ( feof(Finflows.file) ) return;
        fgets(line, MAXLINE, Finflows.file);

        // --- parse date & time from line
        if ( strtok(line, SEPSTR) == NULL ) return;
        s = strtok(NULL, SEPSTR);
        if ( s == NULL ) return;
        yr  = atoi(s);
        s = strtok(NULL, SEPSTR);
        if ( s == NULL ) return;
        mon = atoi(s);
        s = strtok(NULL, SEPSTR);
        if ( s == NULL ) return;
        day = atoi(s);
        s = strtok(NULL, SEPSTR);
        if ( s == NULL ) return;
        hr  = atoi(s);
        s = strtok(NULL, SEPSTR);
        if ( s == NULL ) return;
        min = atoi(s);
        s = strtok(NULL, SEPSTR);
        if ( s == NULL ) return;
        sec = atoi(s);

        // --- parse flow value
        s = strtok(NULL, SEPSTR);
        if ( s == NULL ) return;
        NewIfaceValues[i][0] = atof(s) / Qcf[IfaceFlowUnits]; 

        // --- parse pollutant values
        for (j=1; j<=NumIfacePolluts; j++)
        {
            s = strtok(NULL, SEPSTR);
            if ( s == NULL ) return;
            NewIfaceValues[i][j] = atof(s);
        }

    }

    // --- encode date & time values
    NewIfaceDate = datetime_encodeDate(yr, mon, day) +
                   datetime_encodeTime(hr, min, sec);
}

//=============================================================================

void setOldIfaceValues()
//
//  Input:   none
//  Output:  none
//  Purpose: replaces old values read from routing interface file with new ones. 
//
{
    int i, j;
    OldIfaceDate = NewIfaceDate;
    for ( i=0; i<NumIfaceNodes; i++)
    {
        for ( j=0; j<NumIfacePolluts+1; j++ )
        {
            OldIfaceValues[i][j] = NewIfaceValues[i][j];
        }
    }
}

//=============================================================================

int  isOutletNode(int i)
//
//  Input:   i = node index
//  Output:  returns 1 if node is an outlet, 0 if not.
//  Purpose: determines if a node is an outlet point or not.
//
{
    // --- for DW routing only outfalls are outlets
    if ( RouteModel == DW )
    {
        return (Node[i].type == OUTFALL);
    }

    // --- otherwise outlets are nodes with no outflow links (degree is 0)
    else return (Node[i].degree == 0);
}
