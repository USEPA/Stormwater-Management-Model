//-----------------------------------------------------------------------------
//   inlet.c
//
//   Project:  EPA SWMM5
//   Version:  5.2
//   Date:     03/24/21 (Build 5.2.0)
//   Author:   L. Rossman
//
//   Street/Channel Inlet Functions
//
//   Computes capture efficiency of inlets placed in Street conduits
//   or Rectangular/Trapezoidal channels using FHWA HEC-22 methods (see
//   Brown, S.A. et al., Urban Drainage Design Manual, Federal Highway
//   Administration Hydraulic Engineering Circular No. 22, 3rd Edition,
//   FHWA-NHI-10-009, August 2013).
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include "headers.h"
#include "street.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

//-----------------------------------------------------------------------------
//  Enumerations
//-----------------------------------------------------------------------------

enum InletType {
    GRATE_INLET, CURB_INLET, COMBO_INLET, SLOTTED_INLET,
    DROP_GRATE_INLET, DROP_CURB_INLET, CUSTOM_INLET
};

enum GrateType {
    P50, P50x100, P30, CURVED_VANE, TILT_BAR_45,
    TILT_BAR_30, RETICULINE, GENERIC
};

enum InletPlacementType { AUTOMATIC, ON_GRADE, ON_SAG };

enum ThroatAngleType { HORIZONTAL_THROAT, INCLINED_THROAT, VERTICAL_THROAT };

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
static char* InletTypeWords[] = 
    {"GRATE", "CURB", "", "SLOTTED", "DROP_GRATE", "DROP_CURB", "CUSTOM", NULL};

static char* GrateTypeWords[] =
  {"P_BAR-50", "P_BAR-50x100", "P_BAR-30", "CURVED_VANE", "TILT_BAR-45", "TILT_BAR-30",
   "RETICULINE", "GENERIC", NULL};

static char* ThroatAngleWords[] =
    {"HORIZONTAL", "INCLINED", "VERTICAL", NULL};

static char *PlacementTypeWords[] =
    {"AUTOMATIC", "ON_GRADE", "ON_SAG"};

// Coefficients for cubic polynomials fitted to Splash Over Velocity v.
// Grate Length curves in Chart 5B of HEC-22 manual taken from Denver
// UDFCD manual.
static const double SplashCoeffs[][4] = {
    {2.22, 4.03, 0.65, 0.06},     //P_BAR-50
    {0.74, 2.44, 0.27, 0.02},     //P_BAR-50x100
    {1.76, 3.12, 0.45, 0.03},     //P_BAR-30
    {0.30, 4.85, 1.31, 0.15},     //Curved_Vane
    {0.99, 2.64, 0.36, 0.03},     //Tilt_Bar-45
    {0.51, 2.34, 0.2, 0.01},      //Tilt_Bar-30
    {0.28, 2.28, 0.18, 0.01}};    //Reticuline

// Grate opening ratios (Chart 9B of HEC-22 manual)
static const double GrateOpeningRatios[] = {
    0.90,     //P_BAR-50
    0.80,     //P_BAR-50x100
    0.60,     //P_BAR-30
    0.35,     //Curved_Vane
    0.17,     //Tilt_Bar-45 (assumed)
    0.34,     //Tilt_Bar-30   
    0.80,     //Reticuline
    1.00};    //Generic

//-----------------------------------------------------------------------------
//  Imported Variables
//-----------------------------------------------------------------------------
extern TLinkStats*     LinkStats;      // defined in STATS.C

//-----------------------------------------------------------------------------
//  Local Shared Variables
//-----------------------------------------------------------------------------
// Variables as named in the HEC-22 manual.
static double Sx;            // street cross slope
static double SL;            // conduit longitudinal slope
static double Sw;            // street gutter slope
static double a;             // street gutter depression (ft)
static double W;             // street gutter width (ft)
static double T;             // top width of flow spread (ft)
static double n;             // Manning's roughness coeff.

// Additional variables
static int    Nsides;        // 1- or 2-sided street
static double Tcrown;        // distance from street curb to crown (ft)
static double Beta;          // = 1.486 * sqrt(SL) / n
static double Qfactor;       // factor f in Izzard's eqn. Q = f*T^2.67
static TXsect *xsect;        // cross-section data of inlet's conduit
static char   *InletDegree;  // # inflow links to inlet-connected nodes 
static double *InletFlow;    // captured inlet flow received by each node
static TInlet* FirstInlet;   // head of list of deployed inlets

//-----------------------------------------------------------------------------
//  External functions (declared in inlet.h)
//-----------------------------------------------------------------------------
// inlet_create              called by createObjects in project.c
// inlet_delete              called by deleteObjects in project.c
// inlet_readDesignParams    called by parseLine in input.c
// inlet_readUsageParams     called by parseLine in input.c
// inlet_validate            called by project_validate
// inlet_findInletFlows      called by routing_execute
// inlet_convertOverflows    called by routing_execute
// inlet_writeStatsReport    called by statsrpt_writeReport
// inlet_capturedFlow        called by findLinkMassFlow in qualrout.c


//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static int    readGrateInletParams(int inletIndex, char* tok[], int ntoks);
static int    readCurbInletParams(int inletIndex, char* tok[], int ntoks);
static int    readSlottedInletParams(int inletIndex, char* tok[], int ntoks);
static int    readCustomInletParams(int inletIndex, char* tok[], int ntoks);

static void   initInletStats(TInlet *inlet);
static void   updateInletStats(TInlet *inlet, double q);
static void   writeStreetStatsHeader();
static void   writeStreetStats(int link);

static int    getInletPlacement(TInlet *inlet, int node);
static void   getConduitGeometry(TInlet *inlet);
static double getFlowSpread(double flow);
static double getEo(double slopeRatio, double spread, double gutterWidth);

static double getOnGradeCapturedFlow(TInlet *inlet, double flow, double depth);
static double getOnGradeInletCapture(int inletIndex, double flow, double depth);
static double getGrateInletCapture(int inletIndex, double flow);
static double getCurbInletCapture(double flow, double length);
static double getGutterFlowRatio(double gutterWidth);
static double getGutterAreaRatio(double grateWidth, double area);
static double getSplashOverVelocity(int grateType, double grateLength);

static double getOnSagCapturedFlow(TInlet *inlet, double flow, double depth);
static double getOnSagInletCapture(int inletIndex, double depth);
static void   findOnSagGrateFlows(int inletIndex, double depth,
              double *weirFlow, double *orificeFlow);
static void   findOnSagCurbFlows(int inletIndex, double depth,
              double openingLength, double *weirFlow,
              double *orificeFlow);
static double getCurbOrificeFlow(double flowDepth, double openingHeight,
              double openingLength, int throatAngle);
static double getOnSagSlottedFlow(int inletIndex, double depth);

//=============================================================================

int  inlet_create(int numInlets)
//
//  Input:   numInlets = number of inlet designs to create
//  Output:  none
//  Purpose: creats a collection of inlet designs.
//
{
    int i;

    InletDesigns = NULL;
    InletDegree = NULL;
    InletFlow = NULL;
    InletDesignCount = 0;
    UsesInlets = FALSE;
    FirstInlet = NULL;
    InletDesigns = (TInletDesign *)calloc(numInlets, sizeof(TInletDesign));
    if (InletDesigns == NULL) return ERR_MEMORY;
    InletDesignCount = numInlets;

    InletDegree = (char *)calloc(Nobjects[NODE], sizeof(char));
    if (InletDegree == NULL) return ERR_MEMORY;
    InletFlow = (double *)calloc(Nobjects[NODE], sizeof(double));
    if (InletFlow == NULL) return ERR_MEMORY;    

    for (i = 0; i < InletDesignCount; i++)
    {
        InletDesigns[i].customInlet.onGradeCurve = -1;
        InletDesigns[i].customInlet.onSagCurve = -1;
        InletDesigns[i].curbInlet.length = 0.0;
        InletDesigns[i].grateInlet.length = 0.0;
        InletDesigns[i].slottedInlet.length = 0.0;
        InletDesigns[i].type = CUSTOM_INLET;
    }
    return 0;
}

//=============================================================================

void inlet_delete()
//
//  Input:   none
//  Output:  none
//  Purpose: frees all memory allocated for inlet analysis.
//
{
    TInlet *inlet = FirstInlet;
    TInlet *nextInlet;
    while (inlet)
    {
        nextInlet = inlet->nextInlet;
        free(inlet);
        inlet = nextInlet;
    }
    FirstInlet = NULL;
    FREE(InletFlow);
    FREE(InletDegree);
    FREE(InletDesigns);
}

//=============================================================================

int inlet_readDesignParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: extracts a set of inlet design parameters from a tokenized line
//           of the [INLETS] section of a SWMM input file.
//
//  Format of input line is:
//  ID  GRATE       Length  Width  GrateType  (OpenArea)  (SplashVeloc)
//  ID  CURB        Length  Height (ThroatType)
//  ID  SLOTTED     Length  Width
//  ID  DROP_GRATE  Length  Width  GrateType  (OpenArea)  (SplashVeloc)
//  ID  DROP_CURB   Length  Height
//  ID  CUSTOM      DiversionCurve  RatingCurve
//  
{
    int i;

    // --- check for minimum number of tokens
    if ( ntoks < 3 ) return error_setInpError(ERR_ITEMS, "");

    // --- check that design ID already registered in project
    i = project_findObject(INLET, tok[0]);
    if ( i < 0 ) return error_setInpError(ERR_NAME, tok[0]);
    InletDesigns[i].ID = project_findID(INLET, tok[0]);

    // --- retrieve type of inlet design
    InletDesigns[i].type = findmatch(tok[1], InletTypeWords);

    // --- read inlet's design parameters
    switch (InletDesigns[i].type)
    {
        case GRATE_INLET:
        case DROP_GRATE_INLET:
            return readGrateInletParams(i, tok, ntoks);
        case CURB_INLET:
        case DROP_CURB_INLET:
            return readCurbInletParams(i, tok, ntoks);
        case SLOTTED_INLET:
            return readSlottedInletParams(i, tok, ntoks);
        case CUSTOM_INLET:
            return readCustomInletParams(i, tok, ntoks);
        default: return error_setInpError(ERR_KEYWORD, tok[1]);
    }
    return 0;
}
//=============================================================================

int inlet_readUsageParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: extracts inlet usage parameters from a tokenized line
//           of the [INLET_USAGE] section of a SWMM input file.
//
//  Format of input line is:
//    linkID  inletID  nodeID (#Inlets %Clog  Qmax  aLocal wLocal placement)
//  where
//    linkID = ID name of link containing the inlet
//    inletID = ID name of inlet design being used
//    nodeID  = ID name of node receiving captured flow
//    #Inlets = number of identical inlets used (default = 1)
//    %Clog   = percent that inlet is clogged
//    Qmax    = maximum flow that inlet can capture (default = 0 (no limit))
//    aLocal  = local gutter depression (ft or m) (default = 0)
//    wLocal  = width of local gutter depression (ft or m) (default = 0)
//    placement = ON_GRADE, ON_SAG, or AUTO (the default)
//
{
    int    linkIndex, designIndex, nodeIndex, numInlets = 1;
    int    placement = AUTOMATIC;
    double flowLimit = 0.0, pctClogged = 0.0;
    double aLocal = 0.0, wLocal = 0.0;
    TInlet *inlet;

    // --- check that inlet's link exists
    if (ntoks < 3) return error_setInpError(ERR_ITEMS, "");
    linkIndex = project_findObject(LINK, tok[0]);
    if (linkIndex < 0) return error_setInpError(ERR_NAME, tok[0]);

    // --- check that inlet design type exists
    designIndex = project_findObject(INLET, tok[1]);
    if (designIndex < 0) return error_setInpError(ERR_NAME, tok[1]);

    // --- check that receiving node exists
    nodeIndex = project_findObject(NODE, tok[2]);
    if (nodeIndex < 0) return error_setInpError(ERR_NAME, tok[2]);

    // --- get number of inlets
    if (ntoks > 3)
        if (!getInt(tok[3], &numInlets) || numInlets < 0)
            return error_setInpError(ERR_NUMBER, tok[3]);

    // --- get flow limit & percent clogged    
    if (ntoks > 4)
        if (!getDouble(tok[4], &pctClogged) || pctClogged < 0.0
            || pctClogged > 100.)
            return error_setInpError(ERR_NUMBER, tok[4]);
    if (ntoks > 5)
        if (!getDouble(tok[5], &flowLimit) || flowLimit < 0.0)
            return error_setInpError(ERR_NUMBER, tok[5]);

    // --- get local depression parameters
    if (ntoks > 6)
        if (!getDouble(tok[6], &aLocal) || aLocal < 0.0)
            return error_setInpError(ERR_NUMBER, tok[6]);
    if (ntoks > 7)
        if (!getDouble(tok[7], &wLocal) || wLocal < 0.0)
            return error_setInpError(ERR_NUMBER, tok[7]);

    // --- get inlet placement
    if (ntoks > 8)
    {
        placement = findmatch(tok[8], PlacementTypeWords);
        if (placement < 0) return error_setInpError(ERR_KEYWORD, tok[8]);
    }

    // --- create an inlet usage object for the link
    inlet = Link[linkIndex].inlet;
    if (inlet == NULL)
    {
        inlet = (TInlet *)malloc(sizeof(TInlet));
        if (!inlet) return error_setInpError(ERR_MEMORY, "");
        Link[linkIndex].inlet = inlet;
        inlet->nextInlet = FirstInlet;
        FirstInlet = inlet;
    }

    // --- save inlet usage parameters
    inlet->linkIndex = linkIndex;
    inlet->designIndex = designIndex;
    inlet->nodeIndex = nodeIndex;
    inlet->numInlets = numInlets;
    inlet->placement = placement;
    inlet->clogFactor = 1.0 - (pctClogged / 100.);
    inlet->flowLimit = flowLimit / UCF(FLOW);
    inlet->localDepress = aLocal / UCF(LENGTH);
    inlet->localWidth = wLocal / UCF(LENGTH);
    inlet->flowFactor = 0.0;
    initInletStats(inlet);
    UsesInlets = TRUE;
    return 0;
}

//=============================================================================

void inlet_validate()
//
//  Input:   none
//  Output:  none
//  Purpose: checks that inlets have been assigned to conduits with proper
//           cross section shapes and counts the number of inlets that each
//           node receives either bypased or captured flow from.
//
{
    int i, inletType, isValid;
    TInlet *inlet;
    TInlet *prevInlet;

    // --- traverse the list of inlets placed in conduits
    memset(InletDegree, 0, Nobjects[NODE] * sizeof(char));
    prevInlet = FirstInlet;
    inlet = FirstInlet;
    while (inlet)
    {
        // --- check that inlet's conduit can accept the inlet's type
        isValid = FALSE;
        i = inlet->linkIndex;
        xsect = &Link[i].xsect;
        inletType = InletDesigns[inlet->designIndex].type;
        if (xsect->type == TRAPEZOIDAL && 
           (inletType == DROP_GRATE_INLET ||
            inletType == DROP_CURB_INLET ||
            inletType == CUSTOM_INLET))
            isValid = TRUE;
        else if (xsect->type == STREET_XSECT &&
            inletType != DROP_GRATE_INLET &&
            inletType != DROP_CURB_INLET)
            isValid = TRUE;

        // --- if inlet placement is valid then 
        if (isValid)
        {
            // --- update inlet count for inlet's bypass and capture nodes 
            inlet->backflow = 0.0;
            InletDegree[Link[i].node2]++;
            InletDegree[inlet->nodeIndex]++;

            // --- compute street inlet's flow factor
            //     (where Q = flowFactor * Spread^2.67)
            getConduitGeometry(inlet);
            inlet->flowFactor = (0.56/n) * pow(SL,0.5) * pow(Sx,1.67);            

            // --- save reference to current inlet & continue to next inlet
            prevInlet = inlet;
            inlet = inlet->nextInlet;
        }

        // --- if inlet placement is not valid then issue a warning message
        //     and remove the inlet from the conduit
        else
        {
            report_writeWarningMsg(WARN12, Link[i].ID);
            if (inlet == FirstInlet)
            {
                FirstInlet = inlet->nextInlet;
                prevInlet = FirstInlet;
                free(inlet);
                inlet = FirstInlet;
            }
            else
            {
                prevInlet->nextInlet = inlet->nextInlet;
                free(inlet);
                inlet = prevInlet->nextInlet;
            }
            Link[i].inlet = NULL;
        }
    }
}

//=============================================================================

void inlet_findInletFlows(double tStep)
//
//  Input:   tStep = current flow routing time step (sec)
//  Output:  none
//  Purpose: computes flow captured by each inlet and adjusts the
//           lateral flows of the inlet's bpass and capture nodes accordingly.
//
//  This function is called after regular lateral flows to all nodes have been
//  set but before a flow routing step has been taken.
{
    int    i, j, m, p, placement;
    double q, w;
    TInlet *inlet;

    // --- For non-DW routing find conduit flow into each node
    //     (used to limit max. amount of on-sag capture)
    if (RouteModel != DW)
    {
        for (j = 0; j < Nobjects[NODE]; j++)
            Node[j].inflow = 0.0;
        for (i = 0; i < Nobjects[LINK]; i++)
            Node[Link[i].node2].inflow += Link[i].newFlow;
    }

    // --- loop through each inlet
    for (inlet = FirstInlet; inlet != NULL; inlet = inlet->nextInlet)
    {
        // --- identify index of inlet's downstream node 
        i = inlet->linkIndex;
        j = Link[i].node2;

        // --- get inlet's placement (ON_GRADE or ON_SAG)
        placement = getInletPlacement(inlet, j);

        // --- find flow captured by on-grade inlet
        q = fabs(Link[i].newFlow);
        if (placement == ON_GRADE)
        {
            inlet->outflow = getOnGradeCapturedFlow(inlet, q, Node[j].newDepth);
        }

        // --- find flow captured by on-sag inlet
        else
        {
            inlet->outflow = getOnSagCapturedFlow(inlet, q, Node[j].newDepth);
        }
        if (fabs(inlet->outflow) < FUDGE) inlet->outflow = 0.0;

        // --- add to total flow captured by inlet's node
        InletFlow[j] += inlet->outflow;
    }

    // --- make second pass through each inlet
    for (inlet = FirstInlet; inlet != NULL; inlet = inlet->nextInlet)
    {
        // --- identify indexes of inlet's bypass (j) and capture (m) nodes 
        i = inlet->linkIndex;
        j = Link[i].node2;
        m = inlet->nodeIndex;

        // --- for on-sag placement under non-DW routing, captured flow
        //     is limited to inlet's share of bypass node's inflow plus
        //     any stored volume
        if (RouteModel != DW && placement == ON_SAG)
        {
            q = Node[j].newVolume / tStep;
            q += MAX(Node[j].inflow, 0.0);
            if (InletFlow[j] > q)
                inlet->outflow *= q / InletFlow[j];
        }

        // --- adjust lateral flows at bypass and capture nodes
        //     (subtract captured flow from bypass node, add it to capture
        //     node, and add any backflow to bypass node)
        Node[j].newLatFlow -= (inlet->outflow - inlet->backflow);
        Node[m].newLatFlow += inlet->outflow;

        // --- account for pollutant transfer between bypass and capture nodes
        for (p = 0; p < Nobjects[POLLUT]; p++)
        {
            w = inlet->outflow * Node[j].oldQual[p];
            Node[m].newQual[p] += w;

            if (RouteModel != DW)
            {
                w = inlet->backflow * Node[m].oldQual[p];
                Node[j].newQual[p] += w;
            }
        }

        // --- update inlet's performance if reporting has begun
        if (getDateTime(NewRoutingTime) > ReportStart)
            updateInletStats(inlet, fabs(Link[i].newFlow));
    }
}

//=============================================================================

void  inlet_convertOverflows()
//
//  Input:   none
//  Output:  none
//  Purpose: converts any overflows at capture nodes to inlet backflow.
//
//  This function is called after a flow routing time step has been taken.
{
    int i, j, m;
    TInlet *inlet;

    for (inlet = FirstInlet; inlet != NULL; inlet = inlet->nextInlet)
    {
        // --- identify inlet's bypass and capture nodes
        i = inlet->linkIndex;
        j = Link[i].node2;
        m = inlet->nodeIndex;

        // --- save capture node's overflow as inlet's backflow
        if (InletDegree[m] > 0)
        {
            inlet->backflow = Node[m].overflow / (double)InletDegree[m];
            if (fabs(inlet->backflow) < FUDGE) inlet->backflow = 0.0;
        }

        // --- remove all captured flow at the bypass node
        InletFlow[j] = 0.0;
    }

    // --- remove overflows at all inlet capture nodes
    for (inlet = FirstInlet; inlet != NULL; inlet = inlet->nextInlet)
    {
        Node[inlet->nodeIndex].overflow = 0.0;
    }
}

//=============================================================================

void inlet_writeStatsReport()
//
//  Input:   none
//  Output:  none
//  Purpose: writes table of street & inlet flow statistics to SWMM's report file.
//
{
    int j, header = FALSE;

    if (Nobjects[STREET] == 0) return;
    for (j = 0; j < Nobjects[LINK]; j++)
    {
        if (Link[j].xsect.type == STREET_XSECT)
        {
            if (!header)
            {
                writeStreetStatsHeader();
                header = TRUE;
            }
            writeStreetStats(j);
        }
    }
    report_writeLine("");
}

//=============================================================================

double inlet_capturedFlow(int i)
//
//  Input:   i = a link index
//  Output:  returns captured flow rate (cfs)
//  Purpose: gets the current flow captured by an inlet.
//
{
    if (Link[i].inlet) return Link[i].inlet->outflow;
    return 0.0;
}

//=============================================================================

int readGrateInletParams(int i, char* tok[], int ntoks)
{
//
//  Input:   i = inlet index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: extracts a grate's inlet parameters from a set of string tokens.
//
    int    grateType;
    double width, length, areaRatio = 0.0, vSplash = 0.0;

    // --- check for enough tokens
    if (ntoks < 5) return error_setInpError(ERR_ITEMS, "");

    // --- retrieve length & width
    if (!getDouble(tok[2], &length) || length <= 0.0)
        return error_setInpError(ERR_NUMBER, tok[2]);
    if (!getDouble(tok[3], &width) || width <= 0.0)
        return error_setInpError(ERR_NUMBER, tok[3]);

    // --- retrieve grate type
    grateType = findmatch(tok[4], GrateTypeWords);
    if (grateType < 0) return error_setInpError(ERR_KEYWORD, tok[4]);

    // --- only read open area & splash velocity for GENERIC type grate
    if (grateType == GENERIC)
    {
        if (ntoks < 6) return error_setInpError(ERR_ITEMS, "");
        if (!getDouble(tok[5], &areaRatio) || areaRatio <= 0.0
            || areaRatio > 1.0) return error_setInpError(ERR_NUMBER, tok[5]);
        if (ntoks > 6)
        {
            if (!getDouble(tok[6], &vSplash) || vSplash < 0.0)
                return error_setInpError(ERR_NUMBER, tok[6]);
        }
    }

    // --- save grate inlet parameters
    InletDesigns[i].grateInlet.length = length / UCF(LENGTH);
    InletDesigns[i].grateInlet.width = width / UCF(LENGTH);
    InletDesigns[i].grateInlet.type = grateType;
    InletDesigns[i].grateInlet.fracOpenArea = areaRatio;
    InletDesigns[i].grateInlet.splashVeloc = vSplash;

    // --- check if grate is part of a combo inlet (grate is a street and
    //     not a drop grate and data for a curb opening inlet has already
    //     been provided)
    if (InletDesigns[i].type == GRATE_INLET &&
        InletDesigns[i].curbInlet.length > 0.0)
        InletDesigns[i].type = COMBO_INLET;
    return 0;
}

//=============================================================================

int readCurbInletParams(int i, char* tok[], int ntoks)
//
//  Input:   i = inlet index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: extracts curb opening inlet parameters from a set of string tokens.
//
{
    int    throatAngle;
    double height, length;

    // --- check for enough tokens
    if (ntoks < 4) return error_setInpError(ERR_ITEMS, "");

    // --- retrieve length & width of opening
    if (!getDouble(tok[2], &length) || length <= 0.0)
        return error_setInpError(ERR_NUMBER, tok[2]);
    if (!getDouble(tok[3], &height) || height <= 0.0)
        return error_setInpError(ERR_NUMBER, tok[3]);

    // --- retrieve type of throat angle
    if (InletDesigns[i].type == CURB_INLET)
    {
        throatAngle = HORIZONTAL_THROAT;
        if (ntoks > 4)
        {
            throatAngle = findmatch(tok[4], ThroatAngleWords);
            if (throatAngle < 0) return error_setInpError(ERR_KEYWORD, tok[4]);
        }
    }
    else throatAngle = VERTICAL_THROAT; 

    // ---- save curb opening inlet parameters
    InletDesigns[i].curbInlet.length = length / UCF(LENGTH);
    InletDesigns[i].curbInlet.height = height / UCF(LENGTH);
    InletDesigns[i].curbInlet.throatAngle = throatAngle;

    // --- check if curb inlet is part of a combo inlet (opening is for a
    //     street and not a drop inlet and data for a grate inlet has already
    //     been provided)
    if (InletDesigns[i].type == CURB_INLET &&
        InletDesigns[i].grateInlet.length > 0.0)
        InletDesigns[i].type = COMBO_INLET;
    return 0;
}

//=============================================================================

int readSlottedInletParams(int i, char* tok[], int ntoks)
//
//  Input:   i = inlet index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: extracts slotted drain inlet parameters from a set of string tokens.
//
{
    double width, length;

    // --- check for enough tokens
    if (ntoks < 4) return error_setInpError(ERR_ITEMS, "");

    // --- retrieve length and width
    if (!getDouble(tok[2], &length) || length <= 0.0)
        return error_setInpError(ERR_NUMBER, tok[2]);
    if (!getDouble(tok[3], &width) || width <= 0.0)
        return error_setInpError(ERR_NUMBER, tok[3]);

    // --- save slotted inlet parameters
    InletDesigns[i].slottedInlet.length = length / UCF(LENGTH);
    InletDesigns[i].slottedInlet.width = width / UCF(LENGTH);
    return 0;
}

//=============================================================================

int readCustomInletParams(int i, char* tok[], int ntoks)
//
//  Input:   i = inlet index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: extracts custom inlet parameters from a set of string tokens.
//
{
    int c2, c3;    // capture curve indexes

    if (ntoks < 4) return error_setInpError(ERR_ITEMS, "");
    if (match(tok[2], "*")) c2 = -1;
    else
    {
        c2 = project_findObject(CURVE, tok[2]);
        if (c2 < 0) return error_setInpError(ERR_NAME, tok[2]);
    }
    InletDesigns[i].customInlet.onGradeCurve = c2;
    if (match(tok[3], "*"))
    {
        c3 = -1;
        if (c2 == -1) return error_setInpError(ERR_NAME, tok[3]);
    }
    else
    {
        c3 = project_findObject(CURVE, tok[3]);
        if (c3 < 0) return error_setInpError(ERR_NAME, tok[3]);
    }
    InletDesigns[i].customInlet.onSagCurve = c3;
    return 0;
}

//=============================================================================

void initInletStats(TInlet *inlet)
//
//  Input:   inlet = an inlet object placed in a conduit link
//  Output:  none
//  Purpose: initializes the performance statistics of an inlet.
//
{
    if (inlet)
    {
        inlet->outflow = 0.0;
        inlet->backflow = 0.0;
        inlet->stats.capturePeriods = 0;
        inlet->stats.backflowPeriods = 0;
        inlet->stats.peakFlow = 0.0;
        inlet->stats.peakFlowCapture = 0;
        inlet->stats.avgFlowCapture = 0;
        inlet->stats.bypassFreq = 0;
    }
}

//=============================================================================

void updateInletStats(TInlet *inlet, double q)
//
//  Input:   inlet = an inlet object placed in a conduit link
//           q = inlet's approach flow (cfs)
//  Output:  none
//  Purpose: updates the performance statistics of an inlet.
//
{
    double qCapture = inlet->outflow,
           qBackflow = inlet->backflow,
           qNet = qCapture - qBackflow,
           qBypass = q - qNet,
           fCapture = 0.0;

    // --- check for no flow condition
    if (q < MIN_RUNOFF_FLOW  && qBackflow <= 0.0) return;
    inlet->stats.flowPeriods++;

    // --- there is positive net flow from inlet to capture node
    if (qNet > 0.0)
    {
        inlet->stats.capturePeriods++;
        fCapture = qNet / q;
        fCapture = MIN(fCapture, 1.0);
        inlet->stats.avgFlowCapture += fCapture;
        if (qBypass > MIN_RUNOFF_FLOW) inlet->stats.bypassFreq++;
    }

    // --- otherwise inlet receives backflow from capture node
    else inlet->stats.backflowPeriods++;

    // --- update peak flow stats
    if (q > inlet->stats.peakFlow)
    {
        inlet->stats.peakFlow = q;
        inlet->stats.peakFlowCapture = fCapture * 100.0;
    }
}

//=============================================================================

void writeStreetStatsHeader()
{
    report_writeLine("");
    report_writeLine("*******************");
    report_writeLine("Street Flow Summary");
    report_writeLine("*******************");
    report_writeLine("");
    fprintf(Frpt.file,
"\n  ------------------------------------------------------------------------------------------------------------"
"\n                        Peak   Maximum   Maximum                   Peak Flow   Average      Bypass    BackFlow"
"\n                        Flow    Spread     Depth  Inlet              Capture   Capture   Frequency   Frequency");
    if (UnitSystem == US) fprintf(Frpt.file,
"\n  Street Conduit         %3s        ft        ft  Design                   %%         %%           %%           %%",
    FlowUnitWords[FlowUnits]);
    else fprintf(Frpt.file,
"\n  Conduit                %3s         m         m  Design                   %%         %%           %%           %%",
    FlowUnitWords[FlowUnits]);
    fprintf(Frpt.file,
"\n  ------------------------------------------------------------------------------------------------------------");
}

//=============================================================================

void writeStreetStats(int link)
//
//  Input:   link = index of a conduit link containing an inlet
//  Output:  none
//  Purpose: writes flow statistics for a Street conduit and its inlet
//           to SWMM's report file.
//
{
    int    k, t;
    double maxSpread, maxDepth, peakFlow;
    double fp, cp, afc = 0.0, bpf = 0.0;
    TInlet *inlet;

    // --- retrieve street parameters
    k = Link[link].subIndex;
    t = Link[link].xsect.transect;

    // --- get depth & spread at peak flow
    //     (based on flow routing result and street's transect geometry)
    peakFlow = LinkStats[link].maxFlow;
    maxDepth = LinkStats[link].maxDepth;
    maxSpread = xsect_getWofY(&Link[link].xsect, maxDepth) / Street[t].sides;
/*
    // Alternate method from HEC-22's use of Izzard's form of the Manning eqn.)
    Sx = Street[t].slope;
    a = Street[t].gutterDepression;
    W = Street[t].gutterWidth;
    n = Street[t].roughness;
    Qfactor = (0.56 / n) * sqrt(Conduit[k].slope) * pow(Sx, 1.67);
    maxSpread = getFlowSpread(peakFlow / Street[t].sides);
    maxDepth = maxSpread * Sx + a;
*/
    // --- write street stats
    fprintf(Frpt.file, "\n  %-16s", Link[link].ID);
    fprintf(Frpt.file, " %9.2f", peakFlow * UCF(FLOW));
    fprintf(Frpt.file, " %9.2f", maxSpread * UCF(LENGTH));
    fprintf(Frpt.file, " %9.2f", maxDepth * UCF(LENGTH));

    // --- write inlet stats
    inlet = Link[link].inlet;
    if (inlet)
    {
        fprintf(Frpt.file, "  %-16s", InletDesigns[inlet->designIndex].ID);
        fp = inlet->stats.flowPeriods / 100.0;
        if (fp > 0.0)
        {
            cp = inlet->stats.capturePeriods / 100.0;
            fprintf(Frpt.file, " %9.2f", inlet->stats.peakFlowCapture);
            if (cp > 0.0)
            {
                afc = inlet->stats.avgFlowCapture / cp;
                bpf = inlet->stats.bypassFreq / cp;
            }
            fprintf(Frpt.file, " %9.2f", afc);
            fprintf(Frpt.file, "   %9.2f", bpf);
            fprintf(Frpt.file, "   %9.2f", inlet->stats.backflowPeriods / fp);
        }
    }
}

//=============================================================================

int getInletPlacement(TInlet *inlet, int j)
//
//  Input:   inlet = an inlet object placed in a conduit link
//           j = index of inlet's bypass node
//  Output:  returns type of inlet placement
//  Purpose: determines actual placement for an inlet with AUTOMATIC placement.
//
{
    if (inlet->placement == AUTOMATIC)
    {
        if (Node[j].degree > 0) return ON_GRADE;
        else return ON_SAG;
    }
    else return inlet->placement;
}

//=============================================================================

void  getConduitGeometry(TInlet *inlet)
//
//  Input:   inlet = an inlet object placed in a conduit link
//  Output:  none
//  Purpose: assigns properties of an inlet's conduit to
//           module-level shared variables used by other functions.
//
{
    int linkIndex = inlet->linkIndex;
    int t, k = Link[linkIndex].subIndex;

    SL = Conduit[k].slope;                       // longitudinal slope
    Beta = Conduit[k].beta;                      // 1.486 * sqrt(SL) / n
    xsect = &Link[linkIndex].xsect;

    // --- if conduit has a Street cross section
    if (xsect->type == STREET_XSECT)
    {
        t = Link[linkIndex].xsect.transect;
        Sx = Street[t].slope;                    // street cross slope
        a = Street[t].gutterDepression;          // gutter depression
        W = Street[t].gutterWidth;               // gutter width
        n = Street[t].roughness;                 // street roughness
        Nsides = Street[t].sides;                // 1 or 2 sided street
        Tcrown = Street[t].width;                // distance from curb to crown
        Qfactor = inlet->flowFactor;             // factor used in Izzard's eqn.

        // --- add inlet's local depression to street's continuous depression
        if (inlet && inlet->localDepress * inlet->localWidth > 0)
        {
            a += inlet->localDepress;            // inlet depression
            W = inlet->localWidth;               // inlet depressed width
        }

        // --- slope of depressed gutter section
        if (W * a > 0.0) Sw = Sx + a / W;
        else             Sw = Sx;
    }

    // --- conduit has rectangular or trapezoidal cross section
    else
    {
        a = 0.0;
        W = 0.0;
        n = Conduit[k].roughness;
        Nsides = 1;
        Sx = 0.01;
        Sw = Sx;
    }
}

//=============================================================================

double getFlowSpread(double Q)
//  
//  Input:   Q = conduit flow rate (cfs)
//  Output:  returns width of flow spread (ft)
//  Purpose: computes width of flow spread across a Street cross section using
//           HEC-22 equations derived from Izzard's form of the Manning eqn.
//
{
    int    iter;
    double f, f1, Sr, Ts1, Ts2, Tw, Qs, Eo;

    f = Qfactor;   // = (0.56/n) * SL^0.5 * Sx^1.67

    // --- no depressed curb
    if (a == 0.0)
    {
        Ts1 = pow(Q / f, 0.375);                           //HEC-22 Eq(4-2)
    }
    else
    {
        // --- check if spread is within curb width
        f1 = f * pow((a / W) / Sx, 1.67);
        Tw = pow(Q / f1, 0.375);                           //HEC-22 Eq(4-2)
        if (Tw <= W) Ts1 = Tw;
        else
        {
            // --- spread extends beyond curb width
            Sr = (Sx + a / W) / Sx;
            iter = 1;
            Ts1 = pow(Q / f, 0.375) - W;
            if (Ts1 <= 0) Ts1 = Tw - W;
            while (iter < 11)
            {
                Eo = getEo(Sr, Ts1, W);
                Qs = (1.0 - Eo) * Q;                       //HEC-22 Eq(4-6)
                Ts2 = pow(Qs / f, 0.375);                  //HEC-22 Eq(4-2)
                if (fabs(Ts2 - Ts1) < 0.01) break;
                Ts1 = Ts2;
                iter++;
            }
            Ts1 = Ts2 + W;
        } 
    }
    return MIN(Ts1, Tcrown);
}

//=============================================================================

double getEo(double Sr, double Ts, double w)
//
//  Input:   Sr = ratio of gutter slope to street cross slope
//           Ts = amount of flow spread outside of gutter width (ft)
//           w  = gutter width (ft)
//  Output:  returns ratio of gutter flow to total flow in street cross section
//  Purpose: solves HEC-22 Eq. (4-4) for Eo with Ts/w substituted for
//           (T/w) - 1 where Ts = T - w.
//
{
    double x;
    x = Sr / (Ts / w);
    x = pow((1.0 + x), 2.67) - 1.0;
    x = 1.0 + Sr / x;
    return 1.0 / x;
}

//=============================================================================

double getOnGradeCapturedFlow(TInlet *inlet, double q, double d)
//
//  Input:   inlet = an inlet object placed in a conduit link
//           q = flow in link prior to any inlet capture (cfs)
//           d = flow depth seen by inlet (ft)
//  Output:  returns flow captured by the inlet (cfs)
//  Purpose: computes flow captured by an inlet placed on-grade.
//
//  An inlet object placed in a conduit can have multiple inlets of
//  the same type distributed along the conduit's length that all
//  send their captured flow to the same sewer node. This function
//  finds the total captured flow as each individual inlet is analyzed
//  sequentially, where its approach flow has been reduced by the
//  amount of flow captured by prior inlets.
{
    int    i,
           linkIndex;        // index of link containing inlets
    double qApproach,        // single inlet's approach flow (cfs)
           qc,               // single inlet's captured flow (cfs)
           qCaptured,        // total flow captured by link's inlets (cfs)
           qBypassed,        // total flow bypassed by link's inlets (cfs)
           qMax;             // max. flow that a single inlet can capture (cfs)

    if (inlet->numInlets == 0) return 0.0;
    linkIndex = inlet->linkIndex;

    // --- check that link has flow
    qApproach = q;
    if (qApproach < MIN_RUNOFF_FLOW) return 0.0;

    // --- store conduit geometry in shared variables
    getConduitGeometry(inlet);

    // --- adjust flow for 2-sided street
    qApproach /= Nsides;
    qBypassed = qApproach;
    qCaptured = 0.0;

    // --- set limit on max. flow captured per inlet
    qMax = BIG;
    if (inlet->flowLimit > 0.0) qMax = inlet->flowLimit;

    // --- evaluate each inlet
    for (i = 1; i <= inlet->numInlets; i++)
    {
        qc = getOnGradeInletCapture(inlet->designIndex, qBypassed, d) *
            inlet->clogFactor;
        qc = MIN(qc, qMax);
        qCaptured += qc;
        qBypassed -= qc;
        if (qBypassed < MIN_RUNOFF_FLOW) break;
    }
    return qCaptured *= Nsides;
}

//=============================================================================

double getOnGradeInletCapture(int i, double Q, double d)
//
//  Input:   i = an InletDesigns index
//           Q = flow rate seen by inlet (cfs)
//           d = flow depth seen by inlet (ft)
//  Output:  returns captured flow rate (cfs)
//  Purpose: finds the flow captured by a single on-grade inlet.
//
{
    int    c;
    double Q1 = Q, Qc = 0.0, Lsweep = 0.0, Lcurb = 0.0, Lgrate = 0.0;

    // --- custom inlet -- use onGrade curve if present or onSag curve otherwise
    c = InletDesigns[i].customInlet.onGradeCurve;
    if (c >= 0)
    {
        Qc = table_lookupEx(&Curve[c], Q * UCF(FLOW)) / UCF(FLOW);             
        return MIN(Qc, Q);
    }
    c = InletDesigns[i].customInlet.onSagCurve;
    if (c >= 0)
    {
        Qc = table_lookupEx(&Curve[c], d * UCF(LENGTH)) / UCF(FLOW);
        return MIN(Qc, Q);
    }

    // --- drop curb inlet (in non-Street conduit) only operates in on sag mode
    if (InletDesigns[i].type == DROP_CURB_INLET)
    {
        Qc = getOnSagInletCapture(i, d);
        return MIN(Qc, Q);
    }

    // --- drop grate inlet (in non-Street conduit)
    if (InletDesigns[i].type == DROP_GRATE_INLET)
    {
        Qc = getGrateInletCapture(i, Q);
        return MIN(Qc, Q);
    }

    // --- Remaining inlet types apply to Street conduits

    // --- find flow spread
    T = getFlowSpread(Q);

    // --- slotted inlet (behaves as a curb opening inlet per HEC-22)
    if (InletDesigns[i].type == SLOTTED_INLET)
    {
        Qc = getCurbInletCapture(Q, InletDesigns[i].slottedInlet.length);
        return MIN(Qc, Q);
    }

    Lcurb = InletDesigns[i].curbInlet.length;
    Lgrate = InletDesigns[i].grateInlet.length;

    // --- curb opening inlet
    if (Lcurb > 0.0)
    {
        Lsweep = Lcurb - Lgrate;
        if (Lsweep > 0.0)
        {
            Qc = getCurbInletCapture(Q1, Lsweep);
            Q1 -= Qc;
        }
    }

    // --- grate inlet
    if (Lgrate > 0.0 && Q1 > 0.0)
    {
        if (Q1 != Q) T = getFlowSpread(Q1);
        Qc += getGrateInletCapture(i, Q1);
    }
    return Qc;
}

//=============================================================================

double getGrateInletCapture(int i, double Q)
//
//  Input:   i = inlet type index 
//           Q = flow rate seen by inlet (cfs)
//  Output:  returns captured flow rate (cfs)
//  Purpose: finds the flow captured by an on-grade grate inlet.
//
{
    int    grateType;
    double Lg,          // grate length (ft)
           Wg,          // grate width (ft)
           A,           // total cross section flow area (ft2)
           Y,           // flow depth (ft)
           Eo,          // ratio of gutter to total flow
           V,           // flow velocity (ft/s)
           Vo,          // splash-over velocity (ft/s)
           Qo = Q,      // flow over street area (cfs)
           Rf = 1.0,    // ratio of intercepted to total frontal flow
           Rs = 0.0;    // ratio of intercepted to total side flow

// xsect, a, W, & Sx were from getConduitGeometry(). T was from getFlowSpread().

    Lg = InletDesigns[i].grateInlet.length;
    Wg = InletDesigns[i].grateInlet.width;

    // --- flow ratio for drop inlet
    if (xsect->type == TRAPEZOIDAL)
    {
        A = xsect_getAofS(xsect, Q / Beta);
        Y = xsect_getYofA(xsect, A);
        T = xsect_getWofY(xsect, Y);
        Eo = Beta * pow(Y*Wg, 1.67) / pow(Wg + 2*Y, 0.67) / Q;
        if (Wg > 0.99*xsect->yBot && xsect->sBot > 0.0)
        {
          Wg = xsect->yBot;
          Sx = 1.0 / xsect->sBot;
        }
    }

    // --- flow ratio & area for conventional street gutter
    else if (a == 0.0)
    {
        A = T * T * Sx / 2.0;
        Eo = getGutterFlowRatio(Wg);   // flow ratio based on grate width
        if (T >= Tcrown) Qo = Qfactor * pow(Tcrown, 2.67);
    }

    // --- flow ratio & area for composite street gutter
    else
    {
        // --- spread confined to gutter
        if (T <= W) A = T * T * a / W / 2.0;

        // --- spread beyond gutter width
        else A = (T * T * Sx + a * W) / 2.0;

        // flow ratio based on gutter width corrected for grate width
        Eo = getGutterFlowRatio(W);
        if (Eo < 1.0)
        {
            if (T >= Tcrown)
                Qo = Qfactor * pow(Tcrown, 2.67) / (1.0 - Eo);
            Eo = Eo * getGutterAreaRatio(Wg, A);           //HEC-22 Eq(4-20a)
        }
    }

    // --- flow and splash-over velocities
    V = Qo / A;
    grateType = InletDesigns[i].grateInlet.type;
    if (grateType < 0 || grateType == GENERIC)
        Vo = InletDesigns[i].grateInlet.splashVeloc;
    else
        Vo = getSplashOverVelocity(grateType, Lg);

    // --- frontal flow capture efficiency
    if (V > Vo) Rf = 1.0 - 0.09 * (V - Vo);                //HEC-22 Eq(4-18)

    // --- side flow capture efficiency
    if (Eo < 1.0)
    {
        Rs = 1.0 / (1.0 + (0.15 * pow(V, 1.8) /
            Sx / pow(Lg, 2.3)));                          //HEC-22 Eq(4-19)
    }

    // --- return total flow captured
    return Q * (Rf * Eo + Rs * (1.0 - Eo));                //HEC-22 Eq(4-21)
}

//=============================================================================

double getCurbInletCapture(double Q, double L)
//
//  Input:   Q = flow rate seen by inlet (cfs)
//           L = length of inlet opening (ft)
//  Output:  returns captured flow rate (cfs)
//  Purpose: finds the flow captured by an on-sag inlet.
//
{
    double Se = Sx,     // equivalent gutter slope
           Lt,          // length for full capture
           Sr,          // ratio of gutter slope to cross slope
           Eo = 0.0,    // ratio of gutter to total flow
           E = 1.0;     // capture efficiency

// a, W, Sx, Sw, SL, & n were from getConduitGeometry(). T was from getFlowSpread().

    // --- for depressed gutter section
    if (a > 0.0)
    {
        Sr = (Sx + a / W) / Sx;
        Eo = getEo(Sr, T-W, W);
        Se = Sx + Sw * Eo;                                 //HEC-22 Eq(4-24)
    }

    // --- opening length for full capture
    Lt = 0.6 * pow(Q, 0.42) * pow(SL, 0.3) *
         pow(1.0/(n*Se), 0.6);                             //HEC-22 Eq(4-22a)

    // --- capture efficiency for actual opening length
    if (L < Lt)
    {
        E = 1.0 - (L/Lt);
        E = 1 - pow(E, 1.8);                               //HEC-22 Eq(4-23)
    }
    E = MIN(E, 1.0);
    E = MAX(E, 0.0);
    return E * Q;
}

//=============================================================================

double getGutterFlowRatio(double w)
//
//  Input:   w = gutter width (ft) 
//  Output:  returns a flow ratio
//  Purpose: computes the ratio of flow over a width of gutter to the total
//           flow in a street cross section.
//
{
    if (T <= w) return 1.0;
    else if (a > 0.0)
        return getEo(Sw / Sx, T - w, w);
    else
        return 1.0 - pow((1.0 - w / T), 2.67);             //HEC-22 Eq(4-16)
}

//=============================================================================

double getGutterAreaRatio(double Wg, double A)
//
//  Input:   Wg = width of grate inlet (ft)
//           A = gutter area (ft2)
//  Output:  returns an area ratio
//  Purpose: computes the ratio of the flow area above a grate to the flow
//           area above the gutter in a street cross section.
//
{
    double Aw,     // flow area beyond gutter width (ft2)
           Ag;     // flow area beyond grate width (ft2)

    if (Wg >= W) return 1.0;
    Aw = SQR((T - W)) * Sx;
    Ag = Aw + SQR((W - Wg)) * Sw;
    return (A - Ag) / (A - Aw);
}

//=============================================================================

double getSplashOverVelocity(int grateType, double L)
//
//  Input:   grateType = grate inlet type code
//           L = length of grate inlet (ft)
//  Output:  returns a splash over velocity 
//  Purpose: computes the splash over velocity for a standard type of grate
//           inlet as a function of its length.
//
{
    return SplashCoeffs[grateType][0] +
           SplashCoeffs[grateType][1] * L -
           SplashCoeffs[grateType][2] * L * L +
           SplashCoeffs[grateType][3] * L * L * L;
}

//=============================================================================

double getOnSagCapturedFlow(TInlet *inlet, double q, double d)
//
//  Input:   inlet = an inlet object placed in a conduit link
//           q = flow in link prior to any inlet capture (cfs)
//           d = flow depth seen by inlet (ft)
//  Output:  returns flow captured by the inlet (cfs)
//  Purpose: computes flow captured by an inlet placed on-sag.
//
{
    int c, numInlets, linkIndex, designIndex;
    double qCaptured = 0.0, qMax = HUGE;

    numInlets = inlet->numInlets;
    if (numInlets == 0) return 0.0;
    linkIndex = inlet->linkIndex;
    designIndex = inlet->designIndex;

    // --- store conduit geometry in shared variables
    getConduitGeometry(inlet);

    // --- set flow limit per inlet
    if (inlet->flowLimit > 0.0) qMax = inlet->flowLimit;

    // --- find nominal flow captured by inlet
    // --- inlet has a custom rating curve
    c = InletDesigns[designIndex].customInlet.onSagCurve;
    if (c >= 0)
        qCaptured = table_lookupEx(&Curve[c], d * UCF(LENGTH)) / UCF(FLOW);
    else
    {
        // --- inlet has a custom diversion curve
        c = InletDesigns[designIndex].customInlet.onGradeCurve;
        if (c >= 0)
        {
            qCaptured = table_lookupEx(&Curve[c], q * UCF(FLOW)) / UCF(FLOW);
            qCaptured = MIN(qCaptured, q);
        }

        // --- use HEC-22 method for all other inlet types
        else
            qCaptured = getOnSagInletCapture(designIndex, fabs(d));
    }

    // --- find actual flow captured by the inlet
    qCaptured *= inlet->clogFactor;
    qCaptured = MIN(qCaptured, qMax);
    qCaptured *= Nsides * inlet->numInlets;
    return qCaptured;
}

//=============================================================================

double getOnSagInletCapture(int i, double d)
//
//  Input:   i = inlet type index 
//           d = water level seen by inlet (ft)
//  Output:  returns captured flow rate (cfs)
//  Purpose: finds the flow captured by an on-sag inlet.
//
{
    double Lsweep = 0.0, Lcurb = 0.0, Lgrate = 0.0;
    double Qsw = 0.0,   //Sweeper curb opening weir flow
           Qso = 0.0,   //Sweeper curb opening orifice flow
           Qgw = 0.0,   //Grate weir flow
           Qgo = 0.0,   //Grate orifice flow
           Qcw = 0.0,   //Curb opening weir flow
           Qco = 0.0;   //Curb opening orifice flow

    if (InletDesigns[i].slottedInlet.length > 0.0)
        return getOnSagSlottedFlow(i, d);

    Lgrate = InletDesigns[i].grateInlet.length;
    if (Lgrate > 0.0) findOnSagGrateFlows(i, d, &Qgw, &Qgo);

    Lcurb = InletDesigns[i].curbInlet.length;
    if (Lcurb > 0.0)
    {
        Lsweep = Lcurb - Lgrate;
        if (Lsweep > 0.0) findOnSagCurbFlows(i, d, Lsweep, &Qsw, &Qso);
        if (Qgo > 0.0) findOnSagCurbFlows(i, d, Lgrate, &Qcw, &Qco);
    }
    return Qgw + Qgo + Qsw + Qso + Qco;
}

//=============================================================================

void findOnSagGrateFlows(int i, double d, double *Qw, double *Qo)
//
//  Input:   i = inlet type index 
//           d = water level seen by inlet (ft)
//  Output:  Qw = flow captured in weir mode (cfs)
//           Qo = flow captured in orifice mode (cfs)
//  Purpose: finds the flow captured by an on-sag grate inlet.
//
{
    int    grateType = InletDesigns[i].grateInlet.type;
    double Lg = InletDesigns[i].grateInlet.length;
    double Wg = InletDesigns[i].grateInlet.width;
    double P,      // grate perimeter (ft)
           Ao,     // grate opening area (ft2)
           di;     // average flow depth across grate (ft)

    // --- for drop grate inlets
    if (InletDesigns[i].type == DROP_GRATE_INLET)
    {
        di = d;
        P = 2.0 * (Lg + Wg);
    }

    // --- for gutter grate inlets:
    else
    {
        // --- check for spread within grate width
        if (d <= Wg * Sw) Wg = d / Sw;

        // --- avergage depth over grate
        di = d - (Wg / 2.0) * Sw;

        // --- effective grate perimeter & area
        P = Lg + 2.0 * Wg;
    }

    if (grateType == GENERIC)
        Ao = Lg * Wg * InletDesigns[i].grateInlet.fracOpenArea;
    else
        Ao = Lg * Wg * GrateOpeningRatios[grateType];

    // --- weir flow applies (based on  depth where result of
    //     weir eqn. equals result of orifice eqn.)

    if (d <= 1.79 * Ao / P)
    {
        *Qw = 3.0 * P * pow(di, 1.5);                      //HEC-22 Eq(4-26)
    }

    // --- orifice flow applies
    else
    {
        *Qo = 0.67 * Ao * sqrt(2.0 * 32.16 * di);          //HEC-22 Eq(4-27)
    }
}

//=============================================================================

void findOnSagCurbFlows(int i, double d, double L, double *Qw, double *Qo)
//
//  Input:   i = inlet type index 
//           d = water level seen by inlet (ft)
//           L = length of curb opening (ft)
//  Output:  Qw = flow captured in weir mode (cfs)
//           Qo = flow captured in orifice mode (cfs)
//  Purpose: finds the flow captured by an on-sag curb opening inlet.
//
{
    int    throatAngle = InletDesigns[i].curbInlet.throatAngle;
    double h = InletDesigns[i].curbInlet.height;
    double Qweir, Qorif, P;
    double dweir, dorif, r;

    // --- check for orifice flow
    if (L <= 0.0) return;
    if (InletDesigns[i].type == DROP_CURB_INLET) L = L * 4.0;
    dorif = 1.4 * h;
    if (d > dorif)
    {
        *Qo = getCurbOrificeFlow(d, h, L, throatAngle);
        return;
    }
    
    // --- for uniform cross slope or very long opening
    if (a == 0.0 || L > 12.0)
    {
        // --- check for weir flow
        dweir = h;
        if (d < dweir)
        {
            *Qw = 3.0 * L * pow(d, 1.5);                   //HEC-22 Eq(4-30)
            return;
        }
        else Qweir = 3.0 * L * pow(dweir, 1.5);
    }

    // --- for depressed gutter    
    else
    {
        // --- check for weir flow
        P = L + 1.8 * W;
        dweir = h + a;
        if (d < dweir)
        {
            *Qw = 2.3 * P * pow(d, 1.5);                   //HEC-22 Eq(4-28)
            return;
        }
        else Qweir = 2.3 * P * pow(dweir, 1.5);
    }

    // --- interpolate between Qwier at depth dweir and Qorif at depth dorif
    Qorif = getCurbOrificeFlow(dorif, h, L, throatAngle);
    r = (d - dweir) / (dorif - dweir);
    *Qw = r * Qweir;
    *Qo = (1 - r) * Qorif;
}

//=============================================================================

double getCurbOrificeFlow(double di, double h, double L, int throatAngle)
//
//  Input:   di = water level at lip of inlet opening (ft)
//           h = height of curb opening (ft)
//           L = length of curb opening (ft)
//           throatAngle = type of throat angle in curb opening
//  Output:  return flow captured by inlet (cfs)
//  Purpose: finds the flow captured by an on-sag curb opening inlet under
//           orifice flow conditions.
//
{
    double d = di;
    if (throatAngle == HORIZONTAL_THROAT)
        d = di - h / 2.0;
    else if (throatAngle == INCLINED_THROAT)
        d = di + (h / 2.0) * 0.7071;
    return 0.67 * h * L * sqrt(2.0 * 32.16 * d);           //HEC-22 Eq(4-31a)
}

//=============================================================================

double getOnSagSlottedFlow(int i, double d)
//
//  Input:   i = inlet type index 
//           d = water level seen by inlet (ft)
//  Output:  returns captured flow rate (cfs)
//  Purpose: finds the flow captured by an on-sag slotted inlet.
//
{
    double L = InletDesigns[i].slottedInlet.length;
    double w = InletDesigns[i].slottedInlet.width;
    double qw, qo;

    if (d <= 0.2) return 2.48 * L * pow(d, 1.5);           //HEC-22 Eq(4-32)
    if (d >= 0.4) return 0.8 * L * w * sqrt(64.32 * d);    //HEC-22 Eq(4-33)
    qw = 3.9305351 * L;
    qo = 4.057822076 * L * w;
    return qw + (d / 0.2 - 1.0) * (qo - qw);
}
