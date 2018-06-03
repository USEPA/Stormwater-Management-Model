/** @file swmm5.h
 @see http://github.com/openwateranalytics/stormwater-management-model
 
 swmm5.h
 @brief Prototypes for SWMM5 functions exported to swmm5.dll.
 @date 03/24/14  (Build 5.1.001)
 @date 08/01/16  (Build 5.1.011)
 @version 5.1
 @authors L. Rossman, OpenWaterAnalytics members: see <a href="https://github.com/OpenWaterAnalytics/Stormwater-Management-Model/blob/develop/AUTHORS">AUTHORS</a>.
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
int  DLLEXPORT   swmm_run(char* f1, char* f2, char* f3);

/**
 @brief Opens SWMM input file & reads in network data
 @param f1 pointer to name of input file (must exist)
 @param f2 pointer to name of report file (to be created)
 @param f3 pointer to name of binary output file (to be created)
 @return error code
*/
int  DLLEXPORT   swmm_open(char* f1, char* f2, char* f3);

/**
 @brief Start SWMM simulation
 @param saveFlag TRUE or FALSE to save timeseries to report file
 @return error code
*/
int  DLLEXPORT   swmm_start(int saveFlag);

/**
 @brief Step SWMM simulation forward
 @param[out] elapsedTime elapsed simulation time [milliseconds]
 @return error code
*/
int  DLLEXPORT   swmm_step(double* elapsedTime);

/**
 @brief End SWMM simulation
 @return error code
*/
int  DLLEXPORT   swmm_end(void);

/**
 @brief Write text report file
 @return error code
*/
int  DLLEXPORT   swmm_report(void);

/**
 @brief Get routing errors
 @param[out] runoffErr Runoff routing error
 @param[out] flowErr Flow routing error
 @param[out] qualErr Quality routing error
 @return error code
*/
int  DLLEXPORT   swmm_getMassBalErr(float* runoffErr, float* flowErr,
                 float* qualErr);

/**
 @brief Frees all memory and files used by SWMM
 @return Error code
*/
int  DLLEXPORT   swmm_close(void);

/**
 @brief Get Legacy SWMM version number
 @return Version
*/
int  DLLEXPORT   swmm_getVersion(void);

/**
 @brief Get full semantic version number
 @param[out] semver sematic version (char array)
*/
void DLLEXPORT   swmm_getSemVersion(char* semver);

/**
 @brief Get full semantic version number info
 @param[out] major sematic version major number
 @param[out] minor sematic version minor number
 @param[out] patch sematic version patch number
*/
void DLLEXPORT   swmm_getVersionInfo(char* major, char* minor, char* patch);

int  DLLEXPORT   swmm_getError(char* errMsg, int msgLen);                      //(5.1.011)
int  DLLEXPORT   swmm_getWarnings(void);                                       //(5.1.011)
int  swmm_IsOpenFlag(void);
int  swmm_IsStartedFlag(void);

#ifdef __cplusplus 
}   // matches the linkage specification from above */ 
#endif

#endif
