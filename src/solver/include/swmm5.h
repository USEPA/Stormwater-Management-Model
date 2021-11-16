//-----------------------------------------------------------------------------
//   swmm5.h
//
//   Project: EPA SWMM5
//   Version: 5.2
//   Date:    11/01/21  (Build 5.2.0)
//   Author:  L. Rossman
//
//   Prototypes for SWMM5 API functions.
//
//-----------------------------------------------------------------------------

#ifndef SWMM5_H
#define SWMM5_H

// --- define WINDOWS

#undef WINDOWS
#ifdef _WIN32
  #define WINDOWS
#endif
#ifdef __WIN32__
  #define WINDOWS
#endif

// --- define DLLEXPORT

#ifdef WINDOWS
    #define DLLEXPORT __declspec(dllexport) __stdcall
#else
    #define DLLEXPORT
#endif

// --- use "C" linkage for C++ programs

#ifdef __cplusplus
extern "C" { 
#endif

typedef enum {
    swmm_GAGE     = 0,
    swmm_SUBCATCH = 1,
    swmm_NODE     = 2,
    swmm_LINK     = 3,
    swmm_SYSTEM   = 100
} swmm_Object;

typedef enum {
    swmm_JUNCTION = 0,
    swmm_OUTFALL  = 1,
    swmm_STORAGE  = 2,
    swmm_DIVIDER  = 3
} swmm_NodeType;

typedef enum {
    swmm_CONDUIT = 0,
    swmm_PUMP    = 1,
    swmm_ORIFICE = 2,
    swmm_WEIR    = 3,
    swmm_OUTLET  = 4
} swmm_LinkType;

typedef enum {
    swmm_GAGE_RAINFALL = 0
} swmm_GageProperty;

typedef enum {
    swmm_SUBCATCH_AREA      = 0,
    swmm_SUBCATCH_RAINGAGE  = 1,
    swmm_SUBCATCH_RAINFALL  = 2,
    swmm_SUBCATCH_EVAP      = 3,
    swmm_SUBCATCH_INFIL     = 4,
    swmm_SUBCATCH_RUNOFF    = 5
} swmm_SubcatchProperty;

typedef enum {
    swmm_NODE_TYPE     = 0,
    swmm_NODE_ELEV     = 1,
    swmm_NODE_MAXDEPTH = 2,
    swmm_NODE_DEPTH    = 3,
    swmm_NODE_HEAD     = 4,
    swmm_NODE_VOLUME   = 5,
    swmm_NODE_LATFLOW  = 6,
    swmm_NODE_INFLOW   = 7,
    swmm_NODE_OVERFLOW = 8
} swmm_NodeProperty;

typedef enum {
    swmm_LINK_TYPE       = 0,
    swmm_LINK_NODE1      = 1,
    swmm_LINK_NODE2      = 2,
    swmm_LINK_LENGTH     = 3,
    swmm_LINK_SLOPE      = 4,
    swmm_LINK_FULLDEPTH  = 5,
    swmm_LINK_FULLFLOW   = 6,
    swmm_LINK_SETTING    = 7,
    swmm_LINK_TIMEOPEN   = 8,
    swmm_LINK_TIMECLOSED = 9,
    swmm_LINK_FLOW       = 10,
    swmm_LINK_DEPTH      = 11,
    swmm_LINK_VELOCITY   = 12,
    swmm_LINK_TOPWIDTH   = 13
} swmm_LinkProperty;

typedef enum {
    swmm_STARTDATE    = 0,
    swmm_CURRENTDATE  = 1,
    swmm_ELAPSEDTIME  = 2,
    swmm_ROUTESTEP    = 3,
    swmm_MAXROUTESTEP = 4,
    swmm_REPORTSTEP   = 5,
    swmm_TOTALSTEPS   = 6,
    swmm_NOREPORT     = 7
} swmm_SystemProperty;

int    DLLEXPORT swmm_run(const char *f1, const char *f2, const char *f3);
int    DLLEXPORT swmm_open(const char *f1, const char *f2, const char *f3);
int    DLLEXPORT swmm_start(int saveFlag);
int    DLLEXPORT swmm_step(double *elapsedTime);
int    DLLEXPORT swmm_stride(int strideStep, double *elapsedTime);
int    DLLEXPORT swmm_end(void);
int    DLLEXPORT swmm_report(void);
int    DLLEXPORT swmm_close(void);

int    DLLEXPORT swmm_getMassBalErr(float *runoffErr, float *flowErr, float *qualErr);
int    DLLEXPORT swmm_getVersion(void);
int    DLLEXPORT swmm_getError(char *errMsg, int msgLen);
int    DLLEXPORT swmm_getWarnings(void);

int    DLLEXPORT swmm_getCount(int objType);
void   DLLEXPORT swmm_getName(int objType, int index, char *name, int size);
int    DLLEXPORT swmm_getIndex(int objType, const char *name);
double DLLEXPORT swmm_getValue(int objType, int index, int property);
void   DLLEXPORT swmm_setValue(int objType, int index, int property, double value);
double DLLEXPORT swmm_getSavedValue(int objType, int index, int property, int period);
void   DLLEXPORT swmm_writeLine(const char *line);
void   DLLEXPORT swmm_decodeDate(double date, int *year, int *month, int *day,
                 int *hour, int *minute, int *second, int *dayOfWeek);

#ifdef __cplusplus 
}   // matches the linkage specification from above */ 
#endif

#endif //SWMM5_H
