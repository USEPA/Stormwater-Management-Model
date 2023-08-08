//-----------------------------------------------------------------------------
//   street.c
//
//   Project:  EPA SWMM5
//   Version:  5.2
//   Date:     07/13/23  (Build 5.2.4)
//   Author:   L. Rossman
//
//   Street Cross-Section Functions
//
//   Build 5.2.4:
//   - Fixed incorrect index used to retrieve street backing parameters
//     in street_readParams.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  External functions (declared in street.h)
//-----------------------------------------------------------------------------
// street_create             called by createObjects in project.c
// street_delete             called by deleteObjects in project.c
// street_readParams         called by parseLine in input.c

//=============================================================================

int   street_create(int nStreets)
//
//  Input:   nStreets = number of Street objects to create
//  Output:  none
//  Purpose: creats a collection of Street objects.
//
{
    Street = NULL;
    Nobjects[STREET] = 0;
    Street = (TStreet *)calloc(nStreets, sizeof(TStreet));
    if (Street == NULL) return ERR_MEMORY;
    Nobjects[STREET] = nStreets;
    return 0;
}

//=============================================================================

void  street_delete()
//
//  Input:   none
//  Output:  none
//  Purpose: deletes the collection of Street objects.
//
{
    FREE(Street)
}

//=============================================================================

int street_readParams(char* tok[], int ntoks)
//
//  Format is:
//    ID  Tcrown  Hcurb  Sx  nRoad  (Hdep  Wg  Sides  Tback  Sback  nBack)
//  where
//    ID = name assigned to street cross section
//    Tcrown = distance from curb to street crown (ft or m)
//    Hcurb  = curb height (ft or m)
//    Sx     = roadway cross slope (%)
//    nRoad  = roadway Manning's n
//    Hdep   = depressed gutter height (ft or m)
//    Wg     = depressed gutter width (ft or m)
//    Sides  = 1 or 2 sided
//    Tback  = width of street backing (ft or m)
//    Sback  = slope of street backing (ft or m)
//    nBack  = Manning's n of street backing
{
    int    i, k, sides;
    double x[11];
    TStreet *street;

    // --- check for minimum number of tokens
    if (ntoks < 5) return error_setInpError(ERR_ITEMS, "");

    // --- check that street exists in project
    i = project_findObject(STREET, tok[0]);
    if (i < 0) return error_setInpError(ERR_NAME, tok[0]);
    Street[i].ID = project_findID(STREET, tok[0]);

    // --- parse required data
    for (k = 0; k <= 9; k++) x[k] = 0.0;
    for (k = 1; k <= 4; k++)
        if (!getDouble(tok[k], &x[k]) || x[k] <= 0.0)
            return error_setInpError(ERR_NUMBER, tok[k]);

    // --- read gutter depression
    if (ntoks > 5)
        if (!getDouble(tok[5], &x[5]) || x[5] < 0.0)
            return error_setInpError(ERR_NUMBER, tok[5]);

    // --- read gutter width
    if (ntoks > 6)
        if (!getDouble(tok[6], &x[6]) || x[6] < 0.0)
            return error_setInpError(ERR_NUMBER, tok[6]);

    // --- read if 1- or 2-sided
    sides = 2;
    if (ntoks > 7)
        if (!getInt(tok[7], &sides) || sides < 1 || sides > 2)
            return error_setInpError(ERR_NUMBER, tok[7]);

    // --- read street backing parameters
    if (ntoks > 8)
    {
        if (!getDouble(tok[8], &x[8]) || x[8] < 0.0)
            return error_setInpError(ERR_NUMBER, tok[8]);
        if (x[8] > 0.0)
        {
            if (ntoks < 11) return error_setInpError(ERR_ITEMS, "");
            for (k = 9; k <= 10; k++)
                if (!getDouble(tok[k], &x[k]) || x[k] <= 0.0)
                    return error_setInpError(ERR_NUMBER, tok[k]);
        }
    }

    // --- assign input values to street object
    street = &Street[i];
    street->width = x[1] / UCF(LENGTH);
    street->curbHeight = x[2] / UCF(LENGTH);
    street->slope = x[3] / 100.0;
    street->roughness = x[4];
    street->gutterDepression = x[5] / UCF(LENGTH);
    street->gutterWidth = x[6] / UCF(LENGTH);
    street->sides = sides;
    street->backWidth = x[8] / UCF(LENGTH);
    street->backSlope = x[9] / 100.0;
    street->backRoughness = x[10];
    transect_createStreetTransect(street);
    return 0;
}

//=============================================================================

double street_getExtentFilled(int link)
//
//  Input:   link = a link index
//  Output:  degree to which street is filled
//  Purpose: finds the degree to which a street link is filled based on the
//           depth (for DW routing) or cross section area at the higher end.
//
{
    int    k, t;
    double filled;

    t = Link[link].xsect.transect;
    if (t < 0) return 0.0;
    if (RouteModel == DW)
    {
        filled = MAX(Node[Link[link].node1].newDepth,
                     Node[Link[link].node2].newDepth);
    }
    else
    {
        k = Link[link].subIndex;
        filled = MAX(Conduit[k].a1, Conduit[k].a2);
    }
    return filled;
}
