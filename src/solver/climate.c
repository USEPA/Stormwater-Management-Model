//-----------------------------------------------------------------------------
//   climate.c
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    03/20/10 (Build 5.1.001)
//            09/15/14 (Build 5.1.007)
//            03/19/15 (Build 5.1.008)
//            08/05/15 (Build 5.1.010)
//            08/01/16 (Build 5.1.011)
//            05/10/18 (Build 5.1.013)
//   Author:  L. Rossman
//
//   Climate related functions.
//
//   Build 5.1.007:
//   - NCDC GHCN climate file format added.
//   - Monthly adjustments for temperature, evaporation & rainfall added.
//
//   Build 5.1.008:
//   - Monthly adjustments for hyd. conductivity added.
//   - Time series evaporation rates can now vary within a day.
//   - Evaporation rates are now properly updated when only flow routing
//     is being simulated.
//
//   Build 5.1.010:
//   - Hargreaves evaporation now computed using 7-day average temperatures.
//             
//   Build 5.1.011:
//   - Monthly adjustment for hyd. conductivity <= 0 is ignored.
//
//   Build 5.1.013:
//   - Reads names of monthly adjustment patterns for various parameters
//     of a subcatchment from the [ADJUSTMENTS] section of input file.
///-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
enum ClimateFileFormats {UNKNOWN_FORMAT,
                         USER_PREPARED,     // SWMM 5's own user format
                         GHCND,             // NCDC GHCN Daily format
                         TD3200,            // NCDC TD3200 format
                         DLY0204};          // Canadian DLY02 or DLY04 format
static const int    MAXCLIMATEVARS  = 4;
static const int    MAXDAYSPERMONTH = 32;

// These variables are used when processing climate files.
enum   ClimateVarType {TMIN, TMAX, EVAP, WIND};
enum   WindSpeedType  {WDMV, AWND};
static char* ClimateVarWords[] = {"TMIN", "TMAX", "EVAP", "WDMV", "AWND",
                                  NULL};

//-----------------------------------------------------------------------------
//  Data Structures
//-----------------------------------------------------------------------------
typedef struct
{
    double    tAve;          // moving avg. for daily temperature (deg F)
    double    tRng;          // moving avg. for daily temp. range (deg F)
    double    ta[7];         // data window for tAve
    double    tr[7];         // data window for tRng
    int       count;         // length of moving average window
    int       maxCount;      // maximum length of moving average window
    int       front;         // index of front of moving average window
} TMovAve;


//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
// Temperature variables
static double    Tmin;                 // min. daily temperature (deg F)
static double    Tmax;                 // max. daily temperature (deg F)
static double    Trng;                 // 1/2 range of daily temperatures
static double    Trng1;                // prev. max - current min. temp.
static double    Tave;                 // average daily temperature (deg F)
static double    Hrsr;                 // time of min. temp. (hrs)
static double    Hrss;                 // time of max. temp (hrs)
static double    Hrday;                // avg. of min/max temp times
static double    Dhrdy;                // hrs. between min. & max. temp. times
static double    Dydif;                // hrs. between max. & min. temp. times
static DateTime  LastDay;              // date of last day with temp. data
static TMovAve   Tma;                  // moving average of daily temperatures

// Evaporation variables
static DateTime  NextEvapDate;         // next date when evap. rate changes
static double    NextEvapRate;         // next evaporation rate (user units)

// Climate file variables
static int      FileFormat;            // file format (see ClimateFileFormats)
static int      FileYear;              // current year of file data
static int      FileMonth;             // current month of year of file data
static int      FileDay;               // current day of month of file data
static int      FileLastDay;           // last day of current month of file data
static int      FileElapsedDays;       // number of days read from file
static double   FileValue[4];          // current day's values of climate data
static double   FileData[4][32];       // month's worth of daily climate data
static char     FileLine[MAXLINE+1];   // line from climate data file

static int      FileFieldPos[4];       // start of data fields for file record
static int      FileDateFieldPos;      // start of date field for file record 
static int      FileWindType;          // wind speed type

//-----------------------------------------------------------------------------
//  External functions (defined in funcs.h)
//-----------------------------------------------------------------------------
//  climate_readParams                 // called by input_parseLine
//  climate_readEvapParams             // called by input_parseLine
//  climate_validate                   // called by project_validate
//  climate_openFile                   // called by runoff_open
//  climate_initState                  // called by project_init
//  climate_setState                   // called by runoff_execute
//  climate_getNextEvapDate            // called by runoff_getTimeStep

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static int  getFileFormat(void);
static void readFileLine(int *year, int *month);
static void readUserFileLine(int *year, int *month);
static void readTD3200FileLine(int *year, int *month);
static void readDLY0204FileLine(int *year, int *month);
static void readFileValues(void);

static void setNextEvapDate(DateTime thedate);
static void setEvap(DateTime theDate);
static void setTemp(DateTime theDate);
static void setWind(DateTime theDate);
static void updateTempTimes(int day);
static void updateTempMoveAve(double tmin, double tmax);
static double getTempEvap(int day, double ta, double tr);

static void updateFileValues(DateTime theDate);
static void parseUserFileLine(void);
static void parseTD3200FileLine(void);
static void parseDLY0204FileLine(void);
static void setTD3200FileValues(int param);

static int  isGhcndFormat(char* line);
static void readGhcndFileLine(int *year, int *month);
static void parseGhcndFileLine(void);

//=============================================================================

int  climate_readParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//  Purpose: reads climate/temperature parameters from input line of data
//
//  Format of data can be
//    TIMESERIES  name
//    FILE        name
//    WINDSPEED   MONTHLY  v1  v2  ...  v12
//    WINDSPEED   FILE
//    SNOWMELT    v1  v2  ...  v6
//    ADC         IMPERV/PERV  v1  v2  ...  v10
//
{
    int      i, j, k;
    double   x[6], y;
    DateTime aDate;

    // --- identify keyword
    k = findmatch(tok[0], TempKeyWords);
    if ( k < 0 ) return error_setInpError(ERR_KEYWORD, tok[0]);
    switch (k)
    {
      case 0: // Time series name
        // --- check that time series name exists
        if ( ntoks < 2 ) return error_setInpError(ERR_ITEMS, "");
        i = project_findObject(TSERIES, tok[1]);
        if ( i < 0 ) return error_setInpError(ERR_NAME, tok[1]);

        // --- record the time series as being the data source for temperature
        Temp.dataSource = TSERIES_TEMP;
        Temp.tSeries = i;
        Tseries[i].refersTo = TSERIES_TEMP;
        break;

      case 1: // Climate file
        // --- record file as being source of temperature data
        if ( ntoks < 2 ) return error_setInpError(ERR_ITEMS, "");
        Temp.dataSource = FILE_TEMP;

        // --- save name and usage mode of external climate file
        Fclimate.mode = USE_FILE;
        sstrncpy(Fclimate.name, tok[1], MAXFNAME);

        // --- save starting date to read from file if one is provided
        Temp.fileStartDate = NO_DATE;
        if ( ntoks > 2 )
        {
            if ( *tok[2] != '*')
            {
                if ( !datetime_strToDate(tok[2], &aDate) )
                    return error_setInpError(ERR_DATETIME, tok[2]);
                Temp.fileStartDate = aDate;
            }
        }
        break;

      case 2: // Wind speeds
        // --- check if wind speeds will be supplied from climate file
        if ( strcomp(tok[1], w_FILE) )
        {
            Wind.type = FILE_WIND;
        }

        // --- otherwise read 12 monthly avg. wind speed values
        else
        {
            if ( ntoks < 14 ) return error_setInpError(ERR_ITEMS, "");
            Wind.type = MONTHLY_WIND;
            for (i=0; i<12; i++)
            {
                if ( !getDouble(tok[i+2], &y) )
                    return error_setInpError(ERR_NUMBER, tok[i+2]);
                Wind.aws[i] = y;
            }
        }
        break;

      case 3: // Snowmelt params
        if ( ntoks < 7 ) return error_setInpError(ERR_ITEMS, "");
        for (i=1; i<7; i++)
        {
            if ( !getDouble(tok[i], &x[i-1]) )
                return error_setInpError(ERR_NUMBER, tok[i]);
        }
        // --- convert deg. C to deg. F for snowfall temperature
        if ( UnitSystem == SI ) x[0] = 9./5.*x[0] + 32.0;
        Snow.snotmp = x[0];
        Snow.tipm   = x[1];
        Snow.rnm    = x[2];
        Temp.elev   = x[3] / UCF(LENGTH);
        Temp.anglat = x[4];
        Temp.dtlong = x[5] / 60.0;
        break;

      case 4:  // Areal Depletion Curve data
        // --- check if data is for impervious or pervious areas
        if ( ntoks < 12 ) return error_setInpError(ERR_ITEMS, "");
        if      ( match(tok[1], w_IMPERV) ) i = 0;
        else if ( match(tok[1], w_PERV)   ) i = 1;
        else return error_setInpError(ERR_KEYWORD, tok[1]);

        // --- read 10 fractional values
        for (j=0; j<10; j++)
        {
            if ( !getDouble(tok[j+2], &y) || y < 0.0 || y > 1.0 )
                return error_setInpError(ERR_NUMBER, tok[j+2]);
            Snow.adc[i][j] = y;
        }
        break;
    }
    return 0;
}

//=============================================================================

int climate_readEvapParams(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//  Purpose: reads evaporation parameters from input line of data.
//
//  Data formats are:
//    CONSTANT  value
//    MONTHLY   v1 ... v12
//    TIMESERIES name
//    TEMPERATURE
//    FILE      (v1 ... v12)
//    RECOVERY   name
//    DRY_ONLY   YES/NO
//
{
    int i, k;
    double x;

    // --- find keyword indicating what form the evaporation data is in
    k = findmatch(tok[0], EvapTypeWords);
    if ( k < 0 ) return error_setInpError(ERR_KEYWORD, tok[0]);

    // --- check for RECOVERY pattern data
    if ( k == RECOVERY )
    {
        if ( ntoks < 2 ) return error_setInpError(ERR_ITEMS, "");
        i = project_findObject(TIMEPATTERN, tok[1]);
        if ( i < 0 ) return error_setInpError(ERR_NAME, tok[1]);
        Evap.recoveryPattern = i;
        return 0;
    }

    // --- check for no evaporation in wet periods
    if ( k == DRYONLY )
    {
        if ( ntoks < 2 ) return error_setInpError(ERR_ITEMS, "");
        if      ( strcomp(tok[1], w_NO ) )  Evap.dryOnly = FALSE;
        else if ( strcomp(tok[1], w_YES ) ) Evap.dryOnly = TRUE;
        else return error_setInpError(ERR_KEYWORD, tok[1]);
        return 0;
    }

    // --- process data depending on its form
    Evap.type = k;
    if ( k != TEMPERATURE_EVAP && ntoks < 2 )
        return error_setInpError(ERR_ITEMS, "");
    switch ( k )
    {
      case CONSTANT_EVAP:
        // --- for constant evap., fill monthly avg. values with same number
        if ( !getDouble(tok[1], &x) )
            return error_setInpError(ERR_NUMBER, tok[1]);
        for (i=0; i<12; i++) Evap.monthlyEvap[i] = x;
        break;

      case MONTHLY_EVAP:
        // --- for monthly evap., read a value for each month of year
        if ( ntoks < 13 ) return error_setInpError(ERR_ITEMS, "");
        for ( i=0; i<12; i++)
            if ( !getDouble(tok[i+1], &Evap.monthlyEvap[i]) )
                return error_setInpError(ERR_NUMBER, tok[i+1]);
        break;

      case TIMESERIES_EVAP:
        // --- for time series evap., read name of time series
        i = project_findObject(TSERIES, tok[1]);
        if ( i < 0 ) return error_setInpError(ERR_NAME, tok[1]);
        Evap.tSeries = i;
        Tseries[i].refersTo = TIMESERIES_EVAP;
        break;

      case FILE_EVAP:
        // --- for evap. from climate file, read monthly pan coeffs.
        //     if they are provided (default values are 1.0)
        if ( ntoks > 1 )
        {
            if ( ntoks < 13 ) return error_setInpError(ERR_ITEMS, "");
            for (i=0; i<12; i++)
            {
                if ( !getDouble(tok[i+1], &Evap.panCoeff[i]) )
                    return error_setInpError(ERR_NUMBER, tok[i+1]);
            }
        }
        break;
    }
    return 0;
}

//=============================================================================

int climate_readAdjustments(char* tok[], int ntoks)
//
//  Input:   tok[] = array of string tokens
//           ntoks = number of tokens
//  Output:  returns error code
//  Purpose: reads adjustments to monthly evaporation or rainfall
//           from input line of data.
//
//  Data formats are:
//    TEMPERATURE   v1 ... v12
//    EVAPORATION   v1 ... v12
//    RAINFALL      v1 ... v12
//    CONDUCTIVITY  v1 ... v12
//    N-PERV        subcatchID  patternID                                      //(5.1.013
//    DSTORE        subcatchID  patternID                                      //
//    INFIL         subcatchID  patternID                                      //
{
    int i, j;                                                                  //(5.1.013)

    if (ntoks == 1) return 0;

    if ( match(tok[0], "TEMP") )
    {
        if ( ntoks < 13 )  return error_setInpError(ERR_ITEMS, "");
        for (i = 1; i < 13; i++)
        {
            if ( !getDouble(tok[i], &Adjust.temp[i-1]) )
                return error_setInpError(ERR_NUMBER, tok[i]);
        }
        return 0;
    }

    if ( match(tok[0], "EVAP") )
    {
        if ( ntoks < 13 )  return error_setInpError(ERR_ITEMS, "");
        for (i = 1; i < 13; i++)
        {
            if ( !getDouble(tok[i], &Adjust.evap[i-1]) )
                return error_setInpError(ERR_NUMBER, tok[i]);
        }
        return 0;
    }

    if ( match(tok[0], "RAIN") )
    {
        if ( ntoks < 13 )  return error_setInpError(ERR_ITEMS, "");
        for (i = 1; i < 13; i++)
        {
            if ( !getDouble(tok[i], &Adjust.rain[i-1]) )
                return error_setInpError(ERR_NUMBER, tok[i]);
        }
        return 0;
    }

    if ( match(tok[0], "CONDUCT") )
    {
        if ( ntoks < 13 )  return error_setInpError(ERR_ITEMS, "");
        for (i = 1; i < 13; i++)
        {
            if ( !getDouble(tok[i], &Adjust.hydcon[i-1]) )
                return error_setInpError(ERR_NUMBER, tok[i]);
            if ( Adjust.hydcon[i-1] <= 0.0 ) Adjust.hydcon[i-1] = 1.0;
        }
        return 0;
    }

////  Following code segments added to release 5.1.013.  ////                  //(5.1.013)
    if ( match(tok[0], "N-PERV") )
    {
        if ( ntoks < 3 ) return error_setInpError(ERR_ITEMS, "");
        i = project_findObject(SUBCATCH, tok[1]);
        if (i < 0) return error_setInpError(ERR_NAME, tok[1]);
        j = project_findObject(TIMEPATTERN, tok[2]);
        if (j < 0) return error_setInpError(ERR_NAME, tok[2]);
        Subcatch[i].nPervPattern = j;
        return 0;
    }

    if ( match(tok[0], "DSTORE") )
    {
        if (ntoks < 3) return error_setInpError(ERR_ITEMS, "");
        i = project_findObject(SUBCATCH, tok[1]);
        if (i < 0) return error_setInpError(ERR_NAME, tok[1]);
        j = project_findObject(TIMEPATTERN, tok[2]);
        if (j < 0) return error_setInpError(ERR_NAME, tok[2]);
        Subcatch[i].dStorePattern = j;
        return 0;
    }

    if (match(tok[0], "INFIL"))
    {
        if (ntoks < 3) return error_setInpError(ERR_ITEMS, "");
        i = project_findObject(SUBCATCH, tok[1]);
        if (i < 0) return error_setInpError(ERR_NAME, tok[1]);
        j = project_findObject(TIMEPATTERN, tok[2]);
        if (j < 0) return error_setInpError(ERR_NAME, tok[2]);
        Subcatch[i].infilPattern = j;
        return 0;
    }
////
    return error_setInpError(ERR_KEYWORD, tok[0]);
}

//=============================================================================

void climate_validate()
//
//  Input:   none
//  Output:  none
//  Purpose: validates climatological variables
//
{
    int       i;
    double    a, z, pa;

    // --- check if climate data comes from external data file 
    if ( Wind.type == FILE_WIND || Evap.type == FILE_EVAP ||
         Evap.type == TEMPERATURE_EVAP )
    {
        if ( Fclimate.mode == NO_FILE )
        {
            report_writeErrorMsg(ERR_NO_CLIMATE_FILE, "");
        }
    }

    // --- open the climate data file
    if ( Fclimate.mode == USE_FILE ) climate_openFile();

    // --- snow melt parameters tipm & rnm must be fractions
    if ( Snow.tipm < 0.0 ||
         Snow.tipm > 1.0 ||
         Snow.rnm  < 0.0 ||
         Snow.rnm  > 1.0 ) report_writeErrorMsg(ERR_SNOWMELT_PARAMS, "");

    // --- latitude should be between -90 & 90 degrees
    a = Temp.anglat;
    if ( a <= -89.99 ||
         a >= 89.99  ) report_writeErrorMsg(ERR_SNOWMELT_PARAMS, "");
    else Temp.tanAnglat = tan(a * PI / 180.0);

    // --- compute psychrometric constant
    z = Temp.elev / 1000.0;
    if ( z <= 0.0 ) pa = 29.9;
    else  pa = 29.9 - 1.02*z + 0.0032*pow(z, 2.4); // atmos. pressure
    Temp.gamma = 0.000359 * pa;

    // --- convert units of monthly temperature & evap adjustments
    for (i = 0; i < 12; i++)
    {
        if (UnitSystem == SI) Adjust.temp[i] *= 9.0/5.0;
        Adjust.evap[i] /= UCF(EVAPRATE);
    }
}

//=============================================================================

void climate_openFile()
//
//  Input:   none
//  Output:  none
//  Purpose: opens a climate file and reads in first set of values.
//
{
    int i, m, y;

    // --- open the file
    if ( (Fclimate.file = fopen(Fclimate.name, "rt")) == NULL )
    {
        report_writeErrorMsg(ERR_CLIMATE_FILE_OPEN, Fclimate.name);
        return;
    }

    // --- initialize values of file's climate variables
    //     (Temp.ta was previously initialized in project.c)
    FileValue[TMIN] = Temp.ta;
    FileValue[TMAX] = Temp.ta;
    FileValue[EVAP] = 0.0;
    FileValue[WIND] = 0.0;

    // --- find climate file's format
    FileFormat = getFileFormat();
    if ( FileFormat == UNKNOWN_FORMAT )
    {
        report_writeErrorMsg(ERR_CLIMATE_FILE_READ, Fclimate.name);
        return;
    }

    // --- position file to begin reading climate file at either user-specified
    //     month/year or at start of simulation period.
    rewind(Fclimate.file);
    strcpy(FileLine, "");
    if ( Temp.fileStartDate == NO_DATE )
        datetime_decodeDate(StartDate, &FileYear, &FileMonth, &FileDay);
    else
        datetime_decodeDate(Temp.fileStartDate, &FileYear, &FileMonth, &FileDay);
    while ( !feof(Fclimate.file) )
    {
        strcpy(FileLine, "");
        readFileLine(&y, &m);
        if ( y == FileYear && m == FileMonth ) break;
    }
    if ( feof(Fclimate.file) )
    {
        report_writeErrorMsg(ERR_CLIMATE_END_OF_FILE, Fclimate.name);
        return;
    }

    // --- initialize file dates and current climate variable values
    if ( !ErrorCode )
    {
        FileElapsedDays = 0;
        FileLastDay = datetime_daysPerMonth(FileYear, FileMonth);
        readFileValues();
        for (i=TMIN; i<=WIND; i++)
        {
            if ( FileData[i][FileDay] == MISSING ) continue;
            FileValue[i] = FileData[i][FileDay];
        }
    }
}

//=============================================================================

void climate_initState()
//
//  Input:   none
//  Output:  none
//  Purpose: initializes climate state variables.
//
{
    LastDay = NO_DATE;
    Temp.tmax = MISSING;
    Snow.removed = 0.0;
    NextEvapDate = StartDate;
    NextEvapRate = 0.0;

    // --- initialize variables for time series evaporation
    if ( Evap.type == TIMESERIES_EVAP && Evap.tSeries >= 0  )
    {
        // --- initialize NextEvapDate & NextEvapRate to first entry of
        //     time series whose date <= the simulation start date
        table_getFirstEntry(&Tseries[Evap.tSeries],
                            &NextEvapDate, &NextEvapRate);
        if ( NextEvapDate < StartDate )
        {  
            setNextEvapDate(StartDate);
        }
        Evap.rate = NextEvapRate / UCF(EVAPRATE);

        // --- find the next time evaporation rates change after this
        setNextEvapDate(NextEvapDate); 
    }

    // --- initialize variables for temperature evaporation
    if ( Evap.type == TEMPERATURE_EVAP )
    {
        Tma.maxCount = sizeof(Tma.ta) / sizeof(double);
        Tma.count = 0;
        Tma.front = 0;
        Tma.tAve = 0.0;
        Tma.tRng = 0.0;
    }
}

//=============================================================================

void climate_setState(DateTime theDate)
//
//  Input:   theDate = simulation date
//  Output:  none
//  Purpose: sets climate variables for current date.
//
{
    if ( Fclimate.mode == USE_FILE ) updateFileValues(theDate);
    if ( Temp.dataSource != NO_TEMP ) setTemp(theDate);
    setEvap(theDate);
    setWind(theDate);
    Adjust.rainFactor = Adjust.rain[datetime_monthOfYear(theDate)-1];
    Adjust.hydconFactor = Adjust.hydcon[datetime_monthOfYear(theDate)-1];
    setNextEvapDate(theDate);
}

//=============================================================================

DateTime climate_getNextEvapDate()
//
//  Input:   none
//  Output:  returns the current value of NextEvapDate
//  Purpose: gets the next date when evaporation rate changes.
//
{
    return NextEvapDate;
}

//=============================================================================

void setNextEvapDate(DateTime theDate)
//
//  Input:   theDate = current simulation date
//  Output:  sets a new value for NextEvapDate
//  Purpose: finds date for next change in evaporation after the current date.
//
{
    int    yr, mon, day, k;
    double d, e;

    // --- do nothing if current date hasn't reached the current next date
    if ( NextEvapDate > theDate ) return;

    switch ( Evap.type )
    {
      // --- for constant evaporation, use a next date far in the future
      case CONSTANT_EVAP:
         NextEvapDate = theDate + 365.;
         break;

      // --- for monthly evaporation, use the start of the next month
      case MONTHLY_EVAP:
        datetime_decodeDate(theDate, &yr, &mon, &day);
        if ( mon == 12 )
        {
            mon = 1;
            yr++;
        }
        else mon++;
        NextEvapDate = datetime_encodeDate(yr, mon, 1);
        break;

      // --- for time series evaporation, find the next entry in the
      //     series on or after the current date
      case TIMESERIES_EVAP:
        k = Evap.tSeries;
        if ( k >= 0 )
        {
            NextEvapDate = theDate + 365.;
            while ( table_getNextEntry(&Tseries[k], &d, &e) &&
                    d <= EndDateTime )
            {
                if ( d >= theDate )
                {
                    NextEvapDate = d;
                    NextEvapRate = e;
                    break;
                }
            }
        }
        break;

      // --- for climate file daily evaporation, use the next day
      case FILE_EVAP:
        NextEvapDate = floor(theDate) + 1.0;
        break;

      default: NextEvapDate = theDate + 365.;
    }
}

//=============================================================================

void updateFileValues(DateTime theDate)
//
//  Input:   theDate = current simulation date
//  Output:  none
//  Purpose: updates daily climate variables for new day or reads in
//           another month worth of values if a new month begins.
//
//  NOTE:    counters FileElapsedDays, FileDay, FileMonth, FileYear and
//           FileLastDay were initialized in climate_openFile().
//
{
    int i;
    int deltaDays;

    // --- see if a new day has begun
    deltaDays = (int)(floor(theDate) - floor(StartDateTime));
    if ( deltaDays > FileElapsedDays )
    {
        // --- advance day counters
        FileElapsedDays++;
        FileDay++;

        // --- see if new month of data needs to be read from file
        if ( FileDay > FileLastDay )
        {
            FileMonth++;
            if ( FileMonth > 12 )
            {
                FileMonth = 1;
                FileYear++;
            }
            readFileValues();
            FileDay = 1;
            FileLastDay = datetime_daysPerMonth(FileYear, FileMonth);
        }

        // --- set climate variables for new day
        for (i=TMIN; i<=WIND; i++)
        {
            // --- no change in current value if its missing
            if ( FileData[i][FileDay] == MISSING ) continue;
            FileValue[i] = FileData[i][FileDay];
        }
    }
}

//=============================================================================

void setTemp(DateTime theDate)
//
//  Input:   theDate = simulation date
//  Output:  none
//  Purpose: updates temperatures for new simulation date.
//
{
    int      j;                        // snow data object index
    int      k;                        // time series index
    int      mon;                      // month of year 
    int      day;                      // day of year
    DateTime theDay;                   // calendar day
    double   hour;                     // hour of day
    double   tmp;                      // temporary temperature

    // --- see if a new day has started
    mon = datetime_monthOfYear(theDate);
    theDay = floor(theDate);
    if ( theDay > LastDay )
    {
        // --- update min. & max. temps & their time of day
        day = datetime_dayOfYear(theDate);
        if ( Temp.dataSource == FILE_TEMP )
        {
            Tmin = FileValue[TMIN] + Adjust.temp[mon-1];
            Tmax = FileValue[TMAX] + Adjust.temp[mon-1];
            if ( Tmin > Tmax )
            {
                tmp = Tmin;
                Tmin = Tmax;
                Tmax = tmp;
            }
            updateTempTimes(day);
            if ( Evap.type == TEMPERATURE_EVAP )
            {
                updateTempMoveAve(Tmin, Tmax); 
                FileValue[EVAP] = getTempEvap(day, Tma.tAve, Tma.tRng);
            }
        }

        // --- compute snow melt coefficients based on day of year
        Snow.season = sin(0.0172615*(day-81.0));
        for (j=0; j<Nobjects[SNOWMELT]; j++)
        {
            snow_setMeltCoeffs(j, Snow.season);
        }

        // --- update date of last day analyzed
        LastDay = theDate;
    }

    // --- for min/max daily temps. from climate file,
    //     compute hourly temp. by sinusoidal interp.
    if ( Temp.dataSource == FILE_TEMP )
    {
        hour = (theDate - theDay) * 24.0;
        if ( hour < Hrsr )
            Temp.ta = Tmin + Trng1/2.0 * sin(PI/Dydif * (Hrsr - hour));
        else if ( hour >= Hrsr && hour <= Hrss )
            Temp.ta = Tave + Trng * sin(PI/Dhrdy * (Hrday - hour));
        else
            Temp.ta = Tmax - Trng * sin(PI/Dydif * (hour - Hrss));
    }

    // --- for user-supplied temperature time series,
    //     get temperature value from time series
    if ( Temp.dataSource == TSERIES_TEMP )
    {
        k = Temp.tSeries;
        if ( k >= 0)
        {
            Temp.ta = table_tseriesLookup(&Tseries[k], theDate, TRUE);

            // --- convert from deg. C to deg. F if need be
            if ( UnitSystem == SI )
            {
                Temp.ta = (9./5.) * Temp.ta + 32.0;
            }

            // --- apply climate change adjustment factor 
            Temp.ta += Adjust.temp[mon-1];
        }
    }

    // --- compute saturation vapor pressure
    Temp.ea = 8.1175e6 * exp(-7701.544 / (Temp.ta + 405.0265) );
}

//=============================================================================

void setEvap(DateTime theDate)
//
//  Input:   theDate = simulation date
//  Output:  none
//  Purpose: sets evaporation rate (ft/sec) for a specified date.
//
{
    int k;
    int mon = datetime_monthOfYear(theDate);

    switch ( Evap.type )
    {
      case CONSTANT_EVAP:
        Evap.rate = Evap.monthlyEvap[0] / UCF(EVAPRATE);
        break;

      case MONTHLY_EVAP:
        Evap.rate = Evap.monthlyEvap[mon-1] / UCF(EVAPRATE);
        break;

      case TIMESERIES_EVAP:
        if ( theDate >= NextEvapDate )
            Evap.rate = NextEvapRate / UCF(EVAPRATE);
        break;

      case FILE_EVAP:
        Evap.rate = FileValue[EVAP] / UCF(EVAPRATE);
        Evap.rate *= Evap.panCoeff[mon-1];
        break;

      case TEMPERATURE_EVAP:
        Evap.rate = FileValue[EVAP] / UCF(EVAPRATE);
        break;

      default: Evap.rate = 0.0;
    }

    // --- apply climate change adjustment
    Evap.rate += Adjust.evap[mon-1];

    // --- set soil recovery factor
    Evap.recoveryFactor = 1.0;
    k = Evap.recoveryPattern;
    if ( k >= 0 && Pattern[k].type == MONTHLY_PATTERN )
    {
        Evap.recoveryFactor = Pattern[k].factor[mon-1];
    }
}

//=============================================================================

void setWind(DateTime theDate)
//
//  Input:   theDate = simulation date
//  Output:  none
//  Purpose: sets wind speed (mph) for a specified date.
//
{
    int yr, mon, day;

    switch ( Wind.type )
    {
      case MONTHLY_WIND:
        datetime_decodeDate(theDate, &yr, &mon, &day);
        Wind.ws = Wind.aws[mon-1] / UCF(WINDSPEED);
        break;

      case FILE_WIND:
        Wind.ws = FileValue[WIND];
        break;

      default: Wind.ws = 0.0;
    }
}

//=============================================================================

void updateTempTimes(int day)
//
//  Input:   day = day of year
//  Output:  none
//  Purpose: computes time of day when min/max temperatures occur.
//           (min. temp occurs at sunrise, max. temp. at 3 hrs. < sunset)
//
{
    double decl;                       // earth's declination
    double hrang;                      // hour angle of sunrise/sunset
    double arg;

    decl  = 0.40928*cos(0.017202*(172.0-day));
    arg = -tan(decl)*Temp.tanAnglat;
    if      ( arg <= -1.0 ) arg = PI;
    else if ( arg >= 1.0 )  arg = 0.0;
    else                    arg = acos(arg);
    hrang = 3.8197 * arg;
    Hrsr  = 12.0 - hrang + Temp.dtlong;
    Hrss  = 12.0 + hrang + Temp.dtlong - 3.0;
    Dhrdy = Hrsr - Hrss;
    Dydif = 24.0 + Hrsr - Hrss;
    Hrday = (Hrsr + Hrss) / 2.0;
    Tave  = (Tmin + Tmax) / 2.0;
    Trng  = (Tmax - Tmin) / 2.0;
    if ( Temp.tmax == MISSING ) Trng1 = Tmax - Tmin;
    else                        Trng1 = Temp.tmax - Tmin;
    Temp.tmax = Tmax;
}

//=============================================================================

double getTempEvap(int day, double tave, double trng)
//
//  Input:   day = day of year
//           tave = 7-day average temperature (deg F)
//           trng = 7-day average daily temperature range (deg F)
//  Output:  returns evaporation rate in user's units (US:in/day, SI:mm/day)
//  Purpose: uses Hargreaves method to compute daily evaporation rate
//           from daily average temperatures and Julian day.
//
{
    double a = 2.0*PI/365.0;
    double ta = (tave - 32.0)*5.0/9.0;           //average temperature (deg C)
    double tr = trng*5.0/9.0;                    //temperature range (deg C)
    double lamda = 2.50 - 0.002361 * ta;         //latent heat of vaporization
    double dr = 1.0 + 0.033*cos(a*day);          //relative earth-sun distance
    double phi = Temp.anglat*2.0*PI/360.0;       //latitude angle (rad)
    double del = 0.4093*sin(a*(284+day));        //solar declination angle (rad)
    double omega = acos(-tan(phi)*tan(del));     //sunset hour angle (rad)
    double ra = 37.6*dr*                         //extraterrestrial radiation
                (omega*sin(phi)*sin(del) +
                 cos(phi)*cos(del)*sin(omega));
    double e = 0.0023*ra/lamda*sqrt(tr)*(ta+17.8);    //evap. rate (mm/day)
    if ( e < 0.0 ) e = 0.0;
    if ( UnitSystem == US ) e /= MMperINCH;           //evap rate (in/day)
    return e;
}

//=============================================================================

int  getFileFormat()
//
//  Input:   none
//  Output:  returns code number of climate file's format
//  Purpose: determines what format the climate file is in.
//
{
    char recdType[4] = "";
    char elemType[4] = "";
    char filler[5] = "";
    char staID[80];
    char s[80];
    char line[MAXLINE];

    int  y, m, d, n;

    // --- read first line of file
    if ( fgets(line, MAXLINE, Fclimate.file) == NULL ) return UNKNOWN_FORMAT;

    // --- check for TD3200 format
    sstrncpy(recdType, line, 3);
    sstrncpy(filler, &line[23], 4);
    if ( strcmp(recdType, "DLY") == 0 &&
         strcmp(filler, "9999")  == 0 ) return TD3200;

    // --- check for DLY0204 format
    if ( strlen(line) >= 233 )
    {
        sstrncpy(elemType, &line[13], 3);
        n = atoi(elemType);
        if ( n == 1 || n == 2 || n == 151 ) return DLY0204;
    }

    // --- check for USER_PREPARED format
    n = sscanf(line, "%s %d %d %d %s", staID, &y, &m, &d, s);
    if ( n == 5 ) return USER_PREPARED;

    // --- check for GHCND format
    if ( isGhcndFormat(line) ) return GHCND;

    return UNKNOWN_FORMAT;
}

//=============================================================================

void readFileLine(int *y, int *m)
//
//  Input:   none
//  Output:  y = year
//           m = month
//  Purpose: reads year & month from next line of climate file.
//
{
    // --- read next line from climate data file
    while ( strlen(FileLine) == 0 )
    {
        if ( fgets(FileLine, MAXLINE, Fclimate.file) == NULL ) return;
     	if ( FileLine[0] == '\n' ) FileLine[0] = '\0';
    }

    // --- parse year & month from line
    switch (FileFormat)
    {
    case  USER_PREPARED: readUserFileLine(y, m);   break;
    case  TD3200:        readTD3200FileLine(y,m);  break;
    case  DLY0204:       readDLY0204FileLine(y,m); break;
    case  GHCND:         readGhcndFileLine(y,m);   break; 
    }
}

//=============================================================================

void readUserFileLine(int* y, int* m)
//
//  Input:   none
//  Output:  y = year
//           m = month
//  Purpose: reads year & month from line of User-Prepared climate file.
//
{
    int n;
    char staID[80];
    n = sscanf(FileLine, "%s %d %d", staID, y, m);
    if ( n < 3 )
    {
        report_writeErrorMsg(ERR_CLIMATE_FILE_READ, Fclimate.name);
    }
}

//=============================================================================

void readTD3200FileLine(int* y, int* m)
//
//  Input:   none
//  Output:  y = year
//           m = month
//  Purpose: reads year & month from line of TD-3200 climate file.
//
{
    char recdType[4] = "";
    char year[5] = "";
    char month[3] = "";
    int  len;

    // --- check for minimum number of characters
    len = strlen(FileLine);
    if ( len < 30 )
    {
        report_writeErrorMsg(ERR_CLIMATE_FILE_READ, Fclimate.name);
        return;
    }

    // --- check for proper type of record
    sstrncpy(recdType, FileLine, 3);
    if ( strcmp(recdType, "DLY") != 0 )
    {
        report_writeErrorMsg(ERR_CLIMATE_FILE_READ, Fclimate.name);
        return;
    }

    // --- get record's date
    sstrncpy(year,  &FileLine[17], 4);
    sstrncpy(month, &FileLine[21], 2);
    *y = atoi(year);
    *m = atoi(month);
}

//=============================================================================

void readDLY0204FileLine(int* y, int* m)
//
//  Input:   none
//  Output:  y = year
//           m = month
//  Purpose: reads year & month from line of DLY02 or DLY04 climate file.
//
{
    char year[5] = "";
    char month[3] = "";
    int  len;

    // --- check for minimum number of characters
    len = strlen(FileLine);
    if ( len < 16 )
    {
        report_writeErrorMsg(ERR_CLIMATE_FILE_READ, Fclimate.name);
        return;
    }

    // --- get record's date
    sstrncpy(year,  &FileLine[7], 4);
    sstrncpy(month, &FileLine[11], 2);
    *y = atoi(year);
    *m = atoi(month);
}

//=============================================================================

void readFileValues()
//
//  Input:   none
//  Output:  none
//  Purpose: reads next month's worth of data from climate file.
//
{
    int  i, j;
    int  y, m;

    // --- initialize FileData array to missing values
    for ( i=0; i<MAXCLIMATEVARS; i++)
    {
        for (j=0; j<MAXDAYSPERMONTH; j++) FileData[i][j] = MISSING;
    }

    while ( !ErrorCode )
    {
        // --- return when date on line is after current file date
        if ( feof(Fclimate.file) ) return;
        readFileLine(&y, &m);
        if ( y > FileYear || m > FileMonth ) return;

        // --- parse climate values from file line
        switch (FileFormat)
        {
        case  USER_PREPARED: parseUserFileLine();   break;
        case  TD3200:        parseTD3200FileLine();  break;
        case  DLY0204:       parseDLY0204FileLine(); break;
        case  GHCND:         parseGhcndFileLine();   break; 
        }
        strcpy(FileLine, "");
    }
}

//=============================================================================

void parseUserFileLine()
//
//  Input:   none
//  Output:  none
//  Purpose: parses climate variable values from a line of a user-prepared
//           climate file.
//
{
    int   n;
    int   y, m, d;
    char  staID[80];
    char  s0[80];
    char  s1[80];
    char  s2[80];
    char  s3[80];
    double x;

    // --- read day, Tmax, Tmin, Evap, & Wind from file line
    n = sscanf(FileLine, "%s %d %d %d %s %s %s %s",
        staID, &y, &m, &d, s0, s1, s2, s3);
    if ( n < 4 ) return;
    if ( d < 1 || d > 31 ) return;

    // --- process TMAX
    if ( strlen(s0) > 0 && *s0 != '*' )
    {
        x = atof(s0);
        if ( UnitSystem == SI ) x = 9./5.*x + 32.0;
        FileData[TMAX][d] =  x;
    }

    // --- process TMIN
    if ( strlen(s1) > 0 && *s1 != '*' )
    {
        x = atof(s1);
        if ( UnitSystem == SI ) x = 9./5.*x + 32.0;
        FileData[TMIN][d] =  x;
    }

    // --- process EVAP
    if ( strlen(s2) > 0 && *s2 != '*' ) FileData[EVAP][d] = atof(s2);

    // --- process WIND
    if ( strlen(s3) > 0 && *s3 != '*' ) FileData[WIND][d] = atof(s3);
}

//=============================================================================

void parseTD3200FileLine()
//
//  Input:   none
//  Output:  none
//  Purpose: parses climate variable values from a line of a TD3200 file.
//
{
    int  i;
    char param[5] = "";

    // --- parse parameter name
    sstrncpy(param, &FileLine[11], 4);

    // --- see if parameter is temperature, evaporation or wind speed
    for (i=0; i<MAXCLIMATEVARS; i++)
    {
        if (strcmp(param, ClimateVarWords[i]) == 0 ) setTD3200FileValues(i);
    }
}

//=============================================================================

void setTD3200FileValues(int i)
//
//  Input:   i = climate variable code
//  Output:  none
//  Purpose: reads month worth of values for climate variable from TD-3200 file.
//
{
    char valCount[4] = "";
    char day[3] = "";
    char sign[2] = "";
    char value[6] = "";
    char flag2[2] = "";
    double x;
    int  nValues;
    int  j, k, d;
    int  lineLength;

    // --- parse number of days with data from cols. 27-29 of file line
    sstrncpy(valCount, &FileLine[27], 3);
    nValues = atoi(valCount);
    lineLength = strlen(FileLine);

    // --- check for enough characters on line
    if ( lineLength >= 12*nValues + 30 )
    {
        // --- for each day's value
        for (j=0; j<nValues; j++)
        {
            // --- parse day, value & flag from file line
            k = 30 + j*12;
            sstrncpy(day,   &FileLine[k], 2);
            sstrncpy(sign,  &FileLine[k+4], 1);
            sstrncpy(value, &FileLine[k+5], 5);
            sstrncpy(flag2, &FileLine[k+11], 1);

            // --- if value is valid then store it in FileData array
            d = atoi(day);
            if ( strcmp(value, "99999") != 0
                 && ( flag2[0] == '0' || flag2[0] == '1')
                 &&   d > 0
                 &&   d <= 31 )
            {
                // --- convert from string value to numerical value
                x = atof(value);
                if ( sign[0] == '-' ) x = -x;

                // --- convert evaporation from hundreths of inches
                if ( i == EVAP )
                {
                    x /= 100.0;

                    // --- convert to mm if using SI units
                    if ( UnitSystem == SI ) x *= MMperINCH;
                }

                // --- convert wind speed from miles/day to miles/hour
                if ( i == WIND ) x /= 24.0;

                // --- store value
                FileData[i][d] = x;
            }
        }
    }
}

//=============================================================================

void parseDLY0204FileLine()
//
//  Input:   none
//  Output:  none
//  Purpose: parses a month's worth of climate variable values from a line of
//           a DLY02 or DLY04 climate file.
//
{
    int  j, k, p;
    char param[4] = "";
    char sign[2]  = "";
    char value[6] = "";
    char code[2]  = "";
    double x;

    // --- parse parameter name
    sstrncpy(param, &FileLine[13], 3);

    // --- see if parameter is min or max temperature
    p = atoi(param);
    if ( p == 1 ) p = TMAX;
    else if ( p == 2 ) p = TMIN;
    else if ( p == 151 ) p = EVAP;
    else return;

    // --- check for 233 characters on line
    if ( strlen(FileLine) < 233 ) return;

    // --- for each of 31 days
    k = 16;
    for (j=1; j<=31; j++)
    {
        // --- parse value & flag from file line
        sstrncpy(sign,  &FileLine[k], 1);
        sstrncpy(value, &FileLine[k+1], 5);
        sstrncpy(code,  &FileLine[k+6], 1);
        k += 7;

        // --- if value is valid then store it in FileData array

        if ( strcmp(value, "99999") != 0 && strcmp(value, "     ") != 0 )
        {
            switch (p)
            {
            case TMAX:
            case TMIN:
                // --- convert from integer tenths of a degree C to degrees F
                x = atof(value) / 10.0;
                if ( sign[0] == '-' ) x = -x;
                x = 9./5.*x + 32.0;
                break;
            case EVAP:
                // --- convert from 0.1 mm to inches or mm
                x = atof(value) / 10.0;
                if ( UnitSystem == US ) x /= MMperINCH;
                break;
			default: return;
            }
            FileData[p][j] = x;
        }
    }
}

//=============================================================================

int isGhcndFormat(char* line)
//
//  Input:   line = first line of text from a climate file
//  Output:  returns TRUE if climate file is in NCDC GHCN Daily format.
//  Purpose: Checks if a climate file is in the NCDC GHCN Daily format
//           and determines the position of each climate variable field.
//
{
    int i;
    char* ptr;

    // --- find starting position of the DATE field
    ptr = strstr(line, "DATE");
    if ( ptr == NULL ) return FALSE;
    FileDateFieldPos = ptr - line;

    // --- initialize starting position of each data field
    for ( i = TMIN; i <= WIND; i++) FileFieldPos[i] = -1;

    // --- find starting position of each climate variable's data field
    ptr = strstr(line, "TMIN");
    if ( ptr ) FileFieldPos[TMIN] = ptr - line;
    ptr = strstr(line, "TMAX");
    if ( ptr ) FileFieldPos[TMAX] = ptr - line;
    ptr = strstr(line, "EVAP");
    if ( ptr ) FileFieldPos[EVAP] = ptr - line;

    // --- WIND can either be daily movement or average speed
    FileWindType = WDMV;
    ptr = strstr(line, "WDMV");
    if ( ptr == NULL )
    {
        FileWindType = AWND;
        ptr = strstr(line, "AWND");
    }
    if ( ptr ) FileFieldPos[WIND] = ptr - line;

    // --- check if at least one climate variable was found
    for (i = TMIN; i <= WIND; i++) if (FileFieldPos[i] >= 0 ) return TRUE;
    return FALSE;
}

//=============================================================================

void readGhcndFileLine(int* y, int* m)
//
//  Input:   none
//  Output:  y = year
//           m = month
//  Purpose: reads year & month from line of a NCDC GHCN Daily climate file.
//
{
    int n = sscanf(&FileLine[FileDateFieldPos], "%4d%2d", y, m);
    if ( n != 2 )
    {
        *y = -99999;
        *m = -99999;
    }
}

//=============================================================================

void parseGhcndFileLine()
//
//  Input:   none
//  Output:  none
//  Purpose: parses a line of a NCDC GHCN Daily file for daily
//           values of max/min temperature, pan evaporation and
//           wind speed.
//
{
    int y, m, d, n, v;
    double x;

    // --- parse day of month from date field
    n = sscanf(&FileLine[FileDateFieldPos], "%4d%2d%2d", &y, &m, &d);
    if ( n < 3 ) return;
    if ( d < 1 || d > 31 ) return;

    // --- parse temperatures (in tenths of deg. C) to deg F
    if ( FileFieldPos[TMAX] >= 0 )
    {
        if ( sscanf(&FileLine[FileFieldPos[TMAX]], "%8d", &v) > 0 )
        {
            if ( abs(v) < 9999 )
                FileData[TMAX][d] = (double)v*0.1*9.0/5.0 + 32.0;
        }
    }
    if ( FileFieldPos[TMIN] >= 0 )
    {
        if ( sscanf(&FileLine[FileFieldPos[TMIN]], "%8d", &v) > 0 )
        {
            if ( abs(v) < 9999 )
                FileData[TMIN][d] = (double)v*0.1*9.0/5.0 + 32.0;
        }
    }

    // -- parse evaporation (in tenths of mm) to user units
    if ( FileFieldPos[EVAP] >= 0 )
    {
        if ( sscanf(&FileLine[FileFieldPos[EVAP]], "%8d", &v) > 0 )
        {
            if ( abs(v) < 9999 )
            {
                x = (double)v * 0.1;
                if ( UnitSystem == US ) x /= MMperINCH;
                FileData[EVAP][d] = x;
            }
        }
    }

    // --- parse wind speed (in km/day for WDMV or tenths of m/s for AWND)
    //     to miles/hr
    if ( FileFieldPos[WIND] >= 0 )
    {
        if ( sscanf(&FileLine[FileFieldPos[WIND]], "%8d", &v) > 0 )
        {
            if ( abs(v) < 9999 )
            {
                if ( FileWindType == WDMV ) x = (double)v * 0.62137 / 24.;
                else x = (double)v * 0.1 / 1000. * 0.62137 * 3600.;
                FileData[WIND][d] = x;
            }
        }
    }
}

//=============================================================================

void updateTempMoveAve(double tmin, double tmax)
//
//  Input:   tmin = minimum daily temperature (deg F)
//           tmax = maximum daily temperature (deg F)
//  Output:  none
//  Purpose: updates moving averages of average daily temperature
//           and daily temperature range stored in structure Tma.
//
{
    double ta,               // new day's average temperature (deg F)
           tr;               // new day's temperature range (deg F)
    int    count = Tma.count;

    // --- find ta and tr from new day's min and max temperature
    ta = (tmin + tmax) / 2.0;
    tr = fabs(tmax - tmin);

    // --- if the array used to store previous days' temperatures is full
    if ( count == Tma.maxCount )
    {
        // --- update the moving averages with the new day's value
        Tma.tAve = (Tma.tAve * count + ta - Tma.ta[Tma.front]) / count;
        Tma.tRng = (Tma.tRng * count + tr - Tma.tr[Tma.front]) / count;

        // --- replace the values at the front of the moving average window
        Tma.ta[Tma.front] = ta;
        Tma.tr[Tma.front] = tr;

        // --- move the front one position forward
        Tma.front++;
        if ( Tma.front == count ) Tma.front = 0;
    }

    // --- array of previous day's values not full (at start of simulation)
    else
    {
        // --- find new moving averages by adding new values to previous ones
        Tma.tAve = (Tma.tAve * count + ta) / (count + 1);
        Tma.tRng = (Tma.tRng * count + tr) / (count + 1);

        // --- save new day's values
        Tma.ta[Tma.front] = ta;
        Tma.tr[Tma.front] = tr;

        // --- increment count and front of moving average window
        Tma.count++;
        Tma.front++;
        if ( Tma.count == Tma.maxCount ) Tma.front = 0;
    }
}
