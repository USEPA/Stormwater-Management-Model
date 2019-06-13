//-----------------------------------------------------------------------------
//   rdii.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//             04/04/14   (Build 5.1.003)
//             04/14/14   (Build 5.1.004)
//             09/15/14   (Build 5.1.007)
//   Author:   L. Rossman (EPA)
//             R. Dickinson (CDM)
//
//   RDII processing functions.
//
//   Note: RDII means rainfall dependent infiltration/inflow,
//         UH means unit hydrograph.
//
//   Build 5.1.007:
//   - Ignore RDII option implemented.
//   - Rainfall climate adjustment implemented.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "headers.h"

//-----------------------------------------------------------------------------
// Definition of 4-byte integer, 4-byte real and 8-byte real types
//-----------------------------------------------------------------------------
#define INT4  int
#define REAL4 float
#define REAL8 double
#define FILE_STAMP "SWMM5-RDII"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
const double ZERO_RDII = 0.0001;       // Minimum non-zero RDII inflow (cfs)
const char   FileStamp[] = FILE_STAMP;

//-----------------------------------------------------------------------------
// Data Structures
//-----------------------------------------------------------------------------
enum FileTypes {BINARY, TEXT};         // File mode types

typedef struct                         // Data for a single unit hydrograph
{                                      // -------------------------------------
   double*   pastRain;                 // array of past rainfall values
   char*     pastMonth;                // month in which past rainfall occurred
   int       period;                   // current UH time period
   int       hasPastRain;              // true if > 0 past periods with rain
   int       maxPeriods;               // max. past rainfall periods
   long      drySeconds;               // time since last nonzero rainfall
   double    iaUsed;                   // initial abstraction used (in or mm)
}  TUHData;

typedef struct                         // Data for a unit hydrograph group
{                                      //---------------------------------
   int       isUsed;                   // true if UH group used by any nodes
   int       rainInterval;             // time interval for RDII processing (sec)
   double    area;                     // sewered area covered by UH's gage (ft2)
   double    rdii;                     // rdii flow (in rainfall units)
   DateTime  gageDate;                 // calendar date of rain gage period
   DateTime  lastDate;                 // date of last rdii computed
   TUHData   uh[3];                    // data for each unit hydrograph
}  TUHGroup;

//-----------------------------------------------------------------------------
// Shared Variables
//-----------------------------------------------------------------------------
static TUHGroup*  UHGroup;             // processing data for each UH group
static int        RdiiStep;            // RDII time step (sec)
static int        NumRdiiNodes;        // number of nodes w/ RDII data
static int*       RdiiNodeIndex;       // indexes of nodes w/ RDII data
static REAL4*     RdiiNodeFlow;        // inflows for nodes with RDII
static int        RdiiFlowUnits;       // RDII flow units code
static DateTime   RdiiStartDate;       // start date of RDII inflow period
static DateTime   RdiiEndDate;         // end date of RDII inflow period
static double     TotalRainVol;        // total rainfall volume (ft3)
static double     TotalRdiiVol;        // total RDII volume (ft3)
static int        RdiiFileType;        // type (binary/text) of RDII file

//-----------------------------------------------------------------------------
// Imported Variables
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern const double Qcf[];             // flow units conversion factors
                                       // (see swmm5.c)
#else
extern double Qcf[];                   // flow units conversion factors
                                       // (see swmm5.c)
#endif 

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  rdii_readRdiiInflow     (called from parseLine in input.c)
//  rdii_deleteRdiiInflow   (called from deleteObjects in project.c)
//  rdii_initUnitHyd        (called from createObjects in project.c)
//  rdii_readUnitHydParams  (called from parseLine in input.c)
//  rdii_openRdii           (called from rain_open)
//  rdii_closeRdii          (called from rain_close)
//  rdii_getNumRdiiFlows    (called from addRdiiInflows in routing.c)
//  rdii_getRdiiFlow        (called from addRdiiInflows in routing.c)

//-----------------------------------------------------------------------------
// Function Declarations
//-----------------------------------------------------------------------------
// --- functions used to create a RDII file
static int    readOldUHFormat(int j, int m, char* tok[], int ntoks);
static void   setUnitHydParams(int j, int i, int m, double x[]);
static void   createRdiiFile(void);
static int    getNumRdiiNodes(void);
static void   validateRdii(void);

static void   openRdiiProcessor(void);
static int    allocRdiiMemory(void);
static int    getRainInterval(int i);
static int    getMaxPeriods(int i, int k);
static void   initGageData(void);
static void   initUnitHydData(void);
static int    openNewRdiiFile(void);
static void   getRainfall(DateTime currentDate);

static double applyIA(int j, int k, DateTime aDate, double dt,
              double rainDepth);
static void   updateDryPeriod(int j, int k, double rain, int gageInterval);
static void   getUnitHydRdii(DateTime currentDate);
static double getUnitHydConvol(int j, int k, int gageInterval);
static double getUnitHydOrd(int j, int m, int k, double t);

static int    getNodeRdii(void);
static void   saveRdiiFlows(DateTime currentDate);
static void   closeRdiiProcessor(void);
static void   freeRdiiMemory(void);

// --- functions used to read an existing RDII file
static int   readRdiiFileHeader(void);
static void  readRdiiFlows(void);

static void  openRdiiTextFile(void);
static int   readRdiiTextFileHeader(void);
static void  readRdiiTextFlows(void);

//=============================================================================
//                   Management of RDII-Related Data
//=============================================================================

int rdii_readRdiiInflow(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads properties of an RDII inflow from a line of input.
//
{
    int    j, k;
    double a;
    TRdiiInflow* inflow;

    // --- check for proper number of items
    if ( ntoks < 3 ) return error_setInpError(ERR_ITEMS, "");

    // --- check that node receiving RDII exists
    j = project_findObject(NODE, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);

    // --- check that RDII unit hydrograph exists
    k = project_findObject(UNITHYD, tok[1]);
    if ( k < 0 ) return error_setInpError(ERR_NAME, tok[1]);

    // --- read in sewer area value
    if ( !getDouble(tok[2], &a) || a < 0.0 )
        return error_setInpError(ERR_NUMBER, tok[2]);

    // --- create the RDII inflow object if it doesn't already exist
    inflow = Node[j].rdiiInflow;
    if ( inflow == NULL )
    {
        inflow = (TRdiiInflow *) malloc(sizeof(TRdiiInflow));
        if ( !inflow ) return error_setInpError(ERR_MEMORY, "");
    }

    // --- assign UH & area to inflow object
    inflow->unitHyd = k;
    inflow->area = a / UCF(LANDAREA);

    // --- assign inflow object to node
    Node[j].rdiiInflow = inflow;
    return 0;
}

//=============================================================================

void rdii_initUnitHyd(int j)
//
//  Input:   j = UH group index
//  Output:  none
//  Purpose: initializes properties of a unit hydrograph group.
//
{
    int i;                             // individual UH index
    int m;                             // month index

    for ( m=0; m<12; m++)
    {
        for (i=0; i<3; i++)
        {
            UnitHyd[j].iaMax[m][i]   = 0.0;
            UnitHyd[j].iaRecov[m][i] = 0.0;
            UnitHyd[j].iaInit[m][i]  = 0.0;
            UnitHyd[j].r[m][i]       = 0.0;
            UnitHyd[j].tPeak[m][i]   = 0;
            UnitHyd[j].tBase[m][i]   = 0;
        }
    }
}

//=============================================================================

int rdii_readUnitHydParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads parameters of an RDII unit hydrograph from a line of input.
//
{
    int i, j, k, m, g;
    double x[6];

    // --- check that RDII UH object exists in database
    j = project_findObject(UNITHYD, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);

    // --- assign UH ID to name in hash table
    if ( UnitHyd[j].ID == NULL )
        UnitHyd[j].ID = project_findID(UNITHYD, tok[0]);

    // --- line has 2 tokens; assign rain gage to UH object
    if ( ntoks == 2 )
    {
        g = project_findObject(GAGE, tok[1]);
        if ( g < 0 ) return error_setInpError(ERR_NAME, tok[1]);
        UnitHyd[j].rainGage = g;
        return 0;
    }
    else if ( ntoks < 6 ) return error_setInpError(ERR_ITEMS, "");

    // --- find which month UH params apply to
    m = datetime_findMonth(tok[1]);
    if ( m == 0 )
    {
        if ( !match(tok[1], w_ALL) )
            return error_setInpError(ERR_KEYWORD, tok[1]);
    }

    // --- find type of UH being specified
    k = findmatch(tok[2], UHTypeWords);

    // --- if no type match, try using older UH line format
    if ( k < 0 ) return readOldUHFormat(j, m, tok, ntoks);

    // --- read the R-T-K parameters
    for ( i = 0; i < 3; i++ )
    {
        if ( ! getDouble(tok[i+3], &x[i]) )
            return error_setInpError(ERR_NUMBER, tok[i+3]);
    }

    // --- read the IA parameters if present
    for (i = 3; i < 6; i++)
    {
        x[i] = 0.0;
        if ( ntoks > i+3 )
        {
            if ( ! getDouble(tok[i+3], &x[i]) )
                return error_setInpError(ERR_NUMBER, tok[i+2]);
        }
    }

    // --- save UH params
    setUnitHydParams(j, k, m, x);
    return 0;
}

//=============================================================================

int readOldUHFormat(int j, int m, char* tok[], int ntoks)
//
//  Input:   j = unit hydrograph index
//           m = month of year (0 = all months)
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads parameters of a set of RDII unit hydrographs from a line of
//           input.
//
{
    int    i, k;
    double p[9], x[6];

    // --- check for proper number of tokens
    if ( ntoks < 11 ) return error_setInpError(ERR_ITEMS, "");

    // --- read 3 sets of r-t-k values
    for ( i = 0; i < 9; i++ )
    {
        if ( ! getDouble(tok[i+2], &p[i]) )
            return error_setInpError(ERR_NUMBER, tok[i+2]);
    }

    // --- read initial abstraction parameters
    for (i = 0; i < 3; i++)
    {
        x[i+3] = 0.0;
        if ( ntoks > i+11 )
        {
            if ( ! getDouble(tok[i+11], &x[i+3]) )
                return error_setInpError(ERR_NUMBER, tok[i+11]);
        }
    }

    // --- save UH parameters
    for ( k = 0; k < 3; k++)
    {
        for ( i = 0; i < 3; i++)
        {
            x[i] = p[3*k + i];
            setUnitHydParams(j, k, m, x);
        }
    }
    return 0;
}

//=============================================================================

void setUnitHydParams(int j, int i, int m, double x[])
//
//  Input:   j = unit hydrograph index
//           i = type of UH response (short, medium or long term)
//           m = month of year (0 = all months)
//           x = array of UH parameters
//  Output:  none
//  Purpose: assigns parameters to a unit hydrograph for a specified month of year.
//
{
    int    m1, m2;                     // start/end month indexes
    double t,                          // UH time to peak (hrs)
           k,                          // UH k-value
           tBase;                      // UH base time (hrs)

    // --- find range of months that share same parameter values
    if ( m == 0 )
    {
        m1 = 0;
        m2 = 11;
    }
    else
    {
        m1 = m-1;
        m2 = m1;
    }

    // --- for each month in the range
    for (m=m1; m<=m2; m++)
    {
        // --- set UH response ratio, time to peak, & base time
        UnitHyd[j].r[m][i] = x[0];
        t = x[1];
        k = x[2];
        tBase = t * (1.0 + k);                              // hours
        UnitHyd[j].tPeak[m][i] = (long)(t * 3600.);         // seconds
        UnitHyd[j].tBase[m][i] = (long)(tBase * 3600.);     // seconds

        // -- set initial abstraction parameters
        UnitHyd[j].iaMax[m][i]   = x[3];
        UnitHyd[j].iaRecov[m][i] = x[4];
        UnitHyd[j].iaInit[m][i]  = x[5];
    }
}

//=============================================================================

void rdii_deleteRdiiInflow(int j)
//
//  Input:   j = node index
//  Output:  none
//  Purpose: deletes the RDII inflow object for a node.
//
{
    if ( Node[j].rdiiInflow )
    {
        free(Node[j].rdiiInflow);
        Node[j].rdiiInflow = NULL;
    }
}


//=============================================================================
//                 Reading Inflow Data From a RDII File
//=============================================================================

void rdii_openRdii()
//
//  Input:   none
//  Output:  none
//  Purpose: opens an exisiting RDII interface file or creates a new one.
//
{
    char  fStamp[] = FILE_STAMP;

    RdiiNodeIndex = NULL;
    RdiiNodeFlow = NULL;
    NumRdiiNodes = 0;
    RdiiStartDate = NO_DATE;

    // --- create the RDII file if existing file not being used
    if ( IgnoreRDII ) return;
    if ( Frdii.mode != USE_FILE ) createRdiiFile();
    if ( Frdii.mode == NO_FILE || ErrorCode ) return;

    // --- try to open the RDII file in binary mode
    Frdii.file = fopen(Frdii.name, "rb");
    if ( Frdii.file == NULL)
    {
        if ( Frdii.mode == SCRATCH_FILE )
        {
            report_writeErrorMsg(ERR_RDII_FILE_SCRATCH, "");
        }
        else
        {
            report_writeErrorMsg(ERR_RDII_FILE_OPEN, Frdii.name);
        }
        return;
    }

    // --- check for valid file stamp
    fread(fStamp, sizeof(char), strlen(FileStamp), Frdii.file);
    if ( strcmp(fStamp, FileStamp) == 0 )
    {
        RdiiFileType = BINARY;
        ErrorCode = readRdiiFileHeader();
    }

    // --- if stamp invalid try to open the file in text mode
    else
    {
        fclose(Frdii.file);
        RdiiFileType = TEXT;
        openRdiiTextFile();
    }

    // --- catch any error
    if ( ErrorCode )
    {
        report_writeErrorMsg(ErrorCode, Frdii.name);
    }

    // --- read the first set of RDII flows form the file
    else readRdiiFlows();
}

//=============================================================================

void openRdiiTextFile()
{
    // --- try to open the RDII file in text mode
    Frdii.file = fopen(Frdii.name, "rt");
    if ( Frdii.file == NULL)
    {
        if ( Frdii.mode == SCRATCH_FILE )
        {
            report_writeErrorMsg(ERR_RDII_FILE_SCRATCH, "");
        }
        else
        {
            report_writeErrorMsg(ERR_RDII_FILE_OPEN, Frdii.name);
        }
        return;
    }

    // --- read header records from file
    ErrorCode = readRdiiTextFileHeader();
    if ( ErrorCode )
    {
        report_writeErrorMsg(ErrorCode, Frdii.name);
    }
}

//=============================================================================

void rdii_closeRdii()
//
//  Input:   none
//  Output:  none
//  Purpose: closes the RDII interface file.
//
{
    if ( Frdii.file ) fclose(Frdii.file);
    if ( Frdii.mode == SCRATCH_FILE ) remove(Frdii.name);
    FREE(RdiiNodeIndex);
    FREE(RdiiNodeFlow);
}

//=============================================================================

int rdii_getNumRdiiFlows(DateTime aDate)
//
//  Input:   aDate = current date/time
//  Output:  returns 0 if no RDII flow or number of nodes with RDII inflows
//  Purpose: finds number of RDII inflows at a specified date.
//
{
    // --- default result is 0 indicating no RDII inflow at specified date
    if ( NumRdiiNodes == 0 ) return 0;
    if ( !Frdii.file ) return 0;

    // --- keep reading RDII file as need be
    while ( !feof(Frdii.file) )
    {
        // --- return if date of current RDII inflow not reached yet
        if ( RdiiStartDate == NO_DATE ) return 0;
        if ( aDate < RdiiStartDate ) return 0;

        // --- return RDII node count if specified date falls
        //     within time interval of current RDII inflow
        if ( aDate < RdiiEndDate ) return NumRdiiNodes;

        // --- otherwise get next date and RDII flow values from file
        else readRdiiFlows();
    }
    return 0;
}

//=============================================================================

void rdii_getRdiiFlow(int i, int* j, double* q)
//
//  Input:   i = RDII node index
//           j = pointer to project node index
//           q = pointer to RDII flow rate
//  Output:  sets node index and RDII inflow for node
//  Purpose: finds index and current RDII inflow for an RDII node.
//
{
    if ( i >= 0 && i < NumRdiiNodes )
    {
        *j = RdiiNodeIndex[i];
        *q = RdiiNodeFlow[i];
    }
}

//=============================================================================

int readRdiiFileHeader()
//
//  Input:   none
//  Output:  returns error code
//  Purpose: reads header information from a binary RDII file.
//
{
    int i, j;

    // --- extract time step and number of RDII nodes
    fread(&RdiiStep, sizeof(INT4), 1, Frdii.file);
    if ( RdiiStep <= 0 ) return ERR_RDII_FILE_FORMAT;
    fread(&NumRdiiNodes, sizeof(INT4), 1, Frdii.file);
    if ( NumRdiiNodes <= 0 ) return ERR_RDII_FILE_FORMAT;

    // --- allocate memory for RdiiNodeIndex & RdiiNodeFlow arrays
    RdiiNodeIndex = (int *) calloc(NumRdiiNodes, sizeof(int));
    if ( !RdiiNodeIndex ) return ERR_MEMORY;
    RdiiNodeFlow = (REAL4 *) calloc(NumRdiiNodes, sizeof(REAL4));
    if ( !RdiiNodeFlow ) return ERR_MEMORY;

    // --- read indexes of RDII nodes
    if ( feof(Frdii.file) ) return ERR_RDII_FILE_FORMAT;
    fread(RdiiNodeIndex, sizeof(INT4), NumRdiiNodes, Frdii.file);
    for ( i=0; i<NumRdiiNodes; i++ )
    {
        j = RdiiNodeIndex[i];
        if ( Node[j].rdiiInflow == NULL ) return ERR_RDII_FILE_FORMAT;
    }
    if ( feof(Frdii.file) ) return ERR_RDII_FILE_FORMAT;
    return 0;
}

//=============================================================================

int readRdiiTextFileHeader()
//
//  Input:   none
//  Output:  returns error code
//  Purpose: reads header information from a text RDII file.
//
{
    int   i;
    char  line[MAXLINE+1];             // line from RDII data file
    char  s1[MAXLINE+1];               // general string variable
    char  s2[MAXLINE+1];

    // --- check for correct file type
    fgets(line, MAXLINE, Frdii.file);
    sscanf(line, "%s", s1);
    if ( strcmp(s1, "SWMM5") != 0 ) return ERR_RDII_FILE_FORMAT;

    // --- skip title line
    fgets(line, MAXLINE, Frdii.file);

    // --- read RDII UH time step interval (sec)
    RdiiStep = 0;
    fgets(line, MAXLINE, Frdii.file);
    sscanf(line, "%d", &RdiiStep);
    if ( RdiiStep <= 0 ) return ERR_RDII_FILE_FORMAT;

    // --- skip over line with number of constituents (= 1 for RDII)
    fgets(line, MAXLINE, Frdii.file);

    // --- read flow units
    fgets(line, MAXLINE, Frdii.file);
    sscanf(line, "%s %s", s1, s2);
    RdiiFlowUnits = findmatch(s2, FlowUnitWords);
    if ( RdiiFlowUnits < 0 ) return ERR_RDII_FILE_FORMAT;

    // --- read number of RDII nodes
    fgets(line, MAXLINE, Frdii.file);
    if ( sscanf(line, "%d", &NumRdiiNodes) < 1 ) return ERR_RDII_FILE_FORMAT;

    // --- allocate memory for RdiiNodeIndex & RdiiNodeFlow arrays
    RdiiNodeIndex = (int *) calloc(NumRdiiNodes, sizeof(int));
    if ( !RdiiNodeIndex ) return ERR_MEMORY;
    RdiiNodeFlow = (REAL4 *) calloc(NumRdiiNodes, sizeof(REAL4));
    if ( !RdiiNodeFlow ) return ERR_MEMORY;

    // --- read names of RDII nodes from file & save their indexes
    for ( i=0; i<NumRdiiNodes; i++ )
    {
        if ( feof(Frdii.file) ) return ERR_RDII_FILE_FORMAT;
        fgets(line, MAXLINE, Frdii.file);
        sscanf(line, "%s", s1);
        RdiiNodeIndex[i] = project_findObject(NODE, s1);
    }

    // --- skip column heading line
    if ( feof(Frdii.file) ) return ERR_RDII_FILE_FORMAT;
    fgets(line, MAXLINE, Frdii.file);
    return 0;
}

//=============================================================================

void readRdiiFlows()
//
//  Input:   none
//  Output:  none
//  Purpose: reads date and flow values of next RDII inflows from RDII file.
//
{
    if ( RdiiFileType == TEXT ) readRdiiTextFlows();
    else
    {
        RdiiStartDate = NO_DATE;
        RdiiEndDate = NO_DATE;
        if ( feof(Frdii.file) ) return;
        fread(&RdiiStartDate, sizeof(DateTime), 1, Frdii.file);
        if ( RdiiStartDate == NO_DATE ) return;
        if ( fread(RdiiNodeFlow, sizeof(REAL4), NumRdiiNodes, Frdii.file)
            < (size_t)NumRdiiNodes ) RdiiStartDate = NO_DATE;
        else RdiiEndDate = datetime_addSeconds(RdiiStartDate, RdiiStep);
    }
}

//=============================================================================

void readRdiiTextFlows()
//
//  Input:   none
//  Output:  none
//  Purpose: reads date and flow values of next RDII inflows from RDII file.
//
{
    int    i, n;
    int    yr = 0, mon = 0, day = 0,
		   hr = 0, min = 0, sec = 0;   // year, month, day, hour, minute, second
    double x;                          // RDII flow in original units
    char   line[MAXLINE+1];            // line from RDII data file
    char   s[MAXLINE+1];               // node ID label (not used)

    RdiiStartDate = NO_DATE;
    for (i=0; i<NumRdiiNodes; i++)
    {
        if ( feof(Frdii.file) ) return;
        fgets(line, MAXLINE, Frdii.file);
        n = sscanf(line, "%s %d %d %d %d %d %d %lf",
            s, &yr, &mon, &day, &hr, &min, &sec, &x);
        if ( n < 8 ) return;
        RdiiNodeFlow[i] = (REAL4)(x / Qcf[RdiiFlowUnits]);
    }
    RdiiStartDate = datetime_encodeDate(yr, mon, day) +
                    datetime_encodeTime(hr, min, sec);
    RdiiEndDate = datetime_addSeconds(RdiiStartDate, RdiiStep);
}


//=============================================================================
//                   Creation of a RDII Interface File
//=============================================================================

void createRdiiFile()
//
//  Input:   none
//  Output:  none
//  Purpose: computes time history of RDII inflows and saves them to file.
//
{
    int      hasRdii;                  // true when total RDII > 0
    double   elapsedTime;              // current elapsed time (sec)
    double   duration;                 // duration being analyzed (sec)
    DateTime currentDate;              // current calendar date/time

    // --- set RDII reporting time step to Runoff wet step
    RdiiStep = WetStep;

    // --- count nodes with RDII data
    NumRdiiNodes = getNumRdiiNodes();

    // --- if no RDII nodes then re-set RDII file usage to NO_FILE
    if ( NumRdiiNodes == 0 )
    {
        Frdii.mode = NO_FILE;
        return;
    }

    // --- otherwise set file usage to SCRATCH if originally set to NO_FILE
    else if ( Frdii.mode == NO_FILE ) Frdii.mode = SCRATCH_FILE;

    // --- validate RDII data
    validateRdii();
    initGageData();
    if ( ErrorCode ) return;

    // --- open RDII processing system
    openRdiiProcessor();
    if ( !ErrorCode )
    {
        // --- initialize rain gage & UH processing data
        initUnitHydData();

        // --- convert total simulation duration from millisec to sec
        duration = TotalDuration / 1000.0;

        // --- examine rainfall record over each RdiiStep time step
        elapsedTime = 0.0;
        while ( elapsedTime <= duration && !ErrorCode )
        {
            // --- compute current calendar date/time
            currentDate = StartDateTime + elapsedTime / SECperDAY;

            // --- update rainfall at all rain gages
            getRainfall(currentDate);

            // --- compute convolutions of past rainfall with UH's
            getUnitHydRdii(currentDate);

            // --- find RDII at all nodes
            hasRdii = getNodeRdii();

            // --- save RDII at all nodes to file for current date
            if ( hasRdii ) saveRdiiFlows(currentDate);

            // --- advance one time step
            elapsedTime += RdiiStep;
        }
    }

    // --- close RDII processing system
    closeRdiiProcessor();
}

//=============================================================================

int  getNumRdiiNodes()
//
//  Input:   none
//  Output:  returns node count
//  Purpose: counts number of nodes that receive RDII inflow.
//
{
    int j,                             // node index
        n;                             // node count

    n = 0;
    for (j=0; j<Nobjects[NODE]; j++)
    {
        if ( Node[j].rdiiInflow ) n++;
    }
    return n;
}

//=============================================================================

void validateRdii()
//
//  Input:   none
//  Output:  none
//  Purpose: validates UH and RDII inflow object data.
//
{
    int    i,                          // node index
           j,                          // UH group index
           k,                          // individual UH index
           m;                          // month index
    double rsum;                       // sum of UH r-values
//  long   gageInterval;               // rain gage time interval

    // --- check each unit hydrograph for consistency
    for (j=0; j<Nobjects[UNITHYD]; j++)
    {
        for (m=0; m<12; m++)
        {
            rsum = 0.0;
            for (k=0; k<3; k++)
            {
                // --- if no base time then UH doesn't exist
                if ( UnitHyd[j].tBase[m][k] == 0 ) continue;

                // --- restriction on time to peak being less than the
                //     rain gage's recording interval no longer applies

                // --- can't have negative UH parameters
                if ( UnitHyd[j].tPeak[m][k] < 0.0 )
                {
                    report_writeErrorMsg(ERR_UNITHYD_TIMES, UnitHyd[j].ID);
                }

                // --- can't have negative UH response ratio
                if ( UnitHyd[j].r[m][k] < 0.0 )
                {
                    report_writeErrorMsg(ERR_UNITHYD_RATIOS, UnitHyd[j].ID);
                }
                else rsum += UnitHyd[j].r[m][k];
            }
            if ( rsum > 1.01 )
            {
                report_writeErrorMsg(ERR_UNITHYD_RATIOS, UnitHyd[j].ID);
            }
        }
    }

    // --- check each node's RDII inflow object
    for (i=0; i<Nobjects[NODE]; i++)
    {
        if ( Node[i].rdiiInflow )
        {
            // --- check that sewer area is non-negative
            if ( Node[i].rdiiInflow->area < 0.0 )
            {
                report_writeErrorMsg(ERR_RDII_AREA, Node[i].ID);
            }
        }
    }
}

//=============================================================================

void openRdiiProcessor()
//
//  Input:   none
//  Output:  none
//  Purpose: opens RDII processing system.
//
{
    int j;                             // object index
    int n;                             // RDII node count

    // --- set RDII processing arrays to NULL
    UHGroup = NULL;
    RdiiNodeIndex = NULL;
    RdiiNodeFlow = NULL;
    TotalRainVol = 0.0;
    TotalRdiiVol = 0.0;

    // --- allocate memory used for RDII processing
    if ( !allocRdiiMemory() )
    {
        report_writeErrorMsg(ERR_MEMORY, "");
        return;
    }

    // --- open & initialize RDII file
    if ( !openNewRdiiFile() )
    {
        report_writeErrorMsg(ERR_RDII_FILE_SCRATCH, "");
        return;
    }

    // --- identify index of each node with RDII inflow
    n = 0;
    for (j=0; j<Nobjects[NODE]; j++)
    {
        if ( Node[j].rdiiInflow )
        {
            RdiiNodeIndex[n] = j;
            n++;
        }
    }
}

//=============================================================================

int  allocRdiiMemory()
//
//  Input:   none
//  Output:  returns TRUE if successful, FALSE if not
//  Purpose: allocates memory used for RDII processing .
//
//
{
    int i;                             // UH group index
    int k;                             // UH index
    int n;                             // number of past rain periods

    // --- allocate memory for RDII processing data for UH groups
    UHGroup = (TUHGroup *) calloc(Nobjects[UNITHYD], sizeof(TUHGroup));
    if ( !UHGroup ) return FALSE;

    // --- allocate memory for past rainfall data for each UH in each group
    for (i=0; i<Nobjects[UNITHYD]; i++)
    {
        UHGroup[i].rainInterval = getRainInterval(i);
        for (k=0; k<3; k++)
        {
            UHGroup[i].uh[k].pastRain = NULL;
            UHGroup[i].uh[k].pastMonth = NULL;
            UHGroup[i].uh[k].maxPeriods = getMaxPeriods(i, k);
            n = UHGroup[i].uh[k].maxPeriods;
            if ( n > 0 )
            {
                UHGroup[i].uh[k].pastRain =
                    (double *) calloc(n, sizeof(double));
                if ( !UHGroup[i].uh[k].pastRain ) return FALSE;
                UHGroup[i].uh[k].pastMonth =
                    (char *) calloc(n, sizeof(char));
                if ( !UHGroup[i].uh[k].pastMonth ) return FALSE;
            }
        }
    }

    // --- allocate memory for RDII indexes & inflow at each node w/ RDII data
    RdiiNodeIndex = (int *) calloc(NumRdiiNodes, sizeof(int));
    if ( !RdiiNodeIndex ) return FALSE;
    RdiiNodeFlow = (REAL4 *) calloc(NumRdiiNodes, sizeof(REAL4));
    if ( !RdiiNodeFlow ) return FALSE;
    return TRUE;
}

//=============================================================================

int  getRainInterval(int i)
//
//  Input:   i = UH group index
//  Output:  returns a time interval (sec)
//  Purpose: finds rainfall processing time interval for a unit hydrograph group.
//
{
    int ri;        // rainfal processing time interval for the UH group
    int tLimb;     // duration of a UH's rising & falling limbs
    int k, m;

    // --- begin with UH group time step equal to wet runoff step
    ri = WetStep;

    // --- examine each UH in the group
    for (m=0; m<12; m++)
    {
        for (k=0; k<3; k++)
        {
            // --- make sure the UH exists
            if ( UnitHyd[i].tPeak[m][k] > 0 )
            {
                // --- reduce time step if rising/falling limb is smaller
                tLimb = UnitHyd[i].tPeak[m][k];
                ri = MIN(ri, tLimb);
                tLimb = UnitHyd[i].tBase[m][k] - tLimb;
                if ( tLimb > 0 ) ri = MIN(ri, tLimb);
            }
        }
    }
    return ri;
}

//=============================================================================

int  getMaxPeriods(int i, int k)
//
//  Input:   i = UH group index
//           k = UH index
//  Output:  returns number of past rainfall values
//  Purpose: finds number of past rainfall values to save for a UH.
//
{
    int   m,                           // month index
          n,                           // number of time periods
          nMax,                        // maximum number of time periods
          rainInterval;                // rainfall processing interval (sec)

    // --- examine each monthly set of UHs
    rainInterval = UHGroup[i].rainInterval;
    nMax = 0;
    for (m=0; m<12; m++)
    {
        // --- compute number of time periods in UH base
        n = (UnitHyd[i].tBase[m][k] / rainInterval) + 1;

        // --- update number of time periods to be saved
        nMax = MAX(n, nMax);
    }
    return nMax;
}

//=============================================================================

void initGageData()
//
//  Input:   none
//  Output:  none
//  Purpose: initializes state of Unit Hydrograph rain gages.
//
{
    int i;                             // unit hyd. index
    int g;                             // rain gage index

    // --- first initialize the state of each rain gage
    for (g=0; g<Nobjects[GAGE]; g++)
    {
        if ( Gage[g].tSeries >= 0 )
        {
            table_tseriesInit(&Tseries[Gage[g].tSeries]);
        }
        gage_initState(g);
    }

    // --- then flag each gage that is used by a Unit Hydrograph set
    for (i=0; i<Nobjects[UNITHYD]; i++)
    {
        g = UnitHyd[i].rainGage;
        if ( g >= 0 )
        {
            Gage[g].isUsed = TRUE;

            // --- if UH's gage uses same time series as a previous gage,
            //     then assign the latter gage to the UH
            if ( Gage[g].coGage >= 0 )
            {
                UnitHyd[i].rainGage = Gage[g].coGage;
                Gage[Gage[g].coGage].isUsed = TRUE;
            }
        }
    }
}

//=============================================================================

void initUnitHydData()
//
//  Input:   none
//  Output:  none
//  Purpose: initializes unit hydrograph processing data.
//
{
    int i,                             // UH group index
        j,                             // node index
        k,                             // UH index
        n;                             // RDII node index
//  int g,                             // rain gage index
    int month;                         // month index

    // --- initialize UHGroup entries for each Unit Hydrograph
    month = datetime_monthOfYear(StartDateTime) - 1;
    for (i=0; i<Nobjects[UNITHYD]; i++)
    {
        for (k=0; k<3; k++)
        {
            // --- make the first recorded rainfall begin a new RDII event
            // --- (new RDII event occurs when dry period > base of longest UH)
            UHGroup[i].uh[k].drySeconds =
                (UHGroup[i].uh[k].maxPeriods * UHGroup[i].rainInterval) + 1;
            UHGroup[i].uh[k].period = UHGroup[i].uh[k].maxPeriods + 1;
            UHGroup[i].uh[k].hasPastRain = FALSE;

            // --- assign initial abstraction used
            UHGroup[i].uh[k].iaUsed = UnitHyd[i].iaInit[month][k];
        }

        // --- initialize gage date to simulation start date
        UHGroup[i].gageDate = StartDateTime;
        UHGroup[i].area = 0.0;
        UHGroup[i].rdii = 0.0;
    }

    // --- assume each UH group is not used
    for (i=0; i<Nobjects[UNITHYD]; i++) UHGroup[i].isUsed = FALSE;

    // --- look at each node with RDII inflow
    for (n=0; n<NumRdiiNodes; n++)
    {
        // --- mark as used the UH group associated with the node
        j = RdiiNodeIndex[n];
        i = Node[j].rdiiInflow->unitHyd;
        UHGroup[i].isUsed = TRUE;

        // --- add node's sewer area to UH group's area
        UHGroup[i].lastDate = StartDateTime;
        UHGroup[i].area += Node[j].rdiiInflow->area;
    }
}

//=============================================================================

int openNewRdiiFile()
//
//  Input:   none
//  Output:  returns TRUE if successful, FALSE if not
//  Purpose: opens a new RDII interface file.
//
{
    int j;                             // node index

    // --- create a temporary file name if scratch file being used
    if ( Frdii.mode == SCRATCH_FILE ) getTempFileName(Frdii.name);

    // --- open the RDII file as a formatted text file
    Frdii.file = fopen(Frdii.name, "w+b");
    if ( Frdii.file == NULL )
    {
        return FALSE;
    }

    // --- write file stamp to RDII file
    fwrite(FileStamp, sizeof(char), strlen(FileStamp), Frdii.file);

    // --- initialize the contents of the file with RDII time step (sec),
    //     number of RDII nodes, and index of each node
    fwrite(&RdiiStep, sizeof(INT4), 1, Frdii.file);
    fwrite(&NumRdiiNodes, sizeof(INT4), 1, Frdii.file);
    for (j=0; j<Nobjects[NODE]; j++)
    {
        if ( Node[j].rdiiInflow ) fwrite(&j, sizeof(INT4), 1, Frdii.file);
    }
    return TRUE;
}

//=============================================================================

void getRainfall(DateTime currentDate)
//
//  Input:   currentDate = current calendar date/time
//  Output:  none
//  Purpose: determines rainfall at current RDII processing date.
//
//
{
    int      j;                        // UH group index
    int      k;                        // UH index
    int      g;                        // rain gage index
    int      i;                        // past rainfall index
    int      month;                    // month of current date
    int      rainInterval;             // rainfall interval (sec)
    double   rainDepth;                // rainfall depth (inches or mm)
    double   excessDepth;              // excess rainfall depth (inches or mm))
    DateTime gageDate;                 // calendar date for rain gage

    // --- examine each UH group
    month = datetime_monthOfYear(currentDate) - 1;
    for (g = 0; g < Nobjects[GAGE]; g++) Gage[g].isCurrent = FALSE;
    for (j = 0; j < Nobjects[UNITHYD]; j++)
    {
        // --- repeat until gage's date reaches or exceeds current date
        g = UnitHyd[j].rainGage;
        rainInterval = UHGroup[j].rainInterval;
        while ( UHGroup[j].gageDate < currentDate )
        {
            // --- get rainfall volume over gage's recording interval
            //     at gage'a current date (in original depth units)
            gageDate = UHGroup[j].gageDate;
            Adjust.rainFactor = Adjust.rain[datetime_monthOfYear(gageDate)-1];
            if (!Gage[g].isCurrent)
            {
                gage_setState(g, gageDate);
                Gage[g].isCurrent = TRUE;
            }
            rainDepth = Gage[g].rainfall * (double)rainInterval / 3600.0;

            // --- update amount of total rainfall volume (ft3)
            TotalRainVol += rainDepth / UCF(RAINDEPTH) * UHGroup[j].area;

            // --- compute rainfall excess for each UH in the group
            for (k=0; k<3; k++)
            {
                // --- adjust rainfall volume for any initial abstraction
                excessDepth = applyIA(j, k, gageDate, rainInterval, rainDepth);

                // --- adjust extent of dry period for the UH
                updateDryPeriod(j, k, excessDepth, rainInterval);

                // --- add rainfall to list of past values,
                //     wrapping array index if necessary
                i = UHGroup[j].uh[k].period;
                if ( i >= UHGroup[j].uh[k].maxPeriods ) i = 0;
                UHGroup[j].uh[k].pastRain[i] = excessDepth;
                UHGroup[j].uh[k].pastMonth[i] = (char)month;
                UHGroup[j].uh[k].period = i + 1;
            }

            // --- advance rain date by gage recording interval
            UHGroup[j].gageDate = datetime_addSeconds(gageDate, rainInterval);
        }
    }
}

//=============================================================================

double  applyIA(int j, int k, DateTime aDate, double dt, double rainDepth)
//
//  Input:   j = UH group index
//           k = unit hydrograph index
//           aDate = current date/time
//           dt = time interval (sec)
//           rainDepth = unadjusted rain depth (in or mm)
//  Output:  returns rainfall adjusted for initial abstraction (IA)
//  Purpose: adjusts rainfall for any initial abstraction and updates the
//           amount of available initial abstraction actually used.
//
{
    int m;
    double ia, netRainDepth;

    // --- determine amount of unused IA
    m = datetime_monthOfYear(aDate) - 1;
    ia = UnitHyd[j].iaMax[m][k] - UHGroup[j].uh[k].iaUsed;
    ia = MAX(ia, 0.0);

    // --- case where there's some rainfall
    if ( rainDepth > 0.0 )
    {
        // --- reduce rain depth by unused IA
        netRainDepth = rainDepth - ia;
        netRainDepth = MAX(netRainDepth, 0.0);

        // --- update amount of IA used up
        ia = rainDepth - netRainDepth;
        UHGroup[j].uh[k].iaUsed += ia;
    }

    // --- case where there's no rainfall
    else
    {
        // --- recover a portion of the IA already used
        UHGroup[j].uh[k].iaUsed -= dt / 86400. * UnitHyd[j].iaRecov[m][k];
        UHGroup[j].uh[k].iaUsed = MAX(UHGroup[j].uh[k].iaUsed, 0.0);
        netRainDepth = 0.0;
    }
    return netRainDepth;
}

//=============================================================================

void updateDryPeriod(int j, int k, double rainDepth, int rainInterval)
//
//  Input:   j = UH group index
//           k = unit hydrograph index
//           rainDepth = excess rain depth (in or mm)
//           rainInterval = rainfall time interval (sec)
//  Output:  none
//  Purpose: adjusts the length of the dry period between rainfall events.
//
{
    int i;

    // --- if rainfall occurs
    if ( rainDepth > 0.0 )
    {
        // --- if previous dry period long enough then begin
        //     new RDII event with time period index set to 0
        if ( UHGroup[j].uh[k].drySeconds >= rainInterval *
            UHGroup[j].uh[k].maxPeriods )
        {
            for (i=0; i<UHGroup[j].uh[k].maxPeriods; i++)
            {
                UHGroup[j].uh[k].pastRain[i] = 0.0;
            }
            UHGroup[j].uh[k].period = 0;
        }
        UHGroup[j].uh[k].drySeconds = 0;
        UHGroup[j].uh[k].hasPastRain = TRUE;
    }

    // --- if no rainfall, update duration of dry period
    else
    {
        UHGroup[j].uh[k].drySeconds += rainInterval;
        if ( UHGroup[j].uh[k].drySeconds >=
            rainInterval * UHGroup[j].uh[k].maxPeriods )
        {
            UHGroup[j].uh[k].hasPastRain = FALSE;
        }
        else UHGroup[j].uh[k].hasPastRain = TRUE;
    }
}

//=============================================================================

void getUnitHydRdii(DateTime currentDate)
//
//  Input:   currentDate = current calendar date/time
//  Output:  none
//  Purpose: computes RDII generated by past rainfall for each UH group.
//
{
    int   j;                           // UH group index
    int   k;                           // UH index
    int   rainInterval;                // rainfall time interval (sec)

    // --- examine each UH group
    for (j=0; j<Nobjects[UNITHYD]; j++)
    {
        // --- skip calculation if group not used by any RDII node or if
        //     current date hasn't reached last date RDII was computed
        if ( !UHGroup[j].isUsed ) continue;
        if ( currentDate < UHGroup[j].lastDate ) continue;

        // --- update date RDII last computed
        UHGroup[j].lastDate = UHGroup[j].gageDate;

        // --- perform convolution for each UH in the group
        rainInterval = UHGroup[j].rainInterval;
        UHGroup[j].rdii = 0.0;
        for (k=0; k<3; k++)
        {
            if ( UHGroup[j].uh[k].hasPastRain )
            {
                UHGroup[j].rdii += getUnitHydConvol(j, k, rainInterval);
            }
        }
    }
}

//=============================================================================

double getUnitHydConvol(int j, int k, int rainInterval)
//
//  Input:   j = UH group index
//           k = UH index
//           rainInterval = rainfall time interval (sec)
//  Output:  returns a RDII flow value
//  Purpose: computes convolution of Unit Hydrographs with past rainfall.
//
{
    int    i;                          // previous rainfall period index
    int    m;                          // month of year index
    int    p;                          // UH time period index
    int    pMax;                       // max. number of periods
    double t;                          // UH time value (sec)
    double u;                          // UH ordinate
    double v;                          // rainfall volume
    double rdii;                       // RDII flow
    TUHData* uh;                       // UH data

    // --- initialize RDII, rain period index and UH period index
    rdii = 0.0;
    uh = &UHGroup[j].uh[k];
    i = uh->period - 1;
    if ( i < 0 ) i = uh->maxPeriods - 1;
    pMax = uh->maxPeriods;
    p = 1;

    // --- evaluate each time period of UH's
    while ( p < pMax )
    {
        // --- if rain period has rainfall
        v = uh->pastRain[i];
        m = uh->pastMonth[i];
        if ( v > 0.0 )
        {
            // --- find mid-point time of UH period in seconds
            t = ((double)(p) - 0.5) * (double)rainInterval;

            // --- convolute rain volume with UH ordinate
            u = getUnitHydOrd(j, m, k, t) * UnitHyd[j].r[m][k];
            rdii += u * v;
        }

        // --- move to next UH period & previous rainfall period
        p = p + 1;
        i = i - 1;
        if ( i < 0 ) i = uh->maxPeriods - 1;
    }
    return rdii;
}

//=============================================================================

double getUnitHydOrd(int h, int m, int k, double t)
//
//  Input:   h = index of UH group
//           m = month index
//           k = individual UH index
//           t = UH time (sec)
//  Output:  returns ordinate of a unit hydrograph
//  Purpose: gets ordinate of a particular unit hydrograph at specified time.
//
{
    double qPeak;                      // peak flow of unit hydrograph
    double f;                          // fraction of time to/from peak on UH
    double t1;                         // time to peak on UH (sec)
    double t2;                         // time after peak on UH (sec)
    double tBase;                      // base time of UH (sec)

    // --- return 0 if past end of UH time base
    tBase = UnitHyd[h].tBase[m][k];
    if ( t >= tBase ) return 0.0;

    // --- compute peak value of UH in original rainfall units (in/hr or mm/hr)
    qPeak = 2. / tBase * 3600.0;

    // --- break UH base into times before & after peak flow
    t1 = UnitHyd[h].tPeak[m][k];
    t2 = tBase - t1;

    // --- find UH flow at time t
    if ( t <= t1 ) f = t / t1;
    else           f = 1.0 - (t - t1) / t2;
    return MAX(f, 0.0) * qPeak;
}

//=============================================================================

int getNodeRdii()
//
//  Input:   none
//  Output:  returns TRUE if any node has RDII inflow, FALSE if not
//  Purpose: computes current RDII inflow at each node.
//
{
    int   hasRdii = FALSE;             // true if any node has some RDII
    int   i;                           // UH group index
    int   j;                           // node index
    int   n;                           // number of nodes w/ RDII
    double rdii;                       // RDII flow (cfs)

    // --- examine each node w/ RDII data
    for (n = 0; n < NumRdiiNodes; n++)
    {
        // --- identify node's index in project's data base
        j = RdiiNodeIndex[n];

        // --- apply node's sewer area to UH RDII to get node RDII in CFS
        i = Node[j].rdiiInflow->unitHyd;
        rdii = UHGroup[i].rdii * Node[j].rdiiInflow->area / UCF(RAINFALL);
        if ( rdii < ZERO_RDII ) rdii = 0.0;
        else hasRdii = TRUE;

        // --- update total RDII volume
        RdiiNodeFlow[n] = (REAL4)rdii;
        if ( rdii > 0.0 )
        {
            TotalRdiiVol += rdii * (double)RdiiStep;
        }
    }
    return hasRdii;
}

//=============================================================================

void saveRdiiFlows(DateTime currentDate)
//
//  Input:   currentDate = current calendar date/time
//  Output:  none
//  Purpose: saves current set of RDII inflows in current flow units to file.
//
{
    fwrite(&currentDate, sizeof(DateTime), 1, Frdii.file);
    fwrite(RdiiNodeFlow, sizeof(REAL4), NumRdiiNodes, Frdii.file);
}

//=============================================================================

void  closeRdiiProcessor()
//
//  Input:   none
//  Output:  none
//  Purpose: closes RDII processing system.
//
{
    // --- write rainfall & RDII totals to report file
    if ( !ErrorCode )
    {
        report_writeRdiiStats(TotalRainVol, TotalRdiiVol);
    }

    // --- free allocated memory and close RDII file
    freeRdiiMemory();
    if ( Frdii.file ) fclose(Frdii.file);
}

//=============================================================================

void freeRdiiMemory()
//
//  Input:   none
//  Output:  none
//  Purpose: frees memory used for RDII processing.
//
{
    int i;
    int k;
    if ( UHGroup )
    {
        for (i = 0; i < Nobjects[UNITHYD]; i++)
        {
            for (k=0; k<3; k++)
            {
                FREE(UHGroup[i].uh[k].pastRain);
                FREE(UHGroup[i].uh[k].pastMonth);
            }
        }
        FREE(UHGroup);
    }
    FREE(RdiiNodeIndex);
    FREE(RdiiNodeFlow);
}
