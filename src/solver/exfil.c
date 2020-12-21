//-----------------------------------------------------------------------------
//   exfil.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     09/15/14  (Build 5.1.007)
//             03/19/15  (Build 5.1.008)
//             08/05/15  (Build 5.1.010)
//             08/01/16  (Build 5.1.011)
//   Author:   L. Rossman
//
//   Storage unit exfiltration functions.
//
//   Build 5.1.008:
//   - Monthly conductivity adjustment applied to exfiltration rate.
//
//   Build 5.1.010:
//   - New modified Green-Ampt infiltration option used.
//
//   Build 5.1.011:
//   - Fixed units conversion error for storage units with surface area curves.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include <stdlib.h>
#include "headers.h"
#include "infil.h"
#include "exfil.h"

static int  createStorageExfil(int k, double x[]);

//=============================================================================

int exfil_readStorageParams(int k, char* tok[], int ntoks, int n)
//
//  Input:   k = storage unit index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//           n = last token processed
//  Output:  returns an error code
//  Purpose: reads a storage unit's exfiltration parameters from a
//           tokenized line of input.
//
{
    int       i;
    double    x[3];    //suction head, Ksat, IMDmax

    // --- read Ksat if it's the only remaining token
    if ( ntoks == n+1 )
    {
        if ( ! getDouble(tok[n], &x[1]) )
            return error_setInpError(ERR_NUMBER, tok[n]);
        x[0] = 0.0;
        x[2] = 0.0;
    }

    // --- otherwise read Green-Ampt infiltration parameters from input tokens
    else if ( ntoks < n + 3 ) return error_setInpError(ERR_ITEMS, "");
    else for (i = 0; i < 3; i++)
    {
        if ( ! getDouble(tok[n+i], &x[i]) )
            return error_setInpError(ERR_NUMBER, tok[n+i]);
    }

    // --- no exfiltration if Ksat is 0
    if ( x[1] == 0.0 ) return 0;

    // --- create an exfiltration object
    return createStorageExfil(k, x);
}

//=============================================================================

void  exfil_initState(int k)
//
//  Input:   k = storage unit index
//  Output:  none
//  Purpose: initializes the state of a storage unit's exfiltration object.
//
{
    int i;
    double a, alast, d;
    TTable* aCurve;
    TExfil* exfil = Storage[k].exfil;

    // --- initialize exfiltration object
    if ( exfil != NULL )
    {
        // --- initialize the Green-Ampt infil. parameters
        grnampt_initState(exfil->btmExfil);
        grnampt_initState(exfil->bankExfil);

        // --- shape given by a Storage Curve
        i = Storage[k].aCurve;
        if ( i >= 0 )
        {
            // --- get bottom area
            aCurve = &Curve[i];
            Storage[k].exfil->btmArea = table_lookupEx(aCurve, 0.0);

            // --- find min/max bank depths and max. bank area
            table_getFirstEntry(aCurve, &d, &a);
            exfil->bankMinDepth = 0.0;
            exfil->bankMaxDepth = 0.0;
            exfil->bankMaxArea = 0.0;
            alast = a;
            while ( table_getNextEntry(aCurve, &d, &a) )
            {
                if ( a < alast ) break;
                else if ( a > alast )
                {
                    exfil->bankMaxArea = a;
                    exfil->bankMaxDepth = d;
                }
                else if ( exfil->bankMaxArea == 0.0 ) exfil->bankMinDepth = d;
                else break;
                alast = a;
            }

            // --- convert from user units to internal units
            exfil->btmArea /= UCF(LENGTH) * UCF(LENGTH);
            exfil->bankMaxArea /= UCF(LENGTH) * UCF(LENGTH);
            exfil->bankMinDepth /= UCF(LENGTH);
            exfil->bankMaxDepth /= UCF(LENGTH);
        }

        // --- functional storage shape curve
        else
        {
            exfil->btmArea = Storage[k].aConst;
            if ( Storage[k].aExpon == 0.0 ) exfil->btmArea +=Storage[k].aCoeff;
            exfil->bankMinDepth = 0.0;
            exfil->bankMaxDepth = BIG;
            exfil->bankMaxArea = BIG;
        }
    }
}

//=============================================================================

double exfil_getLoss(TExfil* exfil, double tStep, double depth, double area)
//
//  Input:   exfil = ptr. to a storage exfiltration object
//           tStep = time step (sec)
//           depth = water depth (ft)
//           area = surface area (ft2)
//  Output:  returns exfiltration rate out of storage unit (cfs)
//  Purpose: computes rate of water exfiltrated from a storage node into
//           the soil beneath it.
//
{
    double exfilRate = 0.0;

    // --- find infiltration through bottom of unit
    if ( exfil->btmExfil->IMDmax == 0.0 )
    {
        exfilRate = exfil->btmExfil->Ks * Adjust.hydconFactor;
    }
    else exfilRate = grnampt_getInfil(exfil->btmExfil, tStep, 0.0, depth,
                                      MOD_GREEN_AMPT);
    exfilRate *= exfil->btmArea;

    // --- find infiltration through sloped banks
    if ( depth > exfil->bankMinDepth )
    {
        // --- get area of banks
        area = MIN(area, exfil->bankMaxArea) - exfil->btmArea;
        if ( area > 0.0 )
        {
            // --- if infil. rate not a function of depth
            if ( exfil->btmExfil->IMDmax == 0.0 )
            {    
                exfilRate += area * exfil->btmExfil->Ks * Adjust.hydconFactor;
            }

            // --- infil. rate depends on depth above bank
            else
            {
                // --- case where water depth is above the point where the
                //     storage curve no longer has increasing area with depth
                if ( depth > exfil->bankMaxDepth )
                {
                    depth = depth - exfil->bankMaxDepth +
                               (exfil->bankMaxDepth - exfil->bankMinDepth) / 2.0;
                }

                // --- case where water depth is below top of bank
                else depth = (depth - exfil->bankMinDepth) / 2.0;

                // --- use Green-Ampt function for bank infiltration
                exfilRate += area * grnampt_getInfil(exfil->bankExfil,
                                    tStep, 0.0, depth, MOD_GREEN_AMPT);
            }
        }
    }
    return exfilRate;
}

//=============================================================================

int  createStorageExfil(int k, double x[])
//
//  Input:   k = index of storage unit node
//           x = array of Green-Ampt infiltration parameters
//  Output:  returns an error code.
//  Purpose: creates an exfiltration object for a storage node.
//
//  Note: the exfiltration object is freed in project.c.
//
{
    TExfil*   exfil;

    // --- create an exfiltration object for the storage node
    exfil = Storage[k].exfil;
    if ( exfil == NULL )
    {
        exfil = (TExfil *) malloc(sizeof(TExfil));
        if ( exfil == NULL ) return error_setInpError(ERR_MEMORY, "");
        Storage[k].exfil = exfil;

        // --- create Green-Ampt infiltration objects for the bottom & banks
        exfil->btmExfil = NULL;
        exfil->bankExfil = NULL;
        exfil->btmExfil = (TGrnAmpt *) malloc(sizeof(TGrnAmpt));
        if ( exfil->btmExfil == NULL ) return error_setInpError(ERR_MEMORY, "");
        exfil->bankExfil = (TGrnAmpt *) malloc(sizeof(TGrnAmpt));
        if ( exfil->bankExfil == NULL ) return error_setInpError(ERR_MEMORY, "");
    }

    // --- initialize the Green-Ampt parameters
    if ( !grnampt_setParams(exfil->btmExfil, x) )
        return error_setInpError(ERR_NUMBER, "");
    grnampt_setParams(exfil->bankExfil, x);
    return 0;
}
