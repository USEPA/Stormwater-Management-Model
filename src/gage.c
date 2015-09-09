//-----------------------------------------------------------------------------
//   gage.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/10  (Build 5.1.001)
//             09/15/14  (Build 5.1.007)
//   Author:   L. Rossman
//
//   Rain gage functions.
//
//   Build 5.1.007:
//   - Support for monthly rainfall adjustments added.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <string.h>
#include <math.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
const double OneSecond = 1.1574074e-5;

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  gage_readParams        (called by input_readLine)
//  gage_validate          (called by project_validate)
//  gage_initState         (called by project_init)
//  gage_setState          (called by runoff_execute & getRainfall in rdii.c)
//  gage_getPrecip         (called by subcatch_getRunoff)
//  gage_getNextRainDate   (called by runoff_getTimeStep)

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static int    readGageSeriesFormat(char* tok[], int ntoks, double x[]);
static int    readGageFileFormat(char* tok[], int ntoks, double x[]);
static int    getFirstRainfall(int gage);
static int    getNextRainfall(int gage);
static double convertRainfall(int gage, double rain);


//=============================================================================

int gage_readParams(int j, char* tok[], int ntoks)
//
//  Input:   j = rain gage index
//           tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns an error code
//  Purpose: reads rain gage parameters from a line of input data
//
//  Data formats are:
//    Name RainType RecdFreq SCF TIMESERIES SeriesName
//    Name RainType RecdFreq SCF FILE FileName Station Units StartDate
//
{
    int      k, err;
    char     *id;
    char     fname[MAXFNAME+1];
    char     staID[MAXMSG+1];
    double   x[7];

    // --- check that gage exists
    if ( ntoks < 2 ) return error_setInpError(ERR_ITEMS, "");
    id = project_findID(GAGE, tok[0]);
    if ( id == NULL ) return error_setInpError(ERR_NAME, tok[0]);

    // --- assign default parameter values
    x[0] = -1.0;         // No time series index
    x[1] = 1.0;          // Rain type is volume
    x[2] = 3600.0;       // Recording freq. is 3600 sec
    x[3] = 1.0;          // Snow catch deficiency factor
    x[4] = NO_DATE;      // Default is no start/end date
    x[5] = NO_DATE;
    x[6] = 0.0;          // US units
    strcpy(fname, "");
    strcpy(staID, "");

    if ( ntoks < 5 ) return error_setInpError(ERR_ITEMS, "");
    k = findmatch(tok[4], GageDataWords);
    if      ( k == RAIN_TSERIES )
    {
        err = readGageSeriesFormat(tok, ntoks, x);
    }
    else if ( k == RAIN_FILE    )
    {
        if ( ntoks < 8 ) return error_setInpError(ERR_ITEMS, "");
        sstrncpy(fname, tok[5], MAXFNAME);
        sstrncpy(staID, tok[6], MAXMSG);
        err = readGageFileFormat(tok, ntoks, x);
    }
    else return error_setInpError(ERR_KEYWORD, tok[4]);

    // --- save parameters to rain gage object
    if ( err > 0 ) return err;
    Gage[j].ID = id;
    Gage[j].tSeries      = (int)x[0];
    Gage[j].rainType     = (int)x[1];
    Gage[j].rainInterval = (int)x[2];
    Gage[j].snowFactor   = x[3];
    Gage[j].rainUnits    = (int)x[6];
    if ( Gage[j].tSeries >= 0 ) Gage[j].dataSource = RAIN_TSERIES;
    else                        Gage[j].dataSource = RAIN_FILE;
    if ( Gage[j].dataSource == RAIN_FILE )
    {
        sstrncpy(Gage[j].fname, fname, MAXFNAME);
        sstrncpy(Gage[j].staID, staID, MAXMSG);
        Gage[j].startFileDate = x[4];
        Gage[j].endFileDate = x[5];
    }
    Gage[j].unitsFactor = 1.0;
    Gage[j].coGage = -1;
    Gage[j].isUsed = FALSE;
    return 0;
}

//=============================================================================

int readGageSeriesFormat(char* tok[], int ntoks, double x[])
{
    int m, ts;
    DateTime aTime;

    if ( ntoks < 6 ) return error_setInpError(ERR_ITEMS, "");

    // --- determine type of rain data
    m = findmatch(tok[1], RainTypeWords);
    if ( m < 0 ) return error_setInpError(ERR_KEYWORD, tok[1]);
    x[1] = (double)m;

    // --- get data time interval & convert to seconds
    if ( getDouble(tok[2], &x[2]) ) x[2] = floor(x[2]*3600 + 0.5);
    else if ( datetime_strToTime(tok[2], &aTime) )
    {
        x[2] = floor(aTime*SECperDAY + 0.5);
    }
    else return error_setInpError(ERR_DATETIME, tok[2]);
    if ( x[2] <= 0.0 ) return error_setInpError(ERR_DATETIME, tok[2]);

    // --- get snow catch deficiency factor
    if ( !getDouble(tok[3], &x[3]) )
        return error_setInpError(ERR_DATETIME, tok[3]);;

    // --- get time series index
    ts = project_findObject(TSERIES, tok[5]);
    if ( ts < 0 ) return error_setInpError(ERR_NAME, tok[5]);
    x[0] = (double)ts;
    strcpy(tok[2], "");
    return 0;
}

//=============================================================================

int readGageFileFormat(char* tok[], int ntoks, double x[])
{
    int   m, u;
    DateTime aDate;
    DateTime aTime;

    // --- determine type of rain data
    m = findmatch(tok[1], RainTypeWords);
    if ( m < 0 ) return error_setInpError(ERR_KEYWORD, tok[1]);
    x[1] = (double)m;

    // --- get data time interval & convert to seconds
    if ( getDouble(tok[2], &x[2]) ) x[2] *= 3600;
    else if ( datetime_strToTime(tok[2], &aTime) )
    {
        x[2] = floor(aTime*SECperDAY + 0.5);
    }
    else return error_setInpError(ERR_DATETIME, tok[2]);
    if ( x[2] <= 0.0 ) return error_setInpError(ERR_DATETIME, tok[2]);

    // --- get snow catch deficiency factor
    if ( !getDouble(tok[3], &x[3]) )
        return error_setInpError(ERR_NUMBER, tok[3]);
 
    // --- get rain depth units
    u = findmatch(tok[7], RainUnitsWords);
    if ( u < 0 ) return error_setInpError(ERR_KEYWORD, tok[7]);
    x[6] = (double)u;

    // --- get start date (if present)
    if ( ntoks > 8 && *tok[8] != '*')
    {
        if ( !datetime_strToDate(tok[8], &aDate) )
            return error_setInpError(ERR_DATETIME, tok[8]);
        x[4] = (float) aDate;
    }
    return 0;
}

//=============================================================================

void  gage_validate(int j)
//
//  Input:   j = rain gage index
//  Output:  none
//  Purpose: checks for valid rain gage parameters
//
//  NOTE: assumes that any time series used by a rain gage has been
//        previously validated.
//
{
    int i, k;
    int gageInterval;

    // --- for gage with time series data:
    if ( Gage[j].dataSource == RAIN_TSERIES )
    {
        // --- check gage's recording interval against that of time series
        k = Gage[j].tSeries;
        if ( Tseries[k].refersTo >= 0 )
        {
            report_writeErrorMsg(ERR_RAIN_GAGE_TSERIES, Gage[j].ID);
        }
        gageInterval = (int)(floor(Tseries[k].dxMin*SECperDAY + 0.5));
        if ( gageInterval > 0 && Gage[j].rainInterval > gageInterval )
        {
            report_writeErrorMsg(ERR_RAIN_GAGE_INTERVAL, Gage[j].ID);
        } 
        if ( Gage[j].rainInterval < gageInterval )
        {
            report_writeWarningMsg(WARN09, Gage[j].ID);
        }
        if ( Gage[j].rainInterval < WetStep )
        {
            report_writeWarningMsg(WARN01, Gage[j].ID);
            WetStep = Gage[j].rainInterval;
        }

        // --- see if gage uses same time series as another gage
        for (i=0; i<j; i++)
        {
            if ( Gage[i].dataSource == RAIN_TSERIES && Gage[i].tSeries == k )
            {
                Gage[j].coGage = i;

                // --- check that both gages record same type of data
                if ( Gage[j].rainType != Gage[i].rainType )
                {
                    report_writeErrorMsg(ERR_RAIN_GAGE_FORMAT, Gage[j].ID);
                }
                return;
            }
        }
    }
}

//=============================================================================

void  gage_initState(int j)
//
//  Input:   j = rain gage index
//  Output:  none
//  Purpose: initializes state of rain gage.
//
{
    // --- assume gage not used by any subcatchment
    //     (will be updated in subcatch_initState)
    Gage[j].isUsed = FALSE;
    Gage[j].rainfall = 0.0;
    Gage[j].reportRainfall = 0.0;
    if ( IgnoreRainfall ) return;

    // --- for gage with file data:
    if ( Gage[j].dataSource == RAIN_FILE )
    {
        // --- set current file position to start of period of record
        Gage[j].currentFilePos = Gage[j].startFilePos;

        // --- assign units conversion factor
        //     (rain depths on interface file are in inches)
        if ( UnitSystem == SI ) Gage[j].unitsFactor = MMperINCH;
    }

    // --- get first & next rainfall values
    if ( getFirstRainfall(j) )
    {
        // --- find date at end of starting rain interval
        Gage[j].endDate = datetime_addSeconds(
                          Gage[j].startDate, Gage[j].rainInterval);

        // --- if rainfall record begins after start of simulation,
        if ( Gage[j].startDate > StartDateTime )
        {
            // --- make next rainfall date the start of the rain record
            Gage[j].nextDate = Gage[j].startDate;
            Gage[j].nextRainfall = Gage[j].rainfall;

            // --- make start of current rain interval the simulation start
            Gage[j].startDate = StartDateTime;
            Gage[j].endDate = Gage[j].nextDate;
            Gage[j].rainfall = 0.0;
        }

        // --- otherwise find next recorded rainfall
        else if ( !getNextRainfall(j) ) Gage[j].nextDate = NO_DATE;
    }
    else Gage[j].startDate = NO_DATE;
}

//=============================================================================

void gage_setState(int j, DateTime t)
//
//  Input:   j = rain gage index
//           t = a calendar date/time
//  Output:  none
//  Purpose: updates state of rain gage for specified date. 
//
{
    // --- return if gage not used by any subcatchment
    if ( Gage[j].isUsed == FALSE ) return;

    // --- set rainfall to zero if disabled
    if ( IgnoreRainfall )
    {
        Gage[j].rainfall = 0.0;
        return;
    }

    // --- use rainfall from co-gage (gage with lower index that uses
    //     same rainfall time series or file) if it exists
    if ( Gage[j].coGage >= 0)
    {
        Gage[j].rainfall = Gage[Gage[j].coGage].rainfall;
        return;
    }

    // --- otherwise march through rainfall record until date t is bracketed
    t += OneSecond;
    for (;;)
    {
        // --- no rainfall if no interval start date
        if ( Gage[j].startDate == NO_DATE )
        {
            Gage[j].rainfall = 0.0;
            return;
        }

        // --- no rainfall if time is before interval start date
        if ( t < Gage[j].startDate )
        {
            Gage[j].rainfall = 0.0;
            return;
        }

        // --- use current rainfall if time is before interval end date
        if ( t < Gage[j].endDate )
        {
            return;
        }

        // --- no rainfall if t >= interval end date & no next interval exists
        if ( Gage[j].nextDate == NO_DATE )
        {
            Gage[j].rainfall = 0.0;
            return;
        }

        // --- no rainfall if t > interval end date & <  next interval date
        if ( t < Gage[j].nextDate )
        {
            Gage[j].rainfall = 0.0;
            return;
        }

        // --- otherwise update next rainfall interval date
        Gage[j].startDate = Gage[j].nextDate;
        Gage[j].endDate = datetime_addSeconds(Gage[j].startDate,
                          Gage[j].rainInterval);
        Gage[j].rainfall = Gage[j].nextRainfall;
        if ( !getNextRainfall(j) ) Gage[j].nextDate = NO_DATE;
    }
}

//=============================================================================

DateTime gage_getNextRainDate(int j, DateTime aDate)
//
//  Input:   j = rain gage index
//           aDate = calendar date/time
//  Output:  next date with rainfall occurring
//  Purpose: finds the next date from  specified date when rainfall occurs.
//
{
    if ( Gage[j].isUsed == FALSE ) return aDate;
    aDate += OneSecond;
    if ( aDate < Gage[j].startDate ) return Gage[j].startDate;
    if ( aDate < Gage[j].endDate   ) return Gage[j].endDate;
    return Gage[j].nextDate;
}

//=============================================================================

double gage_getPrecip(int j, double *rainfall, double *snowfall)
//
//  Input:   j = rain gage index
//  Output:  rainfall = rainfall rate (ft/sec)
//           snowfall = snow fall rate (ft/sec)
//           returns total precipitation (ft/sec)
//  Purpose: determines whether gage's recorded rainfall is rain or snow.
//
{
    *rainfall = 0.0;
    *snowfall = 0.0;
    if ( !IgnoreSnowmelt && Temp.ta <= Snow.snotmp )
    {
       *snowfall = Gage[j].rainfall * Gage[j].snowFactor / UCF(RAINFALL);
    }
    else *rainfall = Gage[j].rainfall / UCF(RAINFALL);
    return (*rainfall) + (*snowfall);
} 

//=============================================================================

void gage_setReportRainfall(int j, DateTime reportDate)
//
//  Input:   j = rain gage index
//           reportDate = date/time value of current reporting time
//  Output:  none
//  Purpose: sets the rainfall value reported at the current reporting time.
//
{
    double result;

    // --- use value from co-gage if it exists
    if ( Gage[j].coGage >= 0)
    {
        Gage[j].reportRainfall = Gage[Gage[j].coGage].reportRainfall;
        return;
    }

    // --- otherwise increase reporting time by 1 second to avoid
    //     roundoff problems
    reportDate += OneSecond;

    // --- use current rainfall if report date/time is before end
    //     of current rain interval
    if ( reportDate < Gage[j].endDate ) result = Gage[j].rainfall;

    // --- use 0.0 if report date/time is before start of next rain interval
    else if ( reportDate < Gage[j].nextDate ) result = 0.0;

    // --- otherwise report date/time falls right on end of current rain
    //     interval and start of next interval so use next interval's rainfall
    else result = Gage[j].nextRainfall;
    Gage[j].reportRainfall = result;
}

//=============================================================================

int getFirstRainfall(int j)
//
//  Input:   j = rain gage index
//  Output:  returns TRUE if successful
//  Purpose: positions rainfall record to date with first rainfall.
//
{
    int    k;                          // time series index
    float  vFirst;                     // first rain volume (ft or m)
    double rFirst;                     // first rain intensity (in/hr or mm/hr)

    // --- assign default values to date & rainfall
    Gage[j].startDate = NO_DATE;
    Gage[j].rainfall = 0.0;

    // --- initialize internal cumulative rainfall value
    Gage[j].rainAccum = 0;

    // --- use rain interface file if applicable
    if ( Gage[j].dataSource == RAIN_FILE )
    {
        if ( Frain.file && Gage[j].endFilePos > Gage[j].startFilePos )
        {
            // --- retrieve 1st date & rainfall volume from file
            fseek(Frain.file, Gage[j].startFilePos, SEEK_SET);
            fread(&Gage[j].startDate, sizeof(DateTime), 1, Frain.file);
            fread(&vFirst, sizeof(float), 1, Frain.file);
            Gage[j].currentFilePos = ftell(Frain.file);

            // --- convert rainfall to intensity
            Gage[j].rainfall = convertRainfall(j, (double)vFirst);
            return 1;
        }
        return 0;
    }

    // --- otherwise access user-supplied rainfall time series
    else
    {
        k = Gage[j].tSeries;
        if ( k >= 0 )
        {
            // --- retrieve first rainfall value from time series
            if ( table_getFirstEntry(&Tseries[k], &Gage[j].startDate,
                                     &rFirst) )
            {
                // --- convert rainfall to intensity
                Gage[j].rainfall = convertRainfall(j, rFirst);
                return 1;
            }
        }
        return 0;
    }
}

//=============================================================================

int getNextRainfall(int j)
//
//  Input:   j = rain gage index
//  Output:  returns 1 if successful; 0 if not
//  Purpose: positions rainfall record to date with next non-zero rainfall
//           while updating the gage's next rain intensity value.
//
//  Note: zero rainfall values explicitly entered into a rain file or
//        time series are skipped over so that a proper accounting of
//        wet and dry periods can be maintained.
//
{
    int    k;                          // time series index
    float  vNext;                      // next rain volume (ft or m)
    double rNext;                      // next rain intensity (in/hr or mm/hr)

    Gage[j].nextRainfall = 0.0;
    do
    {
        if ( Gage[j].dataSource == RAIN_FILE )
        {
            if ( Frain.file && Gage[j].currentFilePos < Gage[j].endFilePos )
            {
                fseek(Frain.file, Gage[j].currentFilePos, SEEK_SET);
                fread(&Gage[j].nextDate, sizeof(DateTime), 1, Frain.file);
                fread(&vNext, sizeof(float), 1, Frain.file);
                Gage[j].currentFilePos = ftell(Frain.file);
                rNext = convertRainfall(j, (double)vNext);
            }
            else return 0;
        }

        else
        {
            k = Gage[j].tSeries;
            if ( k >= 0 )
            {
                if ( !table_getNextEntry(&Tseries[k],
                        &Gage[j].nextDate, &rNext) ) return 0;
                rNext = convertRainfall(j, rNext);
            }
            else return 0;
        }
    } while (rNext == 0.0);
    Gage[j].nextRainfall = rNext;
    return 1;
}

//=============================================================================

double convertRainfall(int j, double r)
//
//  Input:   j = rain gage index
//           r = rainfall value (user units)
//  Output:  returns rainfall intensity (user units)
//  Purpose: converts rainfall value to an intensity (depth per hour).
//
{
    double r1;
    switch ( Gage[j].rainType )
    {
      case RAINFALL_INTENSITY:
        r1 = r;
        break;

      case RAINFALL_VOLUME:
        r1 = r / Gage[j].rainInterval * 3600.0;
        break;

      case CUMULATIVE_RAINFALL:
        if ( r  < Gage[j].rainAccum )
             r1 = r / Gage[j].rainInterval * 3600.0;
        else r1 = (r - Gage[j].rainAccum) / Gage[j].rainInterval * 3600.0;
        Gage[j].rainAccum = r;
        break;

      default: r1 = r;
    }
    return r1 * Gage[j].unitsFactor * Adjust.rainFactor;                       //(5.1.007)
}

//=============================================================================
