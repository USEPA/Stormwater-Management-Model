/*!
* \file swmm5.h
* \brief SWMM5 API Functions
* \author L. Rossman
* \author Caleb Buahin (US EPA ORD/CESER)(Last Editor)
* \date Created On: 11/01/2021 (Build 5.2.0)
* \date Last Update: 06/02/2023
* \note Prototypes for SWMM5 API functions.
*/
#ifndef SWMM5_H
#define SWMM5_H


#undef WINDOWS
#ifdef _WIN32
/*!
* \def WINDOWS
* \brief Windows OS
*/
#define WINDOWS
#endif
#ifdef __WIN32__
/*!
* \def WINDOWS
* \brief Windows OS 32-bit
*/
#define WINDOWS
#endif

// --- define DLLEXPORT

#ifdef WINDOWS
	/*!
	* \def DLLEXPORT
	* \brief Cross-platform DLL export/import specifier
	*/
#define DLLEXPORT __declspec(dllexport) __stdcall
#else
	/*!
	* \def DLLEXPORT
	* \brief Cross-platform DLL export/import specifier
	*/
#define DLLEXPORT
#endif

	// --- use "C" linkage for C++ programs

#ifdef __cplusplus
/*!
* \def extern "C"
* \brief C++ linkage for C programs
*/
extern "C" {
#endif

	/*!
	* \brief SWMM Object Types
	*/
	typedef enum {
		/*!
		* \brief Rain Gage
		*/
		swmm_GAGE = 0,

		/*!
		* \brief Subcatchment
		*/
		swmm_SUBCATCH = 1,

		/*!
		* \brief Node i.e., Junctions, Outfalls, Storages, Dividers
		*/
		swmm_NODE = 2,

		/*!
		* \brief Link i.e., Conduits, Pumps, Orifices, Weirs, Outlets
		*/
		swmm_LINK = 3,

		/*!
		* \brief  System Variables
		*/
		swmm_SYSTEM = 100
	} swmm_Object;

	/*!
	* \brief SWMM Node Types
	*/
	typedef enum {
		/*!
		* \brief Junction
		*/
		swmm_JUNCTION = 0,
		/*!
		* \brief Outfall
		*/
		swmm_OUTFALL = 1,
		/*!
		* \brief Storage Unit
		*/
		swmm_STORAGE = 2,
		/*!
		* \brief Divider
		*/
		swmm_DIVIDER = 3
	} swmm_NodeType;

	/*!
	* \brief SWMM Link Types
	*/
	typedef enum {
		/*!
		* \brief Conduit
		*/
		swmm_CONDUIT = 0,
		/*!
		* \brief Pump
		*/
		swmm_PUMP = 1,

		/*!
		* \brief Orifice
		*/
		swmm_ORIFICE = 2,

		/*!
		* \brief Weir
		*/
		swmm_WEIR = 3,

		/*!
		* \brief Outlet
		*/
		swmm_OUTLET = 4
	} swmm_LinkType;

    /*!
	* \brief Rainfall Gage
	*/
	typedef enum {
		/*!
		* \brief Rainfall Gage
		*/
		swmm_GAGE_RAINFALL = 100
	} swmm_GageProperty;

	/*!
	* \brief Subcatchment
	*/
	typedef enum {
        /*!
		* \brief Subcatchment Area
		*/
		swmm_SUBCATCH_AREA = 200,
        /*!
		* \brief Subcatchment Rain Gage
		*/
		swmm_SUBCATCH_RAINGAGE = 201,
        /*!
		* \brief Subcatchment Rainfall
		*/
		swmm_SUBCATCH_RAINFALL = 202,
		/*!
		* \brief Subcatchment Evaporation
		*/
		swmm_SUBCATCH_EVAP = 203,
		/*!
		* \brief Subcatchment Infiltration
		*/
		swmm_SUBCATCH_INFIL = 204,
		/*!
		* \brief Subcatchment Runoff
		*/
		swmm_SUBCATCH_RUNOFF = 205,
		/*!
		* \brief Subcatchment Report Flag
		*/
		swmm_SUBCATCH_RPTFLAG = 206
	} swmm_SubcatchProperty;

	/*!
	* \brief Node
	*/
	typedef enum {
		/*!
		* \brief Node Type
		*/
		swmm_NODE_TYPE = 300,
		/*!
		* \brief Node Invert Elevation
		*/
		swmm_NODE_ELEV = 301,
		/*!
		* \brief Node Maximum Depth
		*/
		swmm_NODE_MAXDEPTH = 302,
		/*!
		* \brief Node Depth
		*/
		swmm_NODE_DEPTH = 303,
		/*!
		* \brief Node Hydraulic Head
		*/
		swmm_NODE_HEAD = 304,
		/*!
		* \brief Node Volume
		*/
		swmm_NODE_VOLUME = 305,
		/*!
		* \brief Node Lateral Inflow
		*/
		swmm_NODE_LATFLOW = 306,
        /*!
		* \brief Node Total Inflow
		*/
		swmm_NODE_INFLOW = 307,
		/*!
		* \brief Node Overflow
		*/
		swmm_NODE_OVERFLOW = 308,
		/*!
		* \brief Node Report Flag
		*/
		swmm_NODE_RPTFLAG = 309
	} swmm_NodeProperty;

    /*!
	* \brief Link
	*/
	typedef enum {
		/*!
		* \brief Link Type
		*/
		swmm_LINK_TYPE = 400,
		/*!
		* \brief Link Inlet Node
		*/
		swmm_LINK_NODE1 = 401,
		/*!
		* \brief Link Outlet Node
		*/
		swmm_LINK_NODE2 = 402,
		/*!
		* \brief Link Length
		*/
		swmm_LINK_LENGTH = 403,
		/*!
		* \brief Link Slope
		*/
		swmm_LINK_SLOPE = 404,
		/*!
		* \brief Link Full Depth
		*/
		swmm_LINK_FULLDEPTH = 405,
		/*!
		* \brief Link Full Flow
		*/
		swmm_LINK_FULLFLOW = 406,
		/*!
		* \brief Link Setting
		*/
		swmm_LINK_SETTING = 407,
		/*!
		* \brief Link Time Open
		*/
		swmm_LINK_TIMEOPEN = 408,
		/*!
		* \brief Link Time Closed
		*/
		swmm_LINK_TIMECLOSED = 409,
		/*!
		* \brief Link Flow
		*/
		swmm_LINK_FLOW = 410,
		/*!
		* \brief Link Depth
		*/
		swmm_LINK_DEPTH = 411,
		/*!
		* \brief Link Velocity
		*/
		swmm_LINK_VELOCITY = 412,
		/*!
		* \brief Link Top Width
		*/
		swmm_LINK_TOPWIDTH = 413,
		/*!
		* \brief Link Report Flag
		*/
		swmm_LINK_RPTFLAG = 414
	} swmm_LinkProperty;

	/*!
	* \brief System Properties
	*/
	typedef enum {
		/*!
		* \brief Simulation Start Date
		*/
		swmm_STARTDATE = 0,
		/*!
		* \brief Current Simulation Date
		*/
		swmm_CURRENTDATE = 1,
		/*!
		* \brief Elapsed Simulation Time
		*/
		swmm_ELAPSEDTIME = 2,
		/*!
		* \brief Routing Step
		*/
		swmm_ROUTESTEP = 3,
		/*!
		* \brief Maximum Routing Step
		*/
		swmm_MAXROUTESTEP = 4,
		/*!
		* \brief Reporting Step
		*/
		swmm_REPORTSTEP = 5,
		/*!
		* \brief Total Number of Steps
		*/
		swmm_TOTALSTEPS = 6,
		/*!
		* \brief Number of Reporting Steps
		*/
		swmm_NOREPORT = 7,
		/*!
		* \brief Flow Units
		*/
		swmm_FLOWUNITS = 8
	} swmm_SystemProperty;

	/*!
	* \brief Flow Units
	*/
	typedef enum {
		/*!
		* \brief Cubic Feet per Second
		*/
		swmm_CFS = 0,
		/*!
		* \brief Gallons per Minute
		*/
		swmm_GPM = 1,
		/*!
		* \brief Million Gallons per Day
		*/
		swmm_MGD = 2,
		/*!
		* \brief Cubic Meters per Second
		*/
		swmm_CMS = 3,
		/*!
		* \brief Liters per Second
		*/
		swmm_LPS = 4,  // liters per second

		/*!
		* \brief Million Liters per Day
		*/
		swmm_MLD = 5   // million liters per day
	} swmm_FlowUnitsProperty;


	/*!
	* \brief Run SWMM model using the specified input files
	* \param f1 Name of the input file
	* \param f2 Name of the report file
	* \param f3 Name of the output file
	* \return Error code
	*/
	int    DLLEXPORT swmm_run(const char* f1, const char* f2, const char* f3);

	/*!
	* \brief Open SWMM input file & read in network data
	* \param f1 Name of the input file
	* \param f2 Name of the report file
	* \param f3 Name of the output file
	* \return Error code
	*/
	int    DLLEXPORT swmm_open(const char* f1, const char* f2, const char* f3);

	/*!
	* \brief Start SWMM simulation
	* \param saveFlag Save results to binary file flag
	* \return Error code
	*/
	int    DLLEXPORT swmm_start(int saveFlag);

	/*!
	* \brief Step SWMM simulation
	* \param[out] elapsedTime Time to advance simulation in seconds
	* \return Error code
	*/
	int    DLLEXPORT swmm_step(double* elapsedTime);

	/*!
	* \brief Run SWMM simulation over specified stride steps 
	* \param[in] strideStep Number of steps to advance simulation
	* \param[out] elapsedTime Time to advanced
	* \return Error code
	*/
	int    DLLEXPORT swmm_stride(int strideStep, double* elapsedTime);

	/*!
	* \brief End SWMM simulation and free all allocated memory
	* \return Error code
	*/
	int    DLLEXPORT swmm_end(void);

	/*!
	* \brief Write report file
	* \return Error code
	*/
	int    DLLEXPORT swmm_report(void);

	/*!
	* \brief Close SWMM input and output files and delete all allocated memory
	* \return Error code
	*/
	int    DLLEXPORT swmm_close(void);

	int    DLLEXPORT swmm_getMassBalErr(float* runoffErr, float* flowErr, float* qualErr);
	int    DLLEXPORT swmm_getVersion(void);
	int    DLLEXPORT swmm_getError(char* errMsg, int msgLen);
	int    DLLEXPORT swmm_getWarnings(void);

	int    DLLEXPORT swmm_getCount(int objType);
	void   DLLEXPORT swmm_getName(int objType, int index, char* name, int size);
	int    DLLEXPORT swmm_getIndex(int objType, const char* name);
	double DLLEXPORT swmm_getValue(int property, int index);
	void   DLLEXPORT swmm_setValue(int property, int index, double value);
	double DLLEXPORT swmm_getSavedValue(int property, int index, int period);
	void   DLLEXPORT swmm_writeLine(const char* line);
	void   DLLEXPORT swmm_decodeDate(double date, int* year, int* month, int* day,
		int* hour, int* minute, int* second, int* dayOfWeek);

#ifdef __cplusplus 
}   // matches the linkage specification from above */ 
#endif

#endif //SWMM5_H
