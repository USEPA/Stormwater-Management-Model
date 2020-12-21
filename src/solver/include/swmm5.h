/** @file swmm5.h
 @see http://github.com/openwateranalytics/stormwater-management-model

 swmm5.h
 @brief Prototypes for SWMM5 functions exported to swmm5.dll.
 @date 03/24/14  (Build 5.1.001)
 @date 08/01/16  (Build 5.1.011)
 @version 5.1
 @authors L. Rossman, OpenWaterAnalytics members: 
 see <a href="https://github.com/OpenWaterAnalytics/Stormwater-Management-Model/blob/develop/AUTHORS">AUTHORS</a>.
*/

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

//#ifndef DLLEXPORT
#ifdef WINDOWS
	#ifdef __MINGW32__
		// Seems to be more wrapper friendly
		#define DLLEXPORT __declspec(dllexport) __cdecl
	#else
		#define DLLEXPORT __declspec(dllexport) __stdcall
	#endif
#else
	#define DLLEXPORT
#endif
//#endif


//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
static int IsOpenFlag;           // TRUE if a project has been opened
static int IsStartedFlag;        // TRUE if a simulation has been started
static int SaveResultsFlag;      // TRUE if output to be saved to binary file

int swmm_IsOpenFlag(void);
int swmm_IsStartedFlag(void);


// --- use "C" linkage for C++ programs
#ifdef __cplusplus
extern "C" {
#endif


/**
 @brief Opens SWMM input file, reads in network data, runs, and closes
 @param f1 pointer to name of input file (must exist)
 @param f2 pointer to name of report file (to be created)
 @param f3 pointer to name of binary output file (to be created)
 @return error code
*/
int DLLEXPORT swmm_run(const char *f1, const char *f2, const char *f3);

/**
 @brief Opens SWMM input file & reads in network data
 @param f1 pointer to name of input file (must exist)
 @param f2 pointer to name of report file (to be created)
 @param f3 pointer to name of binary output file (to be created)
 @return error code
*/
int DLLEXPORT swmm_open(const char *f1, const char *f2, const char *f3);

/**
 @brief Start SWMM simulation
 @param saveFlag TRUE or FALSE to save timeseries to report file
 @return error code
*/
int DLLEXPORT swmm_start(int saveFlag);

/**
 @brief Step SWMM simulation forward
 @param[out] elapsedTime elapsed simulation time [milliseconds]
 @return error code
*/
int DLLEXPORT swmm_step(double* elapsedTime);

/**
 @brief End SWMM simulation
 @return error code
*/
int DLLEXPORT swmm_end(void);

/**
 @brief Write text report file
 @return error code
*/
int DLLEXPORT swmm_report(void);

/**
 @brief Get routing errors
 @param[out] runoffErr Runoff routing error
 @param[out] flowErr Flow routing error
 @param[out] qualErr Quality routing error
 @return error code
*/
int DLLEXPORT swmm_getMassBalErr(float* runoffErr, float* flowErr, float* qualErr);

/**
 @brief Frees all memory and files used by SWMM
 @return Error code
*/
int DLLEXPORT swmm_close(void);

/**
 @brief Get Legacy SWMM version number
 @return Version
*/
int DLLEXPORT swmm_getVersion(void);


int DLLEXPORT swmm_getError(char* errMsg, int msgLen);

int DLLEXPORT swmm_getWarnings(void);


#ifdef __cplusplus
}   // matches the linkage specification from above */
#endif

#endif
