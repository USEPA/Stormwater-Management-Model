/*!
 *  \file funcs.h
 *  \author Michael Tryby (US EPA - ORD/CESER)
 *  \author Lew Rossman (Editor)
 *  \author Corinne Wiesner-Friedman (US EPA - ORD/CESER) (Editor) 
 *  \date Created On: 10/18/2019
 *  \date Last Edited: 05/14/2024
 *  \brief Global interfacing functions.
 */

//-----------------------------------------------------------------------------
//   funcs.h
//
//   Project:  EPA SWMM5
//   Version:  5.2
//   Date:     06/12/23 (Build 5.2.4)
//   Author:   L. Rossman
//             M. Tryby (EPA)
//
//   Global interfacing functions.
//
//   Update History
//   ==============
//   Build 5.1.007:
//   - climate_readAdjustments() added.
//   Build 5.1.008:
//   - Function list was re-ordered and blank lines added for readability.
//   - Pollutant buildup/washoff functions for the new surfqual.c module added.
//   - Several other functions added, re-named or have modified arguments.
//   Build 5.1.010:
//   - New roadway_getInflow() function added.
//   Build 5.1.013:
//   - Additional arguments added to function stats_updateSubcatchStats.
//   Build 5.1.014:
//   - Arguments to link_getLossRate function changed.
//   Build 5.2.0:
//   - Support added for Streets and Inlets.
//   - Support added for reporting most frequent non-converging links.
//   - Support added for named variables & math expressions in control rules.
//   - Support added for tracking a gage's prior n-hour rainfall total.
//   - Refactored external inflow code.
//   Build 5.2.4:
//   - Additional arguments added to function link_getLossRate.
//-----------------------------------------------------------------------------

#ifndef FUNCS_H
#define FUNCS_H

/*!
 * \brief Project Methods
 */
 /*! \brief opens a new SWMM project
 \param f1 pointer to name of input file
 \param f2 pointer to name of report file
 \param f3 pointer to name of binary output file 
 */
void     project_open(const char *f1, const char *f2, const char *f3);
 /*! \brief closes a SWMM project*/
void     project_close(void);
 /*! \brief retrieves project data from an input file*/
void     project_readInput(void);
 /*! \brief reads a project option from a pair of string tokens
        \param s1 pointer to first string token
        \param s2 pointer to second string token
 */
int      project_readOption(char* s1, char* s2);
 /*! \brief validates data*/
void     project_validate(void);
/*! \brief initializes state of each major system component*/
int      project_init(void);
/*! \brief adds an object ID to a hash table
        \param type type of object
        \param id pointer to object ID
        \param n index of object
 */        
int      project_addObject(int type, char* id, int n);
/*! \brief finds an object's index in the hash table
        \param type type of object
        \param id pointer to object ID
 */
int      project_findObject(int type, const char* id);
/*! \brief finds an object's ID in the hash table
        \param type type of object
        \param id pointer to object ID
 */
char*    project_findID(int type, char* id);
/*! \brief creates a matrix of doubles
        \param nrows number of rows
        \param ncols number of columns
 */
double** project_createMatrix(int nrows, int ncols);
/*! \brief frees a matrix of doubles
        \param m pointer to matrix
 */
void     project_freeMatrix(double** m);

/*!
 * \brief Input Reader Methods
 */
int     input_countObjects(void);
int     input_readData(void);

/*!
 * \brief Report Writer Methods
 */
/*! \brief reads reporting options from a line of input
        \param tok array of string tokens
        \param ntoks number of tokens 
*/
int     report_readOptions(char* tok[], int ntoks);
/*! \brief writes a line of text to the report file
        \param line pointer to line of text
 */
void    report_writeLine(const char* line);
/*! \brief writes starting/ending processing times to report file.
 */
void    report_writeSysTime(void);
/*! \brief writes project title to report file.
 */
void    report_writeLogo(void);
/*! \brief writes title to report file.
 */
void    report_writeTitle(void);
/*! \brief writes project options to report file.
 */
void    report_writeOptions(void);
/*! \brief writes project dimensions to report file.
 */
void    report_writeReport(void);
/*! \brief writes summary of rain data read from file to report file
        \param gage rain gage index
        \param rainStats pointer to rain stats
 */
void    report_writeRainStats(int gage, TRainStats* rainStats);
/*! \brief writes summary of RDII inflow to report file
        \param totalRain total rainfall volume over sewershed
        \param totalRdii total RDII volume produced

*/
void    report_writeRdiiStats(double totalRain, double totalRdii);
/*! \brief write control actions heading
*/
void    report_writeControlActionsHeading(void);
/*! \brief reports action taken by a control rule
        \param aDate date/time of rule action
        \param linkID ID of link being controlled
        \param value new status value of link
        \param ruleID ID of rule implementing the action
*/
void    report_writeControlAction(DateTime aDate, char* linkID, double value,
        char* ruleID);
/*! \brief writes runoff continuity error to report file.
        \param totals accumulated runoff totals
        \param area total area of all subcatchments
*/
void    report_writeRunoffError(TRunoffTotals* totals, double area);
/*! \brief writes runoff loading continuity error to report file.
        \param totals accumulated pollutant loading totals
        \param area total area of all subcatchments
*/
void    report_writeLoadingError(TLoadingTotals* totals);
/*! \brief writes groundwater continuity error to report file.
        \param totals accumulated groundwater totals
        \param area total area of all subcatchments
*/
void    report_writeGwaterError(TGwaterTotals* totals, double area);
/*! \brief writes flow routing continuity error to report file
        \param totals accumulated flow routing totals
*/
void    report_writeFlowError(TRoutingTotals* totals);
/*! \brief writes quality routing continuity error to report file
        \param totals accumulated quality routing totals
*/
void    report_writeQualError(TRoutingTotals* totals);
/*! \brief lists nodes & links with highest mass balance errors and time Courant time step critical
        \remark In report.c there is an additional input listed as follows: maxLinkTimes[] = links most often Courant time step critical
        \param massBalErrs nodes with the highest mass balance errors
        \param CourantCrit nodes most often Courant time step critical
        \param nMaxStats number of most critical nodes/links saved
*/
void    report_writeMaxStats(TMaxStats massBalErrs[], TMaxStats CourantCrit[],
        int nMaxStats);
/*! \brief lists links with highest number of flow turns (i.e., fraction of time periods where the flow is higher (or lower) than the flows in the previous and following periods).
        \param flowTurns links with the highest number of flow turns
        \param nMaxStats = number of links in flowTurns[]
*/
void    report_writeMaxFlowTurns(TMaxStats flowTurns[], int nMaxStats);
/*! \brief lists the nodes with highest number of non-converging time steps
        \param maxNonconverged list of nodes with highest number of non-converging time steps
        \param nMaxStats number of nodes in maxNonconverged[]

*/
void    report_writeNonconvergedStats(TMaxStats maxNonconverged[],
        int nMaxStats);
/*! \brief writes routing time step statistics to report file
        \param timeStepStats writing time step statistics
*/
void    report_writeTimeStepStats(TTimeStepStats* timeStepStats);
/*! \brief writes error message to report file
        \param code error code
        \param msg error message
*/
void    report_writeErrorMsg(int code, char* msg);
/*! \brief writes error message to report file
*/
void    report_writeErrorCode(void);
/*!    \brief writes input error message to report file
        \param k error code
        \param sect number of input data section where error occurred
        \param line line of data containing the error
        \param lineCount line number of data file where error occurred
*/
void    report_writeInputErrorMsg(int k, int sect, char* line, long lineCount);
/*! \brief writes warning message to report file
        \param msg warning message
        \param id ID of object causing warning
*/
void    report_writeWarningMsg(char* msg, char* id); 
/*! \brief writes the date where a times series data is out of order
        \param code error code
        \param tseries pointer to time series data
*/
void    report_writeTseriesErrorMsg(int code, TTable *tseries);
/*! \brief write input summary & project options to report file if requested
*/
void    inputrpt_writeInput(void);
/*! \brief writes summary statistics to report file
*/
void    statsrpt_writeReport(void);

/*!
 * \brief Temperature/Evaporation Methods
 */

/*!
* \brief reads climate/temperature parameters from input line of data
        \param tok array of string tokens
        \param ntoks number of tokens
*/
int      climate_readParams(char* tok[], int ntoks);
/*! \brief reads evaporation parameters from input line of data
        \param tok array of string tokens
        \param ntoks number of tokens
*/
int      climate_readEvapParams(char* tok[], int ntoks);
/* \brief reads climate adjustment parameters from input line of data
        \param tok array of string tokens
        \param ntoks number of tokens
*/
int      climate_readAdjustments(char* tok[], int ntoks);
/*! \brief validates climatological variables
*/
void     climate_validate(void);
/*! \brief opens a climate file and reads in first set of values.
*/
void     climate_openFile(void);
/*! \brief initializes climate state variables
*/
void     climate_initState(void);
/*! \brief sets climate variables for current date.
        \param aDate simulate date
*/
void     climate_setState(DateTime aDate);
/*! \brief gets the next date when evaporation rate changes.
*/
DateTime climate_getNextEvapDate(void);

/*!
 * \brief Rainfall Processing Methods
 */
/*! \brief opens binary rain interface file and RDII processor.
*/
void    rain_open(void);
/*! \brief closes rain interface file and RDII processor.
*/
void    rain_close(void);

/*!
 * \brief Snowmelt Processing Methods
 */
/*! \brief reads snow melt parameters from a tokenized line of input data.
* \param tok array of string tokens
* \param ntoks number of tokens
* \return returns error code
* \remark Format of data are: Name  SubArea   Cmin  Cmax  Tbase  FWF  SD0  FW0  SNN0/SD100, Name  REMOVAL   SDplow Fout Fimperv Fperv Fimelt Fsubcatch (Subcatch)
*/
int     snow_readMeltParams(char* tok[], int ntoks);
/*! \brief creates a snowpack object for a subcatchment.
* \param subcatch = subcatchment index
* \param snowIndex = snow melt parameter set index
* \return returns TRUE if successful
*/
int     snow_createSnowpack(int subcatch, int snowIndex);
/*! \brief checks for valid values in a snow melt parameter set.
* \param snowIndex = snow melt parameter set index
*/
void    snow_validateSnowmelt(int snowIndex);
/*! \brief initializes a snowpack object for a subcatchment.
* \param subcatch = subcatchment index
*/
void    snow_initSnowpack(int subcatch);
/*! \brief initializes values in a snow melt parameter set.
* \param snowIndex = snow melt parameter set index
*/
void    snow_initSnowmelt(int snowIndex);
/*! \brief retrieves the current state of a snow pack object.
* \param subcatch = subcatchment index
* \param subArea = subarea index
* \param x = array of snow pack state variables
* \return updated array of snow pack state variables x
*/
void    snow_getState(int subcatch, int subArea, double x[]);
/*! \brief sets the current state of a snow pack object.
* \param subcatch = subcatchment index
* \param subArea = subarea index
* \param x = array of snow pack state variables
*/
void    snow_setState(int subcatch, int subArea, double x[]);
/*! \brief sets values of snow melt coeffs. for particular time of year.
* \param snowIndex = snow melt parameter set index
* \param season = current season
*/
void    snow_setMeltCoeffs(int snowIndex, double season);
/*! \brief adds new snow to subcatchment and plows it between sub-areas.
* \param subcatch = subcatchment index
* \param tStep = time step (seconds)
*/
void    snow_plowSnow(int subcatch, double tStep);
/*! \brief modifies rainfall input to subcatchment's sub-areas based on
*        possible snow melt and updates snow depth over entire subcatchment.
* \param subcatch = subcatchment index
* \param rainfall = rainfall rate (ft/sec)
* \param snowfall = snowfall rate (ft/sec)
* \param tStep = time step (seconds)
* \param netPrecip = array of net precipitation rates
* \return netPrecip = rainfall + snowmelt on each runoff sub-area (ft/sec),
*         returns new snow depth over subcatchment
*/
double  snow_getSnowMelt(int subcatch, double rainfall, double snowfall,
        double tStep, double netPrecip[]);
/*! \brief computes volume of snow on a subcatchment.
* \param subcatch = subcatchment index
* \return volume of snow on subcatchment (ft3)
*/
double  snow_getSnowCover(int subcatch);

/*!
 * \brief Runoff Analyzer Methods
 */
/*! \brief open runoff processer*/
int     runoff_open(void);
/*! \brief execute runoff processer*/
void    runoff_execute(void);
/*! \brief close runoff processer*/
void    runoff_close(void);

/*!
 * \brief Conveyance System Routing Methods
 */
/*! \brief open routing processer*/
int     routing_open(void);
/*! \brief determines time step used for flow routing at current time period.
* \param routingModel routing method code
* \param fixedStep fixed time step
* \return returns a routing time step (sec)
*/
double  routing_getRoutingStep(int routingModel, double fixedStep);
/*! \brief executes the routing process at the current time period.
* \param routingModel routing method code
* \param routingStep routing time step (sec)
*/
void    routing_execute(int routingModel, double routingStep);
/*! \brief close routing processer
* \param routingModel routing method code
*/
void    routing_close(int routingModel);

/*!
 * \brief Output Filer Methods
 */
/*! \brief open binary output file*/
int     output_open(void);
/*! \brief  write ending records to binary output file*/
void    output_end(void);
/*! \brief close binary output file*/
void    output_close(void);
/*! \brief save current average results to binary file
* \param reportTime time of simulation run
* \remark  after this function is called, averages will be reset to 0
*/
void    output_saveResults(double reportTime);
/*! \brief update current average results
*/
void    output_updateAvgResults(void);
/*! \brief retrieves the date/time for a specific reporting period from the binary output file.
* \param period id of reporting time period
* \param aDate date/time value
*/
void    output_readDateTime(long period, DateTime *aDate);
/*! \brief reads computed results for a subcatchment at a specific time period.
* \param period index of reporting time period
* \param index subcatchment index in binary output file
*/
void    output_readSubcatchResults(long period, int index);
/*! \brief reads computed results for a node at a specific time period.
* \param long index of reporting time period
* \param index node index in binary output file
*/
void    output_readNodeResults(int long, int index);
/*! \brief reads computed results for a link at a specific time period.
* \param long index of reporting time period
* \param index link index in binary output file
*/
void    output_readLinkResults(int long, int index);

/*!
 * \brief Groundwater Methods
 */
/*! \brief reads groundwater parameters from input line of data
* \param aquifer index of aquifer
* \param tok array of string tokens
* \param ntoks number of tokens
* \return returns error code
*/
int     gwater_readAquiferParams(int aquifer, char* tok[], int ntoks);
/*! \brief reads groundwater flow parameters from input line of data
* \param tok array of string tokens
* \param ntoks number of tokens
* \return returns error code
*/
int     gwater_readGroundwaterParams(char* tok[], int ntoks);
/*! \brief reads mathematical expression for lateral or deep groundwater flow for a subcatchment from a line of input data.
* \param tok array of string tokens
* \param ntoks number of tokens
* \return returns error code
*/
int     gwater_readFlowExpression(char* tok[], int ntoks);
/*! \brief deletes a subcatchment's custom groundwater flow expressions.
* \param subcatch subcatchment index
*/
void    gwater_deleteFlowExpression(int subcatch);
/*! \brief validates groundwater aquifer properties
* \param aquifer index of aquifer
*/
void    gwater_validateAquifer(int aquifer);
/*! \brief validates groundwater subcatchment properties
* \param subcatch subcatchment index
* \remark uses aquifer values for missing groundwater parameters
* \remark doesn't allow ground elevation to be below water table elevation
*/
void    gwater_validate(int subcatch);
/*! \brief initializes groundwater state variables
* \param subcatch subcatchment index
*/
void    gwater_initState(int subcatch);
/*! \brief retrieves state of subcatchment's groundwater.
* \param subcatch subcatchment index
* \param x array of groundwater state variables
*/
void    gwater_getState(int subcatch, double x[]);
/*! \brief sets state of subcatchment's groundwater.
* \param subcatch subcatchment index
* \param x array of groundwater state variables
*/
void    gwater_setState(int subcatch, double x[]);
/*! \brief computes groundwater flow between a subcatchment and its aquifer.
* \param subcatch subcatchment index
* \param evap evaporation rate (ft/sec)
* \param infil infiltration rate (ft/sec)
* \param tStep time step (sec)
*/
void    gwater_getGroundwater(int subcatch, double evap, double infil,
        double tStep);
/*! \brief finds volume of groundwater stored in upper & lower zones
* \param subcatch subcatchment index
*/
double  gwater_getVolume(int subcatch);

/*!
 * \brief RDII Methods
 */
/*! \brief reads RDII parameters from input line of data
* \param tok array of string tokens
* \param ntoks number of tokens
* \return returns error code
*/
int     rdii_readRdiiInflow(char* tok[], int ntoks);
/*! \brief deletes RDII inflow data for an object
* \param node node index
*/
void    rdii_deleteRdiiInflow(int node);
/*! \brief initializes RDII unit hydrograph
* \param unitHyd unit hydrograph index
*/
void    rdii_initUnitHyd(int unitHyd);
/*! \brief reads RDII unit hydrograph parameters from input line of data
* \param tok array of string tokens
* \param ntoks number of tokens
* \return returns error code
*/
int     rdii_readUnitHydParams(char* tok[], int ntoks);
/*! \brief  opens an exisiting RDII interface file or creates a new one.
*/
void    rdii_openRdii(void);
/*! \brief  closes rain interface file and RDII processor
*/
void    rdii_closeRdii(void);
/*! \brief finds number of RDII inflows at a specified date.
* \param aDate date/time value
* \return returns 0 if no RDII flow or number of nodes with RDII inflows
*/
int     rdii_getNumRdiiFlows(DateTime aDate);
/*! \brief finds index and current RDII inflow for an RDII node.
* \param index index of RDII inflow
* \param node project node index
* \param q RDII flow rate
*/
void    rdii_getRdiiFlow(int index, int* node, double* q);

/*!
 * \brief Landuse Methods
 */
/*! \brief reads landuse parameters from a tokenized line of input.
* \param landuse land use index
* \param tok array of string tokens
* \param ntoks number of tokens
* \return returns error code
* \remark Format of data are: landuseID  (sweepInterval sweepRemoval sweepDays0)
*/
int     landuse_readParams(int landuse, char* tok[], int ntoks);
/*! \brief reads pollutant parameters from a tokenized line of input.
* \param pollut pollutant index
* \param tok array of string tokens
* \param ntoks number of tokens
* \return returns error code
* \remark Format of data are: ID Units cRain cGW cRDII kDecay (snowOnly coPollut coFrac cDWF cInit)
*/
int     landuse_readPollutParams(int pollut, char* tok[], int ntoks);
/*! \brief reads buildup parameters from a tokenized line of input.
* \param tok array of string tokens
* \param ntoks number of tokens
* \return returns error code
* \remark Format of data are: landuseID  pollutID  buildupType  c1  c2  c3  normalizerType
*/
int     landuse_readBuildupParams(char* tok[], int ntoks);
/*! \brief reads washoff parameters from a tokenized line of input.
* \param tok array of string tokens
* \param ntoks number of tokens
* \return returns error code
* \remark Format of data are: landuseID  pollutID  washoffType  c1  c2  sweepEffic  bmpRemoval
*/
int     landuse_readWashoffParams(char* tok[], int ntoks);
/*! \brief determines the initial buildup of each pollutant on each land use for a given subcatchment.
* \param landFactor arrray of landuse factors
* \param initBuildup total initial buildup of each pollutant
* \param area subcatchment's area (ft2)
* \param curbFraction subcatchment's curb length (users units)
* \return modifies each land use factor's initial pollutant buildup
* \remark uses landFactor values for missing buildup parameters
*/
void    landuse_getInitBuildup(TLandFactor* landFactor,  double* initBuildup,
	    double area, double curb);
/*! \brief computes new pollutant buildup on a landuse after a time increment.
* \param landuse land use index
* \param p pollutant index
* \param area land use area (ac or ha)
* \param curbFraction land use curb length (users units)
* \param buildup current pollutant buildup (lbs or kg)
* \param tStep time increment for buildup (sec)
* \return returns new buildup mass (lbs or kg)
*/
double  landuse_getBuildup(int landuse, int p, double area, double curb,
        double buildup, double tStep);
/*! \brief computes washoff load from a land use.
* \param landuse land use index
* \param p pollutant index
* \param area subcatchment's area (ft2)
* \param landFactor array of land use data for subcatchment
* \param runoff runoff flow generated by subcatchment (ft/sec)
* \param vOutflow runoff volume leaving subcatchment (ft3)
* \return returns pollutant runoff load (mass)
*/
double  landuse_getWashoffLoad(int landuse, int p, double area,
        TLandFactor landFactor[], double runoff, double vOutflow);
/*! \brief computes average BMP removal efficiency for a land use.
* \param j subcatchment index
* \param p pollutant index
* \return returns a BMP removal fraction for pollutant p
*/        
double  landuse_getAvgBmpEffic(int j, int p);
/*! \brief computes average BMP removal efficiency for a land use.
* \param p pollutant index
* \param washoff array of washoff loads
* \return returns washoff mass added by a co-pollutant relation (mass) 
*/
double  landuse_getCoPollutLoad(int p, double washoff[]);

/*!
 * \brief Flow/Quality Routing Methods
 */
/*! \brief initializes flow routing system 
* \param routingModel routing method code
*/
void    flowrout_init(int routingModel);
/*! \brief closes down the routing analyzer
* \param routingModel routing method code
*/
void    flowrout_close(int routingModel);
/*! \brief finds variable time step for dynamic wave routing
* \param routingModel routing method code
* \param fixedStep user-assigned max. routing step (sec)
* \return returns adjusted value of routing time step (sec)
*/
double  flowrout_getRoutingStep(int routingModel, double fixedStep);
/*! \brief routes flow through conveyance network over current time step.
* \param links array of link indexes in topo-sorted order (per routing model)
* \param routingModel type of routing method used
* \param tStep routing time step (sec)
* \return returns number of computational steps taken
*/
int     flowrout_execute(int links[], int routingModel, double tStep);
/*! \brief sorts links from upstream to downstream
* \returns returns array of link indexes in sorted order
*/
void    toposort_sortLinks(int links[]);
/*! \brief finds outflow over time step tStep given flow entering conduit using Kinematic Wave flow routing.
* \param link link index
* \param qin inflow at current time (cfs)
* \param qout outflow at current time (cfs)
* \param tStep routing time step (sec)
* \return returns number of iterations used
*/
int     kinwave_execute(int link, double* qin, double* qout, double tStep);
/*! \brief adjusts dynamic wave routing options */
void    dynwave_validate(void);
/*! \brief initializes dynamic wave routing system */
void    dynwave_init(void);
/*! \brief frees memory allocated for dynamic wave routing method */
void    dynwave_close(void);
/*! \brief computes variable routing time step if applicable
* \param fixedStep user-supplied fixed time step (sec)
* \return computes variable routing time step if applicable
*/
double  dynwave_getRoutingStep(double fixedStep);
/*! \brief routes flows through drainage network over current time step.
* \param tStep routing time step (sec)
* \return returns number of iterations used
* \remark uses links and computes routing step for each of the links
*/
int     dynwave_execute(double tStep);
/*! \brief updates flow in conduit link by solving finite difference form of continuity and momentum equations
* \param j link index
* \param steps number of iteration steps taken
* \param omega under-relaxation parameter
* \param dt time step (sec)
* \return returns number of iterations used
*/
void    dwflow_findConduitFlow(int j, int steps, double omega, double dt);
/*! \brief initializes water quality concentrations in all nodes and links.
*/
void    qualrout_init(void);
/*! \brief routes water quality constituents through the drainage network over the current time step.
* \param tStep routing time step (sec)
*/
void    qualrout_execute(double tStep);

/*!
 * \brief Treatment Methods
 */
/*! \brief allocates memory for computing pollutant removals by treatment.
* \return returns error code
*/
int     treatmnt_open(void);
/*! \brief frees memory used for computing pollutant removals by treatment.
* \return returns error code
*/
void    treatmnt_close(void);
/*! \brief reads a treatment expression from a tokenized line of input.
* \param tok array of string tokens
* \param ntoks number of tokens
* \return returns error code
*/
int     treatmnt_readExpression(char* tok[], int ntoks);
/*! \brief deletes the treatment objects for each pollutant at a node.
* \param node node index
*/
void    treatmnt_delete(int node);
/*! \brief updates pollutant concentrations at a node after treatment.
* \param node node index
* \param q flow rate at node (cfs)
* \param v volume of water at node (ft3)
* \param tStep time step (sec)
 */
void    treatmnt_treat(int node, double q, double v, double tStep);
/*! \brief computes and saves array of inflow concentrations to a node.
* \param qIn inflow flow rate (cfs)
* \param wIn pollutant mass inflow rate (mass/sec)
*/
void    treatmnt_setInflow(double qIn, double wIn[]);

/*!
 * \brief Mass Balance Methods
 */
/*! \brief opens and initializes mass balance continuity checking.
* \return returns error code
*/
int     massbal_open(void);
/*! \brief frees memory used by mass balance system.
*/
void    massbal_close(void);
/*! \brief reports mass balance results.
*/      
void    massbal_report(void);
/*! \brief updates runoff totals after current time step.
* \param type type of flow
* \param v volume of runoff (ft3)
*/
void    massbal_updateRunoffTotals(int type, double v);
/*! \brief adds inflow mass loading to loading totals for current time step.
* \param type type of pollutant
* \param pollut pollutant index
* \param w mass loading 
*/
void    massbal_updateLoadingTotals(int type, int pollut, double w);
/*! \brief updates groundwater totals after current time step.
* \param vInfil infiltration volume (ft3)
* \param vUpperEvap upper zone evaporation volume (ft3)
* \param vLowerEvap lower zone evaporation volume (ft3)
* \param vLowerPerc lower zone percolation volume (ft3)
* \param vGwater groundwater volume (ft3)
*/
void    massbal_updateGwaterTotals(double vInfil, double vUpperEvap,
        double vLowerEvap, double vLowerPerc, double vGwater);
/*! \brief updates overall routing totals with totals from current time step.
* \param tStep time step (sec)
*/
void    massbal_updateRoutingTotals(double tStep);
/*! \brief initializes routing totals for current time step.
*/
void    massbal_initTimeStepTotals(void);
/*! \brief adds inflow flow to flow continuity totals for current time step.
* \param type type of flow
* \param q flow rate (cfs)
*/
void    massbal_addInflowFlow(int type, double q);
/*! \brief adds quality inflow to routing totals for current time step.
* \param type type of inflow
* \param pollut pollutant index
* \param w mass flow rate (mass/sec) 
*/
void    massbal_addInflowQual(int type, int pollut, double w);
/*! \brief adds outflow flow to flow continuity totals for current time step.
* \param q flow rate (cfs)
* \param isFlooded TRUE if link is flooded
*/
void    massbal_addOutflowFlow(double q, int isFlooded);
/*! \brief adds quality outflow to routing totals for current time step.
* \param pollut pollutant index
* \param mass mass flow rate (mass/sec)
* \param isFlooded TRUE if link is flooded
*/
void    massbal_addOutflowQual(int pollut, double mass, int isFlooded);
/*! \brief adds mass loss to flow continuity totals for current time step.
* \param evapLoss evaporation loss (ft3)
* \param infilLoss infiltration loss (ft3)
*/
void    massbal_addNodeLosses(double evapLoss, double infilLoss);
/*! \brief adds link losses over current time step to routing totals.
* \param evapLoss evaporation loss (ft3)
* \param infilLoss infiltration loss (ft3)
*/
void    massbal_addLinkLosses(double evapLoss, double infilLoss);
/*! \brief adds mass reacted during current time step to routing totals.
* \param pollut pollutant index
* \param mass rate of mass reacted (mass/sec)
*/
void    massbal_addReactedMass(int pollut, double mass);
/*! \brief adds mass lost to seepage during current time step to routing totals.
* \param pollut pollutant index
* \param seepLoss mass seepage rate (mass/sec)
*/
void    massbal_addSeepageLoss(int pollut, double seepLoss);
/*! \brief adds mass remaining on dry surface to routing totals.
* \param pollut pollutant index
* \param mass pollutant mass
*/
void    massbal_addToFinalStorage(int pollut, double mass);
/*! \brief computes flow routing mass balance error at current time step.
* \return returns fractional difference between total inflow and outflow.
*/
double  massbal_getStepFlowError(void);
/*! \brief computes runoff mass balance error.
*/
double  massbal_getRunoffError(void);
/*! \brief computes flow routing mass balance error.
*/
double  massbal_getFlowError(void);

/*!
 * \brief Simulation Statistics Methods
 */
/*! \brief opens the simulation statistics system.
* \return returns error code
*/
int     stats_open(void);
/*! \brief closes the simulation statistics system.
*/
void    stats_close(void);
/*! \brief reports simulation statistics.
*/
void    stats_report(void);
/*! \brief updates count of times a node or link was time step-critical.
* \param node node or link index
* \param link TRUE if node is a link
*/
void    stats_updateCriticalTimeCount(int node, int link);
/*! \brief updates various flow routing statistics at current time period.
* \param tStep time step (sec)
* \param aDate current date/time
*/
void    stats_updateFlowStats(double tStep, DateTime aDate);
/*! \brief updates flow routing time step statistics.
* \param tStep time step (sec)
* \param trialsCount number of trials taken
* \param steadyState TRUE if steady state reached
*/
void    stats_updateTimeStepStats(double tStep, int trialsCount, int steadyState);
/*! \brief updates subcatchment runoff statistics at current time period.
* \param subcatch subcatchment index
* \param rainVol rainfall volume (ft3)
* \param runonVol runon volume (ft3)
* \param evapVol evaporation volume (ft3)
* \param infilVol infiltration volume (ft3)
* \param impervVol impervious runoff volume (ft3)
* \param pervVol pervious runoff volume (ft3)
* \param runoffVol total runoff volume (ft3)
* \param runoff total runoff flow rate (cfs)
*/
void    stats_updateSubcatchStats(int subcatch, double rainVol, 
        double runonVol, double evapVol, double infilVol,
        double impervVol, double pervVol, double runoffVol, double runoff);
/*! \brief updates groundwater statistics at current time period.
* \param j subcatchment index
* \param infil infiltration volume (ft3)
* \param evap evaporation volume (ft3)
* \param latFlow lateral flow volume (ft3)
* \param deepFlow deep percolation volume (ft3)
* \param theta moisture content
* \param waterTable water table elevation (ft)
* \param tStep time step (sec)
* \remark uses aquifer values for missing groundwater parameters
*/
void    stats_updateGwaterStats(int j, double infil, double evap,
        double latFlow, double deepFlow, double theta, double waterTable,
        double tStep);
void    stats_updateMaxRunoff(void);
void    stats_updateMaxNodeDepth(int node, double depth);
void    stats_updateConvergenceStats(int node, int converged);


/*!
 * \brief Raingage Methods
 */
int      gage_readParams(int gage, char* tok[], int ntoks);
void     gage_validate(int gage);
void     gage_initState(int gage);
void     gage_setState(int gage, DateTime aDate);
double   gage_getPrecip(int gage, double *rainfall, double *snowfall);
void     gage_setReportRainfall(int gage, DateTime aDate);
DateTime gage_getNextRainDate(int gage, DateTime aDate);
void     gage_updatePastRain(int j, int tStep);
double   gage_getPastRain(int gage, int hrs);

/*!
 * \brief Subcatchment Methods
 */
int     subcatch_readParams(int subcatch, char* tok[], int ntoks);
int     subcatch_readSubareaParams(char* tok[], int ntoks);
int     subcatch_readLanduseParams(char* tok[], int ntoks);
int     subcatch_readInitBuildup(char* tok[], int ntoks);

void    subcatch_validate(int subcatch);
void    subcatch_initState(int subcatch);
void    subcatch_setOldState(int subcatch);

double  subcatch_getFracPerv(int subcatch);
double  subcatch_getStorage(int subcatch);
double  subcatch_getDepth(int subcatch);

void    subcatch_getRunon(int subcatch);
void    subcatch_addRunonFlow(int subcatch, double flow);
double  subcatch_getRunoff(int subcatch, double tStep);

double  subcatch_getWtdOutflow(int subcatch, double wt);
void    subcatch_getResults(int subcatch, double wt, float x[]);

/*!
 * \brief Surface Pollution Buildup/Washoff Methods
 */
void    surfqual_initState(int subcatch);
void    surfqual_getWashoff(int subcatch, double runoff, double tStep);
void    surfqual_getBuildup(int subcatch, double tStep);
void    surfqual_sweepBuildup(int subcatch, DateTime aDate);
double  surfqual_getWtdWashoff(int subcatch, int pollut, double wt);

/*!
 * \brief Conveyance System Node Methods
 */
int     node_readParams(int node, int type, int subIndex, char* tok[], int ntoks);
void    node_validate(int node);

void    node_initState(int node);
void    node_initFlows(int node, double tStep);
void    node_setOldHydState(int node);
void    node_setOldQualState(int node);
void    node_setOutletDepth(int node, double yNorm, double yCrit, double z);

double  node_getSurfArea(int node, double depth);
double  node_getDepth(int node, double volume);
double  node_getVolume(int node, double depth);
double  node_getPondedArea(int node, double depth);

double  node_getOutflow(int node, int link);
double  node_getLosses(int node, double tStep);
double  node_getMaxOutflow(int node, double q, double tStep);
double  node_getSystemOutflow(int node, int *isFlooded);
void    node_getResults(int node, double wt, float x[]);

/*!
 * \brief Conveyance System Inflow Methods
 */
int     inflow_readExtInflow(char* tok[], int ntoks);
int     inflow_readDwfInflow(char* tok[], int ntoks);
int     inflow_readDwfPattern(char* tok[], int ntoks);
int     inflow_setExtInflow(int j, int param, int type, int tSeries,
        int basePat, double cf, double baseline, double sf);
						
void    inflow_initDwfInflow(TDwfInflow* inflow);
void    inflow_initDwfPattern(int pattern);

double  inflow_getExtInflow(TExtInflow* inflow, DateTime aDate);
double  inflow_getDwfInflow(TDwfInflow* inflow, int m, int d, int h);

void    inflow_deleteExtInflows(int node);
void    inflow_deleteDwfInflows(int node);

/*!
 * \brief Routing Interface File Methods
 */
int     iface_readFileParams(char* tok[], int ntoks);
void    iface_openRoutingFiles(void);
void    iface_closeRoutingFiles(void);
int     iface_getNumIfaceNodes(DateTime aDate);
int     iface_getIfaceNode(int index);
double  iface_getIfaceFlow(int index);
double  iface_getIfaceQual(int index, int pollut);
void    iface_saveOutletResults(DateTime reportDate, FILE* file);

/*!
 * \brief Hot Start File Methods
 */
int     hotstart_open(void);
void    hotstart_close(void);

/*!
 * \brief Conveyance System Link Methods
 */
int     link_readParams(int link, int type, int subIndex, char* tok[], int ntoks);
int     link_readXsectParams(char* tok[], int ntoks);
int     link_readLossParams(char* tok[], int ntoks);

void    link_validate(int link);
void    link_initState(int link);
void    link_setOldHydState(int link);
void    link_setOldQualState(int link);

void    link_setTargetSetting(int j);
void    link_setSetting(int j, double tstep);
int     link_setFlapGate(int link, int n1, int n2, double q);

double  link_getInflow(int link);
void    link_setOutfallDepth(int link);
double  link_getLength(int link);
double  link_getYcrit(int link, double q);
double  link_getYnorm(int link, double q);
double  link_getVelocity(int link, double q, double y);
double  link_getFroude(int link, double v, double y);
double  link_getPower(int link);
double  link_getLossRate(int link, int routeModel, double q, double tstep);
char    link_getFullState(double a1, double a2, double aFull);

void    link_getResults(int link, double wt, float x[]);

/*!
 * \brief Link Cross Section Methods
 */
int     xsect_isOpen(int type);
int     xsect_setParams(TXsect *xsect, int type, double p[], double ucf);
void    xsect_setIrregXsectParams(TXsect *xsect);
void    xsect_setCustomXsectParams(TXsect *xsect);
void    xsect_setStreetXsectParams(TXsect *xsect);
double  xsect_getAmax(TXsect* xsect);

double  xsect_getSofA(TXsect* xsect, double area);
double  xsect_getYofA(TXsect* xsect, double area);
double  xsect_getRofA(TXsect* xsect, double area);
double  xsect_getAofS(TXsect* xsect, double sFactor);
double  xsect_getdSdA(TXsect* xsect, double area);
double  xsect_getAofY(TXsect* xsect, double y);
double  xsect_getRofY(TXsect* xsect, double y);
double  xsect_getWofY(TXsect* xsect, double y);
double  xsect_getYcrit(TXsect* xsect, double q);

/*!
 * \brief Culvert/Roadway Methods
 */
double  culvert_getInflow(int link, double q, double h);
double  roadway_getInflow(int link, double dir, double hcrest, double h1,
        double h2);

/*!
 * \brief Force Main Methods
 */
double  forcemain_getEquivN(int j, int k);
double  forcemain_getRoughFactor(int j, double lengthFactor);
double  forcemain_getFricSlope(int j, double v, double hrad);

/*!
 * \brief Cross-Section Transect Methods
 */
int     transect_create(int n);
void    transect_delete(void);
int     transect_readParams(int* count, char* tok[], int ntoks);
void    transect_validate(int j);
void    transect_createStreetTransect(TStreet* street);

/*!
 * \brief Street Cross-Section Methods
 */
int     street_create(int nStreets);
void    street_delete();
int     street_readParams(char* tok[], int ntoks);
double  street_getExtentFilled(int link);

/*!
 * \brief Custom Shape Cross-Section Methods
 */
int     shape_validate(TShape *shape, TTable *curve);

/*!
 * \brief Control Rule Methods
 */
int     controls_create(int n);
void    controls_delete(void);
void    controls_init(void);
void    controls_addToCount(char* s);
int     controls_addVariable(char* tok[], int ntoks);
int     controls_addExpression(char* tok[], int ntoks);
int     controls_addRuleClause(int rule, int keyword, char* Tok[], int nTokens);
int     controls_evaluate(DateTime currentTime, DateTime elapsedTime, 
        double tStep);

/*!
 * \brief Table and Time Series Methods
 */
int     table_readCurve(char* tok[], int ntoks);
int     table_readTimeseries(char* tok[], int ntoks);

int     table_addEntry(TTable* table, double x, double y);
int     table_getFirstEntry(TTable* table, double* x, double* y);
int     table_getNextEntry(TTable* table, double* x, double* y);
void    table_deleteEntries(TTable* table);

void    table_init(TTable* table);
int     table_validate(TTable* table);

double  table_lookup(TTable* table, double x);
double  table_lookupEx(TTable* table, double x);
double  table_intervalLookup(TTable* table, double x);
double  table_inverseLookup(TTable* table, double y);

double  table_getSlope(TTable *table, double x);
double  table_getMaxY(TTable *table, double x);
double  table_getStorageVolume(TTable* table, double x);
double  table_getStorageDepth(TTable* table, double v);

void    table_tseriesInit(TTable *table);
double  table_tseriesLookup(TTable* table, double t, char extend);

/*!
 * \brief Utility Methods
 */
double   UCF(int quantity);                   // units conversion factor
int      getInt(char *s, int *y);             // get integer from string
int      getFloat(char *s, float *y);         // get float from string
int      getDouble(char *s, double *y);       // get double from string
char*    getTempFileName(char *s);            // get temporary file name
int      findmatch(char *s, char *keyword[]); // search for matching keyword
int      match(char *str, char *substr);      // true if substr matches part of str
int      strcomp(const char *s1, const char *s2); // case insensitive string compare
size_t   sstrncpy(char *dest, const char *src,
         size_t n);                           // safe string copy
size_t   sstrcat(char* dest, const char* src,
         size_t destsize);                    // safe string concatenation 
void     writecon(const char *s);             // writes string to console
DateTime getDateTime(double elapsedMsec);     // convert elapsed time to date
void     getElapsedTime(DateTime aDate,       // convert elapsed date
         int* days, int* hrs, int* mins);
char*    addAbsolutePath(char *fname);        // add full path to a file name

#endif //FUNCS_H
