//-----------------------------------------------------------------------------
//   error.c
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    03/20/14  (Build 5.1.001)
//            03/19/15  (Build 5.1.008)
//            08/05/15  (Build 5.1.010)
//   Author:  L. Rossman
//
//   Error messages
//
//   Build 5.1.008:
//   - Text of Error 217 for control rules modified.
//
//   Build 5.1.010:
//   - Text of Error 318 for rainfall data files modified.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <string.h>
#include "error.h"

#define ERR101 "\n  ERROR 101: memory allocation error."
#define ERR103 "\n  ERROR 103: cannot solve KW equations for Link %s."
#define ERR105 "\n  ERROR 105: cannot open ODE solver."
#define ERR107 "\n  ERROR 107: cannot compute a valid time step."

#define ERR108 "\n  ERROR 108: ambiguous outlet ID name for Subcatchment %s."
#define ERR109 "\n  ERROR 109: invalid parameter values for Aquifer %s."
#define ERR110 \
"\n  ERROR 110: ground elevation is below water table for Subcatchment %s."

#define ERR111 "\n  ERROR 111: invalid length for Conduit %s."
#define ERR112 "\n  ERROR 112: elevation drop exceeds length for Conduit %s."
#define ERR113 "\n  ERROR 113: invalid roughness for Conduit %s."
#define ERR114 "\n  ERROR 114: invalid number of barrels for Conduit %s."
#define ERR115 "\n  ERROR 115: adverse slope for Conduit %s."
#define ERR117 "\n  ERROR 117: no cross section defined for Link %s."
#define ERR119 "\n  ERROR 119: invalid cross section for Link %s."
#define ERR121 \
"\n  ERROR 121: missing or invalid pump curve assigned to Pump %s."
#define ERR122 \
"\n  ERROR 122: startup depth not higher than shutoff depth for Pump %s."

#define ERR131 \
"\n  ERROR 131: the following links form cyclic loops in the drainage system:"
#define ERR133 "\n  ERROR 133: Node %s has more than one outlet link."
#define ERR134 "\n  ERROR 134: Node %s has illegal DUMMY link connections."

#define ERR135 "\n  ERROR 135: Divider %s does not have two outlet links."
#define ERR136 "\n  ERROR 136: Divider %s has invalid diversion link."
#define ERR137 "\n  ERROR 137: Weir Divider %s has invalid parameters."
#define ERR138 \
"\n  ERROR 138: Node %s has initial depth greater than maximum depth."
#define ERR139 "\n  ERROR 139: Regulator %s is the outlet of a non-storage node."
#define ERR141 \
"\n  ERROR 141: Outfall %s has more than 1 inlet link or an outlet link."
#define ERR143 "\n  ERROR 143: Regulator %s has invalid cross-section shape."
#define ERR145 "\n  ERROR 145: Drainage system has no acceptable outlet nodes."

#define ERR151 "\n  ERROR 151: a Unit Hydrograph in set %s has invalid time base."
#define ERR153 \
"\n  ERROR 153: a Unit Hydrograph in set %s has invalid response ratios."
#define ERR155 "\n  ERROR 155: invalid sewer area for RDII at node %s."

#define ERR156 "\n  ERROR 156: ambiguous station ID for Rain Gage %s."
#define ERR157 "\n  ERROR 157: inconsistent rainfall format for Rain Gage %s."
#define ERR158 \
"\n  ERROR 158: time series for Rain Gage %s is also used by another object."
#define ERR159 \
"\n  ERROR 159: recording interval greater than time series interval for Rain Gage %s."

#define ERR161 \
"\n  ERROR 161: cyclic dependency in treatment functions at node %s."

#define ERR171 "\n  ERROR 171: Curve %s has invalid or out of sequence data."
#define ERR173 "\n  ERROR 173: Time Series %s has its data out of sequence."

#define ERR181 "\n  ERROR 181: invalid Snow Melt Climatology parameters."
#define ERR182 "\n  ERROR 182: invalid parameters for Snow Pack %s."

#define ERR183 "\n  ERROR 183: no type specified for LID %s."
#define ERR184 "\n  ERROR 184: missing layer for LID %s."
#define ERR185 "\n  ERROR 185: invalid parameter value for LID %s."
#define ERR186 "\n  ERROR 186: invalid parameter value for LID placed in Subcatchment %s."
#define ERR187 "\n  ERROR 187: LID area exceeds total area for Subcatchment %s."
#define ERR188 \
"\n  ERROR 188: LID capture area exceeds total impervious area for Subcatchment %s."

#define ERR191 "\n  ERROR 191: simulation start date comes after ending date."
#define ERR193 "\n  ERROR 193: report start date comes after ending date."
#define ERR195 \
"\n  ERROR 195: reporting time step or duration is less than routing time step."

#define ERR200 "\n  ERROR 200: one or more errors in input file."
#define ERR201 "\n  ERROR 201: too many characters in input line "
#define ERR203 "\n  ERROR 203: too few items "
#define ERR205 "\n  ERROR 205: invalid keyword %s "
#define ERR207 "\n  ERROR 207: duplicate ID name %s "
#define ERR209 "\n  ERROR 209: undefined object %s "
#define ERR211 "\n  ERROR 211: invalid number %s "
#define ERR213 "\n  ERROR 213: invalid date/time %s "
#define ERR217 "\n  ERROR 217: control rule clause invalid or out of sequence "  //(5.1.008)
#define ERR219 "\n  ERROR 219: data provided for unidentified transect "
#define ERR221 "\n  ERROR 221: transect station out of sequence "
#define ERR223 "\n  ERROR 223: Transect %s has too few stations." 
#define ERR225 "\n  ERROR 225: Transect %s has too many stations."
#define ERR227 "\n  ERROR 227: Transect %s has no Manning's N."
#define ERR229 "\n  ERROR 229: Transect %s has invalid overbank locations."
#define ERR231 "\n  ERROR 231: Transect %s has no depth."
#define ERR233 "\n  ERROR 233: invalid treatment function expression "

#define ERR301 "\n  ERROR 301: files share same names."
#define ERR303 "\n  ERROR 303: cannot open input file."
#define ERR305 "\n  ERROR 305: cannot open report file."
#define ERR307 "\n  ERROR 307: cannot open binary results file."
#define ERR309 "\n  ERROR 309: error writing to binary results file."
#define ERR311 "\n  ERROR 311: error reading from binary results file."

#define ERR313 "\n  ERROR 313: cannot open scratch rainfall interface file."
#define ERR315 "\n  ERROR 315: cannot open rainfall interface file %s."
#define ERR317 "\n  ERROR 317: cannot open rainfall data file %s."
#define ERR318 \
"\n  ERROR 318: the following line is out of sequence in rainfall data file %s." //(5.1.010)
#define ERR319 "\n  ERROR 319: unknown format for rainfall data file %s."
#define ERR320 "\n  ERROR 320: invalid format for rainfall interface file."
#define ERR321 "\n  ERROR 321: no data in rainfall interface file for gage %s."

#define ERR323 "\n  ERROR 323: cannot open runoff interface file %s."
#define ERR325 \
"\n  ERROR 325: incompatible data found in runoff interface file."
#define ERR327 \
"\n  ERROR 327: attempting to read beyond end of runoff interface file."
#define ERR329 "\n  ERROR 329: error in reading from runoff interface file."

#define ERR330 "\n  ERROR 330: hotstart interface files have same names."
#define ERR331 "\n  ERROR 331: cannot open hotstart interface file %s."
#define ERR333 \
"\n  ERROR 333: incompatible data found in hotstart interface file."
#define ERR335 "\n  ERROR 335: error in reading from hotstart interface file."

#define ERR336 \
"\n  ERROR 336: no climate file specified for evaporation and/or wind speed."
#define ERR337 "\n  ERROR 337: cannot open climate file %s."
#define ERR338 "\n  ERROR 338: error in reading from climate file %s."
#define ERR339 \
"\n  ERROR 339: attempt to read beyond end of climate file %s."

#define ERR341 "\n  ERROR 341: cannot open scratch RDII interface file."
#define ERR343 "\n  ERROR 343: cannot open RDII interface file %s."
#define ERR345 "\n  ERROR 345: invalid format for RDII interface file."

#define ERR351 "\n  ERROR 351: cannot open routing interface file %s."
#define ERR353 "\n  ERROR 353: invalid format for routing interface file %s."
#define ERR355 "\n  ERROR 355: mis-matched names in routing interface file %s."
#define ERR357 "\n  ERROR 357: inflows and outflows interface files have same name."

#define ERR361 "\n  ERROR 361: could not open external file used for Time Series %s."
#define ERR363 "\n  ERROR 363: invalid data in external file used for Time Series %s."

#define ERR401 "\n  ERROR 401: general system error."
#define ERR402 \
"\n  ERROR 402: cannot open new project while current project still open."
#define ERR403 "\n  ERROR 403: project not open or last run not ended."
#define ERR405 \
"\n  ERROR 405: amount of output produced will exceed maximum file size;" \
"\n             either reduce Ending Date or increase Reporting Time Step."

// API Error Keys
#define ERR501 "\n API Key Error: Object Type Outside Bonds"
#define ERR502 "\n API Key Error: Network Not Initialized (Input file open?)"
#define ERR503 "\n API Key Error: Simulation Not Running"
#define ERR504 "\n API Key Error: Incorrect object type for parameter chosen"
#define ERR505 "\n API Key Error: Object index out of Bounds."
#define ERR506 "\n API Key Error: Invalid Pollutant Index"
#define ERR507 "\n API Key Error: Invalid Inflow Type"
#define ERR508 "\n API Key Error: Invalid Timeseries Index"
#define ERR509 "\n API Key Error: Invalid Pattern Index"

////////////////////////////////////////////////////////////////////////////
//  NOTE: Need to update ErrorMsgs[], ErrorCodes[], and ErrorType
//        (in error.h) whenever a new error message is added.
/////////////////////////////////////////////////////////////////////////////

char* ErrorMsgs[] =
    { "",     ERR101, ERR103, ERR105, ERR107, ERR108, ERR109, ERR110, ERR111,
      ERR112, ERR113, ERR114, ERR115, ERR117, ERR119, ERR121, ERR122, ERR131,
      ERR133, ERR134, ERR135, ERR136, ERR137, ERR138, ERR139, ERR141, ERR143,
      ERR145, ERR151, ERR153, ERR155, ERR156, ERR157, ERR158, ERR159, ERR161,
      ERR171, ERR173, ERR181, ERR182, ERR183, ERR184, ERR185, ERR186, ERR187,
      ERR188, ERR191, ERR193, ERR195, ERR200, ERR201, ERR203, ERR205, ERR207,
      ERR209, ERR211, ERR213, ERR217, ERR219, ERR221, ERR223, ERR225, ERR227,
      ERR229, ERR231, ERR233, ERR301, ERR303, ERR305, ERR307, ERR309, ERR311,
      ERR313, ERR315, ERR317, ERR318, ERR319, ERR320, ERR321, ERR323, ERR325,
      ERR327, ERR329, ERR330, ERR331, ERR333, ERR335, ERR336, ERR337, ERR338,
      ERR339, ERR341, ERR343, ERR345, ERR351, ERR353, ERR355, ERR357, ERR361,
      ERR363, ERR401, ERR402, ERR403, ERR405, ERR501, ERR502, ERR503, ERR504,
	  ERR505, ERR506, ERR507, ERR508, ERR509};

int ErrorCodes[] =
    { 0,      101,    103,    105,    107,    108,    109,    110,    111,
      112,    113,    114,    115,    117,    119,    121,    122,    131,
      133,    134,    135,    136,    137,    138,    139,    141,    143,
      145,    151,    153,    155,    156,    157,    158,    159,    161,
      171,    173,    181,    182,    183,    184,    185,    186,    187,
      188,    191,    193,    195,    200,    201,    203,    205,    207,
      209,    211,    213,    217,    219,    221,    223,    225,    227,
      229,    231,    233,    301,    303,    305,    307,    309,    311,
      313,    315,    317,    318,    319,    320,    321,    323,    325,
      327,    329,    330,    331,    333,    335,    336,    337,    338,
      339,    341,    343,    345,    351,    353,    355,    357,    361,
      363,    401,    402,    403,    405,    501,    502,    503,    504,
	  505,    506,    507,    508,    509};

char  ErrString[256];

char* error_getMsg(int i)
{
    if ( i >= 0 && i < MAXERRMSG ) return ErrorMsgs[i];
    else return ErrorMsgs[0];
};

int  error_getCode(int i)
{
    if ( i >= 0 && i < MAXERRMSG ) return ErrorCodes[i];
    else return 0;
}

int  error_setInpError(int errcode, char* s)
{
    strcpy(ErrString, s);
    return errcode;
}
