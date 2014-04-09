//-----------------------------------------------------------------------------
//   lid.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     07/30/10   (Build 5.0.019)
//             04/20/11   (Build 5.0.022)
//   Author:   L. Rossman (US EPA)
//
//   This module handles all processing involving LID (Low Impact Development)
//   practices used to treat runoff for individual subcatchments within a
//   project. See LidTypes below for the types of LIDs that can be modeled.
//
//   An LID process is described by the TLidProc data structure and consists of
//   size-independent design data for the different vertical layers that make
//   up a specific type of LID. The collection of these LID process designs is
//   stored in the LidProcs array.
//
//   When the user selects a member of LidProcs for deployment in a particular
//   subcatchment, its sizing and treatment data are stored in a TLidUnit data
//   structure. The collection of all TLidUnits deployed in a subcatchment is
//   held in a TLidGroup list data structure. The LidGroups array contains a
//   TLidGroup list for each subcatchment in the project.
//
//   During a runoff time step, each subcatchment calls the lid_getRunoff()
//   function to compute flux rates and a water balance through each layer
//   of each LID unit in the subcatchment. The resulting outflows (runoff,
//   underdrain flow, evaporation and infiltration) are added to those
//   computed for the non-LID portion of the subcatchment.
//
//   An option exists for the detailed time series of flux rates and storage
//   levels for a specific LID unit to be written to a text file named by the
//   user for viewing outside of the SWMM program.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "headers.h"
#include "infil.h"
#include "lid.h"

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
#define STOPTOL  0.00328     //integration error tolerance in ft (= 1 mm)
#define MINFLOW  2.3e-7      //flow cutoff for dry conditions (= 0.01 in/hr)

enum LidTypes {
    BIO_CELL,                // bio-retention cell
    INFIL_TRENCH,            // infiltration trench
    POROUS_PAVEMENT,         // porous pavement
    RAIN_BARREL,             // rain barrel
    VEG_SWALE};              // vegetative swale

enum LidLayerTypes {
    SURF,                    // surface layer
    SOIL,                    // soil layer
    STOR,                    // storage layer
    PAVE,                    // pavement layer
    DRAIN};                  // underdrain system

enum LidRptVars {
    SURF_INFLOW,             // inflow to surface layer
    TOTAL_EVAP,              // evaporation rate from all layers
    SURF_INFIL,              // infiltration into surface layer
    SOIL_PERC,               // percolation through soil layer
    STOR_INFIL,              // infiltration from storage layer
    SURF_OUTFLOW,            // outflow from surface layer
    STOR_OUTFLOW,            // outflow from storage layer
    SURF_DEPTH,              // ponded depth on surface layer
    SOIL_MOIST,              // moisture content of soil layer
    STOR_DEPTH,              // water level in storage layer
    MAX_RPT_VARS};

char* LidLayerWords[] =
    {"SURFACE", "SOIL", "STORAGE", "PAVEMENT", "DRAIN", NULL};

char* LidTypeWords[] =
    {"BC",                   //bio-retention cell
     "IT",                   //infiltration trench
     "PP",                   //porous pavement
     "RB",                   //rain barrel
     "VS",                   //vegetative swale
     NULL};

//-----------------------------------------------------------------------------
//  Data Structures
//-----------------------------------------------------------------------------

// LID Surface Layer
typedef struct
{
    double    thickness;          // depression storage or berm ht. (ft)
    double    voidFrac;           // available fraction of storage volume
    double    roughness;          // surface Mannings n 
    double    surfSlope;          // land surface slope (fraction)
    double    sideSlope;          // swale side slope (run/rise)
    double    alpha;              // slope/roughness term in Manning eqn.
    char      canOverflow;        // 1 if immediate outflow of excess water
}  TSurfaceLayer;

// LID Pavement Layer
typedef struct
{
    double   thickness;           // layer thickness (ft)
    double   voidFrac;            // void volume / total volume
    double   impervFrac;          // impervious area fraction
    double   kSat;                // permeability (ft/sec)
    double   clogFactor;          // clogging factor
}  TPavementLayer;

// LID Soil Layer
typedef struct
{
    double    thickness;          // layer thickness (ft)
    double    porosity;           // void volume / total volume
    double    fieldCap;           // field capacity
    double    wiltPoint;          // wilting point
    double    suction;            // suction head at wetting front (ft)
    double    kSat;               // saturated hydraulic conductivity (ft/sec)
    double    kSlope;             // slope of log(K) v. moisture content curve
}  TSoilLayer;


// LID Storage Layer
typedef struct
{
    double    thickness;          // layer thickness (ft)
    double    voidFrac;           // void volume / total volume
    double    kSat;               // saturated hydraulic conductivity (ft/sec)
    double    clogFactor;         // clogging factor
}  TStorageLayer;

// Underdrain System (part of Storage Layer)
typedef struct
{
    double    coeff;              // underdrain flow coeff. (in/hr or mm/hr)
    double    expon;              // underdrain head exponent (for in or mm)
    double    offset;             // offset height of underdrain (ft)
    double    delay;              // rain barrel drain delay time (sec)
}  TDrainLayer;


// LID Process - generic LID design per unit of area
typedef struct
{
    char*          ID;            // identifying name
    int            lidType;       // type of LID
    TSurfaceLayer  surface;       // surface layer parameters
    TPavementLayer pavement;      // pavement layer parameters
    TSoilLayer     soil;          // soil layer parameters
    TStorageLayer  storage;       // storage layer parameters
    TDrainLayer    drain;         // underdrain system parameters
}  TLidProc;

// Water Balance Statistics
typedef struct
{
    double         inflow;        // total inflow (ft)
    double         evap;          // total evaporation (ft)
    double         infil;         // total infiltration (ft)
    double         surfFlow;      // total surface runoff (ft)
    double         drainFlow;     // total underdrain flow (ft)
    double         initVol;       // initial stored volume (ft)
    double         finalVol;      // final stored volume (ft)
}  TWaterBalance;

// LID Report File
typedef struct
{
    FILE*     file;               // file pointer
    double    nextReportTime;     // next reporting time (msec)
}   TLidRptFile;

// LID Unit - specific LID process applied over a given area
typedef struct
{
    int            lidIndex;      // index of LID process
    int            number;        // number of replicate units
    double         area;          // area of single replicate unit (ft2)
    double         width;         // width of single unit (ft)
    double         initSat;       // initial saturation of soil & storage layers
    double         fromImperv;    // fraction of impervious area runoff treated
    int            toPerv;        // 1 if outflow sent to pervious area; 0 if not
    TLidRptFile*   rptFile;       // pointer to detailed report file

    TGrnAmpt       soilInfil;     // infil. object for biocell soil layer 
    //TGrnAmpt     nativeInfil;   // infil. rate for native soil               //(5.0.022-LR)
    double         surfaceDepth;  // depth of ponded water on surface layer (ft)
    double         soilMoisture;  // moisture content of soil (or pavement) layer
    double         storageDepth;  // depth of water in storage layer (ft)
    double         oldFluxRates[3];  // net inflow - outflow from previous
                                     // time step for surface/soil(pavement)/
                                     // storage layers (ft/sec)
    TWaterBalance  waterBalance;     // water balance quantites
}  TLidUnit;

// LID List - list of LID units contained in an LID group
struct LidList
{
    TLidUnit*        lidUnit;     // ptr. to a LID unit
    struct LidList*  nextLidUnit;
};
typedef struct LidList TLidList;

// LID Group - collection of LID units applied to a specific subcatchment
struct LidGroup
{
    double         pervArea;      // amount of pervious area in group (ft2)
    double         flowToPerv;    // total flow sent to pervious area (cfs)
    double         dryTime;       // time since last wet period (sec)
    double         impervRunoff;  // impervious area runoff volume (ft3)
    TLidList*      lidList;       // list of LID units in the group
};
typedef struct LidGroup* TLidGroup;


//-----------------------------------------------------------------------------
//  Local Variables
//-----------------------------------------------------------------------------
static TLidProc*  LidProcs;            // array of LID processes
static int        LidCount;            // number of LID processes
static TLidGroup* LidGroups;           // array of LID process groups
static int        GroupCount;          // number of LID groups (subcatchments)

static double     LidUnitArea;         // area of current LID unit (ft2)
static double     LidEvapVol;          // LID unit's evaporation loss (ft3)
static double     LidInfilVol;         // LID unit's infiltration loss (ft3)
static double     EvapRate;            // evaporation rate (ft/s)

static double     SurfaceInflow;       // precip. + runon to LID unit (ft/s)
static double     SurfaceInfil;        // infil. rate from surface layer (ft/s)
static double     SurfaceEvap;         // evap. rate from surface layer (ft/s)
static double     SurfaceOutflow;      // outflow from surface layer (ft/s)
static double     SurfaceVolume;       // volume in surface storage (ft)

static double     SoilEvap;            // evap. from soil layer (ft/s)
static double     SoilPerc;            // percolation from soil layer (ft/s)
static double     SoilVolume;          // volume in soil/pavement storage (ft)

static double     StorageInflow;       // inflow rate to storage layer (ft/s)
static double     StorageInfil;        // infil. rate from storage layer (ft/s)
static double     StorageEvap;         // evap.rate from storage layer (ft/s)
static double     StorageOutflow;      // outflow rate from storage layer (ft/s)
static double     StorageVolume;       // volume in storage layer (ft)

//static TGrnAmpt TmpSoilInfil;        // data structure for soil layer G-A infil. //(5.0.022-LR)
//static TGrnAmpt TmpNativeInfil;      // data structure for native soil G-A infil.//(5.0.022-LR)
static double     NativeInfil;         // native soil infil. rate (ft/s)           //(5.0.022-LR)
static double     MaxNativeInfil;      // native soil infil. rate limit (ft/s)

static int        IsSaturated;         // TRUE if LID unit is saturated
static double     Tstep;               // current time step (sec)
static TSubcatch* theSubcatch;         // ptr. to current subcatchment
static TLidGroup  theLidGroup;         // ptr. to current LID group
static TLidUnit*  theLidUnit;          // ptr. to a subcatchment's LID unit
static TLidProc*  theLidProc;          // ptr. to a LID process

static double     TotalEvapVol;        // subcatch. evap loss (ft3)
static double     TotalPervEvapVol;    // evap loss over pervious area (ft3)
static double     TotalInfilVol;       // subcatch infiltration loss (ft3)

static DateTime   OldDate;                  // previous reporting date
static char       theDate[DATE_STR_SIZE];   // string for calendar date
static char       theTime[TIME_STR_SIZE];   // string for time of day


//-----------------------------------------------------------------------------
//  External Functions
//-----------------------------------------------------------------------------
//  lid_create               called by createObjects in project.c
//  lid_delete               called by deleteObjects in project.c
//  lid_validate             called by project_validate
//  lid_initState            called by project_init

//  lid_readParams           called by parseLine in input.c
//  lid_readGroupParams      called by treatmnt_readExpression

//  lid_getPervArea          called by subcatch_getFracPerv
//  lid_getFlowToPerv        called by subcatch_getRunon
//  lid_getSurfaceDepth      called by subcatch_getDepth
//  lid_getDepthOnPavement   called by sweptSurfacesDry in subcatch.c
//  lid_getStoredVolume      called by subcatch_getStorage
//  lid_getRunoff            called by subcatch_getRunoff

//  lid_writeSummary         called by inputrpt_writeInput
//  lid_writeWaterBalance    called by statsrpt_writeReport


//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
static void   freeLidGroup(int j);
static int    readSurfaceData(int j, char* tok[], int ntoks);
static int    readPavementData(int j, char* tok[], int ntoks);
static int    readSoilData(int j, char* tok[], int ntoks);
static int    readStorageData(int j, char* tok[], int ntoks);
static int    readDrainData(int j, char* tok[], int ntoks);

static int    addLidUnit(int j, int k, int n, double x[], char* fname);
static int    createLidRptFile(TLidUnit* lidUnit, char* fname);
static void   validateLidProc(int j);
static void   validateLidGroup(int j);
static int    isLidPervious(int k);
static int    isLidSaturated(void);

static double getImpervAreaInflow(void);
static void   evalLidUnit(double qImperv, double *lidOutflow,
                          double *nonLidOutflow, double *flowToPerv);
static double getLidOutflow(void);

static void   findNativeInfil(int j, double infilVol, double tStep);
static double getSoilInfilRate(double theta);                                  //(5.0.022 - LR)
static double getStorageInfilRate(double storageDepth);
static double getSurfaceOutflow(double depth);
static double getSurfaceOverflow(double* surfaceDepth);
static double getPavementPermRate(void);
static double getSoilPercRate(double theta, double storageDepth);
static double getStorageOutflow(double depth);
static void   getEvapRates(double surfaceVol, double soilVol,
                           double storageVol);

static void   barrelFluxRates(double x[], double f[]);
static void   biocellFluxRates(double x[], double f[]);
static void   pavementFluxRates(double x[], double f[]);
static void   trenchFluxRates(double x[], double f[]);
static void   swaleFluxRates(double x[], double f[]);

static void   initWaterBalance(TLidUnit *lidUnit, double initVol);
static void   updateWaterBalance(TLidUnit *lidUnit, double inflow,
                                 double evap, double infil, double surfFlow,
                                 double drainFlow, double storage);

static void   initLidRptFile(char* id, TLidUnit*  lidUnit);
static void   saveResults(void);

static int    modpuls_solve(int n, double* x, double* xOld, double* xPrev,
                            double* xMin, double* xMax, double* xTol,
                            double* qOld, double* q, double dt,
                            void (*derivs)(double*, double*));

//=============================================================================

void lid_create(int lidCount, int subcatchCount)
//
//  Purpose: creates an array of LID objects.
//  Input:   n = number of LID processes
//  Output:  none
//
{
    int j;

    //... assign NULL values to LID arrays
    LidProcs = NULL;
    LidGroups = NULL;
    GroupCount = subcatchCount;
    LidCount = lidCount;

    //... create LID groups
    if ( GroupCount == 0 ) return;
    LidGroups = (TLidGroup *) calloc(GroupCount, sizeof(TLidGroup));
    if ( LidGroups == NULL )
    {
        ErrorCode = ERR_MEMORY;
        return;
    }

    //... create LID objects
    if ( LidCount == 0 ) return;
    LidProcs = (TLidProc *) calloc(LidCount, sizeof(TLidProc));
    if ( LidProcs == NULL )
    {
        ErrorCode = ERR_MEMORY;
        return;
    }

    //... initialize LID objects
    for (j = 0; j < LidCount; j++)
    {
        LidProcs[j].lidType = -1;
        LidProcs[j].surface.thickness = 0.0;
        LidProcs[j].surface.voidFrac = 1.0;
        LidProcs[j].surface.roughness = 0.0;
        LidProcs[j].surface.surfSlope = 0.0;
        LidProcs[j].pavement.thickness = 0.0;
        LidProcs[j].soil.thickness = 0.0;
        LidProcs[j].storage.thickness = 0.0;
        LidProcs[j].storage.kSat = 0.0;
        LidProcs[j].drain.coeff = 0.0;
        LidProcs[j].drain.offset = 0.0;
    }

    //... initialize LID groups
    for (j = 0; j < GroupCount; j++) LidGroups[j] = NULL;
}

//=============================================================================

void lid_delete()
//
//  Purpose: deletes all LID objects
//  Input:   none
//  Output:  none
//
{
    int j;
    for (j = 0; j < GroupCount; j++) freeLidGroup(j);
    FREE(LidGroups);
    FREE(LidProcs);
    GroupCount = 0;
    LidCount = 0;
}

//=============================================================================

void freeLidGroup(int j)
//
//  Purpose: frees all LID units associated with a subcatchment.
//  Input:   j = group (or subcatchment) index
//  Output:  none
//
{
    TLidGroup  lidGroup = LidGroups[j];
    TLidList*  lidList;
    TLidUnit*  lidUnit;
    TLidList*  nextLidUnit;

    if ( lidGroup == NULL ) return;
    lidList = lidGroup->lidList;
    while (lidList)
    {
        lidUnit = lidList->lidUnit;
        if ( lidUnit->rptFile )
        {
            if ( lidUnit->rptFile->file ) fclose(lidUnit->rptFile->file);
            free(lidUnit->rptFile);
        }
        nextLidUnit = lidList->nextLidUnit;
        free(lidUnit);
        free(lidList);
        lidList = nextLidUnit;
    }
    free(lidGroup);
    LidGroups[j] = NULL;
}

//=============================================================================

int lid_readProcParams(char* toks[], int ntoks)
//
//  Purpose: reads LID process information from line of input data file
//  Input:   toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format for first line that defines a LID process is:
//    LID_ID  LID_Type
//
//  Followed by some combination of lines below depending on LID_Type:
//    LID_ID  SURFACE   <parameters>
//    LID_ID  PAVEMENT  <parameters>
//    LID_ID  SOIL      <parameters>
//    LID_ID  STORAGE   <parameters>
//    LID_ID  DRAIN     <parameters>
//
{
    int j, m;

    // --- check for minimum number of tokens
    if ( ntoks < 2 ) return error_setInpError(ERR_ITEMS, "");

    // --- check that LID exists in database
    j = project_findObject(LID, toks[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, toks[0]);

    // --- assign ID if not done yet
    if ( LidProcs[j].ID == NULL )
        LidProcs[j].ID = project_findID(LID, toks[0]);

    // --- check if second token is the type of LID
    m = findmatch(toks[1], LidTypeWords);
    if ( m >= 0 )
    {
        LidProcs[j].lidType = m;
        return 0;
    }

    // --- check if second token is name of LID layer
    else m = findmatch(toks[1], LidLayerWords);

    // --- read input parameters for the identified layer
    switch (m)
    {
    case SURF:  return readSurfaceData(j, toks, ntoks);
    case SOIL:  return readSoilData(j, toks, ntoks);
    case STOR:  return readStorageData(j, toks, ntoks);
    case PAVE:  return readPavementData(j, toks, ntoks);
    case DRAIN: return readDrainData(j, toks, ntoks);
    }
    return error_setInpError(ERR_KEYWORD, toks[1]);
}

//=============================================================================

int lid_readGroupParams(char* toks[], int ntoks)
//
//  Purpose: reads input data for a LID unit placed in a subcatchment.
//  Input:   toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of input data line is:
//    Subcatch_ID  LID_ID  Number  Area  Width  InitSat  FromImp  ToPerv  (RptFile)
//  where:
//    Subcatch_ID    = name of subcatchment
//    LID_ID         = name of LID process
//    Number     (n) = number of replicate units
//    Area    (x[0]) = area of each unit (ft2 or m2)
//    Width   (x[1]) = width of each unit (ft)
//    InitSat (x[2]) = pct. saturated of soil or storage zone of LID
//    FromImp (x[3]) = pct. of subcatchment's impervious runoff sent to LID
//    ToPerv  (x[4]) = 1 if outflow goes to pervious sub-area; 0 if not
//    RptFile        = name of file where detailed results are saved (optional)
//
{
    int        i, j, k, n;
    double     x[5];
    char*      fname;

    //... check for valid number of input tokens
    if ( ntoks < 8 ) return error_setInpError(ERR_ITEMS, "");

    //... find subcatchment
    j = project_findObject(SUBCATCH, toks[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, toks[0]);

    //... find LID process in list of LID processes
    k = project_findObject(LID, toks[1]);
    if ( k < 0 ) return error_setInpError(ERR_NAME, toks[1]);

    //... get number of replicates
    n = atoi(toks[2]);
    if ( n < 0 ) return error_setInpError(ERR_NUMBER, toks[2]);
    if ( n == 0 ) return 0;

    //... convert next 4 tokens to doubles
    for (i = 3; i <= 7; i++)
    {
        if ( ! getDouble(toks[i], &x[i-3]) || x[i-3] < 0.0 )
            return error_setInpError(ERR_NUMBER, toks[i]);
    }

    //... check for valid percentages on tokens 5 & 6 (x[2] & x[3])
    for (i = 2; i <= 3; i++) if ( x[i] > 100.0 )
        return error_setInpError(ERR_NUMBER, toks[i+3]);

    //... read optional report file name
    if ( ntoks >= 9 )fname = toks[8];
    else fname = NULL;

    //... create a new LID unit and add it to the subcatchment's LID group
    return addLidUnit(j, k, n, x, fname);
}

//=============================================================================

int addLidUnit(int j, int k, int n, double x[], char* fname)
//
//  Purpose: adds an LID unit to a subcatchment's LID group.
//  Input:   j = subcatchment index
//           k = LID control index
//           n = number of replicate units
//           x = LID unit's parameters
//           fname = name of detailed performance report file
//  Output:  returns an error code
//
{
    TLidUnit*  lidUnit;
    TLidList*  lidList;
    TLidGroup  lidGroup;

    //... create a LID group (pointer to an LidGroup struct)
    //    if one doesn't already exist
    lidGroup = LidGroups[j];
    if ( !lidGroup )
    {
        lidGroup = (struct LidGroup *) malloc(sizeof(struct LidGroup));
        if ( !lidGroup ) return error_setInpError(ERR_MEMORY, "");
        lidGroup->lidList = NULL;
        LidGroups[j] = lidGroup;
    }

    //... create a new LID unit to add to the group
    lidUnit = (TLidUnit *) malloc(sizeof(TLidUnit));
    if ( !lidUnit ) return error_setInpError(ERR_MEMORY, "");
    lidUnit->rptFile = NULL;

    //... add the LID unit to the group
    lidList = (TLidList *) malloc(sizeof(TLidList));
    if ( !lidList )
    {
        free(lidUnit);
        return error_setInpError(ERR_MEMORY, "");
    }
    lidList->lidUnit = lidUnit;
    lidList->nextLidUnit = lidGroup->lidList;
    lidGroup->lidList = lidList;

    //... assign parameter values to LID unit
    lidUnit->lidIndex     = k;
    lidUnit->number       = n;
    lidUnit->area         = x[0] / SQR(UCF(LENGTH));
    lidUnit->width        = x[1] / UCF(LENGTH);
    lidUnit->initSat      = x[2] / 100.0;
    lidUnit->fromImperv   = x[3] / 100.0;
    lidUnit->toPerv       = (x[4] > 0.0);

    //... open report file if it was supplied
    if ( fname != NULL )
    {
        if ( !createLidRptFile(lidUnit, fname) ) 
            return error_setInpError(ERR_RPT_FILE, fname);
    }
    return 0;
}

//=============================================================================

int createLidRptFile(TLidUnit* lidUnit, char* fname)
{
    TLidRptFile* rptFile;
    
    rptFile = (TLidRptFile *) malloc(sizeof(TLidRptFile));
    if ( rptFile == NULL ) return 0;
    lidUnit->rptFile = rptFile;
    rptFile->file = fopen(fname, "wt");
    if ( rptFile->file == NULL ) return 0;
    return 1;
}

//=============================================================================

int readSurfaceData(int j, char* toks[], int ntoks)
//
//  Purpose: reads surface layer data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID  SURFACE  StorageHt  VegVolFrac  Roughness  SurfSlope  SideSlope  //(5.0.022 - LR)
//
{
    int    i;
    double x[5];

    if ( ntoks < 7 ) return error_setInpError(ERR_ITEMS, "");
    for (i = 2; i < 7; i++)
    {
        if ( ! getDouble(toks[i], &x[i-2]) || x[i-2] < 0.0 )
            return error_setInpError(ERR_NUMBER, toks[i]);
    }

////  Added for release 5.0.022  ////                                          //(5.0.022 - LR)
    if ( x[1] >= 1.0 ) return error_setInpError(ERR_NUMBER, toks[3]);           
    if ( x[0] == 0.0 ) x[1] = 0.0;
////////////////////////////////////

    LidProcs[j].surface.thickness     = x[0] / UCF(RAINDEPTH);
    LidProcs[j].surface.voidFrac      = 1.0 - x[1];
    LidProcs[j].surface.roughness     = x[2];
    LidProcs[j].surface.surfSlope     = x[3] / 100.0;
    LidProcs[j].surface.sideSlope     = x[4];
    return 0;
}

//=============================================================================

int readPavementData(int j, char* toks[], int ntoks)
//
//  Purpose: reads pavement layer data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID PAVEMENT  Thickness  VoidRatio  FracImperv  Permeability  ClogFactor
//
{
    int    i;
    double x[5];

    if ( ntoks < 7 ) return error_setInpError(ERR_ITEMS, "");
    for (i = 2; i < 7; i++)
    {
        if ( ! getDouble(toks[i], &x[i-2]) || x[i-2] < 0.0 )
            return error_setInpError(ERR_NUMBER, toks[i]);
    }

    //... convert void ratio to void fraction
    x[1] = x[1]/(x[1] + 1.0);

    LidProcs[j].pavement.thickness    = x[0] / UCF(RAINDEPTH);
    LidProcs[j].pavement.voidFrac     = x[1];
    LidProcs[j].pavement.impervFrac   = x[2];
    LidProcs[j].pavement.kSat         = x[3] / UCF(RAINFALL);
    LidProcs[j].pavement.clogFactor   = x[4];
    return 0;
}

//=============================================================================

int readSoilData(int j, char* toks[], int ntoks)
//
//  Purpose: reads soil layer data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID  SOIL  Thickness  Porosity  FieldCap  WiltPt Ksat  Kcoeff  Suction
//
{
    int    i;
    double x[7];

    if ( ntoks < 9 ) return error_setInpError(ERR_ITEMS, "");
    for (i = 2; i < 9; i++)
    {
        if ( ! getDouble(toks[i], &x[i-2]) || x[i-2] < 0.0 )
            return error_setInpError(ERR_NUMBER, toks[i]);
    }
    LidProcs[j].soil.thickness = x[0] / UCF(RAINDEPTH);
    LidProcs[j].soil.porosity  = x[1];
    LidProcs[j].soil.fieldCap  = x[2];
    LidProcs[j].soil.wiltPoint = x[3];
    LidProcs[j].soil.kSat      = x[4] / UCF(RAINFALL);
    LidProcs[j].soil.kSlope    = x[5];
    LidProcs[j].soil.suction   = x[6] / UCF(RAINDEPTH);
    return 0;
}

//=============================================================================

int readStorageData(int j, char* toks[], int ntoks)
//
//  Purpose: reads drainage layer data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID STORAGE  Thickness  VoidRatio  Ksat  ClogFactor
//
{
    int    i;
    double x[4];

    //... read numerical parameters
    if ( ntoks < 6 ) return error_setInpError(ERR_ITEMS, "");
    for (i = 2; i < 6; i++)
    {
        if ( ! getDouble(toks[i], &x[i-2])  || x[i-2] < 0.0 )
            return error_setInpError(ERR_NUMBER, toks[i]);
    }

    //... convert void ratio to void fraction
    x[1] = x[1]/(x[1] + 1.0);

    //... save parameters to LID storage layer structure
    LidProcs[j].storage.thickness   = x[0] / UCF(RAINDEPTH);
    LidProcs[j].storage.voidFrac    = x[1];
    LidProcs[j].storage.kSat        = x[2] / UCF(RAINFALL);
    LidProcs[j].storage.clogFactor  = x[3];
    return 0;
}
 
//=============================================================================

int readDrainData(int j, char* toks[], int ntoks)
//
//  Purpose: reads underdrain data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID DRAIN  coeff  expon  offset  delay
//
{
    int    i;
    double x[4];

    //... read numerical parameters
    if ( ntoks < 6 ) return error_setInpError(ERR_ITEMS, "");
    for (i = 2; i < 6; i++)
    {
        if ( ! getDouble(toks[i], &x[i-2])  || x[i-2] < 0.0 )
            return error_setInpError(ERR_NUMBER, toks[i]);
    }

    //... save parameters to LID drain layer structure
    LidProcs[j].drain.coeff  = x[0];
    LidProcs[j].drain.expon  = x[1];
    LidProcs[j].drain.offset = x[2] / UCF(RAINDEPTH);
    LidProcs[j].drain.delay  = x[3] * 3600.0;                                  //(5.0.022 - LR)
    return 0;
}
 
//=============================================================================

void lid_writeSummary()
//
//  Purpose: writes summary of LID processes used to report file.
//  Input:   none
//  Output:  none
//
{
    int        j, k;
    double     pctArea;
    TLidUnit*  lidUnit;
    TLidList*  lidList;
    TLidGroup  lidGroup;
 
    fprintf(Frpt.file, "\n");
    fprintf(Frpt.file, "\n");
    fprintf(Frpt.file, "\n  *******************");
    fprintf(Frpt.file, "\n  LID Control Summary");
    fprintf(Frpt.file, "\n  *******************");
    fprintf(Frpt.file,
"\n                                   No. of        Unit        Unit      %% Area    %% Imperv");
    fprintf(Frpt.file,
"\n  Subcatchment     LID Control      Units        Area       Width     Covered     Treated");
    fprintf(Frpt.file,
"\n  ---------------------------------------------------------------------------------------");
    for (j = 0; j < GroupCount; j++)
    {
        lidGroup = LidGroups[j];
        if ( lidGroup == NULL ) continue;
        lidList = lidGroup->lidList;
        while ( lidList )
        {
            lidUnit = lidList->lidUnit;
            k = lidUnit->lidIndex;
            pctArea = lidUnit->area * lidUnit->number / Subcatch[j].area * 100.0;
            fprintf(Frpt.file, "\n  %-16s %-16s", Subcatch[j].ID, LidProcs[k].ID);
            fprintf(Frpt.file, "%6d  %10.2f  %10.2f  %10.2f  %10.2f",
                lidUnit->number, lidUnit->area * SQR(UCF(LENGTH)),
                lidUnit->width * UCF(LENGTH), pctArea, lidUnit->fromImperv*100.0);
            lidList = lidList->nextLidUnit;
        }
    }
}

//=============================================================================

void lid_validate()
//
//  Purpose: validates LID process and group parameters.
//  Input:   none 
//  Output:  none
//
{
    int j;
    for (j = 0; j < LidCount; j++) validateLidProc(j);
    for (j = 0; j < GroupCount; j++) validateLidGroup(j);
}

//=============================================================================

void validateLidProc(int j)
//
//  Purpose: validates LID process parameters.
//  Input:   j = LID process index 
//  Output:  none
//
{
    int layerMissing = FALSE;

    //... check that LID type was supplied
    if ( LidProcs[j].lidType < 0 )
    {
            report_writeErrorMsg(ERR_LID_TYPE, LidProcs[j].ID);
            return;
    }

    //... check that required layers were defined
    if ( LidProcs[j].soil.thickness <= 0.0 &&
        LidProcs[j].lidType == BIO_CELL ) layerMissing = TRUE;
    if ( LidProcs[j].pavement.thickness  <= 0.0 &&
        LidProcs[j].lidType == POROUS_PAVEMENT ) layerMissing = TRUE;          //(5.0.022-LR)
    if ( LidProcs[j].lidType == INFIL_TRENCH &&
        LidProcs[j].storage.thickness <= 0.0 ) layerMissing = TRUE;
    if ( layerMissing )
    {
        report_writeErrorMsg(ERR_LID_LAYER, LidProcs[j].ID);
        return;
    }

    //... check pavement layer parameters
    if ( LidProcs[j].lidType == POROUS_PAVEMENT )
    {
        if ( LidProcs[j].pavement.thickness  <= 0.0 
        ||   LidProcs[j].pavement.kSat       <= 0.0 
        ||   LidProcs[j].pavement.voidFrac   <= 0.0
        ||   LidProcs[j].pavement.voidFrac   >  1.0
        ||   LidProcs[j].pavement.impervFrac >  1.0 )
            report_writeErrorMsg(ERR_LID_PARAMS, LidProcs[j].ID);
    }

    //... check soil layer parameters
    if ( LidProcs[j].soil.thickness > 0.0 )
    {
        if ( LidProcs[j].soil.porosity      <= 0.0 
        ||   LidProcs[j].soil.fieldCap      >= LidProcs[j].soil.porosity
        ||   LidProcs[j].soil.wiltPoint     >= LidProcs[j].soil.fieldCap
        ||   LidProcs[j].soil.kSat          <= 0.0
        ||   LidProcs[j].soil.kSlope        <  0.0 )
            report_writeErrorMsg(ERR_LID_PARAMS, LidProcs[j].ID);
    }

    //... check storage layer parameters
    if ( LidProcs[j].storage.thickness > 0.0 )
    {
        if ( LidProcs[j].storage.voidFrac <= 0.0 ||
             LidProcs[j].storage.voidFrac > 1.0 )
            report_writeErrorMsg(ERR_LID_PARAMS, LidProcs[j].ID);
    }

    //... check underdrain parameters
    if ( LidProcs[j].drain.offset > LidProcs[j].storage.thickness )
        report_writeErrorMsg(ERR_LID_PARAMS, LidProcs[j].ID);

    //... compute the surface layer's overland flow constant (alpha)
    if ( LidProcs[j].lidType == VEG_SWALE )
    {
        if ( LidProcs[j].surface.roughness * 
             LidProcs[j].surface.surfSlope <= 0.0 ||                           //(5.0.022 - LR)
             LidProcs[j].surface.thickness == 0.0                              //(5.0.022 - LR)
           ) report_writeErrorMsg(ERR_LID_PARAMS, LidProcs[j].ID);
        else LidProcs[j].surface.alpha = 
            1.49 * sqrt(LidProcs[j].surface.surfSlope) /
                LidProcs[j].surface.roughness;
    }
    else
    {
        //... compute surface overland flow coeff.
        if ( LidProcs[j].surface.roughness > 0.0 )
            LidProcs[j].surface.alpha = 1.49 / LidProcs[j].surface.roughness *
                                        sqrt(LidProcs[j].surface.surfSlope);
        else LidProcs[j].surface.alpha = 0.0;
    }

    //... convert clogging factors to void volume basis
    if ( LidProcs[j].pavement.thickness > 0.0 )
    {
        LidProcs[j].pavement.clogFactor *= 
            LidProcs[j].pavement.thickness * LidProcs[j].pavement.voidFrac *
            (1.0 - LidProcs[j].pavement.impervFrac);
    }
    if ( LidProcs[j].storage.thickness > 0.0 )
    {
        LidProcs[j].storage.clogFactor *=
            LidProcs[j].storage.thickness * LidProcs[j].storage.voidFrac;
    }

    //... for certain LID types, immediate overflow of excess surface water
    //    occurs if either the surface roughness or slope is zero
    LidProcs[j].surface.canOverflow = TRUE;
    switch (LidProcs[j].lidType)
    {
        case INFIL_TRENCH:
        case POROUS_PAVEMENT:
            if ( LidProcs[j].surface.alpha > 0.0 )
                LidProcs[j].surface.canOverflow = FALSE;
    }

    //... rain barrels have 100% void space and impermeable bottom
    if ( LidProcs[j].lidType == RAIN_BARREL )
    {
        LidProcs[j].storage.voidFrac = 1.0;
        LidProcs[j].storage.kSat = 0.0;
    }
}

//=============================================================================

void validateLidGroup(int j)
//
//  Purpose: validates properties of LID units grouped in a subcatchment.
//  Input:   j = subcatchment index 
//  Output:  returns 1 if data are valid, 0 if not
//
{
    int        k;
    double     p[3];
    double     totalArea = Subcatch[j].area;
    double     totalLidArea = 0.0;
    double     fromImperv = 0.0;
    TLidUnit*  lidUnit;
    TLidList*  lidList;
    TLidGroup  lidGroup;

    lidGroup = LidGroups[j];
    if ( lidGroup == NULL ) return;
    lidList = lidGroup->lidList;
    while ( lidList )
    {
        lidUnit = lidList->lidUnit;
        k = lidUnit->lidIndex;

        //... update contributing fractions
        totalLidArea += (lidUnit->area * lidUnit->number);
        fromImperv += lidUnit->fromImperv;

        //... assign biocell soil layer infiltration parameters
        lidUnit->soilInfil.Ks = 0.0;
        if ( LidProcs[k].soil.thickness > 0.0 )
        {
            p[0] = LidProcs[k].soil.suction * UCF(RAINDEPTH);
            p[1] = LidProcs[k].soil.kSat * UCF(RAINFALL);
            p[2] = (LidProcs[k].soil.porosity - LidProcs[k].soil.wiltPoint) *
                   (1.0 - lidUnit->initSat);
            if ( grnampt_setParams(&(lidUnit->soilInfil), p) == FALSE )
            {
                report_writeErrorMsg(ERR_LID_PARAMS, LidProcs[k].ID);
            }
        }
        
////  The following code segment was modified for release 5.0.022  ////        //(5.0.022-LR)
        //... assign vegetative swale infiltration parameters
        if ( LidProcs[k].lidType == VEG_SWALE )
        {
            if ( InfilModel == GREEN_AMPT )
            {
                p[0] = GAInfil[j].S * UCF(RAINDEPTH);
                p[1] = GAInfil[j].Ks * UCF(RAINFALL);
                p[2] = GAInfil[j].IMDmax;
                if ( grnampt_setParams(&(lidUnit->soilInfil), p) == FALSE )
                {
                    report_writeErrorMsg(ERR_LID_PARAMS, LidProcs[k].ID);
                }
            }
            if ( lidUnit->width <= 0.0 )
            {
                report_writeErrorMsg(ERR_LID_PARAMS, LidProcs[k].ID);
            }
        }
        lidList = lidList->nextLidUnit;
    }

////  The following code segment was modified for release 5.0.022  ////        //(5.0.022-LR)
    //... check contributing area fractions
    if ( totalLidArea > 1.001 * totalArea )
    {
        report_writeErrorMsg(ERR_LID_AREAS, Subcatch[j].ID);
    }
    if ( fromImperv > 1.001 )
    {
        report_writeErrorMsg(ERR_LID_CAPTURE_AREA, Subcatch[j].ID);
    }
//////////////////////////////////////////////////////////////////////

    //... Make subcatchment LID area equal total area if the two are close
    if ( totalLidArea > 0.999 * totalArea ) totalLidArea = totalArea;
    Subcatch[j].lidArea = totalLidArea;
}

//=============================================================================

void lid_initState()
//
//  Purpose: initializes the internal state of each LID in a subcatchment.
//  Input:   none 
//  Output:  none
//
{
    int i, j, k;
    TLidUnit*  lidUnit;
    TLidList*  lidList;
    TLidGroup  lidGroup;
    double     initVol;

    for (j = 0; j < GroupCount; j++)
    {
        //... check if group exists
        lidGroup = LidGroups[j];
        if ( lidGroup == NULL ) continue;

        //... initialize group variables
        lidGroup->pervArea = 0.0;
        lidGroup->flowToPerv = 0.0;
        lidGroup->dryTime = StartDryDays * SECperDAY;

        //... examine each LID in the group
        lidList = lidGroup->lidList;
        while ( lidList )
        {
            //... initialize depth & moisture content
            lidUnit = lidList->lidUnit;
            k = lidUnit->lidIndex;
            lidUnit->surfaceDepth = 0.0;
            lidUnit->storageDepth = 0.0;
            lidUnit->soilMoisture = 0.0;
            initVol = 0.0;
            if ( LidProcs[k].soil.thickness > 0.0 )
            {
                lidUnit->soilMoisture = LidProcs[k].soil.wiltPoint + 
                    lidUnit->initSat * (LidProcs[k].soil.porosity -
                    LidProcs[k].soil.wiltPoint);
                initVol += (lidUnit->soilMoisture - 
                            LidProcs[k].soil.wiltPoint) *
                            LidProcs[k].soil.thickness;
            }
            if ( LidProcs[k].storage.thickness > 0.0 )
            {
                lidUnit->storageDepth = lidUnit->initSat *
                    LidProcs[k].storage.thickness;
                initVol += lidUnit->storageDepth * LidProcs[k].storage.voidFrac;
            }

            //... initialize water balance totals
            initWaterBalance(lidUnit, initVol);

            //... initialize report file for the LID
            if ( lidUnit->rptFile ) initLidRptFile(Subcatch[j].ID, lidUnit);

            //... set previous flux rates to 0
            for (i = 0; i < 3; i++) lidUnit->oldFluxRates[i] = 0.0;

            //... initialize infiltration state variables
            if ( lidUnit->soilInfil.Ks > 0.0 )
                grnampt_initState(&(lidUnit->soilInfil));
            //if ( lidUnit->nativeInfil.Ks > 0.0 )                             //(5.0.022-LR)
            //    grnampt_initState(&(lidUnit->nativeInfil));                  //(5.0.022-LR)

            //... add contribution to pervious LID area
            if ( isLidPervious(lidUnit->lidIndex) )
                lidGroup->pervArea += (lidUnit->area * lidUnit->number);
            lidList = lidList->nextLidUnit;
        }
    }
}

//=============================================================================

int isLidPervious(int k)
//
//  Purpose: determines if a LID process allows infiltration or not.
//  Input:   k = LID process index 
//  Output:  returns 1 if process is pervious or 0 if not
//
{
    return ( LidProcs[k].storage.thickness == 0.0 ||
             LidProcs[k].storage.kSat > 0.0 );
}

//=============================================================================

int isLidSaturated()
//
//  Purpose: determines if the current LID unit is saturated or not.
//  Input:   none
//  Output:  returns TRUE if saturated, FALSE if not.
//
{
    if ( theLidProc->surface.thickness > 0.0 &&
         theLidUnit->surfaceDepth < theLidProc->surface.thickness ) return FALSE;
    if ( theLidProc->soil.thickness > 0.0 && 
         theLidUnit->soilMoisture < theLidProc->soil.porosity )     return FALSE;
    if ( theLidProc->storage.thickness > 0.0 &&
         theLidUnit->storageDepth < theLidProc->storage.thickness ) return FALSE;
    return TRUE;
}

//=============================================================================

double lid_getSurfaceDepth(int j)
//
//  Purpose: computes the depth of ponded water on the surface of all LIDs
//           within a subcatchment.
//  Input:   j = subcatchment index 
//  Output:  returns volumetric depth of ponded water (ft)
//
{
    int    k;
    double depth = 0.0;
    TLidUnit*  lidUnit;
    TLidList*  lidList;
    TLidGroup  lidGroup;

    lidGroup = LidGroups[j];
    if ( lidGroup == NULL ) return 0.0;
    if ( Subcatch[j].lidArea == 0.0 ) return 0.0;
    lidList = lidGroup->lidList;
    while ( lidList )
    {
        lidUnit = lidList->lidUnit;
        k = lidUnit->lidIndex;
        depth += lidUnit->surfaceDepth * LidProcs[k].surface.voidFrac *
                 lidUnit->area * lidUnit->number;
        lidList = lidList->nextLidUnit;
    }
    return depth / Subcatch[j].lidArea;
}

//=============================================================================

double lid_getPervArea(int j)
//
//  Purpose: retrieves amount of pervious LID area in a subcatchment.
//  Input:   j = subcatchment index
//  Output:  returns amount of pervious LID area (ft2)
//
{
    if ( LidGroups[j] ) return LidGroups[j]->pervArea;
    else return 0.0;
}

//=============================================================================

double   lid_getFlowToPerv(int j)
//
//  Purpose: retrieves flow returned from LID treatment to pervious area of
//           a subcatchment.
//  Input:   j = subcatchment index
//  Output:  returns flow returned to pervious area (cfs)
//
{
    if ( LidGroups[j] ) return LidGroups[j]->flowToPerv;
    else return 0.0;
}

//=============================================================================

double lid_getStoredVolume(int j)
//
//  Purpose: computes stored volume of water for all LIDs 
//           grouped within a subcatchment.
//  Input:   j = subcatchment index 
//  Output:  returns stored volume of water (ft3)
//
{
    double total = 0.0;
    TLidUnit*  lidUnit;
    TLidList*  lidList;
    TLidGroup  lidGroup;

    lidGroup = LidGroups[j];
    if ( lidGroup == NULL || Subcatch[j].lidArea == 0.0 ) return 0.0;
    lidList = lidGroup->lidList;
    while ( lidList )
    {
        lidUnit = lidList->lidUnit;
        total += lidUnit->waterBalance.finalVol * lidUnit->area * lidUnit->number;
        lidList = lidList->nextLidUnit;
    }
    return total;
}

//=============================================================================

double lid_getDepthOnPavement(int j, double impervDepth)
//
//  Purpose: computes average stored depth of water over a subcatchment's
//           impervious area and the area of any porous pavement LIDs.
//  Input:   j = subcatchment index
//           impervDepth = ponded depth on impervious area (ft)
//  Output:  returns depth of stored water (ft)
//
{
    int k;
    double     pondedLidDepth = 0.0;
    double     pondedLidArea = 0.0;
    double     impervArea;
    TLidUnit*  lidUnit;
    TLidList*  lidList;
    TLidGroup  lidGroup;

    //... check that subcatchment has LIDs
    lidGroup = LidGroups[j];
    if ( lidGroup == NULL ) return impervDepth;
    lidList = lidGroup->lidList;

    //... examine each LID unit
    while ( lidList )
    {
        lidUnit = lidList->lidUnit;
        k = lidUnit->lidIndex;

        //... check for porous pavement LID
        if ( LidProcs[k].lidType == POROUS_PAVEMENT )
        {
            //... add to total LID surface depth and area
            pondedLidDepth += lidUnit->surfaceDepth * lidUnit->area *
                              lidUnit->number;
            pondedLidArea += lidUnit->area * lidUnit->number;
        }
        lidList = lidList->nextLidUnit;
    }

    //... combine depth over non-LID impervious area with that over LID area
    impervArea = Subcatch[j].fracImperv *
                 (Subcatch[j].area - Subcatch[j].lidArea);
    return (impervDepth*impervArea + pondedLidDepth) /
           (impervArea + pondedLidArea);
}

//=============================================================================

double lid_getRunoff(int j, double *outflow, double *evapVol,
                     double *pervEvapVol, double *infilVol, double tStep)
//
//  Purpose: computes total runoff from all LIDs placed in a subcatchment.
//  Input:   j        = subcatchment index 
//           outflow  = subcatchment outflow (cfs)
//           evapVol  = volume of water evaporated (ft3)
//           pervEvapVol = evaporation volume over pervious areas (ft3)
//           infilVol = volume of water infiltrated (ft3)
//           tStep    = time step (sec)
//  Output:  returns total runoff rate (cfs)
//
//
{
    TLidList*  lidList;
    double qImperv;                    // LID inflow from impervious areas (cfs)
    double nonLidOutflow;              // total outflow from non-LID area (cfs)
    double lidOutflow;                 // total outflow from LID area (cfs)
    double flowToPerv;                 // return flow to pervious area (cfs)
    DateTime theDateTime;

    //... return current subcatchment outflow if there are no LID's
    theSubcatch = &Subcatch[j];
    theLidGroup = LidGroups[j];
    if ( !theLidGroup ) return *outflow;
    lidList = theLidGroup->lidList;
    if ( !lidList ) return *outflow;

    //... initialize outflows
    flowToPerv = 0.0;
    lidOutflow = 0.0;
    nonLidOutflow = *outflow;

    //... save subcatchment evap and infil. volumes
    TotalEvapVol = *evapVol;
    TotalPervEvapVol = *pervEvapVol;
    TotalInfilVol = *infilVol;
    Tstep = tStep;

    //... compute evaporation rate
    EvapRate = Evap.rate;
    if ( Evap.dryOnly && Subcatch[j].rainfall > 0.0 ) EvapRate = 0.0;

    //... find subcatchment's infiltration rate into native soil               //(5.0.022-LR)
    findNativeInfil(j, *infilVol, tStep);

    //... update time since last rainfall
    if ( theSubcatch->rainfall > MIN_RUNOFF) theLidGroup->dryTime = 0.0;
    else theLidGroup->dryTime += tStep;

    //... get date and time for reporting purposes
    theDateTime = getDateTime(NewRunoffTime);
    datetime_dateToStr(theDateTime, theDate);
    datetime_timeToStr(theDateTime, theTime);

    //... get inflows from non-LID subareas of subcatchment (cfs)
    qImperv = getImpervAreaInflow() * (Subcatch[j].area - Subcatch[j].lidArea);
    theLidGroup->impervRunoff += qImperv * tStep;

    //... evaluate effect of each LID unit placed in the subcatchment
    while ( lidList )
    {
        theLidUnit = lidList->lidUnit;
        evalLidUnit(qImperv, &lidOutflow, &nonLidOutflow, &flowToPerv);
        lidList = lidList->nextLidUnit;
    }

    //... update runoff flow that leaves the subcatchment (in cfs)
    theLidGroup->flowToPerv = flowToPerv;
    *outflow = nonLidOutflow + lidOutflow - theLidGroup->flowToPerv;

    //... update moisture losses
    *evapVol = TotalEvapVol;
    *pervEvapVol = TotalPervEvapVol;
    *infilVol = TotalInfilVol;

    //... return just the runoff from the LID area
    return lidOutflow;
}

//=============================================================================

////  This function was re-written for release 5.0.022  ////                   //(5.0.022-LR)

void findNativeInfil(int j, double infilVol, double tStep)
//
//  Purpose: determines a subcatchment's current infiltration rate into
//           its native soil.
//  Input:   j = subcatchment index
//           infilVol = infil. volume for non-LID pervious sub-area (ft3)
//           tStep    = time step (sec)
//  Output:  sets values for module-level variables NativeInfil & MaxNativeInfil
//
{
    double nonLidArea;

    //... subcatch. has non-LID pervious area so its infil. was already computed
    nonLidArea = Subcatch[j].area - Subcatch[j].lidArea;
    if ( nonLidArea > 0.0 && Subcatch[j].fracImperv < 1.0 )
    {
        NativeInfil = infilVol / nonLidArea / tStep;
    }

    //... otherwise find infil. rate for the subcatchment's rainfall + runon
    else
    {
        NativeInfil = infil_getInfil(j, InfilModel, tStep, Subcatch[j].rainfall,
                                     Subcatch[j].runon, lid_getSurfaceDepth(j));
    }

    //... see if there is any groundwater-imposed limit on infil.
    if ( !IgnoreGwater && Subcatch[j].groundwater )
    {
        MaxNativeInfil = Subcatch[j].groundwater->maxInfilVol / tStep;
    }
    else MaxNativeInfil = BIG;
}

//=============================================================================

double getImpervAreaInflow()
//
//  Purpose: computes runoff sent by impervious sub-areas into LID unit.
//  Input:   none
//  Output:  returns runoff flow (ft/sec)
//
{
    int    i;
    double q = 0.0;

    // --- runoff from impervious area w/ & w/o depression storage
    for (i = IMPERV0; i <= IMPERV1; i++)
    {
        q += theSubcatch->subArea[i].runoff * theSubcatch->subArea[i].fArea;
    }

    // --- adjust for any fraction of runoff sent to pervious area
    if ( theSubcatch->subArea[IMPERV0].routeTo == TO_PERV &&
         theSubcatch->fracImperv < 1.0 )
            q *= theSubcatch->subArea[IMPERV0].fOutlet;
    return q;
}

//=============================================================================

void evalLidUnit(double qImperv, double *lidOutflow, double *nonLidOutflow,
                 double *flowToPerv)
//
//  Purpose: evaluates performance of current LID unit over current time step.
//  Input:   qImperv = inflow from non-LID impervious area (cfs)
//  Output:  updates:
//           lidOutflow = total outflow from all LID areas (cfs)
//           nonLidOutflow = total outflow from all non-LID area (cfs)
//           flowToPerv = flow returned to non-LID pervious area (cfs)
//
{
    double inflow, outflow;

    //... return if the LID has no area
    LidUnitArea = theLidUnit->area * theLidUnit->number;
    if ( LidUnitArea <= 0.0 ) return;

    //... identify the LID process of the LID unit being analyzed
    theLidProc = &LidProcs[theLidUnit->lidIndex];

    //... initialize evap and infil losses
    LidEvapVol = 0.0;
    LidInfilVol = 0.0;

    //... get inflow to LID unit from non-LID area runoff (cfs)
    //    (reduce non-Lid outflow by this amount)
    inflow = qImperv * theLidUnit->fromImperv;
    *nonLidOutflow -= inflow;

    //... find total inflow to LID unit from all sources (ft/s)
    SurfaceInflow = (inflow / LidUnitArea) + theSubcatch->runon +
                    theSubcatch->rainfall;

    //... outflow from the LID unit (in cfs)
    outflow = getLidOutflow() * LidUnitArea;
    *lidOutflow += outflow;

    //... update flow returned to pervious area & moisture losses (ft3)
    *flowToPerv += theLidUnit->toPerv * outflow;
    TotalEvapVol  += LidEvapVol;
    TotalInfilVol += LidInfilVol;
    if ( isLidPervious(theLidUnit->lidIndex) )
    {
        TotalPervEvapVol += LidEvapVol;
    }

    //... update water balance and save results
    saveResults();
}

//=============================================================================

double getLidOutflow()
//
//  Purpose: computes runoff outflow from a single LID unit.
//  Input:   none
//  Output:  returns runoff outflow (ft/s)
//
{
    int    i;
    double x[3];        // layer moisture levels
    double xOld[3];     // work vector 
    double xPrev[3];    // work vector
    double xMin[3];     // lower limit on moisture levels
    double xMax[3];     // upper limit on moisture levels
    double fOld[3];     // previously computed flux rates
    double f[3];        // newly computed flux rates
    double xTol[] =     // convergence tolerance on moisture levels
        {STOPTOL, STOPTOL, STOPTOL};  //(ft, moisture fraction , ft)

    //... pointer to function that computes flux rates through the LID
    void (*fluxRates) (double *, double *) = NULL;

    //... store current moisture levels in vector x
    x[SURF] = theLidUnit->surfaceDepth;
    x[SOIL] = theLidUnit->soilMoisture;
    x[STOR] = theLidUnit->storageDepth;

    //... initialize layer flux rates and moisture limits
    SurfaceInfil   = 0.0;
    SurfaceEvap    = 0.0;
    SurfaceOutflow = 0.0;
    SoilEvap       = 0.0;
    SoilPerc       = 0.0;
    StorageInflow  = 0.0;
    StorageInfil   = 0.0;
    StorageEvap    = 0.0;
    StorageOutflow = 0.0;
    for (i = 0; i < 3; i++)
    {
        f[i] = 0.0;
        fOld[i] = theLidUnit->oldFluxRates[i];
        xMin[i] = 0.0;
        xMax[i] = BIG;
    }

////  The following code segment was added for release 5.0.022  ////           //(5.0.022-LR)
    //... find Green-Ampt infiltration from surface layer
    if ( theLidUnit->soilInfil.Ks > 0.0 )
    {

        SurfaceInfil =
            grnampt_getInfil(&theLidUnit->soilInfil, Tstep,
                             SurfaceInflow, theLidUnit->surfaceDepth);
    }
    else SurfaceInfil = NativeInfil;

    //... set moisture limits for soil & storage layers
    if ( theLidProc->soil.thickness > 0.0 )
    {
        xMin[SOIL] = theLidProc->soil.wiltPoint;
        xMax[SOIL] = theLidProc->soil.porosity;
    }
    if ( theLidProc->pavement.thickness > 0.0 )
    {
        xMax[SOIL] = theLidProc->pavement.voidFrac;
    }
    if ( theLidProc->storage.thickness > 0.0 )
    {
        xMax[STOR] = theLidProc->storage.thickness;
    }

    //... determine which flux rate function to use
    switch (theLidProc->lidType)
    {
    case BIO_CELL:        fluxRates = &biocellFluxRates;  break;
    case INFIL_TRENCH:    fluxRates = &trenchFluxRates;   break;
    case POROUS_PAVEMENT: fluxRates = &pavementFluxRates; break;
    case RAIN_BARREL:     fluxRates = &barrelFluxRates;   break;
    case VEG_SWALE:       fluxRates = &swaleFluxRates;    break;
    default:              return 0.0;
    }

    //... update moisture levels and flux rates over the time step
    i = modpuls_solve(3, x, xOld, xPrev, xMin, xMax, xTol, fOld, f,
                      Tstep, fluxRates);

    if  (i == 0) 
    {
/** For debugging only ********************************************
        fprintf(Frpt.file,
        "\n  WARNING 09: integration failed to converge at %s %s",
            theDate, theTime);
        fprintf(Frpt.file,
        "\n              for LID %s placed in subcatchment %s.",
            theLidProc->ID, theSubcatch->ID);
*******************************************************************/
    }

    //... add any surface overflow to surface outflow
    if ( theLidProc->surface.canOverflow || theLidUnit->width == 0.0 )         //(5.0.022-LR)
    {
        SurfaceOutflow += getSurfaceOverflow(&x[SURF]);
    }

    //... save updated results
    theLidUnit->surfaceDepth = x[SURF];
    theLidUnit->soilMoisture = x[SOIL];
    theLidUnit->storageDepth = x[STOR];
    for (i = 0; i < 3; i++) theLidUnit->oldFluxRates[i] = f[i];
//    theLidUnit->soilInfil = TmpSoilInfil;                                    //(5.0.022-LR)
//    theLidUnit->nativeInfil = TmpNativeInfil;                                //(5.0.022-LR)

    //... update losses
    LidEvapVol = (SurfaceEvap + SoilEvap + StorageEvap) * Tstep * LidUnitArea;
    LidInfilVol = StorageInfil * Tstep * LidUnitArea;

    //... return total outflow (per unit area) from unit
    return SurfaceOutflow + StorageOutflow;
}    

//=============================================================================

////  This function was modified for release 5.0.022  ////                     //(5.0.022-LR)

double getSoilInfilRate(double theta)
//
//  Purpose: limits the infiltration rate between surface and soil layers of
//           a bio-retention cell LID to available pore volume.
//  Input:   theta = moisture content of soil layer (fraction)
//  Output:  returns modified infiltration rate (ft/s)
//
{
    double maxValue;
    maxValue = (theLidProc->soil.porosity - theta) *
               theLidProc->soil.thickness / Tstep;
    maxValue = MAX(0.0, maxValue);
    return MIN(SurfaceInfil, maxValue);
}

//=============================================================================

double getSurfaceOutflow(double depth)
//
//  Purpose: computes outflow rate from a LID's surface layer.
//  Input:   depth = depth of ponded water on surface layer (ft)
//  Output:  returns outflow from surface layer (ft/s)
//
//  Note: this function should not be applied to swales or rain barrels.
//
{
    double delta;
    double outflow;

    //... no outflow if ponded depth below storage depth
    delta = depth - theLidProc->surface.thickness;
    if ( delta < 0.0 ) return 0.0;

    //... compute outflow from overland flow Manning equation
    outflow = theLidProc->surface.alpha * pow(delta, 5.0/3.0) *
              theLidUnit->width / theLidUnit->area;
    outflow = MIN(outflow, delta / Tstep);
    return outflow;
}

//=============================================================================

double getPavementPermRate()
//
//  Purpose: computes reduced permeability of a pavement layer due to
//           clogging.
//  Input:   none
//  Output:  returns the reduced permeability of the pavement layer (ft/s).
//
{
    double permRate;
    double permReduction;

    permReduction = theLidProc->pavement.clogFactor;
    if ( permReduction > 0.0 )
    {
        permReduction = theLidUnit->waterBalance.inflow / permReduction;
        permReduction = MIN(permReduction, 1.0);
    }
    permRate = theLidProc->pavement.kSat * (1.0 - permReduction);
    return permRate;
}

//=============================================================================
    
double getSoilPercRate(double theta, double storageDepth)
//
//  Purpose: computes percolation rate of water through a LID's soil layer.
//  Input:   theta = moisture content (fraction)
//           storageDepth = depth of water in storage layer (ft)
//  Output:  returns percolation rate within soil layer (ft/s)
//
{
    double percRate;         // percolation rate (ft/s)
    double delta;            // moisture deficit
    double maxValue;         // max. allowable perc. rate (ft/s)

    // ... max. drainable soil moisture 
    maxValue = (theta - theLidProc->soil.fieldCap) * 
               theLidProc->soil.thickness / Tstep;
    if ( maxValue <= 0.0 ) return 0.0;

    // ... perc rate = unsaturated hydraulic conductivity
    delta = theLidProc->soil.porosity - theta;
    percRate = theLidProc->soil.kSat * exp(-delta * theLidProc->soil.kSlope);

    //... rate limited by drainable moisture content
    percRate = MIN(percRate, maxValue);

    // ... perc rate limited by available storage zone capacity
    if ( theLidProc->storage.thickness > 0.0 )                                 //(5.0.022-LR)
    {
        maxValue = (theLidProc->storage.thickness - storageDepth) * 
                    theLidProc->storage.voidFrac / Tstep;
        maxValue = MAX(0.0, maxValue);
        percRate = MIN(percRate, maxValue);
    }
    return percRate;
}

//=============================================================================

////  This function was modified for release 5.0.022.  ////                    //(5.0.022-LR)

double getStorageInfilRate(double depth)
//
//  Purpose: computes infiltration rate between storage zone and
//           native soil beneath a LID.
//  Input:   depth = depth of water in storage zone (ft)
//  Output:  returns infiltration rate (ft/s)
//
{
    double infil = 0.0;
    double clogFactor = 0.0;
    double maxRate;

    if ( theLidProc->storage.kSat == 0.0 ) return 0.0;
    if ( MaxNativeInfil == 0.0 ) return 0.0;

    //... reduction 
    clogFactor = theLidProc->storage.clogFactor;
    if ( clogFactor > 0.0 )
    {
        clogFactor = theLidUnit->waterBalance.inflow / clogFactor;
        clogFactor = MIN(clogFactor, 1.0);
    }

    //... infiltration rate = storage Ksat reduced by any clogging
    infil = theLidProc->storage.kSat * (1.0 - clogFactor);

    //... limit infiltration rate by stored volume
    if ( theLidProc->storage.thickness > 0.0 )
    {
        maxRate = depth * theLidProc->storage.voidFrac / Tstep;
        infil = MIN(infil, maxRate);
    }

    //... limit infiltration rate by any groundwater-imposed limit
    return MIN(infil, MaxNativeInfil);
}

//=============================================================================

double  getStorageOutflow(double depth)
//
//  Purpose: computes outflow rate from underdrain in a LID's storage layer.
//  Input:   depth = depth of water in storage layer (ft)
//  Output:  returns outflow from storage layer (ft/s)
//
//  Note:    drain eqn. is evaluated in user's units.
{
    double maxValue;
    double outflow;
    double delta = depth - theLidProc->drain.offset;

    if ( delta <= ZERO ) outflow = 0.0;
    else
    {
        maxValue = delta * theLidProc->storage.voidFrac / Tstep;
        delta *= UCF(RAINDEPTH);
        outflow = theLidProc->drain.coeff *
                  pow(delta, theLidProc->drain.expon);
        outflow /= UCF(RAINFALL);
        outflow = MIN(outflow, maxValue);
    }
    return outflow;
}

//=============================================================================

void getEvapRates(double surfaceVol, double soilVol, double storageVol)
//
//  Purpose: computes surface, soil, and storage evaporation rates.
//  Input:   surfaceVol = volume of ponded water on surface layer (ft)
//           soilVol    = volume of water in soil (or pavement) pores (ft)
//           storageVol = volume of water in storage layer (ft)
//  Output:  none
//
{
    double availEvap;

    //... surface evaporation flux
    availEvap = EvapRate;
    SurfaceEvap = MIN(availEvap, surfaceVol/Tstep);
    SurfaceEvap = MAX(0.0, SurfaceEvap);
    availEvap = MAX(0.0, (availEvap - SurfaceEvap));
    
    //... soil evaporation flux
    SoilEvap = MIN(availEvap, soilVol / Tstep);
    availEvap = MAX(0.0, (availEvap - SoilEvap));
    
    //... storage evaporation flux
    StorageEvap = MIN(availEvap, storageVol / Tstep);
}

//=============================================================================

////  This function was modified for release 5.0.022.  ////                    //(5.0.022-LR)

void biocellFluxRates(double x[], double f[])
//
//  Purpose: computes flux rates from the layers of a bio-retention cell LID.
//  Input:   x = vector of storage levels
//  Output:  f = vector of flux rates
//
{
    double surfaceDepth;
    double soilTheta;
    double storageDepth;

    //... retrieve state variables from work vector
    surfaceDepth = x[SURF];
    soilTheta = x[SOIL];
    storageDepth = x[STOR];

    //... convert state variables to volumes
    SurfaceVolume = surfaceDepth * theLidProc->surface.voidFrac;
    SoilVolume = (soilTheta - theLidProc->soil.wiltPoint) *
                 theLidProc->soil.thickness;
    StorageVolume = storageDepth * theLidProc->storage.voidFrac;

    //... get ET rates
    getEvapRates(SurfaceVolume, SoilVolume, StorageVolume);

    //... find surface layer flux rate
    SurfaceInfil = getSoilInfilRate(soilTheta);
    SurfaceOutflow = getSurfaceOutflow(surfaceDepth);
    f[SURF] = (SurfaceInflow - SurfaceEvap - SurfaceInfil - SurfaceOutflow) /
              theLidProc->surface.voidFrac;

    //... find soil layer perc rate
    SoilPerc = getSoilPercRate(soilTheta, storageDepth);

    //... find storage layer flux rates
    StorageInfil = getStorageInfilRate(storageDepth);
    StorageOutflow = getStorageOutflow(storageDepth);

    //... make adjustments if no storage layer present
    if ( theLidProc->storage.thickness == 0.0 )
    {
        SoilPerc = MIN(SoilPerc, StorageInfil);
        StorageInfil = SoilPerc;
    }

    //... compute overall soil and storage layer flux rates
    f[SOIL] = (SurfaceInfil - SoilEvap - SoilPerc) /
              theLidProc->soil.thickness;
    f[STOR] = (SoilPerc - StorageEvap - StorageInfil - StorageOutflow) /
              theLidProc->storage.voidFrac;
}

//=============================================================================

void trenchFluxRates(double x[], double f[])
//
//  Purpose: computes flux rates from the layers of an infiltration trench LID.
//  Input:   x = vector of storage levels
//  Output:  f = vector of flux rates
//
{
    double surfaceDepth;
    double storageDepth;
    double maxValue;

    //... extract zone depth levels from work vector
    surfaceDepth = x[SURF];
    storageDepth = x[STOR];

    //... convert depths to volumes
    SurfaceVolume = surfaceDepth * theLidProc->surface.voidFrac;
    SoilVolume = 0.0;
    StorageVolume = storageDepth * theLidProc->storage.voidFrac;

    //... get ET rate loss for each zone 
    getEvapRates(SurfaceVolume, 0.0, StorageVolume);

    //... surface layer flux rate
    StorageInflow = SurfaceInflow + SurfaceVolume / Tstep;
    maxValue = (theLidProc->storage.thickness - storageDepth) *
               theLidProc->storage.voidFrac / Tstep;
    maxValue = MAX(0.0, maxValue);
    StorageInflow = MIN(StorageInflow, maxValue);
    SurfaceInfil = StorageInflow;
    SurfaceOutflow = getSurfaceOutflow(surfaceDepth);
    f[SURF] = SurfaceInflow - SurfaceEvap - StorageInflow - SurfaceOutflow;

    //... storage layer flux rate
    StorageInfil = getStorageInfilRate(storageDepth);
    StorageOutflow = getStorageOutflow(storageDepth);
    f[STOR] = (StorageInflow - StorageEvap - StorageInfil - StorageOutflow) /
              theLidProc->storage.voidFrac;
    f[SOIL] = 0.0;
}

//=============================================================================

////  This function was modified for release 5.0.022.  ////                    //(5.0.022-LR)

void pavementFluxRates(double x[], double f[])
//
//  Purpose: computes flux rates from the layers of a porous pavement LID.
//  Input:   x = vector of storage levels
//  Output:  f = vector of flux rates
//
{
    double surfaceDepth;     // depth of water stored on surface (ft)
    double pavementTheta;    // moisture content of pavement voids
    double pavementPerm;     // pavement permeability (ft/s)
    double storageDepth;     // depth of water in storage layer (ft)
    double pervVolume;       // volume/unit area of pervious pavement (ft)
    double maxValue;

    //... retrieve state variables from work vector
    surfaceDepth = x[SURF];
    pavementTheta = x[SOIL];
    storageDepth = x[STOR];

    //... convert state variables to volumes
    //    (SoilVolume refers to pavement layer)
    SurfaceVolume = surfaceDepth * theLidProc->surface.voidFrac;
    pervVolume = theLidProc->pavement.thickness *
                 (1.0 - theLidProc->pavement.impervFrac);
    SoilVolume = pavementTheta * pervVolume;
    StorageVolume = storageDepth * theLidProc->storage.voidFrac;

    //... get ET rates (arguments are stored volumes in ft)
    getEvapRates(SurfaceVolume, SoilVolume, StorageVolume);

    //... surface layer infiltration is smaller of pavement
    //    permeability, surface inflow + ponded depth, & 
    //    available pavement void space
    pavementPerm = getPavementPermRate();
    SurfaceInfil = pavementPerm;
    maxValue = (SurfaceInflow + SurfaceVolume / Tstep);
    SurfaceInfil = MIN(SurfaceInfil, maxValue);
    maxValue = (theLidProc->pavement.voidFrac - pavementTheta) *
               pervVolume / Tstep;
    SurfaceInfil = MIN(SurfaceInfil, maxValue);

    //... surface outflow
    SurfaceOutflow = getSurfaceOutflow(surfaceDepth);

    //... surface layer flux rate
    f[SURF] = SurfaceInflow - SurfaceEvap - SurfaceInfil - SurfaceOutflow;

    //... pavement (i.e., soil) layer percolation rate is smaller of
    //    permeability, current pavement layer volume and available
    //    storage layer volume
    maxValue = SoilVolume / Tstep;
    SoilPerc = MIN(pavementPerm, maxValue); 
    if ( theLidProc->storage.thickness > 0.0 )
    {
        maxValue = (theLidProc->storage.thickness - storageDepth) *
                    theLidProc->storage.voidFrac / Tstep;
        SoilPerc = MIN(SoilPerc, maxValue);
    }

    //... infiltration from storage layer to native soil
    StorageInfil = getStorageInfilRate(storageDepth);

    //... underdrain outflow from storage layer
    StorageOutflow = getStorageOutflow(storageDepth);

    //... adjustments for no storage layer
    if ( theLidProc->storage.thickness == 0.0 )
    {
        SoilPerc = MIN(SoilPerc, StorageInfil);
        StorageInfil = SoilPerc;
    }

    //... pavement (i.e., soil) layer flux rate
    f[SOIL] = (SurfaceInfil - SoilEvap - SoilPerc) / pervVolume;

    //... storage layer flux rate
    f[STOR] = (SoilPerc - StorageEvap - StorageInfil - StorageOutflow) /
              theLidProc->storage.voidFrac;
}

//=============================================================================

////  This function was modified for release 5.0.022.  ////                    //(5.0.022-LR)

void swaleFluxRates(double x[], double f[])
//
//  Purpose: computes flux rates from a vegetative swale LID.
//  Input:   x = vector of storage levels
//  Output:  f = vector of flux rates
//
{
    double depth;            // depth of surface water in swale (ft)
    double topWidth;         // top width of full swale (ft)
    double botWidth;         // bottom width of swale (ft)
    double length;           // length of swale (ft)
    double surfInflow;       // inflow rate to swale (cfs)
    double surfWidth;        // top width at current water depth (ft)
    double surfArea;         // surface area of current water depth (ft2)
    double flowArea;         // x-section flow area (ft2)
    double lidArea;          // surface area of full swale (ft2)
    double hydRadius;        // hydraulic radius for current depth (ft)
    double slope;            // slope of swale side wall (run/rise)
    double volume;           // swale volume at current water depth (ft3)
    double dVdT;             // change in volume w.r.t. time (cfs)
    double dStore;           // depression storage depth (ft)
    double xDepth;           // depth above depression storage (ft)
    

    //... retrieve state variable from work vector
    depth = x[SURF];
    depth = MIN(depth, theLidProc->surface.thickness);

    //... depression storage depth
    dStore = theSubcatch->subArea[PERV].dStore;

    //... get swale's bottom width
    //    (0.5 ft minimum to avoid numerical problems)
    slope = theLidProc->surface.sideSlope;
    topWidth = theLidUnit->width;
    topWidth = MAX(topWidth, 0.5);
    botWidth = topWidth - 2.0 * slope * theLidProc->surface.thickness;
    if ( botWidth < 0.5 )
    {
        botWidth = 0.5;
        slope = 0.5 * (topWidth - 0.5) / theLidProc->surface.thickness;
    }

    //... swale's length
    lidArea = theLidUnit->area;
    length = lidArea / topWidth;

    //... top width, surface area and flow area of current ponded depth
    surfWidth = botWidth + 2.0 * slope * depth;
    surfArea = length * surfWidth * theLidProc->surface.voidFrac;
    flowArea = (depth * (botWidth + slope * depth)) *
               theLidProc->surface.voidFrac;

    //... wet volume and effective depth
    volume = length * flowArea;

    //... surface inflow into swale (cfs)
    surfInflow = SurfaceInflow * lidArea;

    //... ET rate in cfs
    SurfaceEvap = EvapRate * surfArea;
    SurfaceEvap = MIN(SurfaceEvap, volume/Tstep);

    //... infiltration rate to native soil in cfs
    StorageInfil = SurfaceInfil * surfArea;


    //... no surface outflow if depth below depression storage
    xDepth = depth - dStore;
    if ( xDepth <= ZERO ) SurfaceOutflow = 0.0;

    //... otherwise compute a surface outflow
    else
    {
        //... modify flow area to remove depression storage,
        flowArea -= (dStore * (botWidth + slope * dStore)) *
                     theLidProc->surface.voidFrac;
        if ( flowArea < ZERO ) SurfaceOutflow = 0.0;
        else
        {
            //... compute hydraulic radius
            botWidth = botWidth + 2.0 * dStore * slope;
            hydRadius = botWidth + 2.0 * xDepth * sqrt(1.0 + slope*slope);
            hydRadius = flowArea / hydRadius * theLidProc->surface.voidFrac;

            //... use Manning Eqn. to find outflow rate in cfs
            SurfaceOutflow = theLidProc->surface.alpha * flowArea *
                             pow(hydRadius, 2./3.);
        }
    }

    //... net flux rate (dV/dt) in cfs 
    dVdT = surfInflow - SurfaceEvap - StorageInfil - SurfaceOutflow;

    //... when full, any net positive inflow becomes spillage
    if ( depth == theLidProc->surface.thickness && dVdT > 0.0 )
    {
        SurfaceOutflow += dVdT;
        dVdT = 0.0;
    }

    //... convert flux rates to ft/s
    SurfaceEvap /= lidArea;
    StorageInfil /= lidArea;
    SurfaceOutflow /= lidArea;
    f[SURF] = dVdT / surfArea;
    f[SOIL] = 0.0;
    f[STOR] = 0.0;

    //... assign values to layer volumes
    SurfaceVolume = volume / lidArea;
    SoilVolume = 0.0;
    StorageVolume = 0.0;
}

//=============================================================================

void barrelFluxRates(double x[], double f[])
//
//  Purpose: computes flux rates for a rain barrel LID.
//  Input:   x = vector of storage levels
//  Output:  f = vector of flux rates
//
{
    double storageDepth = x[STOR];
    double dryTime = theLidGroup->dryTime;
    double maxValue;

    //... initialize outflows
    SurfaceOutflow = 0.0;
    StorageOutflow = 0.0;
    
    //... compute outflow if time in dry state exceeds drain delay time
    if ( SurfaceInflow > MIN_RUNOFF ) dryTime = 0.0;
    else
    {
        if ( dryTime > theLidProc->drain.delay )
        {
            StorageOutflow = getStorageOutflow(storageDepth); 
        }
        dryTime += Tstep;
    }
    theLidGroup->dryTime = dryTime;

    //... storage inflow rate limited by reamining empty depth
    StorageInflow = SurfaceInflow;
    maxValue = (theLidProc->storage.thickness - storageDepth) / Tstep;
    maxValue = MAX(0.0, maxValue);
    StorageInflow = MIN(StorageInflow, maxValue);

    //... assign values to layer flux rates
    f[SURF] = SurfaceInflow - StorageInflow;
    f[STOR] = StorageInflow - StorageOutflow;
    f[SOIL] = 0.0;

    //... assign values to layer volumes
    SurfaceVolume = 0.0;
    SoilVolume = 0.0;
    StorageVolume = storageDepth;
}   

//=============================================================================

double getSurfaceOverflow(double* surfaceDepth)
//
//  Purpose: finds surface overflow rate from a LID unit.
//  Input:   surfaceDepth = depth of water stored in surface layer (ft)
//  Output:  returns the overflow rate (ft/s)
//
{
    double delta = *surfaceDepth - theLidProc->surface.thickness;
    if (  delta <= 0.0 ) return 0.0;
    *surfaceDepth = theLidProc->surface.thickness;
    return delta * theLidProc->surface.voidFrac / Tstep;
}

//=============================================================================

void initWaterBalance(TLidUnit *lidUnit, double initVol)
//
//  Purpose: initializes the water balance components of a LID unit.
//  Input:   lidUnit = a particular LID unit
//           initVol = initial water volume stored in the unit (ft)
//  Output:  none
//
{
    lidUnit->waterBalance.inflow = 0.0;
    lidUnit->waterBalance.evap = 0.0;
    lidUnit->waterBalance.infil = 0.0;
    lidUnit->waterBalance.surfFlow = 0.0;
    lidUnit->waterBalance.drainFlow = 0.0;
    lidUnit->waterBalance.initVol = initVol;
    lidUnit->waterBalance.finalVol = lidUnit->waterBalance.initVol;
}

//=============================================================================

void updateWaterBalance(TLidUnit *lidUnit, double inflow, double evap,
    double infil, double surfFlow, double drainFlow, double storage)
//
//  Purpose: updates components of the water mass balance for a LID unit
//           over the current time step.
//  Input:   lidUnit   = a particular LID unit
//           inflow    = runon + rainfall to the LID unit (ft/s)
//           evap      = evaporation rate from the unit (ft/s)
//           infil     = infiltration out the bottom of the unit (ft/s)
//           surfFlow  = surface runoff from the unit (ft/s)
//           drainFlow = underdrain flow from the unit
//           storage   = volume of water stored in the unit (ft)
//  Output:  none
//
{
    lidUnit->waterBalance.inflow += inflow * Tstep;
    lidUnit->waterBalance.evap += evap * Tstep;
    lidUnit->waterBalance.infil += infil * Tstep;
    lidUnit->waterBalance.surfFlow += surfFlow * Tstep;
    lidUnit->waterBalance.drainFlow += drainFlow * Tstep;
    lidUnit->waterBalance.finalVol = storage;
}

//=============================================================================

void lid_writeWaterBalance()
//
//  Purpose: writes a LID performance summary table to the project's report file.
//  Input:   none
//  Output:  none
//
{
    int        j;
    int        k = 0;
    double     ucf = UCF(RAINDEPTH);
    double     inflow;
    double     outflow;
    double     err;
    TLidUnit*  lidUnit;
    TLidList*  lidList;
    TLidGroup  lidGroup;

    //... check that project has LIDs
    for ( j = 0; j < GroupCount; j++ )
    {
        if ( LidGroups[j] ) k++;
    }
    if ( k == 0 ) return;

    //... write table header
    fprintf(Frpt.file,
    "\n"
    "\n  ***********************"
    "\n  LID Performance Summary"
    "\n  ***********************\n");


    fprintf(Frpt.file,
"\n  ------------------------------------------------------------------------------------------------------------------"
"\n                                         Total      Evap     Infil   Surface    Drain      Init.     Final     Pcnt."
"\n                                        Inflow      Loss      Loss   Outflow   Outflow   Storage   Storage     Error");
    if ( UnitSystem == US ) fprintf(Frpt.file, 
"\n  Subcatchment      LID Control             in        in        in        in        in        in        in");
    else fprintf(Frpt.file,
"\n  Subcatchment      LID Control             mm        mm        mm        mm        mm        mm        mm");
    fprintf(Frpt.file,
"\n  ------------------------------------------------------------------------------------------------------------------");

    //... examine each LID unit in each subcatchment
    for ( j = 0; j < GroupCount; j++ )
    {
        lidGroup = LidGroups[j];
        if ( !lidGroup || Subcatch[j].lidArea == 0.0 ) continue;
        lidList = lidGroup->lidList;
        while ( lidList )
        {
            //... write water balance components to report file
            lidUnit = lidList->lidUnit;
            k = lidUnit->lidIndex;
            fprintf(Frpt.file, "\n  %-16s  %-16s", Subcatch[j].ID,
                                                   LidProcs[k].ID);
            fprintf(Frpt.file, "%10.2f%10.2f%10.2f%10.2f%10.2f%10.2f%10.2f",
                    lidUnit->waterBalance.inflow*ucf,
                    lidUnit->waterBalance.evap*ucf,
                    lidUnit->waterBalance.infil*ucf,
                    lidUnit->waterBalance.surfFlow*ucf,
                    lidUnit->waterBalance.drainFlow*ucf,
                    lidUnit->waterBalance.initVol*ucf,
                    lidUnit->waterBalance.finalVol*ucf);

            //... compute flow balance error
            inflow = lidUnit->waterBalance.initVol + 
                     lidUnit->waterBalance.inflow;
            outflow = lidUnit->waterBalance.finalVol +
                      lidUnit->waterBalance.evap +
                      lidUnit->waterBalance.infil +
                      lidUnit->waterBalance.surfFlow +
                      lidUnit->waterBalance.drainFlow;
            if ( inflow > 0.0 ) err = (inflow - outflow) / inflow;
            else                err = 1.0;
            fprintf(Frpt.file, "%10.2f", err*100.0);
            lidList = lidList->nextLidUnit;
        }
    }
}

//=============================================================================

void initLidRptFile(char* id, TLidUnit*  lidUnit)
//
//  Purpose: initializes the report file used for a specific LID unit
//  Input:   id = subcatchment ID name
//           lidUnit = LID unit
//  Output:  none
//
{
    FILE* f = lidUnit->rptFile->file;
    int   k = lidUnit->lidIndex;

    //... check that file was opened
    if ( f ==  NULL ) return;

    //... write title lines
    fprintf(f, "SWMM5 LID Report File\n");
    fprintf(f, "\nProject:  %s", Title[0]);
    fprintf(f, "\nLID Unit: %s in Subcatchment %s\n", LidProcs[k].ID, id);

    //... write column headings
    fprintf(f, 
"\nElapsed\t    Total\t    Total\t  Surface\t     Soil\t   Bottom\t  Surface\t    Drain\t  Surface\t    Soil/\t  Storage" 
"\n   Time\t   Inflow\t     Evap\t    Infil\t     Perc\t    Infil\t   Runoff\t  Outflow\t    Depth\t    Pave \t    Depth");
fprintf(f, 
"\n  Hours\t");
    if ( UnitSystem == US ) fprintf(f,
"    in/hr\t    in/hr\t    in/hr\t    in/hr\t    in/hr\t    in/hr\t    in/hr\t   inches\t    Moist\t   inches");
    else fprintf(f,
"    mm/hr\t    mm/hr\t    mm/hr\t    mm/hr\t    mm/hr\t    mm/hr\t    mm/hr\t       mm\t    Moist\t       mm");
    fprintf(f,
"\n-------\t --------\t --------\t --------\t --------\t --------\t --------\t --------\t --------\t --------\t --------" );

    //... initialize next reporting time
    lidUnit->rptFile->nextReportTime = (double) (ReportStep * 1000.0);
}

//=============================================================================

void saveResults()
//
//  Purpose: updates the mass balance for the current LID unit and saves
//           current flux rates to the LID report file.
//  Input:   none
//  Output:  none
//
{
    int    i;
    int    rptStep;                    // reporting time step (sec) 
    double ucf;                        // units conversion factor
    double totalEvap;                  // total evaporation rate (ft/s)
    double totalVolume;                // total volume stored in LID (ft)
    double rptVars[MAX_RPT_VARS];      // array of reporting variables

    //... find total evap. rate and stored volume
    totalEvap = SurfaceEvap + SoilEvap + StorageEvap;
    totalVolume = SurfaceVolume + SoilVolume + StorageVolume;

    //... update mass balance totals
    updateWaterBalance(theLidUnit, SurfaceInflow, totalEvap, StorageInfil,
                       SurfaceOutflow, StorageOutflow, totalVolume); 

    //... write results to LID report file
    if ( theLidUnit->rptFile )
    {
        //... check if next reporting time not reached yet
        if ( NewRunoffTime < theLidUnit->rptFile->nextReportTime ) return;

        //... convert project's reporting time step from sec to msec
        rptStep = 1000 * ReportStep;

        //... advance next reporting time beyond current runoff time
        while (NewRunoffTime >= theLidUnit->rptFile->nextReportTime)
        {
            theLidUnit->rptFile->nextReportTime += rptStep;
        }

        //... check if dry-weather conditions hold
        if ( SurfaceInflow  < MINFLOW &&
             SurfaceOutflow < MINFLOW &&
             StorageOutflow < MINFLOW &&
             StorageInfil   < MINFLOW &&
             totalEvap      < MINFLOW ) return;

        //... convert rate results to original units (in/hr or mm/hr)
        ucf = UCF(RAINFALL);
        rptVars[SURF_INFLOW]  = SurfaceInflow*ucf;
        rptVars[TOTAL_EVAP]   = totalEvap*ucf;
        rptVars[SURF_INFIL]   = SurfaceInfil*ucf;
        rptVars[SOIL_PERC]    = SoilPerc*ucf;
        rptVars[STOR_INFIL]   = StorageInfil*ucf;
        rptVars[SURF_OUTFLOW] = SurfaceOutflow*ucf;
        rptVars[STOR_OUTFLOW] = StorageOutflow*ucf;

        //... convert storage results to original units (in or mm)
        ucf = UCF(RAINDEPTH);
        rptVars[SURF_DEPTH] = theLidUnit->surfaceDepth*ucf;
        rptVars[SOIL_MOIST] = theLidUnit->soilMoisture;
        rptVars[STOR_DEPTH] = theLidUnit->storageDepth*ucf;

        //... write results to file
        fprintf(theLidUnit->rptFile->file, "\n%7.3f\t",
                NewRunoffTime/1000.0/3600.0);
        for ( i = SURF_INFLOW; i <= STOR_OUTFLOW; i++)
                fprintf(theLidUnit->rptFile->file, " %8.2f\t", rptVars[i]);
        for ( i = SURF_DEPTH; i <= STOR_DEPTH; i++)
                fprintf(theLidUnit->rptFile->file, " %8.2f\t", rptVars[i]);
    }
}

//=============================================================================

int modpuls_solve(int n, double* x, double* xOld, double* xPrev,
                  double* xMin, double* xMax, double* xTol,
                  double* qOld, double* q, double dt,
                  void (*derivs)(double*, double*))
//
//  Purpose: solves system of equations dx/dt = q(x) for x at end of time step
//           dt using a modified Puls method.
//  Input:   n = number of state variables
//           x = vector of state variables
//           xOld = state variable values at start of time step
//           xPrev = state variable values from previous iteration
//           xMin = lower limits on state variables
//           xMax = upper limits on state variables
//           xTol = convergence tolerances on state variables
//           qOld = flux rates at start of time step
//           q = flux rates at end of time step
//           dt = time step (sec)
//           derivs = pointer to function that computes flux rates q as a
//                    function of state variables x
//  Output:  returns number of steps required for convergence (or 0 if 
//           process doesn't converge)
//
{
    int i;
    int canStop;
    int steps = 1;
    int maxSteps = 20;
    double OMEGA = 0.5;      // time-weighting parameter

    //... initialize state variable values
    for (i=0; i<n; i++) 
    {
        xOld[i] = x[i];
        xPrev[i] = x[i];
    }

    //... repeat until convergence achieved
    while (steps < maxSteps)
    {
        //... compute flux rates for current state levels 
        canStop = 1;
        derivs(x, q);

        //... update state levels based on current flux rates
        for (i=0; i<n; i++)
        {
            x[i] = xOld[i] + (OMEGA*qOld[i] + (1.0 - OMEGA)*q[i]) * dt;
            x[i] = MIN(x[i], xMax[i]);
            x[i] = MAX(x[i], xMin[i]);
            if (fabs(x[i] - xPrev[i]) > xTol[i]) canStop = 0;
            xPrev[i] = x[i];
        }

        //... return if process converges
        if (canStop) return steps;
        steps++;
    }

    //... no convergence so return 0
    return 0;
}
