//-----------------------------------------------------------------------------
//   transect.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//   Author:   L. Rossman
//
//   Geometry processing for irregular cross-section transects.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
#define MAXSTATION 1500                // max. number of stations in a transect

//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
static int    Ntransects;              // total number of transects
static int    Nstations;               // number of stations in current transect
static double  Station[MAXSTATION+1];  // x-coordinate of each station
static double  Elev[MAXSTATION+1];     // elevation of each station
static double  Nleft;                  // Manning's n for left overbank
static double  Nright;                 // Manning's n for right overbank
static double  Nchannel;               // Manning's n for main channel
static double  Xleftbank;              // station where left overbank ends
static double  Xrightbank;             // station where right overbank begins
static double  Xfactor;                // multiplier for station spacing
static double  Yfactor;                // factor added to station elevations
static double  Lfactor;                // main channel/flood plain length

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)   
//-----------------------------------------------------------------------------
//  transect_create      (called by createObjects in project.c)
//  transect_delete      (called by deleteObjects in project.c)
//  transect_readParams  (called by parseLine in input.c)
//  transect_validate    (called by input_readData)

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static int    setParams(int transect, char* id, double x[]);
static int    setManning(double n[]);
static int    addStation(double x, double y);
static double getFlow(int k, double a, double wp, int findFlow);
static void   getGeometry(int i, int j, double y);
static void   getSliceGeom(int k, double y, double yu, double yd, double *w,
              double *a, double *wp);
static void   setMaxSectionFactor(int transect);

//=============================================================================

int transect_create(int n)
//
//  Input:   n = number of transect objects to create
//  Output:  returns an error code
//  Purpose: creates an array of cross-section transects.
//
{
    Ntransects = n;
    if ( n == 0 ) return 0;
    Transect = (TTransect *) calloc(Ntransects, sizeof(TTransect));
    if ( Transect == NULL ) return ERR_MEMORY;
    Nchannel = 0.0;
    Nleft = 0.0;
    Nright = 0.0;
    Nstations = 0;
    return 0;
}

//=============================================================================

void transect_delete(void)
//
//  Input:   none
//  Output:  none
//  Purpose: deletes memory allocated for all transects.
//
{
    if ( Ntransects == 0 ) return;
    FREE(Transect);
    Ntransects = 0;
}

//=============================================================================

int transect_readParams(int* count, char* tok[], int ntoks)
//
//  Input:   count = transect index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  updated value of count,
//           returns an error code
//  Purpose: read parameters of a transect from a tokenized line of input data.
//
//  Format of transect data follows that used for HEC-2 program:
//    NC  nLeft  nRight  nChannel
//    X1  name  nSta  xLeftBank  xRightBank  0  0  0  xFactor  yFactor
//    GR  Elevation  Station  ... 
//
{
    int    i, k;
    int    index = *count;             // transect index
    int    errcode;                    // error code
    double x[10];                      // parameter values
    char*  id;                         // transect ID name

    // --- match first token to a transect keyword
    k = findmatch(tok[0], TransectKeyWords);
    if ( k < 0 ) return error_setInpError(ERR_KEYWORD, tok[0]);

    // --- read parameters associated with keyword
    switch ( k )
    {
      // --- NC line: Manning n values
      case 0:

        // --- finish processing the previous transect
        transect_validate(index - 1);

        // --- read Manning's n values
        if ( ntoks < 4 ) return error_setInpError(ERR_ITEMS, "");
        for (i = 1; i <= 3; i++)
        {
            if ( ! getDouble(tok[i], &x[i]) )
                return error_setInpError(ERR_NUMBER, tok[i]);
        }
        return setManning(x);

      // --- X1 line: identifies start of next transect
      case 1:

        // --- check that transect was already added to project
        //     (by input_countObjects)
        if ( ntoks < 10 ) return error_setInpError(ERR_ITEMS, "");
        id = project_findID(TRANSECT, tok[1]);
        if ( id == NULL ) return error_setInpError(ERR_NAME, tok[1]);

        // --- read in rest of numerical values on data line
        for ( i = 2; i < 10; i++ )
        {
            if ( ! getDouble(tok[i], &x[i]) )
                return error_setInpError(ERR_NUMBER, tok[i]);
        }

        // --- update total transect count
        *count = index + 1;

        // --- transfer parameter values to transect's properties
        return setParams(index, id, x);

      // --- GR line: station elevation & location data
      case 2:

        // --- check that line contains pairs of data values
        if ( (ntoks - 1) % 2 > 0 ) return error_setInpError(ERR_ITEMS, "");

        // --- parse each pair of Elevation-Station values
        i = 1;
        while ( i < ntoks )
        {
            if ( ! getDouble(tok[i], &x[1]) )
                return error_setInpError(ERR_NUMBER, tok[i]);
            if ( ! getDouble(tok[i+1], &x[2]) )
                return error_setInpError(ERR_NUMBER, tok[i+1]);
            errcode = addStation(x[1], x[2]);
            if ( errcode ) return errcode;
            i += 2;
        }
        return 0;
    }
    return 0;
}

//=============================================================================

void  transect_validate(int j)
//
//  Input:   j = transect index
//  Output:  none
//  Purpose: validates transect data and creates its geometry tables.
//
{
    int    i, nLast;
    double dy, y, ymin, ymax;
    double oldNchannel = Nchannel;

    // --- check for valid transect data
    if ( j < 0 || j >= Ntransects ) return;
    if ( Nstations < 2 ) 
    {
        report_writeErrorMsg(ERR_TRANSECT_TOO_FEW, Transect[j].ID);
        return;
    }
    if ( Nstations >= MAXSTATION )
    {
        report_writeErrorMsg(ERR_TRANSECT_TOO_MANY, Transect[j].ID);
        return;
    }
    if ( Nchannel <= 0.0 )
    {
        report_writeErrorMsg(ERR_TRANSECT_MANNING, Transect[j].ID);
        return;
    }
    if ( Xleftbank > Xrightbank )
    {
        report_writeErrorMsg(ERR_TRANSECT_OVERBANK, Transect[j].ID);
        return;
    }

    // --- adjust main channel's Mannings n to make its equivalent
    //     length equal to that of entire flood plain
    Nchannel = Nchannel * sqrt(Lfactor);
    Transect[j].lengthFactor = Lfactor;

    // --- find max. depth across transect
    ymax = Elev[1];
    ymin = Elev[1];
    for (i = 2; i <= Nstations; i++)
    {
        ymax = MAX(Elev[i], ymax);
        ymin = MIN(Elev[i], ymin);
    }
    if ( ymin >= ymax )
    {
        report_writeErrorMsg(ERR_TRANSECT_NO_DEPTH, Transect[j].ID);
        return;
    }
    Transect[j].yFull = ymax - ymin;

    // --- add vertical sides to transect to reach full ht. on both ends
    Station[0] = Station[1];
    Elev[0] = ymax;
    Nstations++;
    Station[Nstations] = Station[Nstations-1];
    Elev[Nstations] = Elev[0];

    // --- determine size & depth increment for geometry tables
    Transect[j].nTbl = N_TRANSECT_TBL;
    dy = (ymax - ymin) / (double)(Transect[j].nTbl - 1);

    // --- set 1st table entries to zero
    Transect[j].areaTbl[0] = 0.0;
    Transect[j].hradTbl[0] = 0.0;
    Transect[j].widthTbl[0] = 0.0;

    // --- compute geometry for each depth increment
    y = ymin;
    Transect[j].wMax = 0.0;
    for (i = 1; i < Transect[j].nTbl; i++)
    {
        y += dy;
        Transect[j].areaTbl[i] = 0.0;
        Transect[j].hradTbl[i] = 0.0;
        Transect[j].widthTbl[i] = 0.0;
        getGeometry(i, j, y);
    }

    // --- determine max. section factor 
    setMaxSectionFactor(j);

    // --- normalize geometry table entries
    //     (full cross-section values are last table entries)
    nLast = Transect[j].nTbl - 1;
    Transect[j].aFull = Transect[j].areaTbl[nLast];
    Transect[j].rFull = Transect[j].hradTbl[nLast];
    Transect[j].wMax = Transect[j].widthTbl[nLast];

    for (i = 1; i <= nLast; i++)
    {
        Transect[j].areaTbl[i] /= Transect[j].aFull;
        Transect[j].hradTbl[i] /= Transect[j].rFull;
        Transect[j].widthTbl[i] /= Transect[j].wMax;
    }

    // --- set width at 0 height equal to width at 4% of max. height
    Transect[j].widthTbl[0] = Transect[j].widthTbl[1];

    // --- save unadjusted main channel roughness 
    Transect[j].roughness = oldNchannel;
}

//=============================================================================

int  setManning(double n[])
//
//  Input:   n[] = array of Manning's n values
//  Output:  returns an error code
//  Purpose: sets Manning's n for overbanks and main channel of a transect.
//
{
    int i;
    for (i=1; i<=3; i++)
    {
        if ( n[i] < 0.0 ) return ERR_NUMBER;
    }
    if ( n[1] > 0.0 ) Nleft = n[1];
    if ( n[2] > 0.0 ) Nright = n[2];
    if ( n[3] > 0.0 ) Nchannel = n[3];
    if ( Nleft == 0.0  ) Nleft = Nchannel;
    if ( Nright == 0.0 ) Nright = Nchannel;
    return 0;
}

//=============================================================================

int  setParams(int j, char* id, double x[])
//
//  Input:   j = transect index
//           id = transect ID name
//           x[] = array of parameter values
//  Output:  returns an error code
//  Purpose: assigns parameter values to current transect being processed.
//
{
    if ( j < 0 || j >= Ntransects ) return ERR_NUMBER;
    Transect[j].ID = id;                         // ID name
    Xleftbank = x[3] / UCF(LENGTH);              // left overbank location
    Xrightbank = x[4] / UCF(LENGTH);             // right overbank location
    Lfactor = x[7];                              // channel/bank length
    if ( Lfactor == 0.0 ) Lfactor = 1.0;
    Xfactor = x[8];                              // station location multiplier
    if ( Xfactor == 0.0 ) Xfactor = 1.0;
    Xleftbank *= Xfactor;                        // adjusted left bank
    Xrightbank *= Xfactor;                       // adjusted right bank
    Yfactor = x[9] / UCF(LENGTH);                // elevation offset
    Nstations = 0;
    return 0;
}

//=============================================================================

int  addStation(double y, double x)
//
//  Input:   y = station elevation value
//           x = station distance value
//  Output:  returns an error code
//  Purpose: adds a new station to the transect currently being processed.
//
{
    // --- check for valid number of stations
    if ( Nstations < 0 ) return ERR_TRANSECT_UNKNOWN;
    Nstations++;
    if ( Nstations >= MAXSTATION ) return 0;

    // --- add station distance, modified by distance multiplier
    Station[Nstations] = x * Xfactor / UCF(LENGTH);

    // --- add station elevation, modified by offset elevation
    Elev[Nstations] = (y + Yfactor) / UCF(LENGTH);

    // --- check if station distances are non-increasing
    if ( Nstations > 1
        && Station[Nstations] < Station[Nstations-1] )
        return ERR_TRANSECT_SEQUENCE;
    return 0;    
}

//=============================================================================

void  getGeometry(int i, int j, double y)
//
//  Input:   i = index of current entry in geometry tables
//           j = transect index
//           y = depth of current entry in geometry tables
//  Output:  none
//  Purpose: computes entries in a transect's geometry tables at a given depth. 
//
{
    int    k;                // station index
    double ylo,              // lower elev. of transect slice
           yhi,              // higher elev. of transect slice
           w,                // top width of transect slice
           wp,               // wetted perimeter of transect slice
           wpSum,            // total wetted perimeter across transect
           a,                // area of transect slice
           aSum,             // total area across transect
           q,                // flow across transect slices with same roughness
           qSum;             // total flow across transect
    int   findFlow;          // true if flow thru area slice needs updating

    // --- initialize
    wpSum = 0.0;
    aSum = 0.0;
    qSum = 0.0;

    // --- examine each horizontal station from left to right
    for (k = 1; k <= Nstations; k++)
    {
        // --- determine low & high elevations for transect sub-section
        if ( Elev[k-1] >= Elev[k] )
        {
            yhi = Elev[k-1];
            ylo = Elev[k];
        }
        else
        {
            yhi = Elev[k];
            ylo = Elev[k-1];
        }

        // --- skip station if its totally dry
        if ( ylo >= y ) continue;

        // --- get top width, area & wetted perimeter values for transect
        //     slice between station k and k-1
        getSliceGeom(k, y, ylo, yhi, &w, &a, &wp);

        // --- update total transect values
        wpSum += wp;
        aSum += a;
        Transect[j].areaTbl[i] += a;
        Transect[j].widthTbl[i] += w;

        // --- must update flow if station elevation is above water level
        if ( Elev[k] >= y ) findFlow = TRUE;
        else findFlow = FALSE;

        // --- update flow across transect if called for
        q = getFlow(k, aSum, wpSum, findFlow);
        if ( q > 0.0 )
        {
            qSum += q;
            aSum = 0.0;
            wpSum = 0.0;
        }

    }   // next station k 

    // --- find hyd. radius table entry solving Manning eq. with
    //     total flow, total area, and main channel n
    aSum = Transect[j].areaTbl[i];
    if ( aSum == 0.0 ) Transect[j].hradTbl[i] = Transect[j].hradTbl[i-1];
    else Transect[j].hradTbl[i] = pow(qSum * Nchannel / 1.49 / aSum, 1.5);
}

//=============================================================================

void getSliceGeom(int k, double y, double ylo, double yhi, double *w,
                  double *a, double *wp)
//
//  Input:   k = station index
//           y = water elevation
//           ylo = transect elevation on low side of slice
//           yhi = transect elevation on high side of slice
//  Output   w = width of transect slice
//           a = area of transect slice
//           wp = wetted perimeter of transect slice
//  Purpose: finds area, width & wetted perim. for slice of transect that
//           is covered by given water depth.
//
//      yhi  |           
//           |
//        y  |**********
//           |********** --> slice of transect being analyzed
//      ylo  |**********|
//           |**********|
//           |**********|
//         Station    Station
//           k-1        k
//
{
    double width, ratio;

    // --- compute width & wetted perimeter of transect slice
    width = fabs(Station[k] - Station[k-1]);
    (*w) = width;
    (*wp) = sqrt(width * width + (yhi - ylo) * (yhi - ylo));
    (*a)  = 0.0;

    // --- find area for completely submerged slice
    if ( y > yhi )
    {
        (*a) = width * ( (y - yhi) + (y - ylo) ) / 2.0;
    }

    // --- otherwise find area and adjust width & wetted perim. for
    //     partly submerged slice
    else if ( yhi > ylo )
    {
         ratio = (y - ylo) / (yhi - ylo);
         (*a) = width * (yhi - ylo) / 2.0 * ratio * ratio;
         (*w) *= ratio;
         (*wp) *= ratio;
     }
}

//=============================================================================

double getFlow(int k, double a, double wp, int findFlow)
//
//  Input:   k = index of station at end of transect sub-section
//           a = flow area of sub-section
//           wp = wetted perimeter of flow area of sub-section
//           findFlow = TRUE if flow needs updating 
//  Output:  returns normal flow (per unit of slope)
//  Purpose: finds flow through a sub-section of a transect.
//
{
    double n;                          // Manning's n

    if ( findFlow == FALSE)
    {
        // --- flow needs updating if we are at last station
        if ( k == Nstations - 1 ) findFlow = TRUE;

        // --- flow needs updating if we are at end of left overbank and
        //     there is a change in Manning's n and section not vertical
        else if ( Station[k] == Xleftbank )
        {
            if ( Nleft != Nchannel &&
                Station[k] != Station[k-1] ) findFlow = TRUE;
        }

        // --- flow needs updating if we are at start of right overbank and
        //     there is a change in Manning's n and section not vertical
        else if ( Station[k] == Xrightbank )
        {
            if ( Nright != Nchannel &&
                Station[k] != Station[k+1] ) findFlow = TRUE;
        }
    }

    // --- if flow needs updating
    if ( findFlow )
    {
        // --- find value of Manning's n to use
        n = Nchannel;
        if ( Station[k-1] < Xleftbank ) n = Nleft;
        if ( Station[k] > Xrightbank )  n = Nright;

        // --- compute flow through flow area
        return PHI / n * a * pow(a/wp, 2./3.);
    }
    return 0.0;
}

//=============================================================================

void setMaxSectionFactor(int j)
//
//  Input:   j = transect index
//  Output:  none
//  Purpose: determines the maximum section factor for a transect and the
//           area where this maxumum occurs.
//
{
    int    i;
    double sf;

    Transect[j].aMax = 0.0;
    Transect[j].sMax = 0.0;
    for (i=1; i<Transect[j].nTbl; i++)
    {
        sf = Transect[j].areaTbl[i] * pow(Transect[j].hradTbl[i], 2./3.);
        if ( sf > Transect[j].sMax )
        {
            Transect[j].sMax = sf;
            Transect[j].aMax = Transect[j].areaTbl[i];
        }
    }
}

//=============================================================================
