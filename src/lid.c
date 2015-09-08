//-----------------------------------------------------------------------------
//   lid.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//             05/19/14   (Build 5.1.006)
//             09/15/14   (Build 5.1.007)
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
//   underdrain flow, evaporation and infiltration) are added to those
//   computed for the non-LID portion of the subcatchment.
//
//   An option exists for the detailed time series of flux rates and storage
//   levels for a specific LID unit to be written to a text file named by the
//   user for viewing outside of the SWMM program.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include "headers.h"
#include "lid.h"

//-----------------------------------------------------------------------------
//  Enumerations
//-----------------------------------------------------------------------------
enum LidLayerTypes {
    SURF,                    // surface layer
    SOIL,                    // soil layer
    STOR,                    // storage layer
    PAVE,                    // pavement layer
    DRAINMAT,                // drainage mat layer
    DRAIN};                  // underdrain system

//// Note: DRAINMAT must be placed before DRAIN so the two keywords can
///        be distinguished from one another when parsing a line of input. 

char* LidLayerWords[] =
    {"SURFACE", "SOIL", "STORAGE", "PAVEMENT", "DRAINMAT", "DRAIN", NULL};

char* LidTypeWords[] =
    {"BC",                   //bio-retention cell
     "RG",                   //rain garden
     "GR",                   //green roof
     "IT",                   //infiltration trench
     "PP",                   //porous pavement
     "RB",                   //rain barrel
     "VS",                   //vegetative swale
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
    double         impervRunoff;  // impervious area runoff volume (ft3)
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

static double     TotalEvapVol;        // subcatch. evap loss (ft3)
static double     TotalPervEvapVol;    // evap loss over pervious area (ft3)
static double     TotalInfilVol;       // subcatch infiltration loss (ft3)
static double     NextReportTime;
static int        SaveResults;         // = 1 if detailed results to be saved

//-----------------------------------------------------------------------------
//  External Functions (protoyped in lid.h)
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
static int    readDrainMatData(int j, char* toks[], int ntoks);

static int    addLidUnit(int j, int k, int n, double x[], char* fname);
static int    createLidRptFile(TLidUnit* lidUnit, char* fname);
static void   initLidRptFile(char* title, char* lidID, char* subcatchID,
              TLidUnit* lidUnit);
static void   validateLidProc(int j);
static void   validateLidGroup(int j);
static int    isLidPervious(int k);

static double getImpervAreaInflow(int j);
static void   findNativeInfil(int j, double infilVol, double tStep);
static void   evalLidUnit(TLidUnit* lidUnit, double rainfall, double runon,
              double qImperv, double tStep, double *lidOutflow,
              double *nonLidOutflow, double *flowToPerv);


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
//                                                       DrainTo (RptFile)
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
    lidUnit->fullWidth    = x[1] / UCF(LENGTH);
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
    LidProcs[j].drain.delay  = x[3] * 3600.0;
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
                lidUnit->fullWidth * UCF(LENGTH), pctArea,
                lidUnit->fromImperv*100.0);
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

    //... if no storage layer adjust void fraction and drain offset            //(5.1.007)
    else
    {    
        LidProcs[j].storage.voidFrac = 1.0;
        LidProcs[j].drain.offset = 0.0;
    }

    //... check underdrain parameters
    if ( LidProcs[j].drain.offset > LidProcs[j].storage.thickness )
        report_writeErrorMsg(ERR_LID_PARAMS, LidProcs[j].ID);

    //... compute the surface layer's overland flow constant (alpha)
    if ( LidProcs[j].lidType == VEG_SWALE )
    {
        if ( LidProcs[j].surface.roughness * 
             LidProcs[j].surface.surfSlope <= 0.0 ||
             LidProcs[j].surface.thickness == 0.0
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
    else LidProcs[j].storage.clogFactor = 0.0;                                 //(5.1.007)

    //... for certain LID types, immediate overflow of excess surface water
    //    occurs if either the surface roughness or slope is zero
    LidProcs[j].surface.canOverflow = TRUE;
    switch (LidProcs[j].lidType)
    {
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
            if ( lidUnit->fullWidth <= 0.0 )
            {
                report_writeErrorMsg(ERR_LID_PARAMS, LidProcs[k].ID);
            }
        }
        lidList = lidList->nextLidUnit;
    }

    //... check contributing area fractions
    if ( totalLidArea > 1.001 * totalArea )
    {
        report_writeErrorMsg(ERR_LID_AREAS, Subcatch[j].ID);
    }
    if ( fromImperv > 1.001 )
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

    NextReportTime = (double) (ReportStep * 1000.0); 
    for (j = 0; j < GroupCount; j++)
    {
        //... check if group exists
        lidGroup = LidGroups[j];
        if ( lidGroup == NULL ) continue;

        //... initialize group variables
        lidGroup->pervArea = 0.0;
        lidGroup->flowToPerv = 0.0;

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
            lidUnit->dryTime = initDryTime;
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

            //... initialize water balance totals
            lidproc_initWaterBalance(lidUnit, initVol);

            //... initialize report file for the LID
            if ( lidUnit->rptFile )
            {
                initLidRptFile(Title[0], LidProcs[k].ID, Subcatch[j].ID, lidUnit);
            }

            //... set previous flux rates to 0
            for (i = 0; i < MAX_STATE_VARS; i++)
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
{
    TLidGroup  theLidGroup;
    TLidList*  lidList;
    double qImperv;                    // LID inflow from impervious areas (cfs)
    double nonLidOutflow;              // total outflow from non-LID area (cfs)
    double lidOutflow;                 // total outflow from LID area (cfs)
    double flowToPerv;                 // return flow to pervious area (cfs)

    //... return current subcatchment outflow if there are no LID's
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

    //... determine the actual evaporation rate
    EvapRate = Evap.rate;
    if ( Evap.dryOnly && Subcatch[j].rainfall > 0.0 ) EvapRate = 0.0;

    //... find subcatchment's infiltration rate into native soil
    findNativeInfil(j, *infilVol, tStep);

    //... get inflows from non-LID subareas of subcatchment (cfs)
    qImperv = getImpervAreaInflow(j) * (Subcatch[j].area - Subcatch[j].lidArea);
    theLidGroup->impervRunoff += qImperv * tStep;

    //... check if detailed results should be saved
    if ( NewRunoffTime < NextReportTime ) SaveResults = FALSE;
    else 
    {
        SaveResults = TRUE;
        while ( NewRunoffTime >= NextReportTime )                              //(5.1.007)
        {
            NextReportTime += (double)(1000 * ReportStep);                     //(5.1.006)
        }
    }

    //... evaluate performance of each LID unit placed in the subcatchment
    while ( lidList )
    {
        evalLidUnit(lidList->lidUnit, Subcatch[j].rainfall, Subcatch[j].runon,
                    qImperv, tStep, &lidOutflow, &nonLidOutflow, &flowToPerv);
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

void findNativeInfil(int j, double infilVol, double tStep)
//
//  Purpose: determines a subcatchment's current infiltration rate into
//           its native soil.
//  Input:   j = subcatchment index
//           infilVol = infil. volume for non-LID pervious sub-area (ft3)
//           tStep    = time step (sec)
//  Output:  sets values for module-level variables NativeInfil
//
{
    double nonLidArea;

    //... subcatchment has non-LID pervious area
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

double getImpervAreaInflow(int j)
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
        q += Subcatch[j].subArea[i].runoff * Subcatch[j].subArea[i].fArea;
    }

    // --- adjust for any fraction of runoff sent to pervious area
    if ( Subcatch[j].subArea[IMPERV0].routeTo == TO_PERV &&
         Subcatch[j].fracImperv < 1.0 )
            q *= Subcatch[j].subArea[IMPERV0].fOutlet;
    return q;
}

//=============================================================================

void evalLidUnit(TLidUnit* lidUnit, double rainfall, double runon,
                 double qImperv, double tStep, double *lidOutflow,
                 double *nonLidOutflow, double *flowToPerv)
//
//  Purpose: evaluates performance of current LID unit over current time step.
//  Input:   lidUnit = ptr. to LID unit being evaluated
//           rainfall = rainfall rate (ft/s)
//           runon = inflow from upstream subcatchments (ft/s)
//           tStep = time step (sec)
//           qImperv = inflow from non-LID impervious area (cfs)
//  Output:  updates:
//           lidOutflow = total outflow from all LID areas (cfs)
//           nonLidOutflow = total outflow from all non-LID area (cfs)
//           flowToPerv = flow returned to non-LID pervious area (cfs)
//
{
    TLidProc* lidProc;
    double inflow, outflow, lidEvap, lidInfil, lidUnitArea;

    //... return if the LID has no area
    lidUnitArea = lidUnit->area * lidUnit->number;
    if ( lidUnitArea <= 0.0 ) return;

    //... identify the LID process of the LID unit being analyzed
    lidProc = &LidProcs[lidUnit->lidIndex];

    //... initialize evap and infil losses
    lidEvap = 0.0;
    lidInfil = 0.0;

    //... get inflow to LID unit from non-LID area runoff (cfs)
    //    (reduce non-Lid outflow by this amount)
    inflow = qImperv * lidUnit->fromImperv;
    *nonLidOutflow -= inflow;

    //... find total inflow to LID unit from all sources (ft/s)
    inflow = (inflow / lidUnitArea) + rainfall + runon;

    //... outflow from the LID unit (in ft/s)
    outflow = lidproc_getOutflow(lidUnit, lidProc, inflow, rainfall,
		                         EvapRate, NativeInfil, MaxNativeInfil,
								 tStep, &lidEvap, &lidInfil);
    *lidOutflow += outflow * lidUnitArea;

    //... update flow returned to pervious area & moisture losses (ft3)
    *flowToPerv += lidUnit->toPerv * outflow;
    TotalEvapVol  += lidEvap * tStep * lidUnitArea;
    TotalInfilVol += lidInfil * tStep * lidUnitArea;
    if ( isLidPervious(lidUnit->lidIndex) )
    {
        TotalPervEvapVol += lidEvap * tStep * lidUnitArea;
    }

    //... update time since last rainfall
    if ( rainfall > MIN_RUNOFF ) lidUnit->dryTime = 0.0;
    else lidUnit->dryTime += tStep;

    //... update water balance and save results
    lidproc_saveResults(lidUnit, SaveResults, UCF(RAINFALL), UCF(RAINDEPTH));
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
"\n                                         Total      Evap     Infil   Surface    Drain    Initial     Final   Percent"
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
    FILE* f = lidUnit->rptFile->file;

    //... check that file was opened
    if ( f ==  NULL ) return;

    //... write title lines
    fprintf(f, "SWMM5 LID Report File\n");
    fprintf(f, "\nProject:  %s", title);
    fprintf(f, "\nLID Unit: %s in Subcatchment %s\n", lidID, subcatchID);

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
    lidUnit->rptFile->lastReportTime = 0.0;
}
