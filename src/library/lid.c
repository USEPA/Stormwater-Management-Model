//-----------------------------------------------------------------------------
//   lid.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//             05/19/14   (Build 5.1.006)
//             09/15/14   (Build 5.1.007)
//             03/19/15   (Build 5.1.008)
//             04/30/15   (Build 5.1.009)
//             08/05/15   (Build 5.1.010)
//             08/01/16   (Build 5.1.011)
//             03/14/17   (Build 5.1.012)
//             05/10/18   (Build 5.1.013)
//   Author:   L. Rossman (US EPA)
//
//   This module handles all data processing involving LID (Low Impact
//   Development) practices used to treat runoff for individual subcatchments
//   within a project. The actual computation of LID performance is made by
//   functions within the lidproc.c module. See LidTypes below for the types
//   of LIDs that can be modeled.
//
//   An LID process is described by the TLidProc data structure and consists of
//   size-independent design data for the different vertical layers that make
//   up a specific type of LID. The collection of these LID process designs is
//   stored in the LidProcs array.
//
//   When a member of LidProcs is to be deployed in a particular subcatchment,
//   its sizing and treatment data are stored in a TLidUnit data structure.
//   The collection of all TLidUnits deployed in a subcatchment is held in a
//   TLidGroup list data structure. The LidGroups array contains a TLidGroup
//   list for each subcatchment in the project.
//
//   During a runoff time step, each subcatchment calls the lid_getRunoff()
//   function to compute flux rates and a water balance through each layer
//   of each LID unit in the subcatchment. The resulting outflows (runoff,
//   drain flow, evaporation and infiltration) are added to those computed
//   for the non-LID portion of the subcatchment.
//
//   An option exists for the detailed time series of flux rates and storage
//   levels for a specific LID unit to be written to a text file named by the
//   user for viewing outside of the SWMM program.
//
//   Build 5.1.008:
//   - More input error reporting added.
//   - Rooftop Disconnection added to the types of LIDs.
//   - LID drain flows are now tracked separately.
//   - LID drain flows can now be routed to separate outlets.
//   - Check added to insure LID flows not returned to nonexistent pervious area.
//
//   Build 5.1.009:
//   - Fixed bug where LID's could return outflow to non-LID area when LIDs
//     make up entire subcatchment.
//
//   Build 5.1.010:
//   - Support for new Modified Green Ampt infiltration model added.
//   - Imported variable HasWetLids now properly initialized.
//   - Initial state of reporting (lidUnit->rptFile->wasDry) changed to
//     prevent duplicate printing of first line of detailed report file.
//
//   Build 5.1.011:
//   - The top of the storage layer is no longer used as a limit for an
//     underdrain offset thus allowing upturned drains to be modeled.
//   - Column headings for the detailed LID report file were modified.
//
//   Build 5.1.012:
//   - Redefined initialization of wasDry for LID reporting.
//
//   Build 5.1.013:
//   - Support added for LID units treating pervious area runoff.
//   - Support added for open/closed head levels and multiplier v. head 
//     control curve for underdrain flow.
//   - Support added for unclogging permeable pavement at fixed intervals.
//   - Support added for pollutant removal in underdrain flow.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include "headers.h"
#include "lid.h"

#define ERR_PAVE_LAYER " - check pavement layer parameters"
#define ERR_SOIL_LAYER " - check soil layer parameters"
#define ERR_STOR_LAYER " - check storage layer parameters"
#define ERR_SWALE_SURF " - check swale surface parameters"
#define ERR_GREEN_AMPT " - check subcatchment Green-Ampt parameters"
#define ERR_DRAIN_OFFSET " - drain offset exceeds storage height"
#define ERR_DRAIN_HEADS " - invalid drain open/closed heads"                   //(5.1.013)
#define ERR_SWALE_WIDTH " - invalid swale width"

//-----------------------------------------------------------------------------
//  Enumerations
//-----------------------------------------------------------------------------
enum LidLayerTypes {
    SURF,                    // surface layer
    SOIL,                    // soil layer
    STOR,                    // storage layer
    PAVE,                    // pavement layer
    DRAINMAT,                // drainage mat layer
    DRAIN,                   // underdrain system
    REMOVALS};               // pollutant removals                             //(5.1.013)

//// Note: DRAINMAT must be placed before DRAIN so the two keywords can
///        be distinguished from one another when parsing a line of input. 

char* LidLayerWords[] =
    {"SURFACE", "SOIL", "STORAGE", "PAVEMENT", "DRAINMAT", "DRAIN",
     "REMOVALS", NULL};                                                        //(5.1.013)

char* LidTypeWords[] =
    {"BC",                   //bio-retention cell
     "RG",                   //rain garden
     "GR",                   //green roof
     "IT",                   //infiltration trench
     "PP",                   //porous pavement
     "RB",                   //rain barrel
     "VS",                   //vegetative swale
     "RD",                   //rooftop disconnection
     NULL};

//-----------------------------------------------------------------------------
//  Data Structures
//-----------------------------------------------------------------------------

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
    double         oldDrainFlow;  // total drain flow in previous period (cfs)
    double         newDrainFlow;  // total drain flow in current period (cfs)
    TLidList*      lidList;       // list of LID units in the group
};
typedef struct LidGroup* TLidGroup;


//-----------------------------------------------------------------------------
//  Shared Variables
//-----------------------------------------------------------------------------
static TLidProc*  LidProcs;            // array of LID processes
static int        LidCount;            // number of LID processes
static TLidGroup* LidGroups;           // array of LID process groups
static int        GroupCount;          // number of LID groups (subcatchments)

static double     EvapRate;            // evaporation rate (ft/s)
static double     NativeInfil;         // native soil infil. rate (ft/s)
static double     MaxNativeInfil;      // native soil infil. rate limit (ft/s)

//-----------------------------------------------------------------------------
//  Imported Variables (from SUBCATCH.C)
//-----------------------------------------------------------------------------
// Volumes (ft3) for a subcatchment over a time step 
extern double     Vevap;               // evaporation
extern double     Vpevap;              // pervious area evaporation
extern double     Vinfil;              // non-LID infiltration
extern double     VlidInfil;           // infiltration from LID units
extern double     VlidIn;              // impervious area flow to LID units
extern double     VlidOut;             // surface outflow from LID units
extern double     VlidDrain;           // drain outflow from LID units
extern double     VlidReturn;          // LID outflow returned to pervious area
extern char       HasWetLids;          // TRUE if any LIDs are wet
                                       // (from RUNOFF.C)

//-----------------------------------------------------------------------------
//  External Functions (prototyped in lid.h)
//-----------------------------------------------------------------------------
//  lid_create               called by createObjects in project.c
//  lid_delete               called by deleteObjects in project.c
//  lid_validate             called by project_validate
//  lid_initState            called by project_init

//  lid_readProcParams       called by parseLine in input.c
//  lid_readGroupParams      called by parseLine in input.c

//  lid_setOldGroupState     called by subcatch_setOldState
//  lid_setReturnQual        called by findLidLoads in surfqual.c
//  lid_getReturnQual        called by subcatch_getRunon

//  lid_getPervArea          called by subcatch_getFracPerv
//  lid_getFlowToPerv        called by subcatch_getRunon
//  lid_getSurfaceDepth      called by subcatch_getDepth
//  lid_getDepthOnPavement   called by sweptSurfacesDry in subcatch.c
//  lid_getStoredVolume      called by subcatch_getStorage
//  lid_getRunon             called by subcatch_getRunon
//  lid_getRunoff            called by subcatch_getRunoff

//  lid_addDrainRunon        called by subcatch_getRunon
//  lid_addDrainLoads        called by surfqual_getWashoff
//  lid_addDrainInflow       called by addLidDrainInflows in routing.c

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
static int    readDrainMatData(int j, char* toks[], int ntoks);
static int    readRemovalsData(int j, char* toks[], int ntoks);                //(5.1.013)

static int    addLidUnit(int j, int k, int n, double x[], char* fname,
              int drainSubcatch, int drainNode);
static int    createLidRptFile(TLidUnit* lidUnit, char* fname);
static void   initLidRptFile(char* title, char* lidID, char* subcatchID,
              TLidUnit* lidUnit);
static void   validateLidProc(int j);
static void   validateLidGroup(int j);

static int    isLidPervious(int k);
static double getImpervAreaRunoff(int j);
static double getPervAreaRunoff(int j);                                        //(5.1.013)
static double getSurfaceDepth(int subcatch);
static void   findNativeInfil(int j, double tStep);

static void   evalLidUnit(int j, TLidUnit* lidUnit, double lidArea,
              double lidInflow, double tStep, double *qRunoff,
              double *qDrain, double *qReturn);

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
    LidCount = lidCount;

    //... create LID groups
    GroupCount = subcatchCount;
    if ( GroupCount == 0 ) return;
    LidGroups = (TLidGroup *) calloc(GroupCount, sizeof(TLidGroup));
    if ( LidGroups == NULL )
    {
        ErrorCode = ERR_MEMORY;
        return;
    }

    //... initialize LID groups
    for (j = 0; j < GroupCount; j++) LidGroups[j] = NULL;

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
        LidProcs[j].drainMat.thickness = 0.0;
        LidProcs[j].drainMat.roughness = 0.0;
        LidProcs[j].drainRmvl = NULL;                                          //(5.1.013)
        LidProcs[j].drainRmvl = (double *)                                     //
                                calloc(Nobjects[POLLUT], sizeof(double));      //
        if (LidProcs[j].drainRmvl == NULL)                                     //
        {                                                                      //
            ErrorCode = ERR_MEMORY;                                            //
            return;                                                            //
        }                                                                      // 
    }
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
    for (j = 0; j < LidCount; j++) FREE(LidProcs[j].drainRmvl);                //(5.1.013)
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
//    LID_ID  DRAINMAT  <parameters>
//    LID_ID  REMOVALS  <parameters>                                           //(5.1.013)
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
    case DRAINMAT: return readDrainMatData(j, toks, ntoks);
    case REMOVALS: return readRemovalsData(j, toks, ntoks);                    //(5.1.013)
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
//    Subcatch_ID  LID_ID  Number  Area  Width  InitSat  FromImp  ToPerv
//                                              (RptFile  DrainTo  FromPerv)   //(5.1.013)
//  where:
//    Subcatch_ID    = name of subcatchment
//    LID_ID         = name of LID process
//    Number     (n) = number of replicate units
//    Area    (x[0]) = area of each unit
//    Width   (x[1]) = outflow width of each unit
//    InitSat (x[2]) = % that LID is initially saturated
//    FromImp (x[3]) = % of impervious runoff sent to LID
//    ToPerv  (x[4]) = 1 if outflow goes to pervious sub-area; 0 if not
//    RptFile        = name of detailed results file (optional)
//    DrainTo        = name of subcatch/node for drain flow (optional)
//    FromPerv (x[5]) = % of pervious runoff sent to LID                       //(5.1.013)
//
{
    int        i, j, k, n;
    double     x[6];                                                           //(5.1.013)
    char*      fname = NULL;
    int        drainSubcatch = -1, drainNode = -1;

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
    if ( ntoks >= 9 && strcmp(toks[8], "*") != 0 ) fname = toks[8];

    //... read optional underdrain outlet
    if ( ntoks >= 10 && strcmp(toks[9], "*") != 0 )
    {
        drainSubcatch = project_findObject(SUBCATCH, toks[9]);
        if ( drainSubcatch < 0 )
        {
            drainNode = project_findObject(NODE, toks[9]);
            if ( drainNode < 0 ) return error_setInpError(ERR_NAME, toks[9]);
        }
    }

    //... read percent of pervious area treated by LID unit                    //(5.1.013)
    x[5] = 0.0;                                                                //
    if (ntoks >= 11)                                                           //
    {                                                                          //
        if (!getDouble(toks[10], &x[5]) || x[5] < 0.0 || x[5] > 100.0)         //
            return error_setInpError(ERR_NUMBER, toks[10]);                    //
    }                                                                          //

    //... create a new LID unit and add it to the subcatchment's LID group
    return addLidUnit(j, k, n, x, fname, drainSubcatch, drainNode);
}

//=============================================================================

int addLidUnit(int j, int k, int n, double x[], char* fname,
    int drainSubcatch, int drainNode)
//
//  Purpose: adds an LID unit to a subcatchment's LID group.
//  Input:   j = subcatchment index
//           k = LID control index
//           n = number of replicate units
//           x = LID unit's parameters
//           fname = name of detailed performance report file
//           drainSubcatch = index of subcatchment receiving underdrain flow
//           drainNode = index of node receiving underdrain flow
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
    lidUnit->fullWidth    = x[1] / UCF(LENGTH);
    lidUnit->initSat      = x[2] / 100.0;
    lidUnit->fromImperv   = x[3] / 100.0;
    lidUnit->toPerv       = (x[4] > 0.0);
    lidUnit->fromPerv     = x[5] / 100.0;                                      //(5.1.013)
    lidUnit->drainSubcatch = drainSubcatch;
    lidUnit->drainNode     = drainNode;

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
//  LID_ID  SURFACE  StorageHt  VegVolFrac  Roughness  SurfSlope  SideSlope  DamHt
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
    if ( x[1] >= 1.0 ) return error_setInpError(ERR_NUMBER, toks[3]);           
    if ( x[0] == 0.0 ) x[1] = 0.0;

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
//                                                        (RegenDays RegenDegree) //(5.1.013)
//
{
    int    i;
    double x[7];                                                               //(5.1.013)

    if ( ntoks < 7 ) return error_setInpError(ERR_ITEMS, "");
    for (i = 2; i < 7; i++)
    {
        if ( ! getDouble(toks[i], &x[i-2]) || x[i-2] < 0.0 )
            return error_setInpError(ERR_NUMBER, toks[i]);
    }

    // ... read optional clogging regeneration properties                      //(5.1.013)
    x[5] = 0.0;                                                                //
    if (ntoks > 7)                                                             //
    {                                                                          //
        if (!getDouble(toks[7], &x[5]) || x[5] < 0.0)                          //
            return error_setInpError(ERR_NUMBER, toks[7]);                     //
    }                                                                          //
    x[6] = 0.0;                                                                //
    if (ntoks > 8)                                                             //
    {                                                                          //
        if (!getDouble(toks[8], &x[6]) || x[6] < 0.0 || x[6] > 1.0)            //
            return error_setInpError(ERR_NUMBER, toks[8]);                     //
    }                                                                          //

    //... convert void ratio to void fraction
    x[1] = x[1]/(x[1] + 1.0);

    LidProcs[j].pavement.thickness    = x[0] / UCF(RAINDEPTH);
    LidProcs[j].pavement.voidFrac     = x[1];
    LidProcs[j].pavement.impervFrac   = x[2];
    LidProcs[j].pavement.kSat         = x[3] / UCF(RAINFALL);
    LidProcs[j].pavement.clogFactor   = x[4];
    LidProcs[j].pavement.regenDays    = x[5];                                  //(5.1.013)
    LidProcs[j].pavement.regenDegree  = x[6];                                  //
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
//    LID_ID  SOIL  Thickness  Porosity  FieldCap  WiltPt Ksat  Kslope  Suction
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
    double x[6];

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
//    LID_ID DRAIN  coeff  expon  offset  delay hOpen hClose curve             //(5.1.013)
//
{
    int    i;
    double x[6];                                                               //(5.1.013)

    //... read numerical parameters
    if ( ntoks < 6 ) return error_setInpError(ERR_ITEMS, "");
    for (i = 0; i < 6; i++) x[i] = 0.0;                                        //(5.1.013)
    for (i = 2; i < 8; i++)                                                    //
    {
        if ( ntoks > i && ! getDouble(toks[i], &x[i-2])  || x[i-2] < 0.0 )     //(5.1.013)
            return error_setInpError(ERR_NUMBER, toks[i]);
    }

    i = -1;                                                                    //(5.1.013)
    if ( ntoks >= 9 )                                                          //
    {                                                                          //
        i = project_findObject(CURVE, toks[8]);                                //
        if (i < 0) return error_setInpError(ERR_NAME, toks[8]);                //
    }                                                                          //

    //... save parameters to LID drain layer structure
    LidProcs[j].drain.coeff  = x[0];
    LidProcs[j].drain.expon  = x[1];
    LidProcs[j].drain.offset = x[2] / UCF(RAINDEPTH);
    LidProcs[j].drain.delay  = x[3] * 3600.0;
    LidProcs[j].drain.hOpen  = x[4] / UCF(RAINDEPTH);                          //(5.1.013)
    LidProcs[j].drain.hClose = x[5] / UCF(RAINDEPTH);                          //
    LidProcs[j].drain.qCurve = i;                                              //
    return 0;
}
 
//=============================================================================

int readDrainMatData(int j, char* toks[], int ntoks)
//
//  Purpose: reads drainage mat data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID DRAINMAT  thickness  voidRatio  roughness
//
{
    int    i;
    double x[3];

    //... read numerical parameters
    if ( ntoks < 5 ) return error_setInpError(ERR_ITEMS, "");
	if ( LidProcs[j].lidType != GREEN_ROOF ) return 0;
    for (i = 2; i < 5; i++)
    {
        if ( ! getDouble(toks[i], &x[i-2]) || x[i-2] < 0.0 )
            return error_setInpError(ERR_NUMBER, toks[i]);
    }

    //... save parameters to LID drain layer structure
    LidProcs[j].drainMat.thickness = x[0] / UCF(RAINDEPTH);;
    LidProcs[j].drainMat.voidFrac  = x[1];
    LidProcs[j].drainMat.roughness = x[2];
    return 0;
}

//=============================================================================

////  This function was added to release 5.1.013.  ////                        //(5.1.013)

int readRemovalsData(int j, char* toks[], int ntoks)
//
//  Purpose: reads pollutant removal data for a LID process from line of input
//           data file
//  Input:   j = LID process index 
//           toks = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//
//  Format of data is:
//    LID_ID REMOVALS  pollut1  %removal1  pollut2  %removal2  ...
//
{
    int    i = 2;
    int    p;
    double rmvl;

    //... start with 3rd token
    if (ntoks < 4) return error_setInpError(ERR_ITEMS, "");
    while (ntoks > i)
    {
        //... find pollutant index from its name
        p = project_findObject(POLLUT, toks[i]);
        if (p < 0) return error_setInpError(ERR_NAME, toks[i]);

        //... check that a next token exists
        i++;
        if (ntoks == i) return error_setInpError(ERR_ITEMS, "");

        //... get the % removal value from the next token
        if (!getDouble(toks[i], &rmvl) || rmvl < 0.0 || rmvl > 100.0)
            return error_setInpError(ERR_NUMBER, toks[i]);

        //... save the pollutant removal for the LID process as a fraction
        LidProcs[j].drainRmvl[p] = rmvl / 100.0;
        i++;
    }
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
"\n                                   No. of        Unit        Unit      %% Area    %% Imperv      %% Perv"); //(5.1.013)
    fprintf(Frpt.file,                                                                                         //
"\n  Subcatchment     LID Control      Units        Area       Width     Covered     Treated     Treated");    //
    fprintf(Frpt.file,                                                                                         //
"\n  ---------------------------------------------------------------------------------------------------");    //

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
            fprintf(Frpt.file, "%6d  %10.2f  %10.2f  %10.2f  %10.2f  %10.2f",  //(5.1.013)
                lidUnit->number, lidUnit->area * SQR(UCF(LENGTH)),
                lidUnit->fullWidth * UCF(LENGTH), pctArea,
                lidUnit->fromImperv*100.0, lidUnit->fromPerv*100.0);           //(5.1.013)
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
    switch (LidProcs[j].lidType)
    {
    case BIO_CELL:
    case RAIN_GARDEN:
        if ( LidProcs[j].soil.thickness <= 0.0 ) layerMissing = TRUE;
        break;
    case GREEN_ROOF:
        if ( LidProcs[j].soil.thickness <= 0.0 ) layerMissing = TRUE; 
        if ( LidProcs[j].drainMat.thickness <= 0.0) layerMissing = TRUE;
        break;
    case POROUS_PAVEMENT:
        if ( LidProcs[j].pavement.thickness  <= 0.0 ) layerMissing = TRUE;
        break;
    case INFIL_TRENCH:
        if ( LidProcs[j].storage.thickness <= 0.0 ) layerMissing = TRUE;
        break;
    }
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

        {
            strcpy(Msg, LidProcs[j].ID);
            strcat(Msg, ERR_PAVE_LAYER);
            report_writeErrorMsg(ERR_LID_PARAMS, Msg);
        }
    }

    //... check soil layer parameters
    if ( LidProcs[j].soil.thickness > 0.0 )
    {
        if ( LidProcs[j].soil.porosity      <= 0.0 
        ||   LidProcs[j].soil.fieldCap      >= LidProcs[j].soil.porosity
        ||   LidProcs[j].soil.wiltPoint     >= LidProcs[j].soil.fieldCap
        ||   LidProcs[j].soil.kSat          <= 0.0
        ||   LidProcs[j].soil.kSlope        <  0.0 )
        {
            strcpy(Msg, LidProcs[j].ID);
            strcat(Msg, ERR_SOIL_LAYER);
            report_writeErrorMsg(ERR_LID_PARAMS, Msg);
        }
    }

    //... check storage layer parameters
    if ( LidProcs[j].storage.thickness > 0.0 )
    {
        if ( LidProcs[j].storage.voidFrac <= 0.0 ||
             LidProcs[j].storage.voidFrac > 1.0 )
        {
            strcpy(Msg, LidProcs[j].ID);
            strcat(Msg, ERR_STOR_LAYER);
            report_writeErrorMsg(ERR_LID_PARAMS, Msg);
        }
    }

    //... if no storage layer adjust void fraction and drain offset 
    else
    {    
        LidProcs[j].storage.voidFrac = 1.0;
        LidProcs[j].drain.offset = 0.0;
    }

    //... check for invalid drain open/closed heads                            //(5.1.013)
    if (LidProcs[j].drain.hOpen > 0.0 &&                                       //
        LidProcs[j].drain.hOpen <= LidProcs[j].drain.hClose)                   //
    {                                                                          //
        strcpy(Msg, LidProcs[j].ID);                                           //
        strcat(Msg, ERR_DRAIN_HEADS);                                          //
        report_writeErrorMsg(ERR_LID_PARAMS, Msg);                             //
    }                                                                          //

    //... compute the surface layer's overland flow constant (alpha)
    if ( LidProcs[j].lidType == VEG_SWALE )
    {
        if ( LidProcs[j].surface.roughness * 
             LidProcs[j].surface.surfSlope <= 0.0 ||
             LidProcs[j].surface.thickness == 0.0
           )
        {
            strcpy(Msg, LidProcs[j].ID);
            strcat(Msg, ERR_SWALE_SURF);
            report_writeErrorMsg(ERR_LID_PARAMS, Msg);
        }
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

    //... compute drainage mat layer's flow coeff.
    if ( LidProcs[j].drainMat.roughness > 0.0 )
    {
        LidProcs[j].drainMat.alpha = 1.49 / LidProcs[j].drainMat.roughness *
                                    sqrt(LidProcs[j].surface.surfSlope);
    }
    else LidProcs[j].drainMat.alpha = 0.0;


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
    else LidProcs[j].storage.clogFactor = 0.0;

    //... for certain LID types, immediate overflow of excess surface water
    //    occurs if either the surface roughness or slope is zero
    LidProcs[j].surface.canOverflow = TRUE;
    switch (LidProcs[j].lidType)
    {
        case ROOF_DISCON: LidProcs[j].surface.canOverflow = FALSE; break;
        case INFIL_TRENCH:
        case POROUS_PAVEMENT:
        case BIO_CELL:
        case RAIN_GARDEN:
        case GREEN_ROOF:
            if ( LidProcs[j].surface.alpha > 0.0 )
                LidProcs[j].surface.canOverflow = FALSE;
    }

    //... rain barrels have 100% void space and impermeable bottom
    if ( LidProcs[j].lidType == RAIN_BARREL )
    {
        LidProcs[j].storage.voidFrac = 1.0;
        LidProcs[j].storage.kSat = 0.0;
    }

    //... set storage layer parameters of a green roof 
    if ( LidProcs[j].lidType == GREEN_ROOF )
	{
		LidProcs[j].storage.thickness = LidProcs[j].drainMat.thickness;
		LidProcs[j].storage.voidFrac = LidProcs[j].drainMat.voidFrac;
		LidProcs[j].storage.clogFactor = 0.0;
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
    double     fromPerv = 0.0;                                                 //(5.1.013)
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
        fromPerv += lidUnit->fromPerv;                                         //(5.1.013)

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
                strcpy(Msg, LidProcs[k].ID);
                strcat(Msg, ERR_SOIL_LAYER);
                report_writeErrorMsg(ERR_LID_PARAMS, Msg);
            }
        }
        
        //... assign vegetative swale infiltration parameters
        if ( LidProcs[k].lidType == VEG_SWALE )
        {
            if ( InfilModel == GREEN_AMPT || InfilModel == MOD_GREEN_AMPT )
            {
                p[0] = GAInfil[j].S * UCF(RAINDEPTH);
                p[1] = GAInfil[j].Ks * UCF(RAINFALL);
                p[2] = GAInfil[j].IMDmax;
                if ( grnampt_setParams(&(lidUnit->soilInfil), p) == FALSE )
                {
                    strcpy(Msg, LidProcs[k].ID);
                    strcat(Msg, ERR_GREEN_AMPT);
                    report_writeErrorMsg(ERR_LID_PARAMS, Msg);
                }
            }
            if ( lidUnit->fullWidth <= 0.0 )
            {
                strcpy(Msg, LidProcs[k].ID);
                strcat(Msg, ERR_SWALE_WIDTH);
                report_writeErrorMsg(ERR_LID_PARAMS, Msg);
            }
        }

        //... LID unit cannot send outflow back to subcatchment's
        //    pervious area if none exists
        if ( Subcatch[j].fracImperv >= 0.999 ) lidUnit->toPerv = 0;

        //... assign drain outlet if not set by user
        if ( lidUnit->drainNode == -1 && lidUnit->drainSubcatch == -1 )
        {
            lidUnit->drainNode = Subcatch[j].outNode;
            lidUnit->drainSubcatch = Subcatch[j].outSubcatch;
        }
        lidList = lidList->nextLidUnit;
    }

    //... check contributing area fractions
    if ( totalLidArea > 1.001 * totalArea )
    {
        report_writeErrorMsg(ERR_LID_AREAS, Subcatch[j].ID);
    }
    if ( fromImperv > 1.001 || fromPerv > 1.001 )                              //(5.1.013)
    {
        report_writeErrorMsg(ERR_LID_CAPTURE_AREA, Subcatch[j].ID);
    }

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
    double     initDryTime = StartDryDays * SECperDAY;

    HasWetLids = FALSE;
    for (j = 0; j < GroupCount; j++)
    {
        //... check if group exists
        lidGroup = LidGroups[j];
        if ( lidGroup == NULL ) continue;

        //... initialize group variables
        lidGroup->pervArea = 0.0;
        lidGroup->flowToPerv = 0.0;
        lidGroup->oldDrainFlow = 0.0;
        lidGroup->newDrainFlow = 0.0;

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
            lidUnit->paveDepth = 0.0;
            lidUnit->dryTime = initDryTime;
            lidUnit->volTreated = 0.0;                                         //(5.1.013)
            lidUnit->nextRegenDay = LidProcs[k].pavement.regenDays;            //
            initVol = 0.0;
            if ( LidProcs[k].soil.thickness > 0.0 )
            {
                lidUnit->soilMoisture = LidProcs[k].soil.wiltPoint + 
                    lidUnit->initSat * (LidProcs[k].soil.porosity -
                    LidProcs[k].soil.wiltPoint);
                initVol += lidUnit->soilMoisture * LidProcs[k].soil.thickness;
            }
            if ( LidProcs[k].storage.thickness > 0.0 )
            {
                lidUnit->storageDepth = lidUnit->initSat *
                    LidProcs[k].storage.thickness;
                initVol += lidUnit->storageDepth * LidProcs[k].storage.voidFrac;
            }
            if ( LidProcs[k].drainMat.thickness > 0.0 )
            {
                lidUnit->storageDepth = lidUnit->initSat *
                    LidProcs[k].drainMat.thickness;
                initVol += lidUnit->storageDepth * LidProcs[k].drainMat.voidFrac;
            }
            if ( lidUnit->initSat > 0.0 ) HasWetLids = TRUE;

            //... initialize water balance totals
            lidproc_initWaterBalance(lidUnit, initVol);
            lidUnit->volTreated = 0.0;

            //... initialize report file for the LID
            if ( lidUnit->rptFile )
            {
                initLidRptFile(Title[0], LidProcs[k].ID, Subcatch[j].ID, lidUnit);
            }

            //... initialize drain flows
            lidUnit->oldDrainFlow = 0.0;
            lidUnit->newDrainFlow = 0.0;

            //... set previous flux rates to 0
            for (i = 0; i < MAX_LAYERS; i++)
            {    
                lidUnit->oldFluxRates[i] = 0.0;
            }

            //... initialize infiltration state variables
            if ( lidUnit->soilInfil.Ks > 0.0 )
                grnampt_initState(&(lidUnit->soilInfil));

            //... add contribution to pervious LID area
            if ( isLidPervious(lidUnit->lidIndex) )
                lidGroup->pervArea += (lidUnit->area * lidUnit->number);
            lidList = lidList->nextLidUnit;
        }
    }
}

//=============================================================================

void  lid_setOldGroupState(int j)
//
//  Purpose: saves the current drain flow rate for the LIDs in a subcatchment.
//  Input:   j = subcatchment index 
//  Output:  none
//
{
    TLidList*  lidList;
    if ( LidGroups[j] != NULL )
    {
        LidGroups[j]->oldDrainFlow = LidGroups[j]->newDrainFlow;
        LidGroups[j]->newDrainFlow = 0.0;
        lidList = LidGroups[j]->lidList;
        while (lidList)
        {
            lidList->lidUnit->oldDrainFlow = lidList->lidUnit->newDrainFlow;
            lidList->lidUnit->newDrainFlow = 0.0;
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

double getSurfaceDepth(int j)
//
//  Purpose: computes the depth (volume per unit area) of ponded water on the
//           surface of all LIDs within a subcatchment.
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
    if ( LidGroups[j] != NULL ) return LidGroups[j]->flowToPerv;
    return 0.0;
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

double  lid_getDrainFlow(int j, int timePeriod)
//
//  Purpose: returns flow from all of a subcatchment's LID drains for
//           a designated time period
//  Input:   j = subcatchment index 
//           timePeriod = either PREVIOUS or CURRENT
//  Output:  total drain flow (cfs) from the subcatchment.
{
    if ( LidGroups[j] != NULL )
    {
        if ( timePeriod == PREVIOUS ) return LidGroups[j]->oldDrainFlow;
        else return LidGroups[j]->newDrainFlow;
    }
    return 0.0;
}

//=============================================================================

////  This function was modified for relelase 5.1.013.  ////                   //(5.1.013)

void  lid_addDrainLoads(int j, double c[], double tStep)
//
//  Purpose: adds pollutant loads routed from drains to system
//           mass balance totals.
//  Input:   j = subcatchment index
//           c = array of pollutant washoff concentrations (mass/L)
//           tStep =  time step (sec)
//  Output:  none.
//
{
    int    isRunoffLoad;     // true if drain becomes external runoff load
    int    p;                // pollutant index
    double r;                // pollutant fractional removal 
    double w;                // pollutant mass load (lb or kg)
    TLidUnit*  lidUnit;
    TLidList*  lidList; 
    TLidGroup  lidGroup; 

    //... check if LID group exists
    lidGroup = LidGroups[j];
    if ( lidGroup != NULL )
    {
        //... examine each LID unit in the group
        lidList = lidGroup->lidList;
        while ( lidList )
        {
            lidUnit = lidList->lidUnit;
 
            //... skip LID unit if it sends its drain flow onto
            //    its subcatchment's pervious area
            if (lidUnit->toPerv) continue;

            //... see if unit's drain flow becomes external runoff
            isRunoffLoad = (lidUnit->drainNode >= 0 ||
                            lidUnit->drainSubcatch == j);
            
            //... for each pollutant 
            for (p = 0; p < Nobjects[POLLUT]; p++)
            {
                //... get mass load flowing through the drain
                w = lidUnit->newDrainFlow * c[p] * tStep * LperFT3 * Pollut[p].mcf;

                //... get fractional removal for this load
                r = LidProcs[lidUnit->lidIndex].drainRmvl[p];

                //... update system mass balance totals
                massbal_updateLoadingTotals(BMP_REMOVAL_LOAD, p, r*w);
                if (isRunoffLoad)
                    massbal_updateLoadingTotals(RUNOFF_LOAD, p, w*(1.0-r));
            }

            // process next LID unit in the group
            lidList = lidList->nextLidUnit;
        }
    }
}

//=============================================================================

void lid_addDrainRunon(int j)
//
//  Purpose: adds drain flows from LIDs in a given subcatchment to the
//           subcatchments that were designated to receive them 
//  Input:   j = index of subcatchment contributing underdrain flows
//  Output:  none.
//
{
    int i;                   // index of an LID unit's LID process             //(5.1.013)
    int k;                   // index of subcatchment receiving LID drain flow
    int p;                   // pollutant index
    double q;                // drain flow rate (cfs)
    double w;                // mass of polllutant from drain flow             //(5.1.013)
    TLidUnit*  lidUnit;
    TLidList*  lidList; 
    TLidGroup  lidGroup; 

    //... check if LID group exists
    lidGroup = LidGroups[j];
    if ( lidGroup != NULL )
    {
        //... examine each LID in the group
        lidList = lidGroup->lidList;
        while ( lidList )
        {
            //... see if LID's drain discharges to another subcatchment
            lidUnit = lidList->lidUnit;
            i = lidUnit->lidIndex;                                             //(5.1.013)
            k = lidUnit->drainSubcatch;
            if ( k >= 0 && k != j )
            {
                //... distribute drain flow across subcatchment's areas
                q = lidUnit->oldDrainFlow;
                subcatch_addRunonFlow(k, q);

                //... add pollutant loads from drain to subcatchment
                //    (newQual[] contains loading rate (mass/sec) at this
                //    point which is converted later on to a concentration)
                for (p = 0; p < Nobjects[POLLUT]; p++)
                {
                    w = q * Subcatch[j].oldQual[p] * LperFT3;                  //(5.1.013)
                    w = w * (1.0 - LidProcs[i].drainRmvl[p]);                  //
                    Subcatch[k].newQual[p] += w;                               //
                }
            }
            lidList = lidList->nextLidUnit;
        }
    }
}

//=============================================================================

void  lid_addDrainInflow(int j, double f)
//
//  Purpose: adds LID drain flow to conveyance system nodes 
//  Input:   j = subcatchment index
//           f = time interval weighting factor
//  Output:  none.
//
//  Note:    this function updates the total lateral flow (Node[].newLatFlow)
//           and pollutant mass (Node[].newQual[]) inflow seen by nodes that
//           receive drain flow from the LID units in subcatchment j.
{
    int        i,            // LID process index                              //(5.1.013)
               k,            // node index
               p;            // pollutant index
    double     q,            // drain flow (cfs)
               w, w1, w2;    // pollutant mass loads (mass/sec)
    TLidUnit*  lidUnit;
    TLidList*  lidList;
    TLidGroup  lidGroup;

    //... check if LID group exists
    lidGroup = LidGroups[j];
    if ( lidGroup != NULL )
    {
        //... examine each LID in the group
        lidList = lidGroup->lidList;
        while ( lidList )
        {
            //... see if LID's drain discharges to conveyance system node
            lidUnit = lidList->lidUnit;
            i = lidUnit->lidIndex;                                             //(5.1.013)
            k = lidUnit->drainNode;
            if ( k >= 0 )
            {
                //... add drain flow to node's wet weather inflow
                q = (1.0 - f) * lidUnit->oldDrainFlow + f * lidUnit->newDrainFlow;
                Node[k].newLatFlow += q;
                massbal_addInflowFlow(WET_WEATHER_INFLOW, q);

                //... add pollutant load, based on parent subcatchment quality 
                for (p = 0; p < Nobjects[POLLUT]; p++)
                {
                    //... get previous & current drain loads
                    w1 = lidUnit->oldDrainFlow * Subcatch[j].oldQual[p];
                    w2 = lidUnit->newDrainFlow * Subcatch[j].newQual[p]; 

                    //... add interpolated load to node's wet weather loading
                    w = (1.0 - f) * w1 + f * w2;
                    w = w * (1.0 - LidProcs[i].drainRmvl[p]);                  //(5.1.013)
                    Node[k].newQual[p] += w;
                    massbal_addInflowQual(WET_WEATHER_INFLOW, p, w);
                }
            }
            lidList = lidList->nextLidUnit;
        }
    }
}

//=============================================================================

void lid_getRunoff(int j, double tStep)
//
//  Purpose: computes runoff and drain flows from the LIDs in a subcatchment.
//  Input:   j     = subcatchment index 
//           tStep = time step (sec)
//  Output:  updates following global quantities after LID treatment applied:
//           Vevap, Vpevap, VlidInfil, VlidIn, VlidOut, VlidDrain.
//
{
    TLidGroup  theLidGroup;       // group of LIDs placed in the subcatchment
    TLidList*  lidList;           // list of LID units in the group
    TLidUnit*  lidUnit;           // a member of the list of LID units
    double lidArea;               // area of an LID unit
    double qImperv = 0.0;         // runoff from impervious areas (cfs)
    double qPerv = 0.0;           // runoff from pervious areas (cfs)          //(5.1.013)
    double lidInflow = 0.0;       // inflow to an LID unit (ft/s) 
    double qRunoff = 0.0;         // surface runoff from all LID units (cfs)
    double qDrain = 0.0;          // drain flow from all LID units (cfs)
    double qReturn = 0.0;         // LID outflow returned to pervious area (cfs) 

    //... return if there are no LID's
    theLidGroup = LidGroups[j];
    if ( !theLidGroup ) return;
    lidList = theLidGroup->lidList;
    if ( !lidList ) return;

    //... determine if evaporation can occur
    EvapRate = Evap.rate;
    if ( Evap.dryOnly && Subcatch[j].rainfall > 0.0 ) EvapRate = 0.0;

    //... find subcatchment's infiltration rate into native soil
    findNativeInfil(j, tStep);

    //... get impervious and pervious area runoff from non-LID
    //    portion of subcatchment (cfs)
    if ( Subcatch[j].area > Subcatch[j].lidArea )
    {    
        qImperv = getImpervAreaRunoff(j);
        qPerv = getPervAreaRunoff(j);                                          //(5.1.013)
    }

    //... evaluate performance of each LID unit placed in the subcatchment
    while ( lidList )
    {
        //... find area of the LID unit
        lidUnit = lidList->lidUnit;
        lidArea = lidUnit->area * lidUnit->number;

        //... if LID unit has area, evaluate its performance
        if ( lidArea > 0.0 )
        {
            //... find runoff from non-LID area treated by LID area (ft/sec)
            lidInflow = (qImperv * lidUnit->fromImperv +                       //(5.1.013)
                         qPerv * lidUnit->fromPerv) / lidArea;                 //

            //... update total runoff volume treated
            VlidIn += lidInflow * lidArea * tStep;

            //... add rainfall onto LID inflow (ft/s)
            lidInflow = lidInflow + Subcatch[j].rainfall;

            // ... add upstream runon only if LID occupies full subcatchment
            if ( Subcatch[j].area == Subcatch[j].lidArea )
            {
                lidInflow += Subcatch[j].runon;
            }

            //... evaluate the LID unit's performance, updating the LID group's
            //    total surface runoff, drain flow, and flow returned to
            //    pervious area 
            evalLidUnit(j, lidUnit, lidArea, lidInflow, tStep,
                        &qRunoff, &qDrain, &qReturn);
        }
        lidList = lidList->nextLidUnit;
    }

    //... save the LID group's total drain & return flows
    theLidGroup->newDrainFlow = qDrain;
    theLidGroup->flowToPerv = qReturn;

    //... save the LID group's total surface, drain and return flow volumes
    VlidOut = qRunoff * tStep; 
    VlidDrain = qDrain * tStep;
    VlidReturn = qReturn * tStep;
}

//=============================================================================

void findNativeInfil(int j, double tStep)
//
//  Purpose: determines a subcatchment's current infiltration rate into
//           its native soil.
//  Input:   j = subcatchment index
//           tStep    = time step (sec)
//  Output:  sets values for module-level variables NativeInfil
//
{
    double nonLidArea;

    //... subcatchment has non-LID pervious area
    nonLidArea = Subcatch[j].area - Subcatch[j].lidArea;
    if ( nonLidArea > 0.0 && Subcatch[j].fracImperv < 1.0 )
    {
        NativeInfil = Vinfil / nonLidArea / tStep;
    }

    //... otherwise find infil. rate for the subcatchment's rainfall + runon
    else
    {
        NativeInfil = infil_getInfil(j, InfilModel, tStep,
                                     Subcatch[j].rainfall,
                                     Subcatch[j].runon,
                                     getSurfaceDepth(j));                      //(5.1.008)
    }

    //... see if there is any groundwater-imposed limit on infil.
    if ( !IgnoreGwater && Subcatch[j].groundwater )
    {
        MaxNativeInfil = Subcatch[j].groundwater->maxInfilVol / tStep;
    }
    else MaxNativeInfil = BIG;
}

//=============================================================================

double getImpervAreaRunoff(int j)
//
//  Purpose: computes runoff from impervious area of a subcatchment that
//           is available for LID treatment.
//  Input:   j = subcatchment index
//  Output:  returns runoff flow rate (cfs)
//
{
    int    i;
    double q = 0.0,          // runoff rate (ft/sec)
           nonLidArea;       // non-LID area (ft2)

    // --- runoff from impervious area w/ & w/o depression storage
    for (i = IMPERV0; i <= IMPERV1; i++)
    {
        q += Subcatch[j].subArea[i].runoff * Subcatch[j].subArea[i].fArea;
    }

    // --- adjust for any fraction of runoff sent to pervious area
    if ( Subcatch[j].subArea[IMPERV0].routeTo == TO_PERV &&
         Subcatch[j].fracImperv < 1.0 )
    {
        q *= Subcatch[j].subArea[IMPERV0].fOutlet;
    }
    nonLidArea = Subcatch[j].area - Subcatch[j].lidArea;
    return q * nonLidArea;
}

//=============================================================================

////  This function was added for release 5.1.013.  ////                       //(5.1.013)

double getPervAreaRunoff(int j)
//
//  Purpose: computes runoff from pervious area of a subcatchment that
//           is available for LID treatment.
//  Input:   j = subcatchment index
//  Output:  returns runoff flow rate (cfs)
//
{
    double q = 0.0,          // runoff rate (ft/sec)
           nonLidArea;       // non-LID area (ft2)

    // --- runoff from pervious area
    q = Subcatch[j].subArea[PERV].runoff * Subcatch[j].subArea[PERV].fArea;

    // --- adjust for any fraction of runoff sent to impervious area
    if (Subcatch[j].subArea[PERV].routeTo == TO_IMPERV &&
        Subcatch[j].fracImperv > 0.0)
    {
        q *= Subcatch[j].subArea[PERV].fOutlet;
    }
    nonLidArea = Subcatch[j].area - Subcatch[j].lidArea;
    return q * nonLidArea;
}

//=============================================================================

void evalLidUnit(int j, TLidUnit* lidUnit, double lidArea, double lidInflow,
    double tStep, double *qRunoff, double *qDrain, double *qReturn)
//
//  Purpose: evaluates performance of a specific LID unit over current time step.
//  Input:   j         = subcatchment index
//           lidUnit   = ptr. to LID unit being evaluated
//           lidArea   = area of LID unit
//           lidInflow = inflow to LID unit (ft/s)
//           tStep     = time step (sec)
//  Output:  qRunoff   = sum of surface runoff from all LIDs (cfs)
//           qDrain    = sum of drain flows from all LIDs (cfs)
//           qReturn   = sum of LID flows returned to pervious area (cfs)
//
{
    TLidProc* lidProc;       // LID process associated with lidUnit
    double lidRunoff,        // surface runoff from LID unit (cfs)
           lidEvap,          // evaporation rate from LID unit (ft/s)
           lidInfil,         // infiltration rate from LID unit (ft/s)
           lidDrain;         // drain flow rate from LID unit (ft/s & cfs)

    //... identify the LID process of the LID unit being analyzed
    lidProc = &LidProcs[lidUnit->lidIndex];

    //... initialize evap and infil losses
    lidEvap = 0.0;
    lidInfil = 0.0;

    //... find surface runoff from the LID unit (in cfs)
    lidRunoff = lidproc_getOutflow(lidUnit, lidProc, lidInflow, EvapRate,
                                  NativeInfil, MaxNativeInfil, tStep,
                                  &lidEvap, &lidInfil, &lidDrain) * lidArea;
    
    //... convert drain flow to CFS
    lidDrain *= lidArea;

    //... revise flows if LID outflow returned to pervious area
    if ( lidUnit->toPerv && Subcatch[j].area > Subcatch[j].lidArea )
    {
        //... surface runoff is always returned
        *qReturn += lidRunoff;
        lidRunoff = 0.0;

        //... drain flow returned if it has same outlet as subcatchment
        if ( lidUnit->drainNode == Subcatch[j].outNode &&
            lidUnit->drainSubcatch == Subcatch[j].outSubcatch )
        {
            *qReturn += lidDrain;
            lidDrain = 0.0;
        }
    }
 
    //... update system flow balance if drain flow goes to a
    //    conveyance system node
    if ( lidUnit->drainNode >= 0 )
    {
        massbal_updateRunoffTotals(RUNOFF_DRAINS, lidDrain * tStep);
    }

    //... save new drain outflow
    lidUnit->newDrainFlow = lidDrain;

    //... update moisture losses (ft3)
    Vevap  += lidEvap * tStep * lidArea;
    VlidInfil += lidInfil * tStep * lidArea;
    if ( isLidPervious(lidUnit->lidIndex) )
    {
        Vpevap += lidEvap * tStep * lidArea;
    }

    //... update time since last rainfall (for Rain Barrel emptying)
    if ( Subcatch[j].rainfall > MIN_RUNOFF ) lidUnit->dryTime = 0.0;
    else lidUnit->dryTime += tStep;

    //... update LID water balance and save results
    lidproc_saveResults(lidUnit, UCF(RAINFALL), UCF(RAINDEPTH));

    //... update LID group totals
    *qRunoff += lidRunoff;
    *qDrain += lidDrain;
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
"\n  --------------------------------------------------------------------------------------------------------------------"
"\n                                         Total      Evap     Infil   Surface    Drain    Initial     Final  Continuity"
"\n                                        Inflow      Loss      Loss   Outflow   Outflow   Storage   Storage       Error");
    if ( UnitSystem == US ) fprintf(Frpt.file, 
"\n  Subcatchment      LID Control             in        in        in        in        in        in        in           %%");
    else fprintf(Frpt.file,
"\n  Subcatchment      LID Control             mm        mm        mm        mm        mm        mm        mm           %%");
    fprintf(Frpt.file,
"\n  --------------------------------------------------------------------------------------------------------------------");

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
            fprintf(Frpt.file, "  %10.2f", err*100.0);
            lidList = lidList->nextLidUnit;
        }
    }
}

//=============================================================================

void initLidRptFile(char* title, char* lidID, char* subcatchID, TLidUnit* lidUnit)
//
//  Purpose: initializes the report file used for a specific LID unit
//  Input:   title = project's title
//           lidID = LID process name
//           subcatchID = subcatchment ID name
//           lidUnit = ptr. to LID unit
//  Output:  none
//
{
    static int colCount = 14;
    static char* head1[] = {
        "\n                    \t", "  Elapsed\t",
        "    Total\t", "    Total\t", "  Surface\t", " Pavement\t", "     Soil\t",
        "  Storage\t", "  Surface\t", "    Drain\t", "  Surface\t", " Pavement\t",
        "     Soil\t", "  Storage"};
    static char* head2[] = {
        "\n                    \t", "     Time\t",
        "   Inflow\t", "     Evap\t", "    Infil\t", "     Perc\t", "     Perc\t",
        "    Exfil\t", "   Runoff\t", "  OutFlow\t", "    Level\t", "    Level\t",
        " Moisture\t", "    Level"};
    static char* units1[] = {
        "\nDate        Time    \t", "    Hours\t",
        "    in/hr\t", "    in/hr\t", "    in/hr\t", "    in/hr\t", "    in/hr\t",
        "    in/hr\t", "    in/hr\t", "    in/hr\t", "   inches\t", "   inches\t",
        "  Content\t", "   inches"};
    static char* units2[] = {
        "\nDate        Time    \t", "    Hours\t",
        "    mm/hr\t", "    mm/hr\t", "    mm/hr\t", "    mm/hr\t", "    mm/hr\t",
        "    mm/hr\t", "    mm/hr\t", "    mm/hr\t", "       mm\t", "       mm\t",
        "  Content\t", "       mm"};
    static char line9[] = " ---------";
    int   i;
    FILE* f = lidUnit->rptFile->file;

    //... check that file was opened
    if ( f ==  NULL ) return;

    //... write title lines
    fprintf(f, "SWMM5 LID Report File\n");
    fprintf(f, "\nProject:  %s", title);
    fprintf(f, "\nLID Unit: %s in Subcatchment %s\n", lidID, subcatchID);

    //... write column headings
    for ( i = 0; i < colCount; i++) fprintf(f, "%s", head1[i]);
    for ( i = 0; i < colCount; i++) fprintf(f, "%s", head2[i]);
    if (  UnitSystem == US )
    {
        for ( i = 0; i < colCount; i++) fprintf(f, "%s", units1[i]);
    }
    else for ( i = 0; i < colCount; i++) fprintf(f, "%s", units2[i]);
    fprintf(f, "\n----------- --------");
    for ( i = 1; i < colCount; i++) fprintf(f, "\t%s", line9);

    //... initialize LID dryness state
    lidUnit->rptFile->wasDry = 1;
    strcpy(lidUnit->rptFile->results, "");
}
