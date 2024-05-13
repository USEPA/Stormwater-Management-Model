# Appendix B. SWMM 5's Input File Format {#appendix_b}

SWMM 5's input file is organized in sections, where each section begins with a keyword enclosed in square brackets. The various keywords are listed below. Section keywords can appear in mixed lower and upper case. The sections can appear in any arbitrary order in the input file, and not all sections must be present. Each section can contain one or more lines of data. Blank lines may appear anywhere in the file. A semicolon (;) can be used to indicate that what follows on the line is a comment, not data. Data items can appear in any column of a line.

Clicking on a keyword will take you to a description of the data supplied in that section. When listing the format of a line of data, mandatory keywords are shown in boldface while optional items appear in parentheses. A list of keywords separated by a slash (`YES/NO`) means that only one of the words should appear in the data line.

|                        |                                                              |
| :--------------------- | :----------------------------------------------------------- |
| @subpage title         | project title                                                |
|                        |                                                              |
| @subpage options       | analysis options                                             |
| @subpage report        | output reporting instructions                                |
| @subpage files         | interface file options                                       |
|                        |                                                              |
| @subpage raingages     | rain gage information                                        |
| @subpage evaporation   | evaporation data                                             |
| @subpage temperature   | air temperature and snow melt data                           |
| @subpage adjustments   | monthly adjustments applied to climate variables             |
|                        |                                                              |
| @subpage subcatchments | basic subcatchment information                               |
| @subpage subareas      | subcatchment impervious/pervious sub-area data               |
| @subpage infiltration  | subcatchment infiltration parameters                         |
| @subpage lid_controls  | low impact development control information                   |
| @subpage lid_usage     | assignment of LID controls tosubcatchments                   |
|                        |                                                              |
| @subpage aquifers      | groundwater aquifer parameters                               |
| @subpage groundwater   | subcatchment groundwater parameters                          |
| @subpage gwf           | groundwater flow expressions                                 |
| @subpage snowpacks     | subcatchment snow pack parameters                            |
|                        |                                                              |
| @subpage junctions     | junction node information                                    |
| @subpage outfalls      | outfall node information                                     |
| @subpage dividers      | flow divider node information                                |
| @subpage storage       | storage node information                                     |
|                        |                                                              |
| @subpage conduits      | conduit link information                                     |
| @subpage pumps         | pump link information                                        |
| @subpage orifices      | orifice link information                                     |
| @subpage weirs         | weir link information                                        |
| @subpage outlets       | outlet link information                                      |
|                        |                                                              |
| @subpage xsections     | conduit, orifice, and weir cross-section geometry            |
| @subpage transects     | transect geometry for conduits with irregular cross-sections |
| @subpage streets       | cross-section geometry for street                            |
| @subpage inlets        | design data for street storm                                 |
| @subpage inlet_usage   | assignment of inlets to streets                              |
| @subpage losses        | conduit entrance/exit losses and                             |
| @subpage controls      | rules that control pump and regulator operation              |
|                        |                                                              |
| @subpage pollutants    | pollutant information                                        |
| @subpage landuses      | land use categories                                          |
| @subpage coverages     | assignment of land uses to subcatchments                     |
| @subpage loadings      | initial pollutant loads on subcatchments                     |
| @subpage buildup       | buildup functions for pollutants and land uses               |
| @subpage washoff       | washoff functions for pollutants and land uses               |
| @subpage treatment     | pollutant removal functions at conveyance system nodes       |
|                        |                                                              |
| @subpage inflows       | external hydrograph/pollutograph inflow at nodes             |
| @subpage dwf           | baseline dry weather sanitary inflow at nodes                |
| @subpage rdii          | rainfall-dependent I/I information at nodes                  |
| @subpage hydrographs   | unit hydrograph data used to construct RDII inflows          |
|                        |                                                              |
| @subpage curves        | x-y tabular data referenced in other sections                |
| @subpage timeseries    | time series data referenced in other sections                |
| @subpage patterns      | periodic multipliers referenced in other sections            |

<!---
  title
-->

@page title  [TITLE]

## Purpose:

Attaches a descriptive title to the project being analyzed.

## Format:

Any number of lines may be entered. The first line will be used as a page header in the output report.

<!---
  options
-->

@page options  [OPTIONS]

## Purpose:

Provides values for various analysis options.

## Format:

|                         |                                                                                     |
| :---------------------- | :---------------------------------------------------------------------------------- |
| **FLOW_UNITS**          | **CFS / GPM / MGD / CMS / LPS / MLD**                                               |
| **INFILTRATION**        | **HORTON / MODIFIED_HORTON / GREEN_AMPT / <br> MODIFIED_GREEN_AMPT / CURVE_NUMBER** |
| **FLOW_ROUTING**        | **STEADY / KINWAVE / DYNWAVE**                                                      |
| **LINK_OFFSETS**        | **DEPTH / ELEVATION**                                                               |
| **FORCE_MAIN_EQUATION** | **H-W / D-W**                                                                       |
| **IGNORE_RAINFALL**     | **YES / NO**                                                                        |
| **IGNORE_SNOWMELT**     | **YES / NO**                                                                        |
| **IGNORE_GROUNDWATER**  | **YES / NO**                                                                        |
| **IGNORE_RDII**         | **YES / NO**                                                                        |
| **IGNORE_ROUTING**      | **YES / NO**                                                                        |
| **IGNORE_QUALITY**      | **YES / NO**                                                                        |
| **ALLOW_PONDING**       | **YES / NO**                                                                        |
| **SKIP_STEADY_STATE**   | **YES / NO**                                                                        |
| **SYS_FLOW_TOL**        | _value_                                                                             |
| **LAT_FLOW_TOL**        | _value_                                                                             |
| **START_DATE**          | _month/day/year_                                                                    |
| **START_TIME**          | _hours:minutes_                                                                     |
| **END_DATE**            | _month/day/year_                                                                    |
| **END_TIME**            | _hours:minutes_                                                                     |
| **REPORT_START_DATE**   | _month/day/year_                                                                    |
| **REPORT_START_TIME**   | _hours:minutes_                                                                     |
| **SWEEP_START**         | _month/day_                                                                         |
| **SWEEP_END**           | _month/day_                                                                         |
| **DRY_DAYS**            | _days_                                                                              |
| **REPORT_STEP**         | _hours:minutes:seconds_                                                             |
| **WET_STEP**            | _hours:minutes:seconds_                                                             |
| **DRY_STEP**            | _hours:minutes:seconds_                                                             |
| **ROUTING_STEP**        | _seconds_                                                                           |
| **LENGTHENING_STEP**    | _seconds_                                                                           |
| **VARIABLE_STEP**       | _value_                                                                             |
| **MINIMUM_STEP**        | _seconds_                                                                           |
| **INERTIAL_DAMPING**    | **NONE / PARTIAL / FULL**                                                           |
| **NORMAL_FLOW_LIMITED** | **SLOPE / FROUDE / BOTH / NONE**                                                    |
| **SURCHARGE_METHOD**    | **EXTRAN / SLOT**                                                                   |
| **MIN_SURFAREA**        | _value_                                                                             |
| **MIN_SLOPE**           | _value_                                                                             |
| **MAX_TRIALS**          | _value_                                                                             |
| **HEAD_TOLERANCE**      | _value_                                                                             |
| **THREADS**             | _value_                                                                             |

## Remarks:

**FLOW_UNITS** makes a choice of flow units. Selecting a US flow unit means that all other quantities will be expressed in US customary units, while choosing a metric flow unit will force all quantities to be expressed in SI metric units. (Exceptions are pollutant concentration and Manning’s roughness coefficient (n) which are always in metric units). The default is **CFS**.

**INFILTRATION** selects a model for computing infiltration of rainfall into the upper soil zone of subcatchments. The default model is **HORTON**.

**FLOW_ROUTING** determines which method is used to route flows through the drainage system. **STEADY** refers to sequential steady state routing (i.e. hydrograph translation), **KINWAVE** to kinematic wave routing, **DYNWAVE** to dynamic wave routing. The default routing method is **DYNWAVE**.

**LINK_OFFSETS** determines the convention used to specify the position of a link offset above the invert of its connecting node. **DEPTH** indicates that offsets are expressed as the distance between the node invert and the link while **ELEVATION** indicates that the absolute elevation of the offset is used. The default is **DEPTH**.

**FORCE_MAIN_EQUATION** establishes whether the Hazen-Williams (**H-W**) or the Darcy-Weisbach (**D-W**) equation will be used to compute friction losses for pressurized flow in conduits that have been assigned a Circular Force Main cross-section shape. The default is **H-W**.

**IGNORE_RAINFALL** is set to **YES** if all rainfall data and runoff calculations should be ignored. In this case SWMM only performs flow and pollutant routing based on user-supplied direct and dry weather inflows. The default is **NO**.

**IGNORE_SNOWMELT** is set to **YES** if snowmelt calculations should be ignored when a project file contains snow pack objects. The default is **NO**.

**IGNORE_GROUNDWATER** is set to **YES** if groundwater calculations should be ignored when a project file contains aquifer objects. The default is **NO**.

**IGNORE_RDII** is set to **YES** if rainfall dependent inflow/infiltration should be ignored when RDII unit hydrographs and RDII inflows have been supplied to a project file. The default is **NO**.

**IGNORE_ROUTING** is set to **YES** if only runoff should be computed even if the project contains drainage system links and nodes. The default is **NO**.

**IGNORE_QUALITY** is set to **YES** if pollutant washoff, routing, and treatment should be ignored in a project that has pollutants defined. The default is **NO**.

**ALLOW_PONDING** determines whether excess water is allowed to collect atop nodes and be re-introduced into the system as conditions permit. The default is **NO** ponding. In order for ponding to actually occur at a particular node, a non-zero value for its Ponded Area attribute must be used.

**SKIP_STEADY_STATE** should be set to **YES** if flow routing computations should be skipped during steady state periods of a simulation during which the last set of computed flows will be used. A time step is considered to be in steady state if the percent difference between total system inflow and total system outflow is below the **SYS_FLOW_TOL** and the percent difference between current and previous lateral inflows are below the **LAT_FLOW_TOL**. The default for this option is **NO**.

**SYS_FLOW_TOL** is the maximum percent difference between total system inflow and total system outflow which can occur in order for the **SKIP_STEADY_STATE** option to take effect. The default is 5 percent.

**LAT_FLOW_TOL** is the maximum percent difference between the current and previous lateral inflow at all nodes in the conveyance system in order for the **SKIP_STEADY_STATE** option to take effect. The default is 5 percent.

**START_DATE** is the date when the simulation begins. If not supplied, a date of 1/1/2004 is used.

**START_TIME** is the time of day on the starting date when the simulation begins. The default is 12 midnight (0:00:00).

**END_DATE** is the date when the simulation is to end. The default is the start date.

**END_TIME** is the time of day on the ending date when the simulation will end. The default is 24:00:00.

**REPORT_START_DATE** is the date when reporting of results is to begin. The default is the simulation start date.

**REPORT_START_TIME** is the time of day on the report starting date when reporting is to begin. The default is the simulation start time of day.

**SWEEP_START** is the day of the year (month/day) when street sweeping operations begin. The default is 1/1.

**SWEEP_END** is the day of the year (month/day) when street sweeping operations end. The default is 12/31.

**DRY_DAYS** is the number of days with no rainfall prior to the start of the simulation. The default is 0.

**REPORT_STEP** is the time interval for reporting of computed results. The default is 0:15:00.

**WET_STEP** is the time step length used to compute runoff from subcatchments during periods of rainfall or when ponded water still remains on the surface. The default is 0:05:00.

**DRY_STEP** is the time step length used for runoff computations (consisting essentially of pollutant buildup) during periods when there is no rainfall and no ponded water. The default is 1:00:00.

**ROUTING_STEP** is the time step length in seconds used for routing flows and water quality constituents through the conveyance system. The default is 20 sec. This can be increased if dynamic wave routing is not used. Fractional values (e.g., 2.5) are permissible as are values entered in hours:minutes:seconds format.

**LENGTHENING_STEP** is a time step, in seconds, used to lengthen conduits under dynamic wave routing, so that they meet the Courant stability criterion under full-flow conditions (i.e., the travel time of a wave will not be smaller than the specified conduit lengthening time step). As this value is decreased, fewer conduits will require lengthening. A value of 0 (the default) means that no conduits will be lengthened.

**VARIABLE_STEP** is a safety factor applied to a variable time step computed for each time period under dynamic wave flow routing. The variable time step is computed so as to satisfy the Courant stability criterion for each conduit and yet not exceed the ROUTING_STEP value. If the safety factor is 0 (the default), then no variable time step is used.

**MINIMUM_STEP** is the smallest time step allowed when variable time steps are used for dynamic wave flow routing. The default value is 0.5 seconds.

**INERTIAL_DAMPING** indicates how the inertial terms in the Saint Venant momentum equation will be handled under dynamic wave flow routing. Choosing **NONE** maintains these terms at their full value under all conditions. Selecting **PARTIAL** (the default) will reduce the terms as flow comes closer to being critical (and ignores them when flow is supercritical). Choosing FULL will drop the terms altogether.

**NORMAL_FLOW_LIMITED** specifies which condition is checked to determine if flow in a conduit is supercritical and should thus be limited to the normal flow. Use **SLOPE** to check if the water surface slope is greater than the conduit slope, **FROUDE** to check if the Froude number is greater than 1.0, **BOTH** to check both conditions or **NONE** if no checks are made. The default is **BOTH**.

**SURCHARGE_METHOD** selects which method will be used to handle surcharge conditions. The **EXTRAN** option uses a variation of the Surcharge Algorithm from previous versions of SWMM to update nodal heads when all connecting links become full. The **SLOT** option uses a Preissmann Slot to add a small amount of virtual top surface width to full flowing pipes so that SWMM's normal procedure for updating nodal heads can continue to be used. The default is **EXTRAN**.

**MIN_SURFAREA** is a minimum surface area used at nodes when computing changes in water depth under dynamic wave routing. If 0 is entered, then the default value of 12.566 ft2 (1.167 m2) (i.e., the area of a 4-ft diameter manhole) is used.

**MIN_SLOPE** is the minimum value allowed for a conduit’s slope (%). If zero (the default) then no minimum is imposed (although SWMM uses a lower limit on elevation drop of 0.001 ft (0.00035 m) when computing a conduit slope).

**MAX_TRIALS** is the maximum number of trials allowed during a time step to reach convergence when updating hydraulic heads at the conveyance system’s nodes. The default value is 8.

**HEAD_TOLERANCE** is the difference in computed head at each node between successive trials below which the flow solution for the current time step is assumed to have converged. The default tolerance is 0.005 ft (0.0015 m).

**THREADS** is the number of parallel computing threads to use for dynamic wave flow routing on machines equipped with multi-core processors. The default is 1.

<!---
  report
-->

@page report  [REPORT]

## Purpose:

Describes the contents of the report file that SWMM produces.

## Formats:

|                   |                                                 |
| :---------------- | :---------------------------------------------- |
| **INPUT**         | **YES / NO**                                    |
| **CONTINUITY**    | **YES / NO**                                    |
| **FLOWSTATS**     | **YES / NO**                                    |
| **CONTROLS**      | **YES / NO**                                    |
| **SUBCATCHMENTS** | **ALL / NONE** / \<list of subcatchment names\> |
| **NODES**         | **ALL / NONE** / \<list of node names\>         |
| **LINKS**         | **ALL / NONE** / \<list of link names\>         |
| **LID**           | _Name  Subcatch  Fname_                         |

## Remarks:

**INPUT** specifies whether or not a summary of the input data should be provided in the output report. The default is **NO**.

**CONTINUITY** specifies whether continuity checks should be reported or not. The default is **YES**.

**FLOWSTATS** specifies whether summary flow statistics should be reported or not. The default is **YES**.

**CONTROLS** specifies whether all control actions taken during a simulation should be listed or not. The default is **NO**.

**SUBCATCHMENTS** gives a list of subcatchments whose results are to be reported. The default is **NONE**.

**NODES** gives a list of nodes whose results are to be reported. The default is **NONE**.

**LINKS** gives a list of links whose results are to be reported. The default is **NONE**.

**LID** specifies that the LID control Name in subcatchment *Subcatch* should have a detailed performance report for it written to file Fname.

The SUBCATCHMENTS, NODES, LINKS, and LID lines can be repeated multiple times.

<!---
  files
-->

@page files  [FILES]

## Purpose:

Identifies optional interface files used or saved by a run.

## Formats:

|                             |         |
| --------------------------- | ------- |
| **USE / SAVE** **RAINFALL** | _Fname_ |
| **USE / SAVE** **RUNOFF**   | _Fname_ |
| **USE / SAVE** **HOTSTART** | _Fname_ |
| **USE / SAVE** **RDII**     | _Fname_ |
| **USE** **INFLOWS**         | _Fname_ |
| **SAVE** **OUTFLOWS**       | _Fname_ |

## Parameters:

*Fname* the name of an interface file.

## Remarks:

SWMM can use several different kinds of interface files that contain either externally imposed inputs (e.g., rainfall or infiltration/inflow hydrographs) or the results of previously run analyses (e.g., runoff or routing results). These files can help speed up simulations, simplify comparisons of different loading scenarios, and allow large study areas to be broken up into smaller areas that can be analyzed individually. The different types of interface files that are currently available include:

 - rainfall interface file

 - runoff interface file

 - hot start file

 - RDII interface file

 - routing interface files

Rainfall, Runoff, and RDII files can either be used or saved in a run, but not both. A run can both use and save a Hot Start file (with different names).

Enclose the external file name in double quotes if it contains spaces and include its full path if it resides in a different directory than the SWMM input file.

<!---
  raingages
-->

@page raingages  [RAINGAGES]


## Purpose:

Identifies each rain gage that provides rainfall data for the study
area.

 

## Formats:

Name Form Intvl SCF TIMESERIES Tseries

Name Form Intvl SCF FILE Fname (Sta Units)

 

## Parameters:

  ------ -----------------------------
  Name   name assigned to rain gage.
  ------ -----------------------------

  ------ --------------------------------------------------------------------
  Form   form of recorded rainfall, either INTENSITY, VOLUME or CUMULATIVE.
  ------ --------------------------------------------------------------------

  ------- -------------------------------------------------------------------------------------------------------------------
  Intvl   time interval between gage readings in decimal hours or hours:minutes format (e.g., 0:15 for 15-minute readings).
  ------- -------------------------------------------------------------------------------------------------------------------

  ----- ----------------------------------------------------------------------
  SCF   snow catch deficiency correction factor (use 1.0 for no adjustment).
  ----- ----------------------------------------------------------------------

  --------- -----------------------------------------------------------------------
  Tseries   name of a time series in the [TIMESERIES] section with rainfall data.
  --------- -----------------------------------------------------------------------

  ------- --------------------------------------------------------------------------------------------
  Fname   name of an external file with rainfall data. Rainfall files are discussed in Section 11.3.
  ------- --------------------------------------------------------------------------------------------

  ----- -----------------------------------------------------------------------
  Sta   name of the recording station in a user-prepared formatted rain file.
  ----- -----------------------------------------------------------------------

  ------- ---------------------------------------------------------------------------------------------------------------
  Units   rain depth units for the data in a user-prepared formatted rain file, either IN (inches) or MM (millimeters).
  ------- ---------------------------------------------------------------------------------------------------------------

 

## Remarks:

Enclose the external file name in double quotes if it contains spaces
and include its full path if it resides in a different directory than
the SWMM input file.

The station name and depth units entries are only required when using a
user-prepared formatted rainfall file.

<!---
  evaporation
-->
@page evaporation  [EVAPORATION]


## Purpose:

Specifies how daily potential evaporation rates vary with time for the
study area.

 

## Formats:

+-----------------------------------+-----------------------------------+
| CONSTANT                          | evap                              |
+-----------------------------------+-----------------------------------+
| MONTHLY                           | e1 e2 e3 e4 e5 e6 e7 e8 e9 e10    |
|                                   | e11 e12                           |
+-----------------------------------+-----------------------------------+
| TIMESERIES                        | Tseries                           |
+-----------------------------------+-----------------------------------+
| TEMPERATURE                       |                                   |
+-----------------------------------+-----------------------------------+
| FILE                              | (p1 p2 p3 p4 p5 p6 p7 p8 p9 p10   |
|                                   | p11 p12)                          |
+-----------------------------------+-----------------------------------+
| RECOVERY                          | patternID                         |
+-----------------------------------+-----------------------------------+
| DRY_ONLY                          | NO / YES                          |
+-----------------------------------+-----------------------------------+

## Parameters:

+-----------------------------------+-----------------------------------+
| evap                              | constant evaporation rate (in/day |
|                                   | or mm/day).                       |
+-----------------------------------+-----------------------------------+
| e1                                | evaporation rate in January       |
|                                   | (in/day or mm/day).               |
+-----------------------------------+-----------------------------------+
| ...                               |                                   |
+-----------------------------------+-----------------------------------+
| e12                               | evaporation rate in December      |
|                                   | (in/day or mm/day).               |
+-----------------------------------+-----------------------------------+
| Tseries                           | name of a time series in the      |
|                                   | [TIMESERIES] section with         |
|                                   | evaporation data.                 |
+-----------------------------------+-----------------------------------+
| p1                                | pan coefficient for January.      |
+-----------------------------------+-----------------------------------+
| ...                               |                                   |
+-----------------------------------+-----------------------------------+
| p12                               | pan coefficient for December.     |
+-----------------------------------+-----------------------------------+
| patID                             | name of a monthly time pattern.   |
+-----------------------------------+-----------------------------------+

## Remarks:

Use only one of the above formats (CONSTANT, MONTHLY, TIMESERIES,
TEMPERATURE, or FILE). If no [EVAPORATION] section appears, then
evaporation is assumed to be 0.

TEMPERATURE indicates that evaporation rates will be computed from the
daily air temperatures contained in an external climate file whose name
is provided in the [TEMPERATURE] section. This method also uses the
site’s latitude, which can also be specified in the [TEMPERATURE]
section.

FILE indicates that evaporation data will be read directly from the same
external climate file used for air        temperatures as specified in
the [TEMPERATURE] section. Supplying monthly pan coefficients for these
data is optional.

RECOVERY identifies an optional monthly time pattern of multipliers used
to modify infiltration recovery rates during dry periods. For example,
if the normal infiltration recovery rate was 1% during a specific time
period and a pattern factor of 0.8 applied to this period, then the
actual recovery rate would be 0.8%.

DRY_ONLY determines if evaporation only occurs during periods with no
precipitation. The default is NO.

The evaporation rates provided in this section are potential rates. The
actual amount of water evaporated will depend on the amount available as
a simulation progresses.

<!---
  temperature
-->
@page temperature  [TEMPERATURE]


  ---------- --
  ## Purpose:
  ---------- --

Specifies daily air temperatures, monthly wind speed, and various
snowmelt parameters for the study area. Required only when snowmelt is
being modeled or when evaporation rates are computed from daily
temperatures or are read from an external climate file.

 

## Formats:

  ---------------- ------------------------------------------------
  TIMESERIES       Tseries
  FILE             Fname (Start) (Units)
  WINDSPEED        MONTHLY s1 s2 s3 s4 s5 s6 s7 s8 s9 s10 s11 s12
  WINDSPEED        FILE
  SNOWMELT         Stemp  ATIwt  RNM  Elev  Lat  DTLong
  ADC IMPERVIOUS   f.0 f.1 f.2 f.3 f.4 f.5 f.6 f.7 f.8 f.9
  ADC PERVIOUS     f.0 f.1 f.2 f.3 f.4 f.5 f.6 f.7 f.8 f.9
  ---------------- ------------------------------------------------

 

## Parameters:

  --------- --------------------------------------------------------------------------
  Tseries   name of a time series in the [TIMESERIES] section with temperature data.
  --------- --------------------------------------------------------------------------

  ------- ---------------------------------------------------------
  Fname   name of an external Climate file with temperature data.
  ------- ---------------------------------------------------------

  ------- ------------------------------------------------------------------------------------------------------
  Start   date to begin reading from the file in month/day/year format (default is the beginning of the file).
  ------- ------------------------------------------------------------------------------------------------------

  ------- -------------------------------------------------------------------------------------------------------------------
  Units   temperature units for GHCN files (C10 for tenths of a degree C (the default), C for degrees C or F for degrees F.
  ------- -------------------------------------------------------------------------------------------------------------------

  ---- -----------------------------------------------
  s1   average wind speed in January (mph or km/hr).
  ---- -----------------------------------------------

...

  ----- ------------------------------------------------
  s12   average wind speed in December (mph or km/hr).
  ----- ------------------------------------------------

  ------- --------------------------------------------------------------------
  Stemp   air temperature at which precipitation falls as snow (deg F or C).
  ------- --------------------------------------------------------------------

  ------- -------------------------------------------------------
  ATIwt   antecedent temperature index weight (default is 0.5).
  ------- -------------------------------------------------------

  ----- ---------------------------------------
  RNM   negative melt ratio (default is 0.6).
  ----- ---------------------------------------

  ------ --------------------------------------------------------------------------------
  Elev   average elevation of study area above mean sea level (ft or m) (default is 0).
  ------ --------------------------------------------------------------------------------

  ----- --------------------------------------------------------------
  Lat   latitude of the study area in degrees North (default is 50).
  ----- --------------------------------------------------------------

  -------- -----------------------------------------------------------------------------------------------------
  DTLong   correction, in minutes of time, between true solar time and the standard clock time (default is 0).
  -------- -----------------------------------------------------------------------------------------------------

  ----- ---------------------------------------------------------------------------------------
  f.0   fraction of area covered by snow when ratio of snow depth to depth at 100% cover is 0
  ----- ---------------------------------------------------------------------------------------

...

  ----- ------------------------------------------------------------------------------------------
  f.9   fraction of area covered by snow when ratio of snow depth to depth at 100% cover is 0.9.
  ----- ------------------------------------------------------------------------------------------

 

## Remarks:

Use the TIMESERIES line to read air temperature from a time series or
the FILE line to read it from an external Climate file. Climate files
are discussed in Section 11.4. If neither format is used, then air
temperature remains constant at 70 degrees F.

Enclose the Climate file name in double quotes if it contains spaces and
include its full path if it resides in a different directory than the
SWMM input file.

Temperatures supplied from NOAA's latest Climate Data Online GHCN files
should have their units (C or F) specified. Older versions of these
files listed temperatures in tenths of a degree C (C10). An asterisk can
be entered for the Start date if it defaults to the beginning of the
file.

Wind speed can be specified either by monthly average values or by the
same Climate file used for air temperature. If neither option appears,
then wind speed is assumed to be 0.

Separate Areal Depletion Curves (ADC) can be defined for impervious and
pervious sub-areas. The ADC parameters will default to 1.0 (meaning no
depletion) if no data are supplied for a particular type of sub-area.

<!---
  adjustments
-->
@page adjustments  [ADJUSTMENTS]


## Purpose:

Specifies optional monthly adjustments to be made to temperature,
evaporation rate, rainfall intensity and hydraulic conductivity in each
time period of a simulation.

 

## Formats:

  -------------- ----------------------------------------
  TEMPERATURE    t1 t2 t3 t4 t5 t6 t7 t8 t9 t10 t11 t12
  EVAPORATION    e1 e2 e3 e4 e5 e6 e7 e8 e9 e10 e11 e12
  RAINFALL       r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12
  CONDUCTIVITY   c1 c2 c3 c4 c5 c6 c7 c8 c9 c10 c11 c12
  -------------- ----------------------------------------

## Parameters:

  --------- ------------------------------------------------------------------------------------------------
  t1..t12   adjustments to temperature in January, February, etc., as plus or minus degrees F (degrees C).
  --------- ------------------------------------------------------------------------------------------------

  --------- -----------------------------------------------------------------------------------------------
  e1..e12   adjustments to evaporation rate in January, February, etc., as plus or minus in/day (mm/day).
  --------- -----------------------------------------------------------------------------------------------

  --------- ----------------------------------------------------------------------
  r1..r12   multipliers applied to precipitation rate in January, February, etc.
  --------- ----------------------------------------------------------------------

  --------- ---------------------------------------------------------------------------------------------------------------------------------
  c1..c12   multipliers applied to soil hydraulic conductivity in January, February, etc. used in either Horton or Green-Ampt infiltration.
  --------- ---------------------------------------------------------------------------------------------------------------------------------

## Remarks:

The same adjustment is applied for each time period within a given month
and is repeated for that month in each subsequent year being simulated.

<!---
  subcatchments
-->

@page subcatchments  [SUBCATCHMENTS]


## Purpose:

Identifies each subcatchment within the study area. Subcatchments are
land area units which generate runoff from rainfall.

 

## Format:

Name Rgage OutID Area %Imperv Width Slope Clength (Spack)

 

## Parameters:

  ------ ------------------------------------
  Name   name assigned to the subcatchment.
  ------ ------------------------------------

  ------- ------------------------------------------------------------------------------
  Rgage   name of a rain gage in the [RAINGAGES] section assigned to the subcatchment.
  ------- ------------------------------------------------------------------------------

  ------- ------------------------------------------------------------------------------
  OutID   name of the node or subcatchment that receives runoff from the subcatchment.
  ------- ------------------------------------------------------------------------------

  ------ -----------------------------------------------
  Area   area of the subcatchment (acres or hectares).
  ------ -----------------------------------------------

  --------- -----------------------------------------------------------
  %Imperv   percentage of the subcatchment’s area that is impervious.
  --------- -----------------------------------------------------------

  ------- ----------------------------------------------------------
  Width   characteristic width of the subcatchment (ft or meters).
  ------- ----------------------------------------------------------

  ------- -------------------------------------
  Slope   the subcatchment’s slope (percent).
  ------- -------------------------------------

  --------- ---------------------------------------------------------------------------------------------------
  Clength   total curb length (any length units) used to describe pollutant buildup. Use 0 if not applicable.
  --------- ---------------------------------------------------------------------------------------------------

  ------- --------------------------------------------------------------------------------------------------------------------------------------------
  Spack   optional name of a snow pack object (from the [SNOWPACKS] section) that characterizes snow accumulation and melting over the subcatchment.
  ------- --------------------------------------------------------------------------------------------------------------------------------------------

 
<!---
  subareas
-->
@page subareas  [SUBAREAS]


## Purpose:

Supplies information about pervious and impervious areas for each
subcatchment. Each subcatchment can consist of a pervious sub-area, an
impervious sub-area with depression storage, and an impervious sub-area
without depression storage.

 

## Format:

Subcat Nimp Nperv Simp Sperv %Zero RouteTo (%Routed)

 

## Parameters:

  -------- --------------------
  Subcat   subcatchment name.
  -------- --------------------

  ------ ---------------------------------------------------------------------------
  Nimp   Manning's coefficient (n) for overland flow over the impervious sub-area.
  ------ ---------------------------------------------------------------------------

  ------- -------------------------------------------------------------------------
  Nperv   Manning's coefficient (n) for overland flow over the pervious sub-area.
  ------- -------------------------------------------------------------------------

  ------ ----------------------------------------------------------------
  Simp   depression storage for the impervious sub-area (inches or mm).
  ------ ----------------------------------------------------------------

  ------- --------------------------------------------------------------
  Sperv   depression storage for the pervious sub-area (inches or mm).
  ------- --------------------------------------------------------------

  ------- --------------------------------------------------------
  %Zero   percent of impervious area with no depression storage.
  ------- --------------------------------------------------------

  --------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  RouteTo   IMPERVIOUS if pervious area runoff runs onto impervious area, PERVIOUS if impervious runoff runs onto pervious area, or OUTLET if both areas drain to the subcatchment's outlet (default = OUTLET).
  --------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  --------- ----------------------------------------------------------------------------
  %Routed   percent of runoff routed from one type of area to another (default = 100).
  --------- ----------------------------------------------------------------------------

<!---
  infiltration
-->

@page infiltration  [INFILTRATION]


## Purpose:

Supplies infiltration parameters for each subcatchment. Rainfall lost to
infiltration only occurs over the pervious sub-area of a subcatchment.

 

## Format:

Subcat  p1  p2  p3  (p4  p5)  (Method)

 

## Parameters:

+-----------------------------------+-----------------------------------+
| Subcat                            | subcatchment name.                |
+-----------------------------------+-----------------------------------+
| Method                            | either HORTON, MODIFIED_HORTON,   |
|                                   | GREEN_AMPT,                       |
|                                   |                                   |
|                                   | MODIFIED_GREEN_AMPT, or           |
|                                   | CURVE_NUMBER.                     |
|                                   |                                   |
|                                   | If not specified then the         |
|                                   | infiltration method supplied in   |
|                                   | the [OPTIONS] section is used.    |
+-----------------------------------+-----------------------------------+

For Horton and Modified Horton Infiltration:

  ---- -----------------------------------------------------------------
  p1   maximum infiltration rate on the Horton curve (in/hr or mm/hr).
  ---- -----------------------------------------------------------------

  ---- -----------------------------------------------------------------
  p2   minimum infiltration rate on the Horton curve (in/hr or mm/hr).
  ---- -----------------------------------------------------------------

  ---- -------------------------------------------------
  p3   decay rate constant of the Horton curve (1/hr).
  ---- -------------------------------------------------

  ---- ----------------------------------------------------------
  p4   time it takes for a fully saturated soil to dry  (days).
  ---- ----------------------------------------------------------

  ---- ------------------------------------------------------------------------
  p5   maximum infiltration volume possible (0 if not applicable) (in or mm).
  ---- ------------------------------------------------------------------------

For Green-Ampt and Modified Green-Ampt Infiltration:

  ---- ------------------------------------
  p1   soil capillary suction (in or mm).
  ---- ------------------------------------

  ---- ---------------------------------------------------------
  p2   soil saturated hydraulic conductivity (in/hr or mm/hr).
  ---- ---------------------------------------------------------

  ---- -----------------------------------------------------------------------------
  p3   initial soil moisture deficit (porosity minus moisture content) (fraction).
  ---- -----------------------------------------------------------------------------

For Curve-Number Infiltration:

  ---- -------------------
  p1   SCS Curve Number.
  ---- -------------------

  ---- -----------------
  p2   no longer used.
  ---- -----------------

  ---- ---------------------------------------------------------
  p3   time it takes for a fully saturated soil to dry (days).
  ---- ---------------------------------------------------------

  <!---
    lid_controls
  -->
@page lid_controls  [LID_CONTROLS]


## Purpose:

Defines scale-independent LID controls that can be deployed within
subcatchments.

 

## Formats:

Name        Type

followed by one or more of the following lines depending on Type:

  ------ ---------- ----------------------------------------------
  Name   SURFACE    StorHt VegFrac Rough Slope Xslope
  Name   SOIL       Thick Por FC WP Ksat Kcoeff Suct
  Name   PAVEMENT   Thick Vratio FracImp Perm Vclog (Treg Freg)
  Name   STORAGE    Height Vratio Seepage Vclog (Covrd)
  Name   DRAIN      Coeff Expon Offset Delay (Hopen Hclose Qcrv)
  Name   DRAINMAT   Thick Vratio Rough
  Name   REMOVALS   Pollut Rmvl Pollut Rmvl ...
  ------ ---------- ----------------------------------------------

## Parameters:

  ------ -------------------------------
  Name   name assigned to LID process.
  ------ -------------------------------

  ------ -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Type   BC for bio-retention cell; RG for rain garden; GR for green roof; IT for infiltration trench; PP for permeable pavement; RB for rain barrel; RD for rooftop disconnection; VS for vegetative swale.
  ------ -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  -------- ---------------------
  Pollut   name of a pollutant
  -------- ---------------------

  ------ -------------------------------------------------------------------------------------------------------------------------------------------------------------
  Rmvl   the percent removal the LID achieves for the pollutant (several pollutant removals can be placed on the same line or specified in separate REMOVALS lines).
  ------ -------------------------------------------------------------------------------------------------------------------------------------------------------------

 

For LIDs with Surface Layers:

  -------- -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  StorHt   when confining walls or berms are present this is the maximum depth to which water can pond above the surface of the unit before overflow occurs (in inches or mm). For LIDs that experience overland flow it is the height of any surface depression storage. For swales, it is the height of its trapezoidal cross-section.
  -------- -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  --------- ------------------------------------------------------------------------
  VegFrac   fraction of the surface storage volume that is filled with vegetation.
  --------- ------------------------------------------------------------------------

  ------- ---------------------------------------------------------------------------------------------------------------------------------------------------
  Rough   Manning's coefficient (n) for overland flow over surface soil cover, pavement, roof surface or a vegetative swale. Use 0 for other types of LIDs.
  ------- ---------------------------------------------------------------------------------------------------------------------------------------------------

  ------- ---------------------------------------------------------------------------------------------------------
  Slope   slope of a roof surface, pavement surface or vegetative swale (percent). Use 0 for other types of LIDs.
  ------- ---------------------------------------------------------------------------------------------------------

  -------- ---------------------------------------------------------------------------------------------------------------
  Xslope   slope (run over rise) of the side walls of a vegetative swale's cross-section. Use 0 for other types of LIDs.
  -------- ---------------------------------------------------------------------------------------------------------------

If either Rough or Slope values are 0 then any ponded water that exceeds
the surface storage depth is assumed to completely overflow the LID
control within a single time step.

 

For LIDs with Pavement Layers:

  ------- -------------------------------------------------
  Thick   thickness of the pavement layer (inches or mm).
  ------- -------------------------------------------------

  -------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Vratio   void ratio (volume of void space relative to the volume of solids in the pavement for continuous systems or for the fill material used in modular systems). Note that porosity = void ratio / (1 + void ratio).
  -------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  --------- -----------------------------------------------------------------------------------------------------------------
  FracImp   ratio of impervious paver material to total area for modular systems; 0 for continuous porous pavement systems.
  --------- -----------------------------------------------------------------------------------------------------------------

  ------ ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Perm   permeability of the concrete or asphalt used in continuous systems or hydraulic conductivity of the fill material (gravel or sand) used in modular systems (in/hr or mm/hr).
  ------ ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  ------- --------------------------------------------------------------------------------------------------------------------------------------------
  Vclog   the number of pavement layer void volumes of runoff treated it takes to completely clog the pavement. Use a value of 0 to ignore clogging.
  ------- --------------------------------------------------------------------------------------------------------------------------------------------

  ------ -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Treg   the number of days that the pavement layer is allowed to clog before its permeability is restored, typically by vacuuming its surface. A value of 0 (the default) indicates that no permeability regeneration occurs.
  ------ -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  ------ ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Freg   The fractional degree to which the pavement's permeability is restored when a regeneration interval is reached. The default is 0 (no restoration) while a value of 1 indicates complete restoration to the original permeability value. Once regeneration occurs the pavement begins to clog once again at a rate determined by Vclog.
  ------ ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

 

For LIDs with Soil Layers:

  ------- ---------------------------------------------
  Thick   thickness of the soil layer (inches or mm).
  ------- ---------------------------------------------

  ----- ---------------------------------------------------
  Por   soil porosity (pore space volume / total volume).
  ----- ---------------------------------------------------

  ---- -----------------------------------------------------------------
  FC   soil field capacity (moisture content of a fully drained soil).
  ---- -----------------------------------------------------------------

  ---- --------------------------------------------------------------
  WP   soil wilting point (moisture content of a fully dried soil).
  ---- --------------------------------------------------------------

  ------ -----------------------------------------------------------
  Ksat   soil’s saturated hydraulic conductivity (in/hr or mm/hr).
  ------ -----------------------------------------------------------

  -------- ----------------------------------------------------------------------------------------------------------------------
  Kcoeff   slope of the curve of log(conductivity) versus soil moisture deficit (porosity minus soil moisture) (dimensionless).
  -------- ----------------------------------------------------------------------------------------------------------------------

  ------ ------------------------------------
  Suct   soil capillary suction (in or mm).
  ------ ------------------------------------

 

For LIDs with Storage Layers:

  -------- ---------------------------------------------------------------------------
  Height   thickness of the storage layer or height of a rain barrel (inches or mm).
  -------- ---------------------------------------------------------------------------

  -------- --------------------------------------------------------------------------------------------------------------------------------------
  Vratio   void ratio (volume of void space relative to the volume of solids in the layer). Note that porosity = void ratio / (1 + void ratio).
  -------- --------------------------------------------------------------------------------------------------------------------------------------

  --------- --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Seepage   the rate at which water seeps from the layer into the underlying native soil when first constructed (in/hr or mm/hr). If there is an impermeable floor or liner below the layer then use a value of 0.
  --------- --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  ------- ------------------------------------------------------------------------------------------------------------------------------------
  Vclog   number of storage layer void volumes of runoff treated it takes to completely clog the layer. Use a value of 0 to ignore clogging.
  ------- ------------------------------------------------------------------------------------------------------------------------------------

  ------- -----------------------------------------------------------------
  Covrd   YES (the default) if a rain barrel is covered, NO if it is not.
  ------- -----------------------------------------------------------------

Values for Vratio, Seepage, and Vclog  are ignored for rain barrels
while Covrd applies only to rain barrels.

 

For LIDs with Drain Systems:

  ------- ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Coeff   coefficient C that determines the rate of flow through the drain as a function of height of stored water above the drain bottom. For Rooftop Disconnection it is the maximum flow rate (in inches/hour or mm/hour) that the roof’s gutters and downspouts can handle before overflowing.
  ------- ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  ------- -------------------------------------------------------------------------------------------------------------------------------
  Expon   exponent n that determines the rate of flow through the drain as a function of height of stored water above the drain outlet.
  ------- -------------------------------------------------------------------------------------------------------------------------------

  -------- -----------------------------------------------------------------------------------------------
  Offset   height of the drain line above the bottom of the storage layer or rain barrel (inches or mm).
  -------- -----------------------------------------------------------------------------------------------

  ------- --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Delay   number of dry weather hours that must elapse before the drain line in a rain barrel is opened (the line is assumed to be closed once rainfall begins). A value of 0 signifies that the barrel's drain line is always open and drains continuously.  This parameter is ignored for other types of LIDs.
  ------- --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  ------- -------------------------------------------------------------------------------------------------------------------------------------------------
  Hopen   The height of water  (in inches or mm) in the drain's Storage Layer that causes the drain to automatically open. Use 0 to disable this feature.
  ------- -------------------------------------------------------------------------------------------------------------------------------------------------

  -------- -------------------------------------------------------------------------------------------------------------------------------------------------
  Hclose   The height of water (in inches or mm) in the drain's Storage Layer that causes the drain to automatically close. Use 0 to disable this feature.
  -------- -------------------------------------------------------------------------------------------------------------------------------------------------

  -------- ---------------------------------------------------------------------------------------------------------------------------------------------------------------
  Qcurve   The name of an optional Control Curve that adjusts the computed drain flow as a function of the head of water above the drain. Leave blank if not applicable.
  -------- ---------------------------------------------------------------------------------------------------------------------------------------------------------------

 

For Green Roof LIDs with Drainage Mats:

  ------- -----------------------------------------------
  Thick   thickness of the drainage mat (inches or mm).
  ------- -----------------------------------------------

  -------- --------------------------------------------------
  Vratio   ratio of void volume to total volume in the mat.
  -------- --------------------------------------------------

  ------- ------------------------------------------------------------------------------------------------------
  Rough   Manning's coefficient (n) used to compute the horizontal flow rate of drained water through the mat.
  ------- ------------------------------------------------------------------------------------------------------

 

## Remarks:

The following table shows which layers are required (x) or are optional
(o) for each type of LID process:

 

  ----------------------- --------- ---------- ------ --------- ------- -----------
  LID Type                Surface   Pavement   Soil   Storage   Drain   Drain Mat
  Bio-Retention Cell      x                    x      x         o        
  Rain Garden             x                    x                         
  Green Roof              x                    x                        x
  Infiltration Trench     x                           x         o        
  Permeable Pavement      x         x          o      x         o        
  Rain Barrel                                         x         x        
  Rooftop Disconnection   x                                     x        
  Vegetative Swale        x                                              
  ----------------------- --------- ---------- ------ --------- ------- -----------

 

The equation used to compute flow rate out of the underdrain per unit
area of the LID (in in/hr or mm/hr) is q = C(h-Hd)n  where q is outflow,
h is height of stored water (inches or mm) and Hd is the drain offset
height. Note that the units of C depend on the unit system being used as
well as the value assigned to n.

The actual dimensions of an LID control are provided in the [LID_USAGE]
section when it is placed in a particular subcatchment.

 

## Examples:

;A street planter with no drain

Planter  BC

Planter  SURFACE   6  0.3  0    0     0

Planter  SOIL     24  0.5  0.1  0.05  1.2  2.4

Planter  STORAGE  12  0.5  0.5  0

 

;A green roof with impermeable bottom

GR1  BC

GR1  SURFACE  3  0    0    0     0

GR1  SOIL     3  0.5  0.1  0.05  1.2  2.4

GR1  STORAGE  3  0.5  0    0

GR1  DRAIN    5  0.5  0    0

 

;A rain barrel that drains 6 hours after rainfall ends

RB12  RB

RB12  STORAGE  36  0    0  0

RB12  DRAIN    10  0.5  0  6

 

;A grass swale 24 in. high with 5:1 side slope

Swale  VS

Swale  SURFACE  24  0  0.2  3  5

<!---
  lid_usage
-->

@page lid_usage  [LID_USAGE]


## Purpose:

Deploys LID controls within specific subcatchment areas.

 

## Format:

Subcat LID Number Area Width InitSat FromImp ToPerv

(RptFile DrainTo FromPerv)                                              
                                                 

## Parameters:

  ---------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Subcat     name of the subcatchment using the LID process.
  LID        name of an LID process defined in the [LID_CONTROLS] section.
  Number     number of replicate LID units deployed.
  Area       area of each replicate unit (ft2 or m2)
  Width      width of the outflow face of each identical LID unit (in ft or m). This parameter applies to roofs, pavement, trenches, and swales that use overland flow to convey surface runoff off of the unit. It can be set to 0 for other LID processes, such as bio-retention cells, rain gardens, and rain barrels that simply spill any excess captured runoff over their berms.
  InitSat    the percent to which the LID's soil, storage, and drain mat zones are initially filled with water. For soil zones 0 % saturation corresponds to the wilting point moisture content while 100 % saturation has the moisture content equal to the porosity.
  FromImp    the percent of the impervious portion of the subcatchment’s non-LID area whose runoff is treated by the LID practice. (E.g., if rain barrels are used to capture roof runoff and roofs represent 60% of the impervious area, then the impervious area treated is 60%). If the LID unit treats only direct rainfall, such as with a green roof, then this value should be 0. If the LID takes up the entire subcatchment then this field is ignored.
  ToPerv     a value of 1 indicates that the surface and drain flow from the LID unit should be routed back onto the pervious area of the subcatchment that contains it. This would be a common choice to make for rain barrels, rooftop disconnection, and possibly green roofs. The default value is 0.
  RptFile    optional name of a file to which detailed time series results for the LID will be written. Enclose the name in double quotes if it contains spaces and include its full path if it resides in a different directory than the SWMM input file. Use ‘*’ if not applicable and an entry for DrainTo or FromPerv follows
  DrainTo    optional name of subcatchment or node that receives flow from the unit’s drain line, if different from the outlet of the subcatchment that the LID is placed in. Use ‘*’ if not applicable and an entry for FromPerv follows.
  FromPerv   optional percent of the pervious portion of the subcatchment’s non-LID area whose runoff is treated by the LID practice. The default value is 0.
  ---------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

## Remarks:

If ToPerv is set to 1 and DrainTo set to some other outlet, then only
the excess surface flow from the LID unit will be routed back to the
subcatchment’s pervious area while the underdrain flow will be sent to
DrainTo.

More than one type of LID process can be deployed within a subcatchment
as long as their total area does not exceed that of the subcatchment and
the total percent impervious area treated does not exceed 100.

 

## Examples:

;34 rain barrels of 12 sq ft each are placed in

;subcatchment S1. They are initially empty and treat 17%

;of the runoff from the subcatchment’s impervious area.

;The outflow from the barrels is returned to the ;subcatchment’s
pervious area.

S1  RB14  34  12  0  0  17  1

 

;Subcatchment S2 consists entirely of a single vegetative ;swale 200 ft
long by 50 ft wide.

S2  Swale  1  10000  50  0  0  0  “swale.rpt”
<!---
  aquifers
-->

@page aquifers  [AQUIFERS]


## Purpose:

Supplies parameters for each unconfined groundwater aquifer in the study
area. Aquifers consist of two zones – a lower saturated zone and an
upper unsaturated zone with a moving boundary between the two.

 

## Format:

Name Por WP FC Ks Kslp Tslp ETu ETs Seep Ebot Egw Umc (Epat)      

 

## Parameters:

  ------ ---------------------------
  Name   name assigned to aquifer.
  ------ ---------------------------

  ----- ---------------------------------------------------
  Por   soil porosity (pore space volume / total volume).
  ----- ---------------------------------------------------

  ---- --------------------------------------------------------------
  WP   soil wilting point (moisture content of a fully dried soil).
  ---- --------------------------------------------------------------

  ---- -----------------------------------------------------------------
  FC   soil field capacity (moisture content of a fully drained soil).
  ---- -----------------------------------------------------------------

  ---- ----------------------------------------------------
  Ks   saturated hydraulic conductivity (in/hr or mm/hr).
  ---- ----------------------------------------------------

  ------ -----------------------------------------------------------------------------------------------------------------------------------
  Kslp   slope of the logarithm of hydraulic conductivity versus moisture deficit (porosity minus moisture content) curve (dimensionless).
  ------ -----------------------------------------------------------------------------------------------------------------------------------

  ------ ---------------------------------------------------------------------
  Tslp   slope of soil tension versus moisture content curve (inches or mm).
  ------ ---------------------------------------------------------------------

  ----- -----------------------------------------------------------------------------------------------
  ETu   fraction of total evaporation available for evapotranspiration in the upper unsaturated zone.
  ----- -----------------------------------------------------------------------------------------------

  ----- ------------------------------------------------------------------------------------------------
  ETs   maximum depth into the lower saturated zone over which evapotranspiration can occur (ft or m).
  ----- ------------------------------------------------------------------------------------------------

  ------ --------------------------------------------------------------------------------------------------------------
  Seep   seepage rate from saturated zone to deep groundwater when water table is at ground surface (in/hr or mm/hr).
  ------ --------------------------------------------------------------------------------------------------------------

  ------ ---------------------------------------------------
  Ebot   elevation of the bottom of the aquifer (ft or m).
  ------ ---------------------------------------------------

  ----- ---------------------------------------------------------------
  Egw   groundwater table elevation at start of simulation (ft or m).
  ----- ---------------------------------------------------------------

  ----- ---------------------------------------------------------------------------------
  Umc   unsaturated zone moisture content at start of simulation (volumetric fraction).
  ----- ---------------------------------------------------------------------------------

  ------ ----------------------------------------------------------------------------------------------------------------------------
  Epat   name of optional monthly time pattern used to adjust the upper zone evaporation fraction for different months of the year.
  ------ ----------------------------------------------------------------------------------------------------------------------------

## Remarks:

Local values for Ebot, Egw, and Umc can be assigned to specific
subcatchments in the [GROUNDWATER] section.

<!---
  groundwater
-->
@page groundwater  [GROUNDWATER]


## Purpose:

Supplies parameters that determine the rate of groundwater flow between
the aquifer underneath a subcatchment and a node of the conveyance
system.

 

## Format:

Subcat Aquifer Node Esurf A1 B1 A2 B2 A3 Dsw (Egwt Ebot Egw Umc)

 

## Parameters:

  -------- --------------------
  Subcat   subcatchment name.
  -------- --------------------

  --------- ----------------------------------------------------------
  Aquifer   name of groundwater aquifer underneath the subcatchment.
  --------- ----------------------------------------------------------

  ------ ----------------------------------------------------------------------------------
  Node   name of a node in the conveyance system exchanging groundwater with the aquifer.
  ------ ----------------------------------------------------------------------------------

  ------- --------------------------------------------------
  Esurf   surface elevation of the subcatchment (ft or m).
  ------- --------------------------------------------------

  ---- -------------------------------------------
  A1   groundwater flow coefficient (see below).
  ---- -------------------------------------------

  ---- ----------------------------------------
  B1   groundwater flow exponent (see below).
  ---- ----------------------------------------

  ---- ---------------------------------------------
  A2   surface water flow coefficient (see below).
  ---- ---------------------------------------------

  ---- ------------------------------------------
  B2   surface water flow exponent (see below).
  ---- ------------------------------------------

  ---- ------------------------------------------------------------------
  A3   surface water – groundwater interaction coefficient (see below).
  ---- ------------------------------------------------------------------

  ----- ------------------------------------------------------------------------------------------------------------------------------------------
  Dsw   fixed depth of surface water at the receiving node (ft or m) (set to zero if surface water depth will vary as computed by flow routing).
  ----- ------------------------------------------------------------------------------------------------------------------------------------------

  ------ -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Egwt   threshold groundwater table elevation which must be reached before any flow occurs (ft or m). Leave blank (or enter *) to use the elevation of the receiving node's invert.
  ------ -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

The following optional parameters can be used to override the values
supplied for the subcatchment’s aquifer.

  ------ ---------------------------------------------------
  Ebot   elevation of the bottom of the aquifer (ft or m).
  ------ ---------------------------------------------------

  ----- -----------------------------------------------------------------------
  Egw   groundwater table elevation at the start of the simulation (ft or m).
  ----- -----------------------------------------------------------------------

  ----- ---------------------------------------------------------------------------------
  Umc   unsaturated zone moisture content at start of simulation (volumetric fraction).
  ----- ---------------------------------------------------------------------------------

 

## Remarks:

The flow coefficients are used in the following equation that determines
the lateral groundwater flow rate based on groundwater and surface water
elevations:

 QL  =  A1 (Hgw  –  Hcb) B1  –  A2 (Hsw  –  Hcb) B2  +  A3 Hgw Hsw

where:

QL   =        lateral groundwater flow (cfs per acre or cms per
hectare),

Hgw   =        height of saturated zone above the bottom of the aquifer
(ft or m),

Hsw  =        height of surface water at the receiving node above the
aquifer bottom (ft or m),

Hcb   =        height of the channel bottom above the aquifer bottom (ft
or m).

<!---
  gwf
-->

@page gwf  [GWF]


## Purpose:

Defines custom groundwater flow equations for specific subcatchments.

 

## Format:

Subcat LATERAL/DEEP  Expr

 

## Parameters:

  -------- --------------------
  Subcat   subcatchment name.
  -------- --------------------

  ------ ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Expr   a math formula expressing the rate of groundwater flow (in cfs per acre or cms per hectare for lateral flow or in/hr or mm/hr for deep flow) as a function of the following variables:
  ------ ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  ----------------------------------------------------------------------------------------------- --------------------------------------------------------------------------
  Hgw                                                                                             (for height of the groundwater table)
  Hsw                                                                                             (for height of the surface water)
  Hcb                                                                                             (for height of the channel bottom)
  Hgs                                                                                             (for height of ground surface)
  where all heights are relative to the aquifer bottom and have units of either feet or meters;
  Ks                                                                                              (for saturated hydraulic conductivity in in/hr or mm/hr)
  K                                                                                               (for unsaturated hydraulic conductivity in in/hr or mm/hr)
  Theta                                                                                           (for moisture content of the unsaturated zone)
  Phi                                                                                             (for aquifer soil porosity)
  Fi                                                                                              (for infiltration rate from the ground surface in in/hr or mm/hr)
  Fu                                                                                              (for percolation rate from the upper unsaturated zone in in/hr or mm/hr)
  A                                                                                               (for subcatchment area in acres or hectares)
  ----------------------------------------------------------------------------------------------- --------------------------------------------------------------------------

## Remarks:

Use LATERAL to designate an expression for lateral groundwater flow (to
a node of the conveyance network) and DEEP for vertical loss to deep
groundwater.

See the [TREATMENT] section for a list of built-in math functions that
can be used in Expr. In particular, the STEP(x) function is 1 when x > 0
and is 0 otherwise.

 

## Examples:

;Two-stage linear reservoir for lateral flow

Subcatch1 LATERAL 0.001*Hgw + 0.05*(Hgw–5)*STEP(Hgw–5)

 

;Constant seepage rate to deep aquifer

Subactch1  DEEP  0.002

<!---
  snowpacks
-->
@page snowpacks  [SNOWPACKS]


## Purpose:

Specifies parameters that govern how snowfall accumulates and melts on
the plowable, impervious and pervious surfaces of subcatchments.

 

## Formats:

  ------ ------------ --------------------------------------------
  Name   PLOWABLE     Cmin  Cmax  Tbase  FWF  SD0  FW0  SNN0
  Name   IMPERVIOUS   Cmin  Cmax  Tbase  FWF  SD0  FW0  SD100
  Name   PERVIOUS     Cmin  Cmax  Tbase  FWF  SD0  FW0  SD100
  Name   REMOVAL      Dplow Fout Fimp Fperv Fimelt (Fsub Scatch)
  ------ ------------ --------------------------------------------

## Parameters:

  ------ -------------------------------------------
  Name   name assigned to snowpack parameter set .
  ------ -------------------------------------------

  ------ --------------------------------------------------------
  Cmin   minimum melt coefficient (in/hr-deg F or mm/hr-deg C).
  ------ --------------------------------------------------------

  ------ --------------------------------------------------------
  Cmax   maximum melt coefficient (in/hr-deg F or mm/hr-deg C).
  ------ --------------------------------------------------------

  ------- ----------------------------------------------
  Tbase   snow melt base temperature (deg F or deg C).
  ------- ----------------------------------------------

  ----- ----------------------------------------------------------------
  FWF   ratio of free water holding capacity to snow depth (fraction).
  ----- ----------------------------------------------------------------

  ----- -------------------------------------------------
  SD0   initial snow depth (in or mm water equivalent).
  ----- -------------------------------------------------

  ----- ----------------------------------------
  FW0   initial free water in pack (in or mm).
  ----- ----------------------------------------

  ------ -------------------------------------------------
  SNN0   fraction of impervious area that can be plowed.
  ------ -------------------------------------------------

  ------- -------------------------------------------------------------------------
  SD100   snow depth above which there is 100% cover (in or mm water equivalent).
  ------- -------------------------------------------------------------------------

  ------- --------------------------------------------------------------------------
  Dplow   depth of snow on plowable areas at which snow removal begins (in or mm).
  ------- --------------------------------------------------------------------------

  ------ -----------------------------------------------------------------
  Fout   fraction of snow on plowable area transferred out of watershed.
  ------ -----------------------------------------------------------------

  ------ ------------------------------------------------------------------------------
  Fimp   fraction of snow on plowable area transferred to impervious area by plowing.
  ------ ------------------------------------------------------------------------------

  ------- ----------------------------------------------------------------------------
  Fperv   fraction of snow on plowable area transferred to pervious area by plowing.
  ------- ----------------------------------------------------------------------------

  -------- ------------------------------------------------------------------
  Fimelt   fraction of snow on plowable area converted into immediate melt.
  -------- ------------------------------------------------------------------

  ------ -----------------------------------------------------------------------------------------
  Fsub   fraction of snow on plowable area transferred to pervious area in another subcatchment.
  ------ -----------------------------------------------------------------------------------------

  -------- -----------------------------------------------------------------------
  Scatch   name of subcatchment receiving the Fsub fraction of transferred snow.
  -------- -----------------------------------------------------------------------

 

 

## Remarks:

Use one set of PLOWABLE, IMPERVIOUS, and PERVIOUS lines for each snow
pack parameter set created. Snow pack parameter sets are assigned to
specific subcatchments in the [SUBCATCHMENTS] section. Multiple
subcatchments can share the same set of snow pack parameters.

The PLOWABLE line contains parameters for the impervious area of a
subcatchment that is subject to snow removal by plowing but not to areal
depletion. This area is the fraction SNN0 of the total impervious area.
The IMPERVIOUS line contains parameter values for the remaining
impervious area and the PERVIOUS line does the same for the entire
pervious area. Both of the latter two areas are subject to areal
depletion.

The REMOVAL line describes how snow removed from the plowable area is
transferred onto other areas. The various transfer fractions should sum
to no more than 1.0. If the line is omitted then no snow removal takes
place.

<!---
  junctions
-->

@page junctions  [JUNCTIONS]


## Purpose:

Identifies each junction node of the drainage system.  Junctions are
points in space where channels and pipes connect together. For sewer
systems they can be either connection fittings or manholes.

 

## Format:

Name  Elev  (Ymax  Y0  Ysur  Apond)

 

## Parameters:

  ------ ---------------------------------
  Name   name assigned to junction node.
  ------ ---------------------------------

  ------ -----------------------------------------------
  Elev   elevation of the junction’s invert (ft or m).
  ------ -----------------------------------------------

  ------ -----------------------------------------------------------------
  Ymax   depth from ground to invert elevation (ft or m) (default is 0).
  ------ -----------------------------------------------------------------

  ---- ----------------------------------------------------------------------
  Y0   water depth at the start of the simulation (ft or m) (default is 0).
  ---- ----------------------------------------------------------------------

  ------ ------------------------------------------------------------------------------------------------------------------------------------------------
  Ysur   maximum additional pressure head above the ground elevation that the junction can sustain under surcharge conditions (ft or m) (default is 0).
  ------ ------------------------------------------------------------------------------------------------------------------------------------------------

  ------- ------------------------------------------------------------------------------------
  Apond   area subjected to surface ponding once water depth exceeds Ymax + Ysur (ft2 or m2)
  ------- ------------------------------------------------------------------------------------

(default is 0).

 

## Remarks:

If Ymax is 0 then SWMM sets the junction’s maximum depth to the distance
from its invert to the top of the highest connecting link.

If the junction is part of a force main section of the system then set
Ysur to the maximum pressure that the system can sustain.

Surface ponding can only occur when Apond is non-zero and the
ALLOW_PONDING analysis option is turned on.

<!---
  outfalls
-->
@page outfalls  [OUTFALLS]


## Purpose:

Identifies each outfall node (i.e., final downstream boundary) of the
drainage system and the corresponding water stage elevation.  Only one
link can be incident on an outfall node.

 

## Formats:

+-----------+-----------+-----------+-----------+-----------+-----------+
| Name      | Elev      | FREE      | (Gated)   | (RouteTo) |           |
+-----------+-----------+-----------+-----------+-----------+-----------+
| Name      | Elev      | NORMAL    | (Gated)   | (RouteTo) |           |
+-----------+-----------+-----------+-----------+-----------+-----------+
| Name      | Elev      | FIXED     | Stage     | (Gated)   | (RouteTo) |
+-----------+-----------+-----------+-----------+-----------+-----------+
| Name      | Elev      | TIDAL     | Tcurve    | (Gated)   | (RouteTo) |
+-----------+-----------+-----------+-----------+-----------+-----------+
| Name      | Elev      | T         | Tseries   | (Gated)   | (RouteTo) |
|           |           | IMESERIES |           |           |           |
+-----------+-----------+-----------+-----------+-----------+-----------+

## Parameters:

  ------ --------------------------------
  Name   name assigned to outfall node.
  ------ --------------------------------

  ------ ------------------------------------
  Elev   node’s invert elevation (ft or m).
  ------ ------------------------------------

  ------- -----------------------------------------------
  Stage   elevation of a fixed stage outfall (ft or m).
  ------- -----------------------------------------------

  -------- -----------------------------------------------------------------------------------------------------------------------------------
  Tcurve   name of a curve in the [CURVES] section containing tidal height (i.e., outfall stage) v. hour of day over a complete tidal cycle.
  -------- -----------------------------------------------------------------------------------------------------------------------------------

  --------- --------------------------------------------------------------------------------------------------
  Tseries   name of a time series in [TIMESERIES] section that describes how outfall stage varies with time.
  --------- --------------------------------------------------------------------------------------------------

  ------- ------------------------------------------------------------------------------------------------------
  Gated   YES or NO depending on whether a flap gate is present that prevents reverse flow. The default is NO.
  ------- ------------------------------------------------------------------------------------------------------

  --------- -----------------------------------------------------------------------------------------------------------------------------
  RouteTo   optional name of a subcatchment that receives the outfall's discharge. The default is not to route the outfall’s discharge.
  --------- -----------------------------------------------------------------------------------------------------------------------------

  ------------ ----------------------------------------
@page dividers  [DIVIDERS]
  ------------ ----------------------------------------

## Purpose:

Identifies each flow divider node of the drainage system. Flow dividers
are junctions with exactly two outflow conduits where the total outflow
is divided between the two in a prescribed manner.

 

## Formats:

  ------ ------ --------- ---------- ---------------------------------
  Name   Elev   DivLink   OVERFLOW   (Ymax Y0 Ysur Apond)
  Name   Elev   DivLink   CUTOFF     Qmin (Ymax Y0 Ysur Apond)
  Name   Elev   DivLink   TABULAR    Dcurve (Ymax Y0 Ysur Apond)
  Name   Elev   DivLink   WEIR       Qmin Ht Cd (Ymax Y0 Ysur Apond)
  ------ ------ --------- ---------- ---------------------------------

## Parameters:

  ------ --------------------------------
  Name   name assigned to divider node.
  ------ --------------------------------

  ------ ------------------------------------
  Elev   node’s invert elevation (ft or m).
  ------ ------------------------------------

  --------- ---------------------------------------------
  DivLink   name of the link to which flow is diverted.
  --------- ---------------------------------------------

  ------ ----------------------------------------------------------------------------------
  Qmin   flow at which diversion begins for either a CUTOFF or WEIR divider (flow units).
  ------ ----------------------------------------------------------------------------------

  -------- ---------------------------------------------------------------------------------
  Dcurve   name of a curve for a TABULAR divider that relates diverted flow to total flow.
  -------- ---------------------------------------------------------------------------------

  ---- -------------------------------------
  Ht   height of a WEIR divider (ft or m).
  ---- -------------------------------------

  ---- -------------------------------------------
  Cd   discharge coefficient for a WEIR divider.
  ---- -------------------------------------------

  ------ --------------------------------------------------------------------------------
  Ymax   depth from the ground to the node’s invert elevation (ft or m) (default is 0).
  ------ --------------------------------------------------------------------------------

  ---- ----------------------------------------------------------------------
  Y0   water depth at the start of the simulation (ft or m) (default is 0).
  ---- ----------------------------------------------------------------------

  ------ --------------------------------------------------------------------------------------------------------------------------------------------
  Ysur   maximum additional pressure head above the ground elevation that the node can sustain under surcharge conditions (ft or m) (default is 0).
  ------ --------------------------------------------------------------------------------------------------------------------------------------------

  ------- -----------------------------------------------------------------------------------------------------
  Apond   area subjected to surface ponding once water depth exceeds Ymax + Ysur  (ft2 or m2) (default is 0).
  ------- -----------------------------------------------------------------------------------------------------

<!---
  storage
-->

@page storage  [STORAGE]


## Purpose:

Identifies each storage node of the drainage system. Storage nodes can
have any shape as specified by a surface area versus water depth
relation.

 

## Formats:

  ------ ------ ------ ---- ------------ ------------------------------------
  Name   Elev   Ymax   Y0   TABULAR      Acurve   (Ysur Fevap Psi Ksat IMD)
  Name   Elev   Ymax   Y0   FUNCTIONAL   A1 A2 A0 (Ysur Fevap Psi Ksat IMD)
  Name   Elev   Ymax   Y0   Shape        L  W  Z  (Ysur Fevap Psi Ksat IMD)
  ------ ------ ------ ---- ------------ ------------------------------------

## Parameters:

  ------ --------------------------------
  Name   name assigned to storage node.
  ------ --------------------------------

  ------ ------------------------------------
  Elev   node’s invert elevation (ft or m).
  ------ ------------------------------------

  ------ ------------------------------------------------------
  Ymax   water depth when the storage node is full (ft or m).
  ------ ------------------------------------------------------

  ---- -------------------------------------------------------
  Y0   water depth at the start of the simulation (ft or m).
  ---- -------------------------------------------------------

  -------- ------------------------------------------------------------------------------------------------------------------------
  Acurve   name of a curve in the [CURVES] section that relates surface area (ft2 or m2) to depth (ft or m) for TABULAR geometry.
  -------- ------------------------------------------------------------------------------------------------------------------------

  ---- ----------------------------------------------------------------------
  A1   coefficient of a FUNCTIONAL relation between surface area and depth.
  ---- ----------------------------------------------------------------------

  ---- -------------------------------------------------------------------
  A2   exponent of a FUNCTIONAL relation between surface area and depth.
  ---- -------------------------------------------------------------------

  ---- -------------------------------------------------------------------
  A0   constant of a FUNCTIONAL relation between surface area and depth.
  ---- -------------------------------------------------------------------

  ------- ---------------------------------------------------------------------------------------------------------
  Shape   shape used to relate surface area to depth; choices are CYLINDRICAL, CONICAL, PARABOLOID, or PYRAMIDAL.
  ------- ---------------------------------------------------------------------------------------------------------

  ------ -----------------------------------------------------------------------------------------------------------------------------------------------
  Ysur   maximum additional pressure head above full depth that a closed storage unit can sustain under surcharge conditions (ft or m) (default is 0).
  ------ -----------------------------------------------------------------------------------------------------------------------------------------------

  --------- -----------------------------------------------------------
  L, W, Z   dimensions of the storage unit's shape (see table below).
  --------- -----------------------------------------------------------

  ------- --------------------------------------------------------------------------------------------------
  Fevap   fraction of potential evaporation from the storage unit’s water surface realized (default is 0).
  ------- --------------------------------------------------------------------------------------------------

Optional seepage parameters for soil surrounding the storage unit:

  ----- ------------------------------
  Psi   suction head (inches or mm).
  ----- ------------------------------

  ------ -----------------------------------------------------
  Ksat   saturated hydraulic conductivity  (in/hr or mm/hr).
  ------ -----------------------------------------------------

  ----- ------------------------------------------------------------------------
  IMD   initial moisture deficit (porosity minus moisture content) (fraction).
  ----- ------------------------------------------------------------------------

## Remarks:

A1, A2, and A0 are used in the following expression that relates surface
area (ft2 or m2) to water depth (ft or m) for a storage unit with
FUNCTIONAL geometry:

[]

For TABULAR geometry, the surface area curve will be extrapolated
outwards to meet the unit's maximum depth if need be.

The dimensions of storage units with other shapes are defined as
follows:

+-----------------+-----------------+-----------------+-----------------+
| Shape           | L               | W               | Z               |
+=================+=================+=================+=================+
| CYLINDRICAL     | major axis      | minor axis      | not used        |
|                 | length          | width           |                 |
| []              |                 |                 |                 |
+-----------------+-----------------+-----------------+-----------------+
| CONICAL         | major axis      | minor axis      | side slope      |
|                 | length of base  | width of base   | (run/rise)      |
|     []          |                 |                 |                 |
+-----------------+-----------------+-----------------+-----------------+
| PARABOLOID      | major axis      | minor axis      | full height     |
|                 | length at full  | width at full   |                 |
| []              | height          | height          |                 |
+-----------------+-----------------+-----------------+-----------------+
| PYRAMIDAL       | base length     | base width      | side slope      |
|                 |                 |                 | (run/rise)      |
|       []        |                 |                 |                 |
+-----------------+-----------------+-----------------+-----------------+

 

The parameters Psi, Ksat, and IMD need only be supplied if seepage loss
through the soil at the bottom and sloped sides of the storage unit
should be considered. They are the same Green-Ampt infiltration
parameters described in the [INFILTRATION] section. If Ksat is zero then
no seepage occurs while if IMD is zero then seepage occurs at a constant
rate equal to Ksat. Otherwise seepage rate will vary with storage depth.

<!---
  conduits
-->
@page conduits  [CONDUITS]


## Purpose:

Identifies each conduit link of the drainage system. Conduits are pipes
or channels that convey water from one node to another.

 

## Format:

Name  Node1  Node2  Length  N  Z1  Z2  (Q0  Qmax)

 

## Parameters:

  ------ --------------------------------
  Name   name assigned to conduit link.
  ------ --------------------------------

  ------- --------------------------------------
  Node1   name of the conduit’s upstream node.
  ------- --------------------------------------

  ------- ----------------------------------------
  Node2   name of the conduit’s downstream node.
  ------- ----------------------------------------

  -------- ---------------------------
  Length   conduit length (ft or m).
  -------- ---------------------------

  --- --------------------------------------
  N   Manning’s roughness coefficient (n).
  --- --------------------------------------

  ---- ---------------------------------------------------------------------------------------
  Z1   offset of the conduit’s upstream end above the invert of its upstream node (ft or m).
  ---- ---------------------------------------------------------------------------------------

  ---- -------------------------------------------------------------------------------------------
  Z2   offset of the conduit’s downstream end above the invert of its downstream node (ft or m).
  ---- -------------------------------------------------------------------------------------------

  ---- ---------------------------------------------------------------------------------
  Q0   flow in the conduit at the start of the simulation (flow units) (default is 0).
  ---- ---------------------------------------------------------------------------------

  ------ -------------------------------------------------------------------------
  Qmax   maximum flow allowed in the conduit (flow units) (default is no limit).
  ------ -------------------------------------------------------------------------

 

## Remarks:

The figure below illustrates the meaning of the Z1 and Z2 parameters.

[Link_offset]

These offsets are expressed as a relative distance above the node invert
if the LINK_OFFSETS option is set to DEPTH (the default) or as an
absolute elevation if it is set to ELEVATION.

<!---
  pumps
-->

@page pumps  [PUMPS]


## Purpose:

Identifies each pump link of the drainage system.

 

## Format:

Name  Node1  Node2  Pcurve  (Status  Startup  Shutoff)

 

## Parameters:

  ------ -----------------------------
  Name   name assigned to pump link.
  ------ -----------------------------

  ------- --------------------------------
  Node1   name of the pump’s inlet node.
  ------- --------------------------------

  ------- ---------------------------------
  Node2   name of the pump’s outlet node.
  ------- ---------------------------------

  -------- -------------------------------------------------------------------
  Pcurve   name of a pump curve listed in the [CURVES] section of the input.
  -------- -------------------------------------------------------------------

  -------- ---------------------------------------------------------------------------------
  Status   pump’s status at the start of the simulation (either ON or OFF; default is ON).
  -------- ---------------------------------------------------------------------------------

  --------- --------------------------------------------------------------------------
  Startup   depth at the inlet node when the pump turns on (ft or m) (default is 0).
  --------- --------------------------------------------------------------------------

  --------- -----------------------------------------------------------------------
  Shutoff   depth at inlet node when the pump shuts off (ft or m) (default is 0).
  --------- -----------------------------------------------------------------------

 

## Remarks:

A pump curve describes the relation between a pump's flow rate and
conditions at its inlet and outlet nodes. The following types of pump
curves are supported:

  ------- -----------------------------------------------------------------------------------------------------
  Type1   An off-line pump with a wet well where flow increases incrementally with available wet well volume.
  ------- -----------------------------------------------------------------------------------------------------

  ------- ---------------------------------------------------------------------------
  Type2   An in-line pump where flow increases incrementally with inlet node depth.
  ------- ---------------------------------------------------------------------------

  ------- ---------------------------------------------------------------------------------------------------------
  Type3   An in-line pump where flow varies continuously with head difference between the inlet and outlet nodes.
  ------- ---------------------------------------------------------------------------------------------------------

  ------- -------------------------------------------------------------------------------------
  Type4   A variable speed in-line pump where flow varies continuously with inlet node depth.
  ------- -------------------------------------------------------------------------------------

  ------- -----------------------------------------------------------------------------------------------------------------------
  Type5   A variable speed version of the Type3 pump where the head v. flow curve shifts position as the speed setting changes.
  ------- -----------------------------------------------------------------------------------------------------------------------

  ------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Ideal   A transfer pump that does not require a pump curve and is used mainly for preliminary design. Its flow rate equals the inflow rate to its inlet node no matter what the head difference is between its inlet and outlet nodes. Use an asterisk (*) as the value for Pcurve.
  ------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

 <!---
  orifices
-->

@page orifices  [ORIFICES]


## Purpose:

Identifies each orifice link of the drainage system. An orifice link
serves to limit the flow exiting a node and is often used to model flow
diversions and storage node outlets.

 

## Format:

Name  Node1  Node2  Type  Offset  Cd  (Gated  Orate)

 

## Parameters:

  ------ --------------------------------
  Name   name assigned to orifice link.
  ------ --------------------------------

  ------- -----------------------------------
  Node1   name of the orifice’s inlet node.
  ------- -----------------------------------

  ------- ------------------------------------
  Node2   name of the orifice’s outlet node.
  ------- ------------------------------------

  ------ ----------------------------------------------------------------------------------------------------------------
  Type   the type of orifice - either SIDE if oriented in a vertical plane or BOTTOM if oriented in a horizontal plane.
  ------ ----------------------------------------------------------------------------------------------------------------

  -------- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Offset   amount that a Side Orifice’s bottom or the position of a Bottom Orifice is offset above the invert of inlet node (ft or m, expressed as either a depth or as an elevation, depending on the LINK_OFFSETS option setting).
  -------- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  ---- -----------------------------------
  Cd   discharge coefficient (unitless).
  ---- -----------------------------------

  ------ ----------------------------------------------------------------------
  Flap   YES if a flap gate prevents reverse flow, NO if not (default is NO).
  ------ ----------------------------------------------------------------------

  ------- ----------------------------------------------------------------------------------------------------------------------------------------
  Orate   time in decimal hours to open a fully closed orifice (or close a fully open one). Use 0 if the orifice can open/close instantaneously.
  ------- ----------------------------------------------------------------------------------------------------------------------------------------

 

## Remarks:

The geometry of an orifice’s opening must be described in the
[XSECTIONS] section. The only allowable shapes are CIRCULAR and
RECT_CLOSED (closed rectangular).

 

[]

 

 <!---
  weirs
-->

 @page weirs [WEIRS]


## Purpose:

Identifies each weir link of the drainage system. Weirs are used to
model flow diversions and storage node outlets.

 

## Format:        

Name Node1 Node2 Type CrstHt Cd (Gated EC Cd2 Sur (Width Surf))

 

## Parameters:

  ------ -----------------------------
  Name   name assigned to weir link.
  ------ -----------------------------

  ------- --------------------------------
  Node1   name of the weir’s inlet node.
  ------- --------------------------------

  ------- ---------------------------------
  Node2   name of the weir’s outlet node.
  ------- ---------------------------------

  ------ --------------------------------------------------------
  Type   TRANSVERSE, SIDEFLOW, V-NOTCH, TRAPEZOIDAL or ROADWAY.
  ------ --------------------------------------------------------

  -------- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  CrstHt   amount that the weir’s opening is offset above the invert of inlet node (ft or m, expressed as either a depth or as an elevation, depending on the LINK_OFFSETS option setting).
  -------- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  ---- ------------------------------------------------------------------------------------------------
  Cd   weir discharge coefficient (for CFS if using US flow units or CMS if using metric flow units).
  ---- ------------------------------------------------------------------------------------------------

  ------- ----------------------------------------------------------------------
  Gated   YES if a flap gate prevents reverse flow, NO if not (default is NO).
  ------- ----------------------------------------------------------------------

  ---- ---------------------------------------------------------------------------------
  EC   number of end contractions for a TRANSVERSE or TRAPEZOIDAL weir (default is 0).
  ---- ---------------------------------------------------------------------------------

  ----- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Cd2   discharge coefficient for the triangular ends of a TRAPEZOIDAL weir (for CFS if using US flow units or CMS if using metric flow units) (default is the value of Cd).
  ----- ----------------------------------------------------------------------------------------------------------------------------------------------------------------------

  ----- ----------------------------------------------------------------------------------------------------------------------------------------------
  Sur   YES if the weir can surcharge (have an upstream water level higher than the height of the weir’s opening); NO if it cannot (default is YES).
  ----- ----------------------------------------------------------------------------------------------------------------------------------------------

The following parameters apply only to ROADWAY weirs:

  ------- -----------------------------------------------------------------
  Width   width of road lanes and shoulders for a ROADWAY weir (ft or m).
  ------- -----------------------------------------------------------------

  ------ -----------------------------------------------------------
  Surf   type of road surface for a ROADWAY weir: PAVED or GRAVEL.
  ------ -----------------------------------------------------------

 

## Remarks:

The geometry of a weir’s opening is described in the [XSECTIONS]
section. The following shapes must be used with each type of weir:

 

  ------------- ---------------------
  Weir Type     Cross-Section Shape
  Transverse    RECT_OPEN
  Sideflow      RECT_OPEN
  V-Notch       TRIANGULAR
  Trapezoidal   TRAPEZOIDAL
  Roadway       RECT_OPEN
  ------------- ---------------------

 

The ROADWAY weir is a broad crested rectangular weir used model roadway
crossings usually in conjunction with culvert-type conduits. It uses the
FHWA HDS-5 method to determine a discharge coefficient as a function of
flow depth and roadway width and surface. If no roadway data are
provided then the weir behaves as a TRANSVERSE weir with Cd as its
discharge coefficient. Note that if roadway data are provided, then
values for the other optional weir parameters (NO for Gated, 0 for EC, 0
for Cd2, and NO for Sur) must be entered even though they do not apply
to ROADWAY weirs.

<!---
outlets
-->
----------- ----------------------------------------
@page outlets  [OUTLETS]
  ----------- ----------------------------------------

## Purpose:

Identifies each outlet flow control device of the drainage system. These
are devices used to model outflows from storage units or flow diversions
that have a user-defined relation between flow rate and water depth.

 

## Formats:

  ------ ------- ------- -------- ------------------ ----------------
  Name   Node1   Node2   Offset   TABULAR/DEPTH      Qcurve (Gated)
  Name   Node1   Node2   Offset   TABULAR/HEAD       Qcurve (Gated)
  Name   Node1   Node2   Offset   FUNCTIONAL/DEPTH   C1 C2 (Gated)
  Name   Node1   Node2   Offset   FUNCTIONAL/HEAD    C1 C2 (Gated)
  ------ ------- ------- -------- ------------------ ----------------

## Parameters:

  ------ -------------------------------
  Name   name assigned to outlet link.
  ------ -------------------------------

  ------- ----------------------------------
  Node1   name of the outlet’s inlet node.
  ------- ----------------------------------

  ------- -----------------------------------
  Node2   name of the outlet’s outlet node.
  ------- -----------------------------------

  -------- ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Offset   amount that the outlet is offset above the invert of its inlet node (ft or m, expressed as either a depth or as an elevation, depending on the LINK_OFFSETS option setting).
  -------- ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  -------- --------------------------------------------------------------------------------------------------------------------
  Qcurve   name of the rating curve listed in the [CURVES] section that describes outflow rate (flow units) as a function of:
  -------- --------------------------------------------------------------------------------------------------------------------

  --- -----------------------------------------------------------------------------------------------
  §   water depth above the offset elevation at the inlet node (ft or m) for a TABULAR/DEPTH outlet
  --- -----------------------------------------------------------------------------------------------

  --- ------------------------------------------------------------------------------------------
  §   head difference (ft or m) between the inlet and outflow nodes for a TABULAR/HEAD outlet.
  --- ------------------------------------------------------------------------------------------

  -------- --------------------------------------------------------------------------------------
  C1, C2   coefficient and exponent, respectively, of a power function that relates outflow to:
  -------- --------------------------------------------------------------------------------------

  --- --------------------------------------------------------------------------------------------------
  §   water depth (ft or m) above the offset elevation at the inlet node for a FUNCTIONAL/DEPTH outlet
  --- --------------------------------------------------------------------------------------------------

  --- ----------------------------------------------------------------------------------------------
  §   head difference  (ft or m) between the inlet and outflow nodes for a FUNCTIONAL/HEAD outlet.
  --- ----------------------------------------------------------------------------------------------

  ------- ----------------------------------------------------------------------
  Gated   YES if a flap gate prevents reverse flow, NO if not (default is NO).
  ------- ----------------------------------------------------------------------

  <!---
    xsections
  -->

 @page xsections [XSECTIONS]


## Purpose:

Provides cross-section geometric data for conduit and regulator links of
the drainage system.

 

## Formats:

  ------ ----------- -------------------------------------------
  Link   Shape       Geom1 Geom2 Geom3 Geom4 (Barrels Culvert)
  Link   IRREGULAR   Tsect
  Link   STREET      Street
  ------ ----------- -------------------------------------------

## Parameters:

  ------ --------------------------------------
  Link   name of a conduit, orifice, or weir.
  ------ --------------------------------------

  ------- --------------------------------------------------------------------
  Shape   a cross-section shape (see Tables D-1 below for available shapes).
  ------- --------------------------------------------------------------------

  ------- ---------------------------------------------
  Geom1   full height of the cross-section (ft or m).
  ------- ---------------------------------------------

  --------- -------------------------------------------------------------------------
  Geom2-4   auxiliary parameters (width, side slopes, etc.) as listed in Table D-1.
  --------- -------------------------------------------------------------------------

  --------- ----------------------------------------------------------------------------------------------------------------------------------
  Barrels   number of barrels (i.e., number of parallel pipes of equal size, slope, and roughness) associated with a conduit (default is 1).
  --------- ----------------------------------------------------------------------------------------------------------------------------------

  --------- -------------------------------------------------------------------------------------------------------------------------------------------------
  Culvert   code number from Table A.10 for the conduit’s inlet geometry if it is a culvert subject to possible inlet flow control (leave blank otherwise).
  --------- -------------------------------------------------------------------------------------------------------------------------------------------------

  ------- -------------------------------------------------------------------------------------------------------
  Curve   name of a Shape Curve in the [CURVES] section that defines how cross-section width varies with depth.
  ------- -------------------------------------------------------------------------------------------------------

  ------- ----------------------------------------------------------------------------------------------------------------
  Tsect   name of an entry in the [TRANSECTS] section that describes the cross-section geometry of an irregular channel.
  ------- ----------------------------------------------------------------------------------------------------------------

  -------- --------------------------------------------------------------------------------------------------
  Street   name of an entry in the [STREETS] section that describes the cross-section geometry of a street.
  -------- --------------------------------------------------------------------------------------------------

 

## Remarks:

The standard conduit shapes and their geometric parameters are listed in
the following table:

  ------------------ ------------- ---------------- ----------------- --------------
  Shape              Geom1         Geom2            Geom3             Geom4
  CIRCULAR           Diameter                                          
  FORCE_MAIN         Diameter      Roughness1                          
  FILLED_CIRCULAR2   Diameter      Sediment Depth                      
  RECT_CLOSED        Full Height   Top Width                           
  RECT_OPEN          Full Height   Top Width                           
  TRAPEZOIDAL        Full Height   Base Width       Left  Slope3      Right Slope3
  TRIANGULAR         Full Height   Top Width                           
  HORIZ_ELLIPSE      Full Height   Max. Width       Size Code4         
  VERT_ELLIPSE       Full Height   Max. Width       Size Code4         
  ARCH               Full Height   Max. Width       Size Code5         
  PARABOLIC          Full Height   Top Width                           
  POWER              Full Height   Top Width        Exponent           
  RECT_TRIANGULAR    Full Height   Top Width        Triangle Height    
  RECT_ROUND         Full Height   Top Width        Bottom Radius      
  MODBASKETHANDLE    Full Height   Base Width       Top Radius6        
  EGG                Full Height                                       
  HORSESHOE          Full Height                                       
  GOTHIC             Full Height                                       
  CATENARY           Full Height                                       
  SEMIELLIPTICAL     Full Height                                       
  BASKETHANDLE       Full Height                                       
  SEMICIRCULAR       Full Height                                       
  CUSTOM             Full Height   Shape Curve                         
  ------------------ ------------- ---------------- ----------------- --------------

  ---- ----------------------------------------------------------------------------------------------------------------------------------------------------
  1.   C-factors are used when H-W is the FORCE_MAIN_EQUATION choice in the [OPTIONS] section while roughness heights (in inches or mm) are used for D-W.
  ---- ----------------------------------------------------------------------------------------------------------------------------------------------------

  ---- -------------------------------------------------------------------------
  2.   A circular conduit partially filled with sediment to a specified depth.
  ---- -------------------------------------------------------------------------

  ---- --------------------------------------------
  3.   Slopes are horizontal run / vertical rise.
  ---- --------------------------------------------

  ---- ---------------------------------------------------------------------------------------------------------
  4.   Size code of a standard shaped elliptical pipe. Leave blank (or 0) if the pipe has a custom dimensions.
  ---- ---------------------------------------------------------------------------------------------------------

  ---- -------------------------------------------------------------------------------------------
  5.   Size code of a standard arch pipe. Leave blank (or 0) if the pipe has custom dimensions).
  ---- -------------------------------------------------------------------------------------------

  ---- ----------------------------------------------------------------------------------------------------
  6.   Set to zero to use a standard modified baskethandle shape whose top radius is half the base width.
  ---- ----------------------------------------------------------------------------------------------------

 

Size codes for standard elliptical and arch pipes and for culverts can
be found in the SWMM Users Manual.

The CUSTOM shape is a closed conduit whose width versus height is
described by a user-supplied Shape Curve.

An IRREGULAR cross-section is used to model an open channel whose
geometry is described by a Transect object.

A STREET cross-section is used to model street conduits and inlet flow
capture (see the [INLETS] and [INLETS_USAGE] sections).

The Culvert code number is used only for closed conduits acting as
culverts that should be analyzed for inlet control conditions using the
FHWA HDS-5 methodology.

<!---
  transects
-->

@page transects  [TRANSECTS]


## Purpose:

Describes the cross-section geometry of natural channels or conduits
with irregular shapes following the HEC-2 data format.

 

## Formats:

  ---- ----------------------------------- -------- --------------------------------------------
  NC   Nleft                               Nright   Nchanl
  X1   Name                                Nsta     Xleft Xright 0 0 0 Lfactor Wfactor Eoffset
  GR   Elev  Station  ...  Elev  Station
  ---- ----------------------------------- -------- --------------------------------------------

## Parameters:

  ------- ----------------------------------------------------------------------------------------------------------------------
  Nleft   Manning’s roughness coefficient (n) of right overbank portion of channel (use 0 if no change from previous NC line).
  ------- ----------------------------------------------------------------------------------------------------------------------

  -------- ---------------------------------------------------------------------------------------------------------------------
  Nright   Manning’s roughness coefficient (n) of right overbank portion of channel (use 0 if no change from previous NC line.
  -------- ---------------------------------------------------------------------------------------------------------------------

  -------- -------------------------------------------------------------------------------------------------------------------
  Nchanl   Manning’s roughness coefficient (n) of main channel portion of channel (use 0 if no change from previous NC line.
  -------- -------------------------------------------------------------------------------------------------------------------

  ------ --------------------------------
  Name   name assigned to the transect.
  ------ --------------------------------

  ------ ------------------------------------------------------------------------------------------
  Nsta   number of stations across the cross-section’s width at which elevation data is supplied.
  ------ ------------------------------------------------------------------------------------------

  ------- ---------------------------------------------------------------------------------
  Xleft   station position which ends the left overbank portion of the channel (ft or m).
  ------- ---------------------------------------------------------------------------------

  -------- ------------------------------------------------------------------------------------
  Xright   station position which begins the right overbank portion of the channel (ft or m).
  -------- ------------------------------------------------------------------------------------

  --------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Lfactor   meander modifier that represents the ratio of the length of a meandering main channel to the length of the overbank area that surrounds it (use 0 if not applicable).
  --------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------

  --------- -------------------------------------------------------------------------------------------------------------------------------------------------
  Wfactor   factor by which distances between stations should be multiplied to increase (or decrease) the width of the channel (enter 0 if not applicable).
  --------- -------------------------------------------------------------------------------------------------------------------------------------------------

  --------- ----------------------------------------------------------------------------------
  Eoffset   amount to be added (or subtracted) from the elevation of each station (ft or m).
  --------- ----------------------------------------------------------------------------------

  ------ --------------------------------------------------------------------------------------------------------
  Elev   elevation of the channel bottom at a cross-section station relative to some fixed reference (ft or m).
  ------ --------------------------------------------------------------------------------------------------------

  --------- --------------------------------------------------------------------------
  Station   distance of a cross-section station from some fixed reference (ft or m).
  --------- --------------------------------------------------------------------------

 

 

## Remarks:

Transect geometry is described as shown below, assuming that one is
looking in a downstream direction:

[]

 

The first line in this section must always be a NC line. After that, the
NC line is only needed when a transect has different Manning’s n values
than the previous one.

The Manning’s n values on the NC line will supersede any roughness value
entered for the conduit which uses the irregular cross-section.

There should be one X1 line for each transect. Any number of GR lines
may follow, and each GR line can have any number of Elevation-Station
data pairs. (In HEC-2 the GR line is limited to 5 stations.)

The station that defines the left overbank boundary on the X1 line must
correspond to one of the station entries on the GR lines that follow.
The same holds true for the right overbank boundary. If there is no
match, a warning will be issued and the program will assume that no
overbank area exists.

The meander modifier is applied to all conduits that use this particular
transect for their cross section. It assumes that the length supplied
for these conduits is that of the longer main channel. SWMM will use the
shorter overbank length in its calculations while increasing the main
channel roughness to account for its longer length.

<!---
  streets
-->
----------- ----------------------------------------
@page streets  [STREETS]
  ----------- ----------------------------------------

## Purpose:

Describes the cross-section geometry of conduits that represent streets.

 

## Format:

Name Tcrown Hcurb Sx nRoad (a W)(Sides Tback Sback nBack)

 

## Parameters:

  ------ -------------------------------------------
  Name   name assigned to the street cross-section
  ------ -------------------------------------------

  -------- ----------------------------------------------------
  Tcrown   distance from street’s curb to its crown (ft or m)
  -------- ----------------------------------------------------

  ------- -----------------------
  Hcurb   curb height (ft or m)
  ------- -----------------------

  ---- ------------------------
  Sx   street cross slope (%)
  ---- ------------------------

  ------- ---------------------------------------------------------
  nRoad   Manning’s roughness coefficient (n) of the road surface
  ------- ---------------------------------------------------------

  --- ---------------------------------------------------
  a   gutter depression height (in or mm) (default = 0)
  --- ---------------------------------------------------

  --- ------------------------------------------------
  W   depressed gutter width (ft or m) (default = 0)
  --- ------------------------------------------------

  ------- -------------------------------------------------------------------
  Sides   1 for single sided street or 2 for two-sided street (default = 2)
  ------- -------------------------------------------------------------------

  ------- ----------------------------------------------
  Tback   street backing width (ft or m) (default = 0)
  ------- ----------------------------------------------

  ------- ----------------------------------------
  Sback   street backing slope (%) (default = 0)
  ------- ----------------------------------------

  ------- ------------------------------------------------------------------
  nBack   street backing Manning’s roughness coefficient (n) (default = 0)
  ------- ------------------------------------------------------------------

 

## Remarks:

[]

 

If the street has no depressed gutter (a = 0) then the gutter width
entry is ignored. If the street has no backing then the three backing
parameters can be omitted.

 <!---
  inlets
-->
  ---------- ----------------------------------------
@page inlets  [INLETS]
  ---------- ----------------------------------------

## Purpose:

Defines inlet structure designs used to capture street and channel flow
that are sent to below ground sewers.

 

## Format:

  ------ ------------------ -----------------------------------
  Name   GRATE/DROP_GRATE   Length Width Type (Aopen Vsplash)
  Name   CURB/DROP_CURB     Length Height (Throat)
  Name   SLOTTED            Length Width
  Name   CUSTOM             Dcurve/Rcurve
  ------ ------------------ -----------------------------------

## Parameters:

  ------ ---------------------------------------
  Name   name assigned to the inlet structure.
  ------ ---------------------------------------

  -------- ------------------------------------------------------------
  Length   length of the inlet parallel to the street curb (ft or m).
  -------- ------------------------------------------------------------

  ------- ----------------------------------------------
  Width   width of a GRATE or SLOTTED inlet (ft or m).
  ------- ----------------------------------------------

  -------- -------------------------------------------
  Height   height of a CURB opening inlet (ft or m).
  -------- -------------------------------------------

  ------ ---------------------------------
  Type   type of GRATE used (see below).
  ------ ---------------------------------

  ------- --------------------------------------------------
  Aopen   fraction of a GENERIC grate’s area that is open.
  ------- --------------------------------------------------

  --------- ---------------------------------------------------------
  Vsplash   splash over velocity for a GENERIC grate (ft/s or m/s).
  --------- ---------------------------------------------------------

  -------- ------------------------------------------------------------------------------
  Throat   the throat angle of a CURB opening inlet (HORIZONTAL, INCLINED or VERTICAL).
  -------- ------------------------------------------------------------------------------

  -------- -------------------------------------------------------------------------------------
  Dcurve   name of a Diversion-type curve (captured flow v. approach flow) for a CUSTOM inlet.
  -------- -------------------------------------------------------------------------------------

  -------- --------------------------------------------------------------------------------
  Rcurve   name of a Rating-type curve (captured flow v. water depth) for a CUSTOM inlet.
  -------- --------------------------------------------------------------------------------

 

## Remarks:

These are the different types of standard inlets that SWMM can model:

[]

Use one line for each inlet design except for a combination inlet where
one GRATE line describes its grated inlet and a second CURB line (with
the same inlet name) describes its curb opening inlet.

GRATE, CURB, and SLOTTED inlets are used with STREET conduits,
DROP_GRATE and DROP_CURB inlets with open channels, and a CUSTOM inlet
with any conduit.

GRATE and DROP_GRATE  types can be any of the following:

+-----------------------+-----------------------+-----------------------+
| Grate Type            | Sketch                | Description           |
+-----------------------+-----------------------+-----------------------+
| P_BAR-50              | []                    | Parallel bar grate    |
|                       |                       | with bar spacing      |
|                       |                       | 1-7/8-in on center    |
+-----------------------+-----------------------+-----------------------+
| P_BAR-50X100          | []                    | Parallel bar grate    |
|                       |                       | with bar spacing      |
|                       |                       | 1-7/8-in on center    |
|                       |                       | and 3/8-in diameter   |
|                       |                       | lateral rods spaced   |
|                       |                       | at 4-in on center     |
+-----------------------+-----------------------+-----------------------+
| P_BAR-30              | []                    | Parallel bar grate    |
|                       |                       | with 1-1/8-in on      |
|                       |                       | center bar spacing    |
+-----------------------+-----------------------+-----------------------+
| CURVED_VANE           | []                    | Curved vane grate     |
|                       |                       | with 3-1/4-in         |
|                       |                       | longitudinal bar and  |
|                       |                       | 4-1/4-in transverse   |
|                       |                       | bar spacing on center |
+-----------------------+-----------------------+-----------------------+
| TILT_BAR-45           | []                    | 45 degree tilt bar    |
|                       |                       | grate with 2-1/4-in   |
|                       |                       | longitudinal bar and  |
|                       |                       | 4-in transverse bar   |
|                       |                       | spacing on center     |
+-----------------------+-----------------------+-----------------------+
| TILT_BAR-30           | []                    | 30 degree tilt bar    |
|                       |                       | grate with 3-1/4-in   |
|                       |                       | and 4-in on center    |
|                       |                       | longitudinal and      |
|                       |                       | lateral bar spacing   |
|                       |                       | respectively          |
+-----------------------+-----------------------+-----------------------+
| RETICULINE            | []                    | "Honeycomb" pattern   |
|                       |                       | of lateral bars and   |
|                       |                       | longitudinal bearing  |
|                       |                       | bars                  |
+-----------------------+-----------------------+-----------------------+
| GENERIC               |                       | A generic grate       |
|                       |                       | design.               |
+-----------------------+-----------------------+-----------------------+

 

Only a GENERIC type grate requires that Aopen and Vsplash values be
provided. The other standard grate types have predetermined values of
these parameters. (Splash over velocity is the minimum velocity that
will cause some water to shoot over the inlet thus reducing its capture
efficiency).

A CUSTOM inlet takes the name of either a Diversion curve or a Rating
curve as its only parameter (see the [CURVES] section).  Diversion
curves are best suited for on-grade inlets and Rating curves for on-sag
inlets.

 

## Examples:

; A 2-ft x 2-ft parallel bar grate

InletType1  GRATE  2  2  P-BAR-30

; A combination inlet

InletType2  GRATE  2  2  CURVED_VANE

InletType2  CURB   4  0.5  HORIZONTAL

; A custom inlet using Curve1 as its capture curve

InletType3  CUSTOM  Curve1

<!---
  inlet_usage
-->

@page inlet_usage  [INLET_USAGE]


## Purpose:

Assigns inlet structures to specific street and open channel conduits.

 

## Format:

Conduit Inlet Node (Number %Clogged Qmax aLocal wLocal Placement)

 

## Parameters:

  --------- ----------------------------------------------------------------
  Conduit   name of a street or open channel conduit containing the inlet.
  --------- ----------------------------------------------------------------

  ------- ----------------------------------------------------------------
  Inlet   name of an inlet structure (from the [INLETS] section) to use.
  ------- ----------------------------------------------------------------

  ------ --------------------------------------------------------------
  Node   name of the sewer node receiving flow captured by the inlet.
  ------ --------------------------------------------------------------

  -------- ---------------------------------------------------------------
  Number   number of replicate inlets placed on each side of the street.
  -------- ---------------------------------------------------------------

  ---------- ----------------------------------------------------------------
  %Clogged   degree to which inlet capacity is reduced due to clogging (%).
  ---------- ----------------------------------------------------------------

  ------ -------------------------------------------------------
  Qmax   maximum flow that the inlet can capture (flow units).
  ------ -------------------------------------------------------

  -------- -----------------------------------------------
  aLocal   height of local gutter depression (in or mm).
  -------- -----------------------------------------------

  -------- ---------------------------------------------
  wLocal   width of local gutter depression (ft or m).
  -------- ---------------------------------------------

  ----------- ---------------------------------
  Placement   AUTOMATIC, ON_GRADE, or ON_SAG.
  ----------- ---------------------------------

 

## Remarks:

Only conduits with a STREET cross section can be assigned a curb and
gutter inlet while drop inlets can only be assigned to conduits with a
RECT_OPEN or TRAPEZOIDAL cross section.

Only the first three parameters are required. The default number of
inlets is 1 (for each side of a two-sided street) while the remaining
parameters have default values of 0.

A Qmax value of 0 indicates that the inlet has no flow restriction.

The local gutter depression applies only over the length of the inlet
unlike the continuous depression for a STREET cross section which exists
over the full curb length.

The default inlet placement is AUTOMATIC, meaning that the program uses
the network topography to determine whether an inlet operates on-grade
or on-sag. On-grade means the inlet is located on a continuous grade.
On-sag means the inlet is located at a sag or sump point where all
adjacent conduits slope towards the inlet leaving no place for water to
flow except into the inlet.

<!---
  losses
-->
@page losses  [LOSSES]


## Purpose:

Specifies minor head loss coefficients, flap gates, and seepage rates
for conduits.

 

## Format:

Conduit  Kentry  Kexit  Kavg  (Flap  Seepage)

 

## Parameters:

  --------- ---------------------------------------------------------------------------------------------
  Conduit   name of a conduit.
  Kentry    minor head loss coefficient at the conduit’s entrance.
  Kexit     minor head loss coefficient at the conduit’s exit.
  Kavg      average minor head loss coefficient across the length of the conduit.
  Flap      YES if the conduit has a flap valve that prevents back flow, NO otherwise. (Default is NO).
  Seepage   Rate of seepage loss into the surrounding soil (in/hr or mm/hr). (Default is 0.)
  --------- ---------------------------------------------------------------------------------------------

## Remarks:

Minor losses are only computed for the Dynamic Wave flow routing option
(see the [OPTIONS] section). They are computed as Kv2/2g where K = minor
loss coefficient, v = velocity, and g = acceleration of gravity.
Entrance losses are based on the velocity at the entrance of the
conduit, exit losses on the exit velocity, and average losses on the
average velocity.

Only enter data for conduits that actually have minor losses, flap
valves, or seepage losses.

<!---
  controls
-->

@page controls  [CONTROLS]


## Purpose:

Determines how pumps and regulators will be adjusted based on simulation
time or conditions at specific nodes and links.

 

## Formats:

Each control rule is a series of statements of the form:

RULE        ruleID

IF                condition_1

AND        condition_2

OR                condition_3

AND        condition_4

Etc.

THEN        action_1

AND        action_2

Etc.

ELSE        action_3

AND        action_4

Etc.

PRIORITY value

 

## Parameters:

  ------------- ------------------------------------------------
  ruleID        an ID label assigned to the rule.
  condition_n   a condition clause.
  action_n      an action clause.
  value         a priority value (e.g., a number from 1 to 5).
  ------------- ------------------------------------------------

## Remarks:

Keywords are shown in boldface and ruleID is an ID label assigned to the
rule, condition_n is a Condition Clause, action_n is an Action Clause,
and value is a priority value (e.g., a number from 1 to 5). The formats
used for Condition and Action clauses are discussed below.

 

Only the RULE, IF and THEN portions of a rule are required; the ELSE and
PRIORITY portions are optional.

 

Blank lines between clauses are permitted and any text to the right of a
semicolon is considered a comment.

 

When mixing AND and OR clauses, the OR operator has higher precedence
than AND, i.e.,

IF A or B and C

is equivalent to

IF (A or B) and C.

If the interpretation was meant to be

IF A or (B and C)

then this can be expressed using two rules as in

IF A THEN ...

IF B and C THEN ...

 

The PRIORITY value is used to determine which rule applies when two or
more rules require that conflicting actions be taken on a link. A
conflicting rule with a higher priority value has precedence over one
with a lower value (e.g., PRIORITY 5 outranks PRIORITY 1). A rule
without a priority value always has a lower priority than one with a
value. For two rules with the same priority value, the rule that appears
first is given the higher priority.

<!---
  pollutants
-->

@page pollutants  [POLLUTANTS]


## Purpose:

Identifies the pollutants being analyzed.

 

## Format:

Name Units Crain Cgw Cii Kd (Sflag CoPoll CoFract Cdwf Cinit)

 

## Parameters:

  ------ -------------------------------
  Name   name assigned to a pollutant.
  ------ -------------------------------

  ------- ------------------------------------------------------------------------------------------------------------------------
  Units   concentration units (MG/L for milligrams per liter, UG/L for micrograms per liter, or #/L for direct count per liter).
  ------- ------------------------------------------------------------------------------------------------------------------------

  ------- -------------------------------------------------------------------
  Crain   concentration of the pollutant in rainfall (concentration units).
  ------- -------------------------------------------------------------------

  ----- ----------------------------------------------------------------------
  Cgw   concentration of the pollutant in groundwater (concentration units).
  ----- ----------------------------------------------------------------------

  ----- ------------------------------------------------------------------------------
  Cii   concentration of the pollutant in inflow/infiltration (concentration units).
  ----- ------------------------------------------------------------------------------

  -------- -----------------------------------------
  Kdecay   first-order decay coefficient (1/days).
  -------- -----------------------------------------

  ------- ----------------------------------------------------------------------------------------------
  Sflag   YES if pollutant buildup occurs only when there is snow cover, NO otherwise (default is NO).
  ------- ----------------------------------------------------------------------------------------------

  -------- ------------------------------------------------------------------------
  CoPoll   name of a co-pollutant (default is no co-pollutant designated by a *).
  -------- ------------------------------------------------------------------------

  --------- --------------------------------------------------------------
  CoFract   fraction of the co-pollutant’s concentration (default is 0).
  --------- --------------------------------------------------------------

  ------ -------------------------------------------------------------
  Cdwf   pollutant concentration in dry weather flow (default is 0).
  ------ -------------------------------------------------------------

  ------- ---------------------------------------------------------------------------------------------------------
  Cinit   pollutant concentration throughout the conveyance system at the start of the simulation (default is 0).
  ------- ---------------------------------------------------------------------------------------------------------

## Remarks:

FLOW  is a reserved word and cannot be used to name a pollutant.

Parameters Sflag through Cinit can be omitted if they assume their
default values. If there is no co-pollutant but non-default values for
Cdwf or Cinit, then enter an asterisk (*) for the co-pollutant name.

When pollutant X has a co-pollutant Y, it means that fraction CoFract of
pollutant Y’s runoff concentration is added to pollutant X’s runoff
concentration when wash off from a subcatchment is computed.

The dry weather flow concentration can be overridden for any specific
node of the conveyance system by editing the node’s Inflows property
(see the [INFLOWS] section).

<!---
  landuses
-->

@page landuses  [LANDUSES]

## Purpose:

Identifies the various categories of land uses within the drainage area.
Each subcatchment area can be assigned a different mix of land uses.
Each land use can be subjected to a different street sweeping schedule.
Land uses are only used in conjunction with pollutant buildup and wash
off.

 

## Format:

Name  (SweepInterval  Availability  LastSweep)

 

## Parameters:

  ------ ----------------
  Name   land use name.
  ------ ----------------

  --------------- -------------------------------
  SweepInterval   days between street sweeping.
  --------------- -------------------------------

  -------------- -------------------------------------------------------------------------
  Availability   fraction of pollutant buildup available for removal by street sweeping.
  -------------- -------------------------------------------------------------------------

  ----------- ----------------------------------------------------------
  LastSweep   days since last sweeping at the start of the simulation.
  ----------- ----------------------------------------------------------

  <!---
    coverages
  -->

@page coverages  [COVERAGES]


## Purpose:

Specifies the percentage of a subcatchment’s area that is covered by
each category of land use.

 

## Format:

 Subcat  Landuse  Percent  Landuse  Percent  . . .

 

## Parameters:

  -------- --------------------
  Subcat   subcatchment name.
  -------- --------------------

  --------- ----------------
  Landuse   land use name.
  --------- ----------------

  --------- -------------------------------------------------------------
  Percent   percent of the subcatchment’s area covered by the land use.
  --------- -------------------------------------------------------------

 

## Remarks:

More than one pair of land use - percentage values can be entered per
line. If more than one line is needed, then the subcatchment name must
still be entered first on the succeeding lines.

If a land use does not pertain to a subcatchment, then it does not have
to be entered.

If no land uses are associated with a subcatchment then no pollutants
will appear in the runoff from the subcatchment.

<!---
  loadings
-->

@page loadings  [LOADINGS]


## Purpose:

Specifies the pollutant buildup that exists on each subcatchment at the
start of a simulation.

 

## Format:

Subcat  Pollut  InitBuildup  Pollut  InitBuildup ...

 

## Parameters:

  ------------- ------------------------------------------------------------
  Subcat        name of a subcatchment.
  Pollut        name of a pollutant.
  InitBuildup   initial buildup of the pollutant (lbs/acre or kg/hectare).
  ------------- ------------------------------------------------------------

## Remarks:

More than one pair of pollutant - buildup values can be entered per
line. If more than one line is needed, then the subcatchment name must
still be entered first on the succeeding lines.

If an initial buildup is not specified for a pollutant, then its initial
buildup is computed by applying the DRY_DAYS option (specified in the
[OPTIONS] section) to the pollutant’s buildup function for each land use
in the subcatchment.

<!---
  buildup
-->

@page buildup  [BUILDUP]


## Purpose:

Specifies the rate at which pollutants build up over different land uses
between rain events.

 

## Format:

Landuse  Pollutant  FuncType  C1  C2  C3  PerUnit

 

## Parameters:

  ----------- ---------------------------------------------------------------------
  Landuse     land use name.
  Pollutant   pollutant name.
  FuncType    buildup function type: ( POW / EXP / SAT / EXT ).
  C1,C2,C3    buildup function parameters (see table below).
  PerUnit     AREA if buildup is per unit area, CURBLENGTH if per length of curb.
  ----------- ---------------------------------------------------------------------

## Remarks:

Buildup is measured in pounds (kilograms) per unit of area (or curb
length) for pollutants whose concentration units are either mg/L or
ug/L. If the concentration units are counts/L, then buildup is expressed
as counts per unit of area (or curb length).

 

The buildup functions associated with each function type are:

  ------ ------------- ---------------------
  Type   Function      Equation*
  POW    Power         Min (C1, C2*tC3)
  EXP    Exponential   C1*(1 – exp(-C2*t))
  SAT    Saturation    C1*t / (C3 + t)
  EXT    External      See below
  ------ ------------- ---------------------

*t is antecedent dry days.

 

For the EXT buildup function, C1 is the maximum possible buildup (mass
per area or curb length), C2 is a scaling factor, and C3 is the name of
a Time Series that contains buildup rates (as mass per area or curb
length per day) as a function of time.

<!---
  washoff
-->

@page washoff  [WASHOFF]


## Purpose:

Specifies the rate at which pollutants are washed off from different
land uses during rain events.

 

## Format:

Landuse  Pollutant  FuncType  C1  C2  SweepRmvl BmpRmvl

 

## Parameters:

  ----------- -------------------------------------------------
  Landuse     land use name.
  Pollutant   pollutant name.
  FuncType    washoff function type: EXP / RC / EMC.
  C1, C2      washoff function coefficients(see table below).
  SweepRmvl   street sweeping removal efficiency (percent).
  BmpRmvl     BMP removal efficiency (percent).
  ----------- -------------------------------------------------

## Remarks:

The equations used for each type of washoff function are as follows:

+-----------------+-----------------+-----------------+-----------------+
| Type            | Function        | Equation        | Units           |
+-----------------+-----------------+-----------------+-----------------+
| EXP             | Exponential     | C1 (runoff)C2   | Mass/hour       |
|                 |                 | (buildup)       |                 |
+-----------------+-----------------+-----------------+-----------------+
| RC              | Rating Curve    | C1 (runoff)C2   | Mass/sec        |
+-----------------+-----------------+-----------------+-----------------+
| EMC             | Event Mean      | C1              | Mass/Liter      |
|                 |                 |                 |                 |
|                 | Concentration   |                 |                 |
+-----------------+-----------------+-----------------+-----------------+

 

Each washoff function expresses its results in different units.

For the Exponential function the runoff variable is expressed in
catchment depth per unit of time (inches per hour or millimeters per
hour), while for the Rating Curve function it is in whatever flow units
were specified in the [OPTIONS] section of the input file (e.g., CFS,
CMS, etc.).

The buildup parameter in the Exponential function is the current total
buildup over the subcatchment’s land use area in mass units. The units
of C1 in the Exponential function are (in/hr)-C2  per hour (or
(mm/hr)-C2  per hour). For the Rating Curve function, the units of C1
depend on the flow units employed. For the EMC (event mean
concentration) function, C1 is always in concentration units.

<!---
  treatment
-->

@page treatment  [TREATMENT]


## Purpose:

Specifies the degree of treatment received by pollutants at specific
nodes of the drainage system.

 

## Format:

Node  Pollut  Result = Func

 

## Parameters:

  ------ ------------------------------------------
  Node   Name of the node where treatment occurs.
  ------ ------------------------------------------

  -------- ----------------------------------------
  Pollut   Name of pollutant receiving treatment.
  -------- ----------------------------------------

  -------- -----------------------------------------------------
  Result   Result computed by treatment function. Choices are:
  -------- -----------------------------------------------------

C  (function computes effluent concentration)

R  (function computes fractional removal).

  ------ -------------------------------------------------------------------------------------------------------------------------------------------------------
  Func   mathematical function expressing treatment result in terms of pollutant concentrations, pollutant removals, and other standard variables (see below).
  ------ -------------------------------------------------------------------------------------------------------------------------------------------------------

 

## Remarks:

Treatment functions can be any well-formed mathematical expression
involving:

  --- --------------------------------------------------------------------------------------
  ·   inlet pollutant concentrations (use the pollutant name to represent a concentration)
  --- --------------------------------------------------------------------------------------

  --- --------------------------------------------------------------------------------------------
  ·   removal of other pollutants (use R_ pre-pended to the pollutant name to represent removal)
  --- --------------------------------------------------------------------------------------------

  --- ----------------------------------
  ·   process variables which include:
  --- ----------------------------------

FLOW        for flow rate into node (user’s flow units)

DEPTH        for water depth above node invert (ft or m)

AREA        for node surface area (ft2 or m2)

DT                for routing time step (seconds)

HRT        for hydraulic residence time (hours)

 

Any of the following math functions can be used in a treatment function:

  --- --------------------------------
  ·   abs(x) for absolute value of x
  --- --------------------------------

  --- -----------------------------------------------
  ·   sgn(x) which is +1 for x >= 0 or -1 otherwise
  --- -----------------------------------------------

  --- -----------------------------------------------
  ·   step(x) which is 0 for x <= 0 and 1 otherwise
  --- -----------------------------------------------

  --- ----------------------------------
  ·   sqrt(x) for the square root of x
  --- ----------------------------------

  --- ----------------------------------
  ·   log(x) for logarithm base e of x
  --- ----------------------------------

  --- -------------------------------------
  ·   log10(x) for logarithm base 10 of x
  --- -------------------------------------

  --- ------------------------------------
  ·   exp(x) for e raised to the x power
  --- ------------------------------------

  --- ------------------------------------------------------
  ·   the standard trig functions (sin, cos, tan, and cot)
  --- ------------------------------------------------------

  --- ---------------------------------------------------------
  ·   the inverse trig functions (asin, acos, atan, and acot)
  --- ---------------------------------------------------------

  --- ------------------------------------------------------------
  ·   the hyperbolic trig functions (sinh, cosh, tanh, and coth)
  --- ------------------------------------------------------------

along with the standard operators +, -, *, /, ^ (for exponentiation )
and any level of nested parentheses.

 

Examples:

; 1-st order decay of BOD

Node23  BOD   C = BOD * exp(-0.05*HRT)

 

; lead removal is 20% of TSS removal

Node23  Lead  R = 0.2 * R_TSS    

<!---
  inflows
-->

@page inflows  [INFLOWS]


## Purpose:

Specifies external hydrographs and pollutographs that enter the drainage
system at specific nodes.

 

## Formats:

  ------ -------- --------- -----------------------------------
  Node   FLOW     Tseries   (FLOW (1.0 Sfactor Base Pat))
  Node   Pollut   Tseries   (Type (Mfactor Sfactor Base Pat))
  ------ -------- --------- -----------------------------------

## Parameters:

  ------ ------------------------------------------------
  Node   name of the node where external inflow enters.
  ------ ------------------------------------------------

  -------- ----------------------
  Pollut   name of a pollutant.
  -------- ----------------------

  --------- -----------------------------------------------------------------------------------------------------------------------
  Tseries   name of a time series in the [TIMESERIES] section describing how external flow or pollutant loading varies with time.
  --------- -----------------------------------------------------------------------------------------------------------------------

  ------ ------------------------------------------------------------------------------------------------------------------------------
  Type   CONCEN if pollutant inflow is described as a concentration, MASS if it is described as a mass flow rate (default is CONCEN).
  ------ ------------------------------------------------------------------------------------------------------------------------------

  --------- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Mfactor   the factor that converts the inflow’s mass flow rate units into the project’s mass units per second, where the project’s mass units are those specified for the pollutant in the [POLLUTANTS] section (default is 1.0 - see example below).
  --------- ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  --------- ------------------------------------------------------------------------------------
  Sfactor   a scaling factor that multiplies the recorded time series values (default is 1.0).
  --------- ------------------------------------------------------------------------------------

  ------ ----------------------------------------------------------------------------
  Base   a constant baseline value added to the time series value (default is 0.0).
  ------ ----------------------------------------------------------------------------

  ----- -------------------------------------------------------------------------------------------------------------------
  Pat   name of an optional time pattern in the [PATTERNS] section used to adjust the baseline value on a periodic basis.
  ----- -------------------------------------------------------------------------------------------------------------------

 

## Remarks:

External inflows are represented by both a constant and time varying
component as follows:

Inflow = (Baseline value)*(Pattern factor) +

(Scaling factor)*(Time series value)

If an external inflow of a pollutant concentration is specified for a
node, then there must also be an external inflow of FLOW provided for
the same node, unless the node is an Outfall. In that case a pollutant
can enter the system during periods when the outfall is submerged and
reverse flow occurs. External pollutant mass inflows do not require a
FLOW inflow.

 

## Examples:

; NODE2 receives flow inflow from time series N2FLOW

; and TSS concentration from time series N2TSS

NODE2   FLOW  N2FLOW

NODE2   TSS   N33TSS  CONCEN

 

; NODE65 has a mass inflow of BOD from time series N65BOD

; listed in lbs/hr (126 converts lbs/hr to mg/sec)

NODE65  BOD  N65BOD  MASS  126

 

; Flow inflow to Node N176 consists of the flow time series

; FLOW_176 scaled at 0.5 plus a baseline flow of 12.7

; adjusted by pattern FlowPat

N176  FLOW  FLOW_176  FLOW  1.0  0.5  12.7  FlowPat

<!---
  dwf
-->

@page dwf  [DWF]


## Purpose:

Specifies dry weather flow and its quality entering the drainage system
at specific nodes.

 

## Format:

Node  Type  Base  (Pat1  Pat2  Pat3  Pat4)

 

## Parameters:

  ------ -----------------------------------------------
  Node   name of a node where dry weather flow enters.
  ------ -----------------------------------------------

  ------ ----------------------------------------------------------------------
  Type   keyword FLOW for flow or a pollutant name for a quality constituent.
  ------ ----------------------------------------------------------------------

  ------ --------------------------------------------------------------------------------------
  Base   average baseline value for corresponding constituent  (flow or concentration units).
  ------ --------------------------------------------------------------------------------------

  ------- --
  Pat1,
  ------- --

Pat2,

  ------ ------------------------------------------------------------------------
  etc.   names of up to four time patterns appearing in the [PATTERNS] section.
  ------ ------------------------------------------------------------------------

 

## Remarks:

The actual dry weather input will equal the product of the baseline
value and any adjustment factors supplied by the specified patterns. (If
not supplied, an adjustment factor defaults to 1.0.)

The patterns can be any combination of monthly, daily, hourly and
weekend hourly patterns, listed in any order. See the [PATTERNS] section
for more details.

<!---
  rdii
-->

@page rdii  [RDII]

## Purpose:

Specifies the parameters that describe rainfall-dependent
infiltration/inflow (RDII) entering the drainage system at specific
nodes.

 

## Format:

 Node  UHgroup  SewerArea

 

## Parameters:

  ------ -------------------------------------
  Node   name of a node receiving RDII flow.
  ------ -------------------------------------

  --------- -------------------------------------------------------------------------------
  UHgroup   name of an RDII unit hydrograph group appearing in the [HYDROGRAPHS] section.
  --------- -------------------------------------------------------------------------------

  ----------- ------------------------------------------------------------------------------
  SewerArea   area of the sewershed that contributes RDII to the node (acres or hectares).
  ----------- ------------------------------------------------------------------------------

  <!---
    hydrographs
  -->

@page hydrographs  [HYDROGRAPHS]


## Purpose:

Specifies the shapes of the triangular unit hydrographs that determine
the amount of rainfall-dependent infiltration/inflow (RDII) entering the
drainage system.

 

## Format:

Name  Raingage

Name  Month SHORT/MEDIUM/LONG  R  T  K (Dmax Drec D0)

 

## Remarks:

  ------ -------------------------------------------
  Name   name assigned to a unit hydrograph group.
  ------ -------------------------------------------

  ---------- ----------------------------------------------------------
  Raingage   name of the rain gage used by the unit hydrograph group.
  ---------- ----------------------------------------------------------

  ------- -----------------------------------------------------------------
  Month   month of the year (e.g., JAN, FEB, etc. or ALL for all months).
  ------- -----------------------------------------------------------------

  --- -----------------------------------------
  R   response ratio for the unit hydrograph.
  --- -----------------------------------------

  --- -----------------------------------------------
  T   time to peak (hours) for the unit hydrograph.
  --- -----------------------------------------------

  --- -----------------------------------------------
  K   recession limb ratio for the unit hydrograph.
  --- -----------------------------------------------

  ------ --------------------------------------------------------------------
  Dmax   maximum initial abstraction depth available (in rain depth units).
  ------ --------------------------------------------------------------------

  ------ -----------------------------------------------------------------
  Drec   initial abstraction recovery rate (in rain depth units per day)
  ------ -----------------------------------------------------------------

  ---- ------------------------------------------------------------------------------------------------
  D0   initial abstraction depth already filled at the start of the simulation (in rain depth units).
  ---- ------------------------------------------------------------------------------------------------

 

## Remarks:

For each group of unit hydrographs, use one line to specify its rain
gage followed by as many lines as are needed to define each unit
hydrograph used by the group throughout the year. Three separate unit
hydrographs, that represent the short-term, medium-term, and long-term
RDII responses, can be defined for each month (or all months taken
together). Months not listed are assumed to have no RDII.

The response ratio (R) is the fraction of a unit of rainfall depth that
becomes RDII. The sum of the ratios for a set of three hydrographs does
not have to equal 1.0.

The recession limb ratio (K) is the ratio of the duration of the
hydrograph’s recession limb to the time to peak (T) making the
hydrograph time base equal to T*(1+K) hours. The area under each unit
hydrograph is 1 inch (or mm).

The optional initial abstraction parameters determine how much rainfall
is lost at the start of a storm to interception and depression storage.
If not supplied then the default is no initial abstraction.

 

# Example:

;  All three unit hydrographs in this group have the same shapes except
those in July,

;  which have only a short- and medium-term response and a different
shape.

UH101  RG1

UH101  ALL SHORT  0.033 1.0  2.0

UH101  ALL MEDIUM 0.300 3.0  2.0

UH101  ALL LONG   0.033 10.0 2.0

UH101  JUL SHORT  0.033 0.5  2.0

UH101  JUL MEDIUM 0.011 2.0  2.0

<!---
  curves
-->

@page curves  [CURVES]


## Purpose:

Describes a relationship between two variables in tabular format.

 

## Format:

Name  Type

Name  X-value  Y-value  ...

 

## Parameters:

+-----------------------------------+-----------------------------------+
| Name                              | name assigned to the curve.       |
+-----------------------------------+-----------------------------------+
| Type                              | the type of curve being defined:  |
|                                   |                                   |
|                                   | STORAGE / SHAPE / DIVERSION /     |
|                                   | TIDAL /                           |
|                                   |                                   |
|                                   | PUMP1 / PUMP2 / PUMP3 / PUMP4 /   |
|                                   | PUMP5 /                           |
|                                   |                                   |
|                                   | RATING / CONTROL / WEIR.          |
+-----------------------------------+-----------------------------------+
| X-value                           | an X (independent variable)       |
|                                   | value.                            |
+-----------------------------------+-----------------------------------+
| Y-value                           | the Y (dependent variable) value  |
|                                   | corresponding to X.               |
+-----------------------------------+-----------------------------------+

## Remarks:

Each curve should have its name and type on the first line with its data
points entered on subsequent lines.

Multiple pairs of x-y values can appear on a line. If more than one line
is needed, repeat the curve's name on subsequent lines.

X-values must be entered in increasing order.

Choices for curve type have the following meanings (flows are expressed
in the user’s choice of flow units set in the [OPTIONS] section):

  --------- ---------------------------------------------------------------------
  STORAGE   surface area in ft2 (m2) v. depth in ft (m) for a storage unit node
  --------- ---------------------------------------------------------------------

  ------- ----------------------------------------------------------------------------------------------
  SHAPE   width v. depth for a custom closed cross-section, both normalized with respect to full depth
  ------- ----------------------------------------------------------------------------------------------

  ----------- ----------------------------------------------------------------------------
  DIVERSION   diverted outflow v. total inflow for a flow divider node or a Custom inlet
  ----------- ----------------------------------------------------------------------------

  ------- --------------------------------------------------------------------------
  TIDAL   water surface elevation in ft (m) v. hour of the day for an outfall node
  ------- --------------------------------------------------------------------------

  ------- ------------------------------------------------------------
  PUMP1   pump outflow v. increment of inlet node volume in ft3 (m3)
  ------- ------------------------------------------------------------

  ------- ---------------------------------------------------------
  PUMP2   pump outflow v. increment of inlet node depth in ft (m)
  ------- ---------------------------------------------------------

  ------- ------------------------------------------------------------------------------------------------------------------------
  PUMP3   pump outflow v. head difference between outlet and inlet nodes in ft (m) that has decreasing flow with increasing head
  ------- ------------------------------------------------------------------------------------------------------------------------

  ------- -------------------------------------------------------
  PUMP4   pump outflow v. continuous inlet node depth in ft (m)
  ------- -------------------------------------------------------

  ------- ------------------------------------------------------------------------------------------------------------------------
  PUMP5   pump outflow v. head difference between outlet and inlet nodes in ft (m) that has decreasing flow with increasing head
  ------- ------------------------------------------------------------------------------------------------------------------------

  -------- -------------------------------------------------------------
  RATING   flow v. head in ft (m) for an Outlet link or a Custom inlet
  -------- -------------------------------------------------------------

  --------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  CONTROL   control setting for a pump or flow regulator v. a controller variable (such as a node water level) in a modulated control; flow adjustment setting v. head for an LID unit’s underdrain
  --------- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  ------ ---------------------------------------------------------------
  WEIR   discharge coefficient for flow in CFS (CMS) v. head in ft (m)
  ------ ---------------------------------------------------------------

 

Examples:

; Storage curve (x = depth, y = surface area)

AC1  STORAGE

AC1  0  1000  2  2000  4  3500  6  4200  8  5000

 

; Type 1 pump curve (x = inlet wet well volume, y = flow)

PC1  PUMP1

PC1  100  5  300  10  500  20

 

; Type 5 pump curve (x = pump head, y = pump flow)

PC2  PUMP5

PC2  0  4

PC2  4  2

PC2  6  0

<!---
  timeseries
-->

@page timeseries  [TIMESERIES]


## Purpose:

Describes how a quantity varies over time.

 

## Formats:

Name  ( Date )  Hour  Value  ...

Name  Time  Value  ...

Name FILE  Fname

 

## Parameters:

  ------ -----------------------------------
  Name   name assigned to the time series.
  ------ -----------------------------------

  ------ -------------------------------------------------------------------------
  Date   date in Month/Day/Year format (e.g., June 15, 2001 would be 6/15/2001).
  ------ -------------------------------------------------------------------------

  ------ -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Hour   24-hour military time (e.g., 8:40 pm would be 20:40) relative to the last date specified (or to midnight of the starting date of the simulation if no previous date was specified).
  ------ -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  ------ ----------------------------------------------------------------------------------------------------------------------------------
  Time   hours since the start of the simulation, expressed as a decimal number or as hours:minutes (where hours can be greater than 24).
  ------ ----------------------------------------------------------------------------------------------------------------------------------

  ------- -------------------------------------------------------
  Value   a value corresponding to the specified date and time.
  ------- -------------------------------------------------------

  ------- -------------------------------------------------------------
  Fname   the name of a file in which the time series data are stored
  ------- -------------------------------------------------------------

 

## Remarks:

There are two options for supplying the data for a time series:

  ---- -------------------------------------------------------------------------------
  i.   directly within this input file section as described by the first two formats
  ---- -------------------------------------------------------------------------------

  ----- ------------------------------------------------------------
  ii.   through an external data file named with the third format.
  ----- ------------------------------------------------------------

When direct data entry is used, multiple date-time-value or time-value
entries can appear on a line. If more than one line is needed, the
table's name must be repeated as the first entry on subsequent lines.

When an external file is used, each line in the file must use the same
formats listed above, except that only one date-time-value (or
time-value) entry is allowed per line. Any line that begins with a
semicolon is considered a comment line and is ignored. Blank lines are
also permitted. Enclose the external file name in double quotes if it
contains spaces and include its full path if it resides in a different
directory than the SWMM input file.

There are two options for describing the occurrence time of time series
data:

  --- -------------------------------------------------------------------------------------------------------------------
  ·   as calendar date plus time of day (which requires that at least one date, at the start of the series, be entered)
  --- -------------------------------------------------------------------------------------------------------------------

  --- -----------------------------------------------------
  ·   as elapsed hours since the start of the simulation.
  --- -----------------------------------------------------

For the first method, dates need only be entered at points in time when
a new day occurs.

For rainfall time series, it is only necessary to enter periods with
non-zero rainfall amounts. SWMM interprets the rainfall value as a
constant value lasting over the recording interval specified for the
rain gage which utilizes the time series. For all other types of time
series, SWMM uses interpolation to estimate values at times that fall in
between the recorded values.

 

## Examples:

; Hourly rainfall time series with dates specified using

; one data point per line to emphasize when dates change

TS1 6-15-2001 7:00  0.1

TS1           8:00  0.2

TS1           9:00  0.05

TS1           10:00 0

TS1 6-21-2001 4:00  0.2

TS2           5:00  0

TS2           14:00 0.1

TS2           15:00 0

 

;Inflow hydrograph - time relative to start of simulation

HY1  0  0  1.25 100  2:30 150  3.0 120  4.5 0

HY1  32:10 0  34.0 57  35.33 85  48.67 24  50 0

<!---
  patterns
-->

@page patterns  [PATTERNS]


## Purpose:

Specifies time patterns of dry weather flow or quality in the form of
adjustment factors applied as multipliers to baseline values.

 

## Format:

  ------ --------- ---------------------------------
  Name   MONTHLY   Factor1  Factor2  ...  Factor12
  Name   DAILY     Factor1  Factor2  ...  Factor7
  Name   HOURLY    Factor1  Factor2  ...  Factor24
  Name   WEEKEND   Factor1  Factor2  ...  Factor24
  ------ --------- ---------------------------------

## Parameters:

  ------ ------------------------------------
  Name   name used to identify the pattern.
  ------ ------------------------------------

Factor1,

Factor2,

  ------ --------------------
  etc.   multiplier values.
  ------ --------------------

 

## Remarks:

The MONTHLY format is used to set monthly pattern factors for dry
weather flow constituents.

The DAILY format is used to set dry weather pattern factors for each day
of the week, where Sunday is day 1.

The HOURLY format is used to set dry weather factors for each hour of
the day starting from midnight. If these factors are different for
weekend days than for weekday days then the WEEKEND format can be used
to specify hourly adjustment factors just for weekends.

More than one line can be used to enter a pattern’s factors by repeating
the pattern’s name (but not the pattern type) at the beginning of each
additional line.

The pattern factors are applied as multipliers to any baseline dry
weather flows or quality concentrations supplied in the [DWF] section.

 

## Examples:

; Day of week adjustment factors

D1  DAILY  0.5  1.0  1.0  1.0  1.0  1.0  0.5

D2  DAILY  0.8  0.9  1.0  1.1  1.0  0.9  0.8

 

; Hourly adjustment factors

H1 HOURLY  0.5 0.6 0.7 0.8 0.8 0.9

H1         1.1 1.2 1.3 1.5 1.1 1.0

H1         0.9 0.8 0.7 0.6 0.5 0.5

H1         0.5 0.5 0.5 0.5 0.5 0.5
