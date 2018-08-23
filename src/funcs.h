//-----------------------------------------------------------------------------
//   funcs.h
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14  (Build 5.1.000)
//             09/15/14  (Build 5.1.007)
//             04/02/15  (Build 5.1.008)
//             08/05/15  (Build 5.1.010)
//             05/10/18  (Build 5.1.013)
//   Author:   L. Rossman (EPA)
//             M. Tryby (EPA)
//
//   Global interfacing functions.
//
//   Build 5.1.007:
//   - climate_readAdjustments() added.
//
//   Build 5.1.008:
//   - Function list was re-ordered and blank lines added for readability.
//   - Pollutant buildup/washoff functions for the new surfqual.c module added.
//   - Several other functions added, re-named or have modified arguments.
//
//   Build 5.1.010:
//   - New roadway_getInflow() function added.
//
//   Build 5.1.013:
//   - Additional arguments added to function stats_updateSubcatchStats.
//
//-----------------------------------------------------------------------------
void     project_open(char *f1, char *f2, char *f3);
void     project_close(void);

void     project_readInput(void);
int      project_readOption(char* s1, char* s2);
void     project_validate(void);
int      project_init(void);

int      project_addObject(int type, char* id, int n);
int      project_findObject(int type, char* id);
char*    project_findID(int type, char* id);

double** project_createMatrix(int nrows, int ncols);
void     project_freeMatrix(double** m);

//-----------------------------------------------------------------------------
//   Input Reader Methods
//-----------------------------------------------------------------------------
int     input_countObjects(void);
int     input_readData(void);

//-----------------------------------------------------------------------------
//   Report Writer Methods
//-----------------------------------------------------------------------------
int     report_readOptions(char* tok[], int ntoks);

void    report_writeLine(char* line);
void    report_writeSysTime(void);
void    report_writeLogo(void);
void    report_writeTitle(void);
void    report_writeOptions(void);
void    report_writeReport(void);

void    report_writeRainStats(int gage, TRainStats* rainStats);
void    report_writeRdiiStats(double totalRain, double totalRdii);

void    report_writeControlActionsHeading(void);
void    report_writeControlAction(DateTime aDate, char* linkID, double value,
        char* ruleID);

void    report_writeRunoffError(TRunoffTotals* totals, double area);
void    report_writeLoadingError(TLoadingTotals* totals);
void    report_writeGwaterError(TGwaterTotals* totals, double area);
void    report_writeFlowError(TRoutingTotals* totals);
void    report_writeQualError(TRoutingTotals* totals);

void    report_writeMaxStats(TMaxStats massBalErrs[], TMaxStats CourantCrit[],
        int nMaxStats);
void    report_writeMaxFlowTurns(TMaxStats flowTurns[], int nMaxStats);
void    report_writeSysStats(TSysStats* sysStats);

void    report_writeErrorMsg(int code, char* msg);
void    report_writeErrorCode(void);
void    report_writeInputErrorMsg(int k, int sect, char* line, long lineCount);
void    report_writeWarningMsg(char* msg, char* id); 
void    report_writeTseriesErrorMsg(int code, TTable *tseries);

void    inputrpt_writeInput(void);
void    statsrpt_writeReport(void);

//-----------------------------------------------------------------------------
//   Temperature/Evaporation Methods
//-----------------------------------------------------------------------------
int      climate_readParams(char* tok[], int ntoks);
int      climate_readEvapParams(char* tok[], int ntoks);
int      climate_readAdjustments(char* tok[], int ntoks);
void     climate_validate(void);
void     climate_openFile(void);
void     climate_initState(void);
void     climate_setState(DateTime aDate);
DateTime climate_getNextEvapDate(void);

//-----------------------------------------------------------------------------
//   Rainfall Processing Methods
//-----------------------------------------------------------------------------
void    rain_open(void);
void    rain_close(void);

//-----------------------------------------------------------------------------
//   Snowmelt Processing Methods
//-----------------------------------------------------------------------------
int     snow_readMeltParams(char* tok[], int ntoks);
int     snow_createSnowpack(int subcacth, int snowIndex);

void    snow_validateSnowmelt(int snowIndex);
void    snow_initSnowpack(int subcatch);
void    snow_initSnowmelt(int snowIndex);

void    snow_getState(int subcatch, int subArea, double x[]);
void    snow_setState(int subcatch, int subArea, double x[]);

void    snow_setMeltCoeffs(int snowIndex, double season);
void    snow_plowSnow(int subcatch, double tStep);
double  snow_getSnowMelt(int subcatch, double rainfall, double snowfall,
        double tStep, double netPrecip[]);
double  snow_getSnowCover(int subcatch);

//-----------------------------------------------------------------------------
//   Runoff Analyzer Methods
//-----------------------------------------------------------------------------
int     runoff_open(void);
void    runoff_execute(void);
void    runoff_close(void);

//-----------------------------------------------------------------------------
//   Conveyance System Routing Methods
//-----------------------------------------------------------------------------
int     routing_open(void);
double  routing_getRoutingStep(int routingModel, double fixedStep);
void    routing_execute(int routingModel, double routingStep);
void    routing_close(int routingModel);

//-----------------------------------------------------------------------------
//   Output Filer Methods
//-----------------------------------------------------------------------------
int     output_open(void);
void    output_end(void);
void    output_close(void);
void    output_checkFileSize(void);
void    output_saveResults(double reportTime);
void    output_updateAvgResults(void);
void    output_readDateTime(int period, DateTime *aDate);
void    output_readSubcatchResults(int period, int area);
void    output_readNodeResults(int period, int node);
void    output_readLinkResults(int period, int link);

//-----------------------------------------------------------------------------
//   Groundwater Methods
//-----------------------------------------------------------------------------
int     gwater_readAquiferParams(int aquifer, char* tok[], int ntoks);
int     gwater_readGroundwaterParams(char* tok[], int ntoks);
int     gwater_readFlowExpression(char* tok[], int ntoks);
void    gwater_deleteFlowExpression(int subcatch);

void    gwater_validateAquifer(int aquifer);
void    gwater_validate(int subcatch);

void    gwater_initState(int subcatch);
void    gwater_getState(int subcatch, double x[]);
void    gwater_setState(int subcatch, double x[]);

void    gwater_getGroundwater(int subcatch, double evap, double infil,
        double tStep);
double  gwater_getVolume(int subcatch);

//-----------------------------------------------------------------------------
//   RDII Methods
//-----------------------------------------------------------------------------
int     rdii_readRdiiInflow(char* tok[], int ntoks);
void    rdii_deleteRdiiInflow(int node);
void    rdii_initUnitHyd(int unitHyd);
int     rdii_readUnitHydParams(char* tok[], int ntoks);
void    rdii_openRdii(void);
void    rdii_closeRdii(void);
int     rdii_getNumRdiiFlows(DateTime aDate);
void    rdii_getRdiiFlow(int index, int* node, double* q);

//-----------------------------------------------------------------------------
//   Landuse Methods
//-----------------------------------------------------------------------------
int     landuse_readParams(int landuse, char* tok[], int ntoks);
int     landuse_readPollutParams(int pollut, char* tok[], int ntoks);
int     landuse_readBuildupParams(char* tok[], int ntoks);
int     landuse_readWashoffParams(char* tok[], int ntoks);

void    landuse_getInitBuildup(TLandFactor* landFactor,  double* initBuildup,
	    double area, double curb);
double  landuse_getBuildup(int landuse, int pollut, double area, double curb,
        double buildup, double tStep);

double  landuse_getWashoffLoad(int landuse, int p, double area,
        TLandFactor landFactor[], double runoff, double vOutflow);
double  landuse_getAvgBmpEffic(int j, int p);
double  landuse_getCoPollutLoad(int p, double washoff[]);

//-----------------------------------------------------------------------------
//   Flow/Quality Routing Methods
//-----------------------------------------------------------------------------
void    flowrout_init(int routingModel);
void    flowrout_close(int routingModel);
double  flowrout_getRoutingStep(int routingModel, double fixedStep);
int     flowrout_execute(int links[], int routingModel, double tStep);

void    toposort_sortLinks(int links[]);
int     kinwave_execute(int link, double* qin, double* qout, double tStep);

void    dynwave_validate(void);
void    dynwave_init(void);
void    dynwave_close(void);
double  dynwave_getRoutingStep(double fixedStep);
int     dynwave_execute(double tStep);
void    dwflow_findConduitFlow(int j, int steps, double omega, double dt);

void    qualrout_init(void);
void    qualrout_execute(double tStep);

//-----------------------------------------------------------------------------
//   Treatment Methods
//-----------------------------------------------------------------------------
int     treatmnt_open(void);
void    treatmnt_close(void);
int     treatmnt_readExpression(char* tok[], int ntoks);
void    treatmnt_delete(int node);
void    treatmnt_treat(int node, double q, double v, double tStep);
void    treatmnt_setInflow(double qIn, double wIn[]);

//-----------------------------------------------------------------------------
//   Mass Balance Methods
//-----------------------------------------------------------------------------
int     massbal_open(void);
void    massbal_close(void);
void    massbal_report(void);

void    massbal_updateRunoffTotals(int type, double v);
void    massbal_updateLoadingTotals(int type, int pollut, double w);
void    massbal_updateGwaterTotals(double vInfil, double vUpperEvap,
        double vLowerEvap, double vLowerPerc, double vGwater);
void    massbal_updateRoutingTotals(double tStep);

void    massbal_initTimeStepTotals(void);
void    massbal_addInflowFlow(int type, double q);
void    massbal_addInflowQual(int type, int pollut, double w);
void    massbal_addOutflowFlow(double q, int isFlooded);
void    massbal_addOutflowQual(int pollut, double mass, int isFlooded);
void    massbal_addNodeLosses(double evapLoss, double infilLoss);
void    massbal_addLinkLosses(double evapLoss, double infilLoss);
void    massbal_addReactedMass(int pollut, double mass);
void    massbal_addSeepageLoss(int pollut, double seepLoss);
void    massbal_addToFinalStorage(int pollut, double mass);
double  massbal_getStepFlowError(void);
double  massbal_getRunoffError(void);
double  massbal_getFlowError(void);

//-----------------------------------------------------------------------------
//   Simulation Statistics Methods
//-----------------------------------------------------------------------------
int     stats_open(void);
void    stats_close(void);
void    stats_report(void);

void    stats_updateCriticalTimeCount(int node, int link);
void    stats_updateFlowStats(double tStep, DateTime aDate, int stepCount,
        int steadyState);
void    stats_updateSubcatchStats(int subcatch, double rainVol, 
        double runonVol, double evapVol, double infilVol,
        double impervVol, double pervVol, double runoffVol, double runoff);    //(5.1.013)
void    stats_updateGwaterStats(int j, double infil, double evap,
        double latFlow, double deepFlow, double theta, double waterTable,
        double tStep);
void    stats_updateMaxRunoff(void);
void    stats_updateMaxNodeDepth(int node, double depth);

//-----------------------------------------------------------------------------
//   Raingage Methods
//-----------------------------------------------------------------------------
int      gage_readParams(int gage, char* tok[], int ntoks);
void     gage_validate(int gage);
void     gage_initState(int gage);
void     gage_setState(int gage, DateTime aDate);
double   gage_getPrecip(int gage, double *rainfall, double *snowfall);
void     gage_setReportRainfall(int gage, DateTime aDate);
DateTime gage_getNextRainDate(int gage, DateTime aDate);

//-----------------------------------------------------------------------------
//   Subcatchment Methods
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
//  Surface Pollutant Buildup/Washoff Methods
//-----------------------------------------------------------------------------
void    surfqual_initState(int subcatch);
void    surfqual_getWashoff(int subcatch, double runoff, double tStep);
void    surfqual_getBuildup(int subcatch, double tStep);
void    surfqual_sweepBuildup(int subcatch, DateTime aDate);
double  surfqual_getWtdWashoff(int subcatch, int pollut, double wt);

//-----------------------------------------------------------------------------
//   Conveyance System Node Methods
//-----------------------------------------------------------------------------
int     node_readParams(int node, int type, int subIndex, char* tok[], int ntoks);
void    node_validate(int node);

void    node_initState(int node);
void    node_initInflow(int node, double tStep);
void    node_setOldHydState(int node);
void    node_setOldQualState(int node);

void    node_setOutletDepth(int node, double yNorm, double yCrit, double z);
void    node_setDividerCutoff(int node, int link);

double  node_getSurfArea(int node, double depth);
double  node_getDepth(int node, double volume);
double  node_getVolume(int node, double depth);
double  node_getPondedArea(int node, double depth);

double  node_getOutflow(int node, int link);
double  node_getLosses(int node, double tStep);
double  node_getMaxOutflow(int node, double q, double tStep);
double  node_getSystemOutflow(int node, int *isFlooded);
void    node_getResults(int node, double wt, float x[]);

//-----------------------------------------------------------------------------
//   Conveyance System Inflow Methods
//-----------------------------------------------------------------------------
int     inflow_readExtInflow(char* tok[], int ntoks);
int     inflow_readDwfInflow(char* tok[], int ntoks);
int     inflow_readDwfPattern(char* tok[], int ntoks);
int     inflow_setExtInflow(int j, int param, int type, 
						int tSeries, int basePat, double cf, 
						double baseline, double sf);
int     inflow_validate(int param, int type, int tSeries, 
						int basePat, double *cf);					
						
void    inflow_initDwfInflow(TDwfInflow* inflow);
void    inflow_initDwfPattern(int pattern);

double  inflow_getExtInflow(TExtInflow* inflow, DateTime aDate);
double  inflow_getDwfInflow(TDwfInflow* inflow, int m, int d, int h);
double  inflow_getPatternFactor(int p, int month, int day, int hour);

void    inflow_deleteExtInflows(int node);
void    inflow_deleteDwfInflows(int node);

//-----------------------------------------------------------------------------
//   Routing Interface File Methods
//-----------------------------------------------------------------------------
int     iface_readFileParams(char* tok[], int ntoks);
void    iface_openRoutingFiles(void);
void    iface_closeRoutingFiles(void);
int     iface_getNumIfaceNodes(DateTime aDate);
int     iface_getIfaceNode(int index);
double  iface_getIfaceFlow(int index);
double  iface_getIfaceQual(int index, int pollut);
void    iface_saveOutletResults(DateTime reportDate, FILE* file);

//-----------------------------------------------------------------------------
//   Hot Start File Methods
//-----------------------------------------------------------------------------
int     hotstart_open(void);
void    hotstart_close(void);

//-----------------------------------------------------------------------------
//   Conveyance System Link Methods
//-----------------------------------------------------------------------------
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
double  link_getLossRate(int link, double q, double tStep);
char    link_getFullState(double a1, double a2, double aFull);

void    link_getResults(int link, double wt, float x[]);

//-----------------------------------------------------------------------------
//   Link Cross-Section Methods
//-----------------------------------------------------------------------------
int     xsect_isOpen(int type);
int     xsect_setParams(TXsect *xsect, int type, double p[], double ucf);
void    xsect_setIrregXsectParams(TXsect *xsect);
void    xsect_setCustomXsectParams(TXsect *xsect);
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

//-----------------------------------------------------------------------------
//   Culvert/Roadway Methods
//-----------------------------------------------------------------------------
double  culvert_getInflow(int link, double q, double h);
double  roadway_getInflow(int link, double dir, double hcrest, double h1,
        double h2);

//-----------------------------------------------------------------------------
//   Force Main Methods
//-----------------------------------------------------------------------------
double  forcemain_getEquivN(int j, int k);
double  forcemain_getRoughFactor(int j, double lengthFactor);
double  forcemain_getFricSlope(int j, double v, double hrad);

//-----------------------------------------------------------------------------
//   Cross-Section Transect Methods
//-----------------------------------------------------------------------------
int     transect_create(int n);
void    transect_delete(void);
int     transect_readParams(int* count, char* tok[], int ntoks);
void    transect_validate(int j);

//-----------------------------------------------------------------------------
//   Custom Shape Cross-Section Methods
//-----------------------------------------------------------------------------
int     shape_validate(TShape *shape, TTable *curve);

//-----------------------------------------------------------------------------
//   Control Rule Methods
//-----------------------------------------------------------------------------
int     controls_create(int n);
void    controls_delete(void);
int     controls_addRuleClause(int rule, int keyword, char* Tok[], int nTokens);
int     controls_evaluate(DateTime currentTime, DateTime elapsedTime, 
        double tStep);

//-----------------------------------------------------------------------------
//   Table & Time Series Methods
//-----------------------------------------------------------------------------
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
double  table_getArea(TTable* table, double x);
double  table_getInverseArea(TTable* table, double a);

void    table_tseriesInit(TTable *table);
double  table_tseriesLookup(TTable* table, double t, char extend);

//-----------------------------------------------------------------------------
//   Utility Methods
//-----------------------------------------------------------------------------
double   UCF(int quantity);                   // units conversion factor
int      getInt(char *s, int *y);             // get integer from string
int      getFloat(char *s, float *y);         // get float from string
int      getDouble(char *s, double *y);       // get double from string
char*    getTempFileName(char *s);            // get temporary file name
int      findmatch(char *s, char *keyword[]); // search for matching keyword
int      match(char *str, char *substr);      // true if substr matches part of str
int      strcomp(char *s1, char *s2);         // case insensitive string compare
char*    sstrncpy(char *dest, const char *src,
         size_t maxlen);                      // safe string copy
void     writecon(char *s);                   // writes string to console
DateTime getDateTime(double elapsedMsec);     // convert elapsed time to date
void     getElapsedTime(DateTime aDate,       // convert elapsed date
         int* days, int* hrs, int* mins);
