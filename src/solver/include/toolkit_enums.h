/*
 *  toolkit_enums.h
 *
 *  Created on: November 13, 2020
 *  Updated on:
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/CESER
 *
 */


#ifndef TOOLKIT_ENUMS_H_
#define TOOLKIT_ENUMS_H_


/// Object type codes
typedef enum {
    SM_GAGE         = 0,  /**< Rain gage */
    SM_SUBCATCH     = 1,  /**< Subcatchment */
    SM_NODE         = 2,  /**< Conveyance system node */
    SM_LINK         = 3,  /**< Conveyance system link */
    SM_POLLUT       = 4,  /**< Pollutant */
    SM_LANDUSE      = 5,  /**< Land use category */
    SM_TIMEPATTERN  = 6,  /**< Dry weather flow time pattern */
    SM_CURVE        = 7,  /**< Generic table of values */
    SM_TSERIES      = 8,  /**< Generic time series of values */
    SM_CONTROL      = 9,  /**< Conveyance system control rules */
    SM_TRANSECT     = 10, /**< Irregular channel cross-section */
    SM_AQUIFER      = 11, /**< Groundwater aquifer */
    SM_UNITHYD      = 12, /**< RDII unit hydrograph */
    SM_SNOWMELT     = 13, /**< Snowmelt parameter set */
    SM_SHAPE        = 14, /**< Custom conduit shape */
    SM_LID          = 15  /**< LID treatment units */
} SM_ObjectType;

/// Node object type codes
typedef enum {
    SM_JUNCTION    = 0,  /**< Manhole Junction */
    SM_OUTFALL     = 1,  /**< Outfall */
    SM_STORAGE     = 2,  /**< Storage */
    SM_DIVIDER     = 3   /**< Divider */
} SM_NodeType;

/// Link object type codes
typedef enum {
    SM_CONDUIT     = 0,  /**< Conduit */
    SM_PUMP        = 1,  /**< Pump */
    SM_ORIFICE     = 2,  /**< Orifice */
    SM_WEIR        = 3,  /**< Weir */
    SM_OUTLET      = 4   /**< Outlet */
} SM_LinkType;

/// Simulation Option codes
typedef enum {
    SM_STARTDATE    = 0, /**< Simulation Start Date */
    SM_ENDDATE      = 1, /**< Simulation End Date */
    SM_REPORTDATE   = 2  /**< Simulation Report Start Date */
} SM_TimePropety;

/// Simulation Unit Codes
typedef enum {
    SM_SYSTEMUNIT   = 0, /**< System Units */
    SM_FLOWUNIT     = 1, /**< Flow Units */
} SM_Units;

/// Simulation Options
typedef enum {
    SM_ALLOWPOND    = 0, /**< Allow Ponding */
    SM_SKIPSTEADY   = 1, /**< Skip Steady State*/
    SM_IGNORERAIN   = 2, /**< Ignore Rainfall*/
    SM_IGNORERDII   = 3, /**< Ignore RDII */
    SM_IGNORESNOW   = 4, /**< Ignore Snowmelt */
    SM_IGNOREGW     = 5, /**< Ignore Groundwater */
    SM_IGNOREROUTE  = 6, /**< Ignore Routing */
    SM_IGNORERQUAL  = 7  /**< Ignore Quality */
} SM_SimOption;

/// Simulation Settings
typedef enum {
    SM_ROUTESTEP     = 0,  /**< Routing Step (sec) */
    SM_MINROUTESTEP  = 1,  /**< Minimum Routing Step (sec) */
    SM_LENGTHSTEP    = 2,  /**< Lengthening Step (sec) */
    SM_STARTDRYDAYS  = 3,  /**< Antecedent dry days */
    SM_COURANTFACTOR = 4,  /**< Courant time step factor */
    SM_MINSURFAREA   = 5,  /**< Minimum nodal surface area */
    SM_MINSLOPE      = 6,  /**< Minimum conduit slope */
    SM_RUNOFFERROR   = 7,  /**< Runoff continuity error */
    SM_GWERROR       = 8,  /**< Groundwater continuity error */
    SM_FLOWERROR     = 9,  /**< Flow routing error */
    SM_QUALERROR     = 10, /**< Quality routing error */
    SM_HEADTOL       = 11, /**< DW routing head tolerance (ft) */
    SM_SYSFLOWTOL    = 12, /**< Tolerance for steady system flow */
    SM_LATFLOWTOL    = 13  /**< Tolerance for steady nodal inflow */
} SM_SimSetting;

/// Node property codes
typedef enum {
    SM_INVERTEL     = 0,  /**< Invert Elevation */
    SM_FULLDEPTH    = 1,  /**< Full Depth */
    SM_SURCHDEPTH   = 2,  /**< Surcharge Depth */
    SM_PONDAREA     = 3,  /**< Ponding Area */
    SM_INITDEPTH    = 4,  /**< Initial Depth */
} SM_NodeProperty;

/// Link property codes
typedef enum {
    SM_OFFSET1      = 0,  /**< Inlet Offset */
    SM_OFFSET2      = 1,  /**< Outlet Offset */
    SM_INITFLOW     = 2,  /**< Initial Flow Rate */
    SM_FLOWLIMIT    = 3,  /**< Flow limit */
    SM_INLETLOSS    = 4,  /**< Inlet Loss */
    SM_OUTLETLOSS   = 5,  /**< Outles Loss */
    SM_AVELOSS      = 6,  /**< Average Loss */
} SM_LinkProperty;

/// Subcatchment property codes
typedef enum {
    SM_WIDTH        = 0,  /**< Width */
    SM_AREA         = 1,  /**< Area */
    SM_FRACIMPERV   = 2,  /**< Impervious Fraction */
    SM_SLOPE        = 3,  /**< Slope */
    SM_CURBLEN      = 4   /**< Curb Length */
} SM_SubcProperty;

/// Node result property codes
typedef enum {
    SM_TOTALINFLOW    = 0,  /**< Total Inflow */
    SM_TOTALOUTFLOW   = 1,  /**< Total Outflow */
    SM_LOSSES         = 2,  /**< Node Losses */
    SM_NODEVOL        = 3,  /**< Stored Volume */
    SM_NODEFLOOD      = 4,  /**< Flooding Rate */
    SM_NODEDEPTH      = 5,  /**< Node Depth */
    SM_NODEHEAD       = 6,  /**< Node Head */
    SM_LATINFLOW      = 7   /**< Lateral Inflow Rate */
} SM_NodeResult;

/// Node pollutant result property codes
typedef enum {
    SM_NODEQUAL       = 0,  /**< Current Node Quality */
} SM_NodePollut;

/// Link result property codes
typedef enum {
    SM_LINKFLOW        = 0,  /**< Flowrate */
    SM_LINKDEPTH       = 1,  /**< Depth */
    SM_LINKVOL         = 2,  /**< Volume */
    SM_USSURFAREA      = 3,  /**< Upstream Surface Area */
    SM_DSSURFAREA      = 4,  /**< Downstream Surface Area */
    SM_SETTING         = 5,  /**< Setting */
    SM_TARGETSETTING   = 6,  /**< Target Setting */
    SM_FROUDE          = 7   /**< Froude Number */
} SM_LinkResult;

/// Link pollutant result property codes
typedef enum {
    SM_LINKQUAL      = 0,  /**< Current Link Quality */
    SM_TOTALLOAD     = 1,  /**< Total Quality Mass Loading */
} SM_LinkPollut;

/// Subcatchment result property codes
typedef enum {
    SM_SUBCRAIN      = 0,  /**< Rainfall Rate */
    SM_SUBCEVAP      = 1,  /**< Evaporation Loss */
    SM_SUBCINFIL     = 2,  /**< Infiltration Loss */
    SM_SUBCRUNON     = 3,  /**< Runon Rate */
    SM_SUBCRUNOFF    = 4,  /**< Runoff Rate */
    SM_SUBCSNOW      = 5,  /**< Snow Depth */
} SM_SubcResult;

/// Subcatchment pollutant result property codes
typedef enum {
    SM_BUILDUP        = 0,  /**< Pollutant Buildup Load */
    SM_CPONDED        = 1,  /**< Ponded Pollutant Concentration */
    SM_SUBCQUAL       = 2,  /**< Current Pollutant Runoff Quality */
    SM_SUBCTOTALLOAD  = 3,  /**< Total Pollutant Washoff load */
} SM_SubcPollut;

/// Gage precip array property codes
typedef enum {
    SM_TOTALPRECIP   = 0,  /**< Total Precipitation Rate */
    SM_RAINFALL      = 1,  /**< Rainfall Rate */
    SM_SNOWFALL      = 2   /**< Snowfall Rate */
} SM_GagePrecip;

/// Lid control layer codes
typedef enum {
    SM_SURFACE      = 0,  /**< Lid Surface Layer */
    SM_SOIL         = 1,  /**< Lid Soil Layer */
    SM_STOR         = 2,  /**< Lid Storage Layer */
    SM_PAVE         = 3,  /**< Lid Pavement Layer */
    SM_DRAIN        = 4,  /**< Lid Underdrain Layer */
    SM_DRAINMAT     = 5,  /**< Lid Drainage Mat Layer */
} SM_LidLayer;

/// Lid control layer property codes
typedef enum {
    SM_THICKNESS    = 0,  /**< Storage Height */
    SM_VOIDFRAC     = 1,  /**< Available Fraction of Storage Volume */
    SM_ROUGHNESS    = 2,  /**< Manning n */
    SM_SURFSLOPE    = 3,  /**< Surface Slope (fraction) */
    SM_SIDESLOPE    = 4,  /**< Side Slope (run/rise) */
    SM_ALPHA        = 5,  /**< Slope/Roughness Term in Manning Eqn */
    SM_POROSITY     = 6,  /**< Void Volume / Total Volume */
    SM_FIELDCAP     = 7,  /**< Field Capacity */
    SM_WILTPOINT    = 8,  /**< Wilting Point */
    SM_SUCTION      = 9,  /**< Suction Head at Wetting Front */
    SM_KSAT         = 10, /**< Saturated Hydraulic Conductivity */
    SM_KSLOPE       = 11, /**< Slope of Log(k) v. Moisture Content Curve */
    SM_CLOGFACTOR   = 12, /**< Clogging Factor */
    SM_IMPERVFRAC   = 13, /**< Impervious Area Fraction */
    SM_COEFF        = 14, /**< Underdrain Flow Coefficient */
    SM_EXPON        = 15, /**< Underdrain Head Exponent */
    SM_OFFSET       = 16, /**< Offset Height of Underdrain */
    SM_DELAY        = 17, /**< Rain Barrel Drain Delay Time */
    SM_HOPEN        = 18, /**< Head When Drain Opens */
    SM_HCLOSE       = 19, /**< Head When Drain Closes */
    SM_QCURVE       = 20, /**< Curve Controlling FLow Rate (Optional) */
    SM_REGENDAYS    = 21, /**< Clogging Regeneration Interval (Days) */
    SM_REGENDEGREE  = 22, /**< Degree of Clogging Regeneration */
} SM_LidLayerProperty;

/// Lid unit property codes
typedef enum {
    SM_UNITAREA     = 0,  /**< Area of Single Replicate Unit */
    SM_FWIDTH       = 1,  /**< Full Top Width of Single Unit */
    SM_BWIDTH       = 2,  /**< Bottom Width of Single Unit */
    SM_INITSAT      = 3,  /**< Initial Saturation of Soil and Storage Layer */
    SM_FROMIMPERV   = 4,  /**< Fraction of Impervious Area Runoff Treated */
    SM_FROMPERV     = 5,  /**< Fraction of Pervious Area Runoff Treated */
} SM_LidUProperty;

/// Lid unit option codes
typedef enum {
    SM_INDEX        = 0,  /**< Lid Process Index */
    SM_NUMBER       = 1,  /**< Number of Replicate Units */
    SM_TOPERV       = 2,  /**< Outflow to Pervious Area */
    SM_DRAINSUB     = 3,  /**< Subcatchment Recieving Drain Flow */
    SM_DRAINNODE    = 4,  /**< Node Recieving Drain Flow */
} SM_LidUOptions;

/// Lid unit result codes
typedef enum {
    SM_INFLOW       = 0,  /**< Total Inflow */
    SM_EVAP         = 1,  /**< Total Evaporation */
    SM_INFIL        = 2,  /**< Total Infiltration */
    SM_SURFFLOW     = 3,  /**< Total Surface runoff */
    SM_DRAINFLOW    = 4,  /**< Total Underdrain flow */
    SM_INITVOL      = 5,  /**< Initial Stored Volume */
    SM_FINALVOL     = 6,  /**< Final Stored Volume */
    SM_SURFDEPTH    = 7,  /**< Depth of Ponded Water on Surface Layer */
    SM_PAVEDEPTH    = 8,  /**< Depth of Water in Porous Pavement Layer */
    SM_SOILMOIST    = 9,  /**< Moisture Content of Biocell Soil Layer */
    SM_STORDEPTH    = 10, /**< Depth of Water in Storage Layer */
    SM_DRYTIME      = 11, /**< Time Since Last Rainfall */
    SM_OLDDRAINFLOW = 12, /**< Previous Drain Flow (unit and group) */
    SM_NEWDRAINFLOW = 13, /**< Current Drain Flow (unit and group) */
    SM_PERVAREA     = 14, /**< Amount of Pervious Area (group) */
    SM_FLOWTOPERV   = 15, /**< Total Flow Sent to Pervious Area */
    SM_EVAPRATE     = 16, /**< Evaporate Rate */
    SM_NATIVEINFIL  = 17, /**< Native soil infil. rate limit */
    SM_SURFINFLOW   = 18, /**< Precip. + runon to LID unit */
    SM_SURFINFIL    = 19, /**< Infil. rate from surface layer */
    SM_SURFEVAP     = 20, /**< Evaporate rate from surface layer */
    SM_SURFOUTFLOW  = 21, /**< Outflow from surface layer */
    SM_PAVEEVAP     = 22, /**< Evaporation from pavement layer */
    SM_PAVEPERC     = 23, /**< Percolation from pavement layer */
    SM_SOILEVAP     = 24, /**< Evaporation from soil layer */
    SM_SOILPERC     = 25, /**< Percolation from soil layer */
    SM_STORAGEINFLOW= 26, /**< Inflow rate to storage layer */
    SM_STORAGEEXFIL = 27, /**< Exfilration rate from storage layer */
    SM_STORAGEEVAP  = 28, /**< Evaporation from storage layer */
    SM_STORAGEDRAIN = 29, /**< Underdrain flow rate layer */
} SM_LidResult;


#endif /* TOOLKIT_ENUMS_H_ */
