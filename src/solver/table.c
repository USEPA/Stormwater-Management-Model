//-----------------------------------------------------------------------------
//   table.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//             09/15/14   (Build 5.1.007)
//             03/19/15   (Build 5.1.008)
//   Author:   L. Rossman
//
//   Table (curve and time series) functions.
//
//   Curve and Time Series objects in SWMM 5 are both modeled with
//   TTable data structures.
//
//   The table_getFirstEntry and table_getNextEntry functions, as well as the
//   Time Series functions that use them, are not thread safe.
//
//   Build 5.1.008:
//   - The lookup functions used for Curve tables (table_lookup, table_lookupEx,
//     table_intervalLookup, table_inverseLookup, table_getSlope, table_getMaxY,
//     table_getArea, and table_getInverseArea) were made thread-safe (thanks to
//     suggestions by CHI).
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
int    table_getNextFileEntry(TTable* table, double* x, double* y);
int    table_parseFileLine(char* line, TTable* table, double* x, double* y);
double table_interpolate(double x, double x1, double y1, double x2, double y2);


//=============================================================================

double table_interpolate(double x, double x1, double y1, double x2, double y2)
//
//  Input:   x = x value being interpolated
//           x1, x2 = x values on either side of x
//           y1, y2 = y values corrresponding to x1 and x2, respectively
//  Output:  returns the y value corresponding to x
//  Purpose: interpolates a y value for a given x value.
//
{
    double dx = x2 - x1;
    if ( fabs(dx) < 1.0e-20 ) return (y1 + y2) / 2.;
    return y1 + (x - x1) * (y2 - y1) / dx;
}

//=============================================================================

int table_readCurve(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads a tokenized line of data for a curve table.
//
{
    int    j, m, k, k1 = 1;
    double x, y;

    // --- check for minimum number of tokens
    if ( ntoks < 3 ) return error_setInpError(ERR_ITEMS, "");

    // --- check that curve exists in database
    j = project_findObject(CURVE, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);

    // --- check if this is first line of curve's data
    //     (curve's ID will not have been assigned yet)
    if ( Curve[j].ID == NULL )
    {
        // --- assign ID pointer & curve type
        Curve[j].ID = project_findID(CURVE, tok[0]);
        m = findmatch(tok[1], CurveTypeWords);
        if ( m < 0 ) return error_setInpError(ERR_KEYWORD, tok[1]);
        Curve[j].curveType = m;
        k1 = 2;
    }

    // --- start reading pairs of X-Y value tokens
    for ( k = k1; k < ntoks; k = k+2)
    {
        if ( k+1 >= ntoks ) return error_setInpError(ERR_ITEMS, "");
        if ( ! getDouble(tok[k], &x) )
            return error_setInpError(ERR_NUMBER, tok[k]);
        if ( ! getDouble(tok[k+1], &y) )
            return error_setInpError(ERR_NUMBER, tok[k+1]);
        table_addEntry(&Curve[j], x, y);
    }
    return 0;
}

//=============================================================================

int table_readTimeseries(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads a tokenized line of data for a time series table.
//
{
    int    j;                          // time series index
    int    k;                          // token index
    int    state;                      // 1: next token should be a date
                                       // 2: next token should be a time
                                       // 3: next token should be a value 
    double x, y;                       // time & value table entries
    DateTime d;                        // day portion of date/time value
    DateTime t;                        // time portion of date/time value

    // --- check for minimum number of tokens
    if ( ntoks < 3 ) return error_setInpError(ERR_ITEMS, "");

    // --- check that time series exists in database
    j = project_findObject(TSERIES, tok[0]);
    if ( j < 0 ) return error_setInpError(ERR_NAME, tok[0]);

    // --- if first line of data, assign ID pointer
    if ( Tseries[j].ID == NULL )
        Tseries[j].ID = project_findID(TSERIES, tok[0]);

    // --- check if time series data is in an external file
    if ( strcomp(tok[1], w_FILE ) )
    {
        sstrncpy(Tseries[j].file.name, tok[2], MAXFNAME);
        Tseries[j].file.mode = USE_FILE;
        return 0;
    }

    // --- parse each token of input line
    x = 0.0;
    k = 1;
    state = 1;               // start off looking for a date
    while ( k < ntoks )
    {
        switch(state)
        {
          case 1:            // look for a date entry
            if ( datetime_strToDate(tok[k], &d) )
            {
                Tseries[j].lastDate = d;
                k++;
            }

            // --- next token must be a time
            state = 2;
            break;

          case 2:            // look for a time entry
            if ( k >= ntoks ) return error_setInpError(ERR_ITEMS, "");

            // --- first check for decimal hours format
            if ( getDouble(tok[k], &t) ) t /= 24.0;

            // --- then for an hrs:min format
            else if ( !datetime_strToTime(tok[k], &t) )
                return error_setInpError(ERR_NUMBER, tok[k]);

            // --- save date + time in x
            x = Tseries[j].lastDate + t;

            // --- next token must be a numeric value
            k++;
            state = 3;
            break;

          case 3:
            // --- extract a numeric value from token
            if ( k >= ntoks ) return error_setInpError(ERR_ITEMS, "");
            if ( ! getDouble(tok[k], &y) )
                return error_setInpError(ERR_NUMBER, tok[k]);

            // --- add date/time & value to time series
            table_addEntry(&Tseries[j], x, y);

            // --- start over looking first for a date
            k++;
            state = 1;
            break;
        }
    }
    return 0;
}

//=============================================================================

int table_addEntry(TTable* table, double x, double y)
//
//  Input:   table = pointer to a TTable structure
//           x = x value
//           y = y value
//  Output:  returns TRUE if successful, FALSE if not
//  Purpose: adds a new x/y entry to a table.
//
{
    TTableEntry *entry;
    entry = (TTableEntry *) malloc(sizeof(TTableEntry));
    if ( !entry ) return FALSE;
    entry->x = x;
    entry->y = y;
    entry->next = NULL;
    if ( table->firstEntry == NULL )  table->firstEntry = entry;
    else table->lastEntry->next = entry;
    table->lastEntry = entry;
    return TRUE;
}

//=============================================================================

void   table_deleteEntries(TTable *table)
//
//  Input:   table = pointer to a TTable structure
//  Output:  none
//  Purpose: deletes all x/y entries in a table.
//
{
    TTableEntry *entry;
    TTableEntry *nextEntry;
    entry = table->firstEntry;
    while (entry)
    {
        nextEntry = entry->next;
        free(entry);
        entry = nextEntry;
    }
    table->firstEntry = NULL;
    table->lastEntry  = NULL;
    table->thisEntry  = NULL;

    if (table->file.file)
    { 
        fclose(table->file.file);
        table->file.file = NULL;
    }
}

//=============================================================================

void   table_init(TTable *table)
//
//  Input:   table = pointer to a TTable structure
//  Output:  none
//  Purpose: initializes properties when table is first created.
//
{
    table->ID = NULL;
    table->refersTo = -1;
    table->firstEntry = NULL;
    table->lastEntry = NULL;
    table->thisEntry = table->firstEntry;
    table->lastDate = 0.0;
    table->x1 = 0.0;
    table->x2 = 0.0;
    table->y1 = 0.0;
    table->y2 = 0.0;
    table->dxMin = 0.0;
    table->file.mode = NO_FILE;
    table->file.file = NULL;
    table->curveType = -1;
}

//=============================================================================

int   table_validate(TTable *table)
//
//  Input:   table = pointer to a TTable structure
//  Output:  returns error code
//  Purpose: checks that table's x-values are in ascending order.
//
{
    int    result;
    double x1, x2, y1, y2;
    double dx, dxMin = BIG;

    // --- open external file if used as the table's data source
    if ( table->file.mode == USE_FILE )
    {
        table->file.file = fopen(table->file.name, "rt");
        if ( table->file.file == NULL ) return ERR_TABLE_FILE_OPEN;
    }

    // --- retrieve the first data entry in the table
    result = table_getFirstEntry(table, &x1, &y1);

    // --- return error condition if external file has no valid data
    if ( !result && table->file.mode == USE_FILE ) return ERR_TABLE_FILE_READ;

    // --- retrieve successive table entries and check for non-increasing x-values
    while ( table_getNextEntry(table, &x2, &y2) )
    {
        dx = x2 - x1;
        if ( dx <= 0.0 )
        {
            table->x2 = x2;
            return ERR_CURVE_SEQUENCE;
        }
        dxMin = MIN(dxMin, dx);
        x1 = x2;
    }
    table->dxMin = dxMin;

    // --- return error if external file could not be read completely
    if ( table->file.mode == USE_FILE && !feof(table->file.file) )
        return ERR_TABLE_FILE_READ;
    return 0;
}

//=============================================================================

int table_getFirstEntry(TTable *table, double *x, double *y)
//
//  Input:   table = pointer to a TTable structure
//  Output:  x = x-value of first table entry
//           y = y-value of first table entry
//           returns TRUE if successful, FALSE if not
//  Purpose: retrieves the first x/y entry in a table.
//
//  NOTE: also moves the current position pointer (thisEntry) to the 1st entry.
//
{
    TTableEntry *entry;
    *x = 0;
    *y = 0.0;

    if ( table->file.mode == USE_FILE )
    {
        if ( table->file.file == NULL ) return FALSE;
        rewind(table->file.file);
        return table_getNextFileEntry(table, x, y);
    }

    entry = table->firstEntry;
    if ( entry )
    {
        *x = entry->x;
        *y = entry->y;
        table->thisEntry = entry;
        return TRUE;
    }
    else return FALSE;
}

//=============================================================================

int table_getNextEntry(TTable *table, double *x, double *y)
//
//  Input:   table = pointer to a TTable structure
//  Output:  x = x-value of next table entry
//           y = y-value of next table entry
//           returns TRUE if successful, FALSE if not
//  Purpose: retrieves the next x/y entry in a table.
//
//  NOTE: also updates the current position pointer (thisEntry).
//
{
    TTableEntry *entry;

    if ( table->file.mode == USE_FILE )
        return table_getNextFileEntry(table, x, y);
    
    entry = table->thisEntry->next;
    if ( entry )
    {
        *x = entry->x;
        *y = entry->y;
        table->thisEntry = entry;
        return TRUE;
    }
    else return FALSE;
}

//=============================================================================

double table_lookup(TTable *table, double x)
//
//  Input:   table = pointer to a TTable structure
//           x = an x-value
//  Output:  returns a y-value
//  Purpose: retrieves the y-value corresponding to an x-value in a table,
//           using interploation if necessary.
//
//  NOTE: if x is below the first table entry, then the first y-value is
//        returned; if x is above the last entry, then the last y-value is
//        returned.
//
{
    double x1,y1,x2,y2;
    TTableEntry* entry;

    entry = table->firstEntry;
    if ( entry == NULL ) return 0.0;
    x1 = entry->x;
    y1 = entry->y;
    if ( x <= x1 ) return y1;
    while ( entry->next )
    {
        entry = entry->next;
        x2 = entry->x;
        y2 = entry->y;
        if ( x <= x2 ) return table_interpolate(x, x1, y1, x2, y2);
        x1 = x2;
        y1 = y2;
    }
    return y1;
}

//=============================================================================

double table_getSlope(TTable *table, double x)
//
//  Input:   table = pointer to a TTable structure
//           x = an x-value
//  Output:  returns the slope of the curve at x
//  Purpose: retrieves the slope of the curve at the line segment containing x.
//
{
    double x1,y1,x2,y2;
    double dx;
    TTableEntry* entry;

    entry = table->firstEntry;
    if ( entry == NULL ) return 0.0;
    x1 = entry->x;
    y1 = entry->y;
    x2 = x1;
    y2 = y1;
    while ( entry->next )
    {
        entry = entry->next;
        x2 = entry->x;
        y2 = entry->y;
        if ( x <= x2 ) break;
        x1 = x2;
        y1 = y2;
    }
    dx = x2 - x1;
    if ( dx == 0.0 ) return 0.0;
    return (y2 - y1) / dx;
}

//=============================================================================

double table_lookupEx(TTable *table, double x)
//
//  Input:   table = pointer to a TTable structure
//           x = an x-value
//  Output:  returns a y-value
//  Purpose: retrieves the y-value corresponding to an x-value in a table,
//           using interploation if necessary within the table and linear
//           extrapolation outside of the table.
//
{
    double x1,y1,x2,y2;
    double s = 0.0;
    TTableEntry* entry;

    entry = table->firstEntry;
    if (entry == NULL ) return 0.0;
    x1 = entry->x;
    y1 = entry->y;
    if ( x <= x1 )
    {
        if (x1 > 0.0 ) return x/x1*y1;
        else return y1;
    }
    while ( entry->next )
    {
        entry = entry->next;
        x2 = entry->x;
        y2 = entry->y;
        if ( x2 != x1 ) s = (y2 - y1) / (x2 - x1);
        if ( x <= x2 ) return table_interpolate(x, x1, y1, x2, y2);
        x1 = x2;
        y1 = y2;
    }
    if ( s < 0.0 ) s = 0.0;
    return y1 + s*(x - x1);
}

//=============================================================================

double table_intervalLookup(TTable *table, double x)
//
//  Input:   table = pointer to a TTable structure
//           x = an x-value
//  Output:  returns a y-value
//  Purpose: retrieves the y-value corresponding to the first table entry
//           whose x-value is > x.
//
{
    TTableEntry* entry;

    entry = table->firstEntry;
    if (entry == NULL ) return 0.0;
    if ( x < entry->x ) return entry->y;
    while ( entry->next )
    {
        entry = entry->next;
        if ( x < entry->x ) return entry->y;
    }
    return entry->y;
}

//=============================================================================

double table_inverseLookup(TTable *table, double y)
//
//  Input:   table = pointer to a TTable structure
//           y = a y-value
//  Output:  returns an x-value
//  Purpose: retrieves the x-value corresponding to an y-value in a table,
//           using interploation if necessary.
//
//  NOTE: if y is below the first table entry, then the first x-value is
//        returned; if y is above the last entry, then the last x-value is
//        returned.
//
{
    double x1,y1,x2,y2;
    TTableEntry* entry;

    entry = table->firstEntry;
    if (entry == NULL ) return 0.0;
    x1 = entry->x;
    y1 = entry->y;
    if ( y <= y1 ) return x1;
    while ( entry->next )
    {
        entry = entry->next;
        x2 = entry->x;
        y2 = entry->y;
        if ( y <= y2 ) return table_interpolate(y, y1, x1, y2, x2);
        x1 = x2;
        y1 = y2;
    }
    return x1;
}

//=============================================================================

double  table_getMaxY(TTable *table, double x)
//
//  Input:   table = pointer to a TTable structure
//           x = an x-value
//  Output:  returns the maximum y-value for x-values below x.
//  Purpose: finds the largest y value in the initial non-decreasing
//           portion of a table that appear before value x.
//
{
    double ymax;
    TTableEntry* entry;

    entry = table->firstEntry;
    ymax = entry->y;
    while ( x > entry->x && entry->next )
    {
        entry = entry->next;
        if ( entry->y < ymax ) return ymax;
        ymax = entry->y;
    }
    return 0.0;
}

//=============================================================================

double  table_getArea(TTable* table, double x)
//
//  Input:   table = pointer to a TTable structure
//           x = an value
//  Output:  returns area value
//  Purpose: finds area under a tabulated curve from 0 to x;
//           requires that table's x values be non-negative
//           and non-decreasing.
//
//  The area within each interval i of the table is given by:
//     Integral{ y(x)*dx } from x(i) to x
//  where y(x) = y(i) + s*dx
//        dx = x - x(i)
//        s = [y(i+1) - y(i)] / [x(i+1) - x(i)]
//  This results in the following expression for a(i):
//     a(i) = y(i)*dx + s*dx*dx/2
//
{
    double x1, x2;
    double y1, y2;
    double dx = 0.0, dy = 0.0;
    double a, s = 0.0;
    TTableEntry* entry;

    // --- get area up to first table entry
    //     and see if x-value lies in this interval
    entry = table->firstEntry;
    if (entry == NULL ) return 0.0;
    x1 = entry->x;
    y1 = entry->y;
    if ( x1 > 0.0 ) s = y1/x1;
    if ( x <= x1 ) return s*x*x/2.0;
    a = y1*x1/2.0;
    
    // --- add next table entry to area until target x-value is bracketed
    while ( entry->next )
    {
        entry = entry->next;
        x2 = entry->x;
        y2 = entry->y;
        dx = x2 - x1;
        dy = y2 - y1;
        if ( x <= x2 )
        {
            if ( dx <= 0.0 ) return a;
            y2 = table_interpolate(x, x1, y1, x2, y2);
            return a + (x - x1) * (y1 + y2) / 2.0;
        }
        a += (y1 + y2) * dx / 2.0;
        x1 = x2;
        y1 = y2;
    }

    // --- extrapolate area if table limit exceeded
    if ( dx > 0.0 ) s = dy/dx;
    else s = 0.0;
    dx = x - x1;
    return a + y1*dx + s*dx*dx/2.0;
}

//=============================================================================

double  table_getInverseArea(TTable* table, double a)
//
//  Input:   table = pointer to a TTable structure
//           a = an area value
//  Output:  returns an x value
//  Purpose: finds x value for given area under a curve.
//
//  Refer to table_getArea function to see how area is computed.
//
{
    double x1, x2;
    double y1, y2;
    double dx = 0.0, dy = 0.0;
    double a1, a2, s;
    TTableEntry* entry;

    // --- see if target area is below that of 1st table entry
    entry = table->firstEntry;
    if (entry == NULL ) return 0.0;
    x1 = entry->x;
    y1 = entry->y;
    a1 = y1*x1/2.0;
    if ( a <= a1 )
    {
        if ( y1 > 0.0 ) return sqrt(2.0*a*x1/y1);
        else return 0.0;
    }

    // --- add next table entry to area until target area is bracketed
    while ( entry->next )
    {
        entry = entry->next;
        x2 = entry->x;
        y2 = entry->y;
        dx = x2 - x1;
        dy = y2 - y1;
        a2 = a1 + y1*dx + dy*dx/2.0;
        if ( a <= a2 )
        {
            if ( dx <= 0.0 ) return x1;
            if ( dy == 0.0 )
            {
                if ( a2 == a1 ) return x1;
                else return x1 + dx * (a - a1) / (a2 - a1);
            }

            // --- if y decreases with x then replace point 1 with point 2
            if ( dy < 0.0 )
            {
                x1 = x2;
                y1 = y2;
                a1 = a2;
            }

            s = dy/dx;
            dx = (sqrt(y1*y1 + 2.0*s*(a-a1)) - y1) / s;
            return x1 + dx;
        }
        x1 = x2;
        y1 = y2;
        a1 = a2;
    }

    // --- extrapolate area if table limit exceeded
    if ( dx == 0.0 || dy == 0.0 )
    {
        if ( y1 > 0.0 ) dx = (a - a1) / y1;
        else dx = 0.0;
    }
    else
    {
        s = dy/dx;
        dx = (sqrt(y1*y1 + 2.0*s*(a-a1)) - y1) / s;
        if (dx < 0.0) dx = 0.0;
    }
    return x1 + dx;
}

//=============================================================================

void   table_tseriesInit(TTable *table)
//
//  Input:   table = pointer to a TTable structure
//  Output:  none
//  Purpose: initializes the time bracket within a time series table.
//
{
    table_getFirstEntry(table, &(table->x1), &(table->y1));
    table->x2 = table->x1;
    table->y2 = table->y1;
    table_getNextEntry(table, &(table->x2), &(table->y2));
}

//=============================================================================

double table_tseriesLookup(TTable *table, double x, char extend)
//
//  Input:   table = pointer to a TTable structure
//           x = a date/time value
//           extend = TRUE if time series extended on either end
//  Output:  returns a y-value
//  Purpose: retrieves the y-value corresponding to a time series date,
//           using interploation if necessary.
//
//  NOTE: if extend is FALSE and date x is outside the range of the table
//        then 0 is returned; if TRUE then the first or last value is
//        returned.
//
{
    // --- x lies within current time bracket
    if ( table->x1 <= x
    &&   table->x2 >= x
    &&   table->x1 != table->x2 )
    return table_interpolate(x, table->x1, table->y1, table->x2, table->y2);

    // --- x lies before current time bracket:
    //     move to start of time series
    if ( table->x1 == table->x2 || x < table->x1 )
    {
        table_getFirstEntry(table, &(table->x1), &(table->y1));
        if ( x < table->x1 )
        {
            if ( extend == TRUE ) return table->y1;
            else return 0;
        }
    }

    // --- x lies beyond current time bracket:
    //     update start of next time bracket
    table->x1 = table->x2;
    table->y1 = table->y2;

    // --- get end of next time bracket
    while ( table_getNextEntry(table, &(table->x2), &(table->y2)) )
    {
        // --- x lies within the bracket
        if ( x <= table->x2 )
            return table_interpolate(x, table->x1, table->y1,
                                        table->x2, table->y2);
        // --- otherwise move to next time bracket
        table->x1 = table->x2;
        table->y1 = table->y2;
    }

    // --- return last value or 0 if beyond last data value
    if ( extend == TRUE ) return table->y1;
    else return 0.0;
}

//=============================================================================

int  table_getNextFileEntry(TTable* table, double* x, double* y)
//
//  Input:   table = pointer to a TTable structure
//           x = pointer to a date (as decimal days)
//           y = pointer to a time series value
//  Output:  updates values of x and y;
//           returns TRUE if successful, FALSE if not
//  Purpose: retrieves the next date and value for a time series
//           table stored in an external file.
//
{
    char line[MAXLINE+1];
    int  code;
    if ( table->file.file == NULL ) return FALSE;
    while ( !feof(table->file.file) && fgets(line, MAXLINE, table->file.file) != NULL )
    {
        code = table_parseFileLine(line, table, x, y);
        if ( code < 0 ) continue;      //skip blank & comment lines
        return code;
    }
    return FALSE;
}

//=============================================================================

int  table_parseFileLine(char* line, TTable* table, double* x, double* y)
//
//  Input:   table = pointer to a TTable structure
//           x = pointer to a date (as decimal days)
//           y = pointer to a time series value
//  Output:  updates values of x and y;
//           returns -1 if line was a comment, 
//           TRUE if line successfully parsed,
//           FALSE if line could not be parsed
//  Purpose: parses a line of time series data from an external file.
//
{
    int   n;
    char  s1[50],
          s2[50],
          s3[50];
    char* tStr;              // time as string
    char* yStr;              // value as string
    double yy;               // value as double
    DateTime d;              // day portion of date/time value
    DateTime t;              // time portion of date/time value

    // --- get 3 string tokens from line and check if its a comment
    n = sscanf(line, "%s %s %s", s1, s2, s3);

    // --- return if line is blank or is a comment
    tStr = strtok(line, SEPSTR);
    if ( tStr == NULL || *tStr == ';' ) return -1;

    // --- line only has a time and a value
    if ( n == 2 )
    {
        // --- calendar date is same as last recorded date
        d = table->lastDate;
        tStr = s1;
        yStr = s2;
    }

    // --- line has date, time and a value
    else if ( n == 3 )
    {
        // --- convert date string to numeric value
        if ( !datetime_strToDate(s1, &d) ) return FALSE;

        // --- update last recorded calendar date
        table->lastDate = d;
        tStr = s2;
        yStr = s3;
    }
    else return FALSE;

    // --- convert time string to numeric value
    if ( getDouble(tStr, &t) ) t /= 24.0;
    else if ( !datetime_strToTime(tStr, &t) ) return FALSE;

    // --- convert value string to numeric value
    if ( !getDouble(yStr, &yy) ) return FALSE;

    // --- assign values to current date and value
    *x = d + t;
    *y = yy;
    return TRUE;
}
