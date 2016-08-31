//-----------------------------------------------------------------------------
//   inputAPI.h
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    08/30/2016
//   Author:  B. McDonnell (EmNet LLC)
//
//   Exportable Functions for Project Definition API.
//
//-----------------------------------------------------------------------------


#ifdef WINDOWS
	#ifdef __MINGW32__
		#define DLLEXPORT __declspec(dllexport) __cdecl // <- More wrapper friendly
	#else
		#define DLLEXPORT __declspec(dllexport) __stdcall
	#endif
#else
	#define DLLEXPORT
#endif


#ifdef __cplusplus
extern "C" { 
#endif 

// Input API Exportable Functions
int DLLEXPORT  swmm_countObjects(int type, int *count);
int DLLEXPORT  swmm_getObjectId(int type, int index, char *id);
int DLLEXPORT  swmm_getNodeParam(int index, int Param, float *value);


#ifdef __cplusplus 
}   // matches the linkage specification from above */ 
#endif


