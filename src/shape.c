//-----------------------------------------------------------------------------
//   shape.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//   Author:   L. Rossman
//
//   Geometry functions for custom cross-section shapes.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <math.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
static double Atotal;
static double Ptotal;

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  shape_validate                (called from project_validate in project.c)

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static int    computeShapeTables(TShape *shape, TTable *curve);
static void   getSmax(TShape *shape);
static int    normalizeShapeTables(TShape *shape);
static int    getNextInterval(TTable *curve, double y, double yLast,
              double wLast, double *y1, double *y2, double *w1, double *w2,
              double *wMax);
static double getWidth(double y, double y1, double y2, double w1, double w2);
static double getArea(double y, double w, double y1, double w1);
static double getPerim(double y, double w, double y1, double w1);

//=============================================================================

int  shape_validate(TShape *shape, TTable *curve)
//
//  Input:   shape = pointer to a custom x-section TShape object
//           curve = pointer to shape's table of width v. height
//  Output:  returns TRUE if successful. FALSE if not
//  Purpose: computes the entries in a custom x-section shape's geometry
//           tables from its user-supplied width v. height curve.
//
{
    if ( !computeShapeTables(shape, curve) ) return FALSE;
    if ( !normalizeShapeTables(shape) ) return FALSE;
    return TRUE;
}

//=============================================================================

int  computeShapeTables(TShape *shape, TTable *curve)
//
//  Input:   shape = pointer to a TShape object
//           curve = pointer to shape's table of width v. depth
//  Output:  returns TRUE if successful. FALSE if not
//  Purpose: computes the entries in a shape's geometry tables from
//           the shape's width v. height curve normalized with repsect
//           to full height.
//
//  Note:    the shape curve is a user-supplied table of width v. height
//           for a custom x-section of unit height.
{
    int    i, n;
    double dy, y, y1, y2, w, w1, w2;
	double yLast, wLast, wMax;

    // --- get first entry of user's shape curve
    if ( !table_getFirstEntry(curve, &y1, &w1) ) return FALSE;
    if ( y1 < 0.0 || y1 >= 1.0 || w1 < 0.0 ) return FALSE;
    wMax = w1;
    
    // --- if first entry not at zero ht. then add an initial entry
    if ( y1 != 0.0 )
    {
        y2 = y1;
        w2 = w1;
        y1 = 0.0;
        w1 = 0.0;
    }

    // --- otherwise get next entry in the user's shape curve
    else
    {
        if ( !table_getNextEntry(curve, &y2, &w2) ) return FALSE;
        if ( y2 < y1 || w2 < 0.0 ) return FALSE;
        if ( y2 > 1.0 ) y2 = 1.0;
        if ( w2 > wMax ) wMax = w2;
    }

    // --- determine number of entries & interval size in geom. tables
    shape->nTbl = N_SHAPE_TBL;
    n = shape->nTbl - 1;
    dy = 1.0 / (double)(n);

    // --- initialize geometry tables
    shape->areaTbl[0] = 0.0;
    shape->hradTbl[0] = 0.0;
    shape->widthTbl[0] = w1;
    Ptotal = w1;
    Atotal = 0.0;

    // --- fill in rest of geometry tables
    y = 0.0;
    w = w1;
    for ( i = 1; i <= n; i++ )
    {
        // --- advance to next relative height level
        yLast = y;
        wLast = w;
        y = y + dy;

        // --- do not allow height to exceed 1.0
        if ( fabs(y - 1.0) < TINY ) y = 1.0;

        // --- if height exceeds current shape curve interval,
        //     move to next interval of shape curve
        if ( y > y2 )
        {
            if ( !getNextInterval(curve, y, yLast, wLast, &y1, &y2, &w1,
                                  &w2, &wMax) )
                return FALSE;
            yLast = y1;
            wLast = w1;
        }

        // --- get top width, area, & perimeter of current interval
        w = getWidth(y, y1, y2, w1, w2);
        Atotal += getArea(y, w, yLast, wLast); 
        Ptotal += getPerim(y, w, yLast, wLast);

        // --- add top width to total perimeter if at top of shape
        if ( y == 1.0 ) Ptotal += w2;

        // --- update table values
        shape->widthTbl[i] = w;
        shape->areaTbl[i] = Atotal;
        if ( Ptotal > 0.0) shape->hradTbl[i] = Atotal / Ptotal;
        else               shape->hradTbl[i] = 0.0;
    }

    // --- assign values to shape'a area and hyd. radius when full
    shape->aFull = shape->areaTbl[n];
    shape->rFull = shape->hradTbl[n];

    // --- assign values to shape's max. width and section factor
    shape->wMax  = wMax;
    getSmax(shape);
    return TRUE;
}

//=============================================================================

void  getSmax(TShape *shape)
//
//  Input:   shape = pointer to a TShape object
//  Output:  none
//  Purpose: computes the max. section factor and corresponding area
//           for a shape of unit height.
//
{
    int    i;
    int    n = shape->nTbl - 1;
    double sf;

    shape->sMax = 0.0;
    shape->aMax = 0.0;
    for ( i = 1; i <= n; i++ )
    {
        sf = shape->areaTbl[i] * pow(shape->hradTbl[i], 2./3.);
        if ( sf > shape->sMax )
        {
            shape->sMax = sf;
            shape->aMax = shape->areaTbl[i];
        }
    }
}

//=============================================================================

int  normalizeShapeTables(TShape *shape)
//
//  Input:   shape = pointer to a TShape object
//  Output:  returns TRUE if successful. FALSE if not
//  Purpose: normalizes a shape's area tables to its full (or max.) condition.
//
{
    int    i;
    int    n = shape->nTbl - 1;        // highest table entry index
    double aFull = shape->aFull;       // area when full
    double rFull = shape->rFull;       // hyd. radius when full
    double wMax = shape->wMax;         // max. width

    // --- check that normalizing factors are non-zero
    if ( aFull == 0.0 || rFull == 0.0 || wMax == 0.0 ) return FALSE;

    // --- normalize entries in each table by their respective factors
    for ( i = 0; i <= n; i++ )
    {
        shape->areaTbl[i] /= aFull;
        shape->hradTbl[i] /= rFull;
        shape->widthTbl[i] /= wMax;
    }
    return TRUE;
}

//=============================================================================

int getNextInterval(TTable *curve, double y, double yLast, double wLast, 
                    double *y1, double *y2, double *w1, double *w2, 
                    double *wMax)
//
//  Input:   curve = pointer to a user-supplied shape curve table
//           y = current height in a geometry table
//           yLast = previous height in a geometry table
//           wLast = previous width in a geometry table
//           y1 = height at start of current curve interval
//           y2 = height at end of current curve interval
//           w1 = width at start of current curve interval
//           w2 = width at end of current curve interval
//           wMax = current maximum width of curve
//  Output:  updated values for yLast, wLast, y1, y2, w1, w2, and wMax;
//           returns TRUE if successful, FALSE if not.
//  Purpose: advances to the next height interval of a shape's curve that
//           contains the current height being evaluated in the shape's
//           geometry table.
//
//  Note:    heights and widths are with repsect to a shape of unit height.
{
    // --- keep advancing while the current geom. table height is
    //     above the end of the curve table interval
    while ( y > *y2 )
    {
        // --- move start of geom. table interval up to the end of
        //     the current curve table interval
        if ( *y2 > yLast )
        {
            Atotal += getArea(*y2, *w2, yLast, wLast); 
            Ptotal += getPerim(*y2, *w2, yLast, wLast);
            yLast = *y2;
            wLast = *w2;
        }

        // --- move to the next curve table interval
        *y1 = *y2;
        *w1 = *w2;
        if ( !table_getNextEntry(curve, y2, w2) )
        {
            *y2 = 1.0;
            return TRUE;
        }

        // --- update curve table's max. width
        if ( *w2 > *wMax ) *wMax = *w2;

        // --- check for valid curve table values
        if ( *y2 < *y1 || *w2 < 0.0 ) return FALSE;
        if ( *y2 > 1.0 ) *y2 = 1.0;
    }
    return TRUE;
}

//=============================================================================

double  getWidth(double y, double y1, double y2, double w1, double w2)
//
//  Input:   y = height along a shape curve
//           y1 = height at start of a shape curve interval
//           y2 = height at end of a shape curve interval
//           w1 = width at start of a shape curve interval
//           w2 = width at end of a shape curve interval
//  Output:  returns the width corresponding to height y
//  Purpose: interpolates a width within a given height interval along a
//           x-section's shape curve.
//
{
    if ( y2 == y1 ) return w2;
    return w1 + (y - y1) / (y2 - y1) * (w2 - w1);
}

//=============================================================================

double getArea(double y, double w, double y1, double w1)
//
//  Input:   y = height along a shape curve
//           w = width that corresponds to y
//           y1 = height at start of a shape curve interval
//           w1 = width at start of a shape curve interval
//  Output:  returns the area within the trapezoid formed by the input points
//  Purpose: computes the area of an interval along a x-section's shape curve.
//
{
    double wMin, wMax;
    if ( w > w1 )
    {
        wMin = w1;
        wMax = w;
    }
    else
    {
        wMin = w;
        wMax = w1;
    }
    return (wMin + (wMax - wMin)/2.) * (y - y1);
}

//=============================================================================

double getPerim(double y, double w, double y1, double w1)
//
//  Input:   y = height along a shape curve
//           w = width that corresponds to y
//           y1 = height at start of a shape curve interval
//           w1 = width at start of a shape curve interval
//  Output:  returns the length of the sides of the trapezoid formed by the
//           input points
//  Purpose: computes the length of the wetted perimeter contributed by an
//           interval along a x-section's shape curve.
//
{
    double dy = y - y1;    
    double dw = fabs(w - w1) / 2.0;
    return 2.0 * sqrt(dy*dy + dw*dw);
}
