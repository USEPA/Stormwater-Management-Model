# SWMM API-Guide {#api-guide}

## Introduction

The EPA Storm Water Management Model Version 5 (SWMM 5) is a dynamic rainfall-runoff simulation model used for single event or long-term (continuous) simulation of runoff quantity and quality from primarily urban areas.

![Urban Wet Weather Flows](urbanwetweatherflows.zoom75.jpg)

SWMM's computational engine contains a library of functions (or Application Programming Interface) written in C that allow programmers to customize its use within their own programs. Some typical use cases for the API might include:

- supplying externally obtained rainfall and lateral inflows to a system
- providing real time control of pumps and regulators based on monitoring data
- linking SWMM with other models, such as a 2-D surface flow or a groundwater0 flow model
- running the engine within a graphical user interface.

This guide describes what a programmer needs to know to successfully use the SWMM 5 engine within an application. The principal topics include:

- SWMM's Data Model
- Files used by SWMM
- Running the SWMM engine
- Examining object properties
- Controlling simulation conditions
- Retrieving simulation results
- Language bindings

In addition, there are three appendices containing useful reference information:

- [Appendix A](#appendix_a) - describes each API function in detail
- [Appendix B](#appendix_b) - describes SWMM's input file format
- [Appendix C](#appendix_c) - describes the format of SWMM's output results file

<!---
  Data Model
-->

## SWMM's Data Model {#data_model}

Before using the SWMM engine API  one should be familiar with the way that SWMM represents a drainage area and the design and operating information it requires to simulate the area\'s hydrology, hydraulics, and water quality. The figure below shows how SWMM conceptualizes the physical elements that comprise a drainage basin with a standard set of modeling objects.

![](embim1.jpg)

The principal objects used to model the rainfall/runoff process are Rain Gages and Subcatchments. The conveyance portion of the drainage system is modeled with a network of Nodes and Links. Nodes are points that represent simple junctions, flow dividers, storage units, or outfalls. Links connect nodes to one another with conduits (pipes and channels), pumps, or flow regulators (orifices, weirs, or outlets). Land Use and Pollutant objects are used to describe water quality. Finally, a group of data objects that includes Curves, Time Series, Time Patterns, and Control Rules, are used to characterize the inflows and operating behavior of the various physical objects in a SWMM model. The table below provides a summary of the various objects used in SWMM.

| Category      | Object Type        | Description                                                                                                                                                                                         |
| :------------ | :----------------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Hydrology     | Rain Gage          | Source of precipitation data to one or more subcatchments.                                                                                                                                          |
| ^             | Subcatchment       | A land parcel that receives precipitation associated with a rain gage and generates runoff that flows into a drainage system node or to another subcatchment.                                       |
| ^             | Aquifer            | A subsurface area that receives infiltration from the subcatchment above it and exchanges groundwater flow with a conveyance system node.                                                           |
| ^             | Snow Pack          | Accumulated snow that covers a subcatchment.                                                                                                                                                        |
| ^             | Unit Hydrograph    | A response function that describes the amount of sewer inflow/infiltration generated over time per unit of instantaneous rainfall.                                                                  |
| Hydraulics    | Junction           | A point in the conveyance system where conduits connect to one another with negligible storage volume (e.g., manholes, pipe fittings, or stream junctions).                                         |
| ^             | Outfall            | An end point of the conveyance system where water is discharged to receptor (such as a receiving stream or treatment plant) with known water surface elevation.                                     |
| ^             | Divider            | A point in the conveyance system where the inflow splits into two outflow conduits according to a known relationship.                                                                               |
| ^             | Storage Unit       | A pond, lake, impoundment, or chamber that provides water storage.                                                                                                                                  |
| ^             | Conduit            | A channel or pipe that conveys water from one conveyance system node to another.                                                                                                                    |
| ^             | Pump               | A device that raises the hydraulic head of water.                                                                                                                                                   |
| ^             | Regulator          | A weir, orifice or outlet used to direct and regulate flow between two nodes of the conveyance system.                                                                                              |
| Water Quality | Pollutant          | A contaminant that can build up and be washed off of the land surface or be introduced directly into the conveyance system.                                                                         |
| ^             | Land Use           | A classification used to characterize the functions that describe pollutant buildup and washoff.                                                                                                    |
| Treatment     | LID Control        | A low impact development control, such as a bio-retention cell, porous pavement, or vegetative swale, used to reduce surface runoff through enhanced infiltration.                                  |
| ^             | Treatment Function | A user-defined function that describes how pollutant concentrations are reduced at a conveyance system node as a function of certain variables, such as concentration, flow rate, water depth, etc. |
| Data Object   | Curve              | A tabular function that defines the relationship between two quantities (e.g., flow rate and hydraulic head for a pump, surface area and depth for a storage node, etc.).                           |
| ^             | Time Series        | A tabular function that describes how a quantity varies with time (e.g., rainfall, outfall surface elevation, etc.).                                                                                |
| ^             | Time Pattern       | A set of factors that repeats over a period of time (e.g., diurnal hourly pattern, weekly daily pattern, etc.).                                                                                     |
| ^             | Control Rules      | IF-THEN-ELSE statements that determine when specific control actions are taken (e.g., turn a pump on or off when the flow depth at a given node is above or below a certain value).                 |

More information on the data objects used by SWMM can be found in the SWMM Users Manual and in the three-volume SWMM Reference Manual which are available on [US EPA's Storm Water Management Model](https://www.epa.gov/water-research/storm-water-management-model-swmm) web site.

<!---
  Files Used
-->

## Files Used By SWMM {#files_used}

The following files are used when linking the SWMM 5 engine with other applications:

**A SWMM 5 input file.** This is a plain text file that contains all of the information needed to run a simulation for a given study area. The contents and format of this file are described in detail in [Appendix B](#appendix_b). The application using the SWMM API must either identify the name of an existing input file or create the file itself and save it to disk. Projects created and saved when running the Windows version of SWMM can be used as input files to the SWMM API.

**A SWMM 5 report file.** This is a plain text file which can contain a run's status report, including any error messages, as well as summary results tables. The SWMM 5 engine will open and write the contents of this file, but the interfacing application must provide a valid file name to the engine. The API allows one to control how much information will appear in the file. The application can decide whether or not to display the report file and whether to save or delete it.

**A SWMM 5 output file.** This is an optional binary file that will contain computed results at uniform reporting time steps of a simulation for all of the elements that were marked for reporting in the project's input file. The SWMM engine will open and write the contents of this file, but the interfacing application must provide a valid file name to the engine if the file will be saved to disk once the SWMM engine is closed. If no file name is supplied then
SWMM will use a temporary file instead.  After a SWMM run is completed, the application can use the API to access this file to retrieve simulation results. The format of this file is described in [Appendix C](#appendix_c) in case one wishes to read its contents after a SWMM 5 project has been closed.

<!---
  Units
-->

## Units of Measurement {#units}

SWMM can use either US customary units or SI metric units for its design and computed variables. The choice of flow units set in SWMM\'s input file determines what unit system is in effect:

- selecting CFS (cubic feet per second), GPM (gallons per minutes), or MGD (million gallons per day) for flow units implies that US units will be used throughout

- selecting CMS (cubic meters per second), LPS (liters per second), or MLD (million liters per day) as flow units implies that SI units will be used throughout.

- pollutant concentration and Manning's roughness coefficient (n) are always expressed in metric units.

The API function `swmm_getValue(swmm_FLOWUNITS, 0)` can be used to determine which choice of flow units was specified in a project\'s input file.

<!---
  Running SWMM
-->

## Running the SWMM Engine {#running}

After an application has obtained a SWMM 5 input file, it can be analyzed using the following general steps:

1. Use the `swmm_open` function, with the input, report, and optional output file names as arguments, to open a SWMM project and process its input data.

2. Call `swmm_start` to initialize the various processing modules.

3. Make repeated calls to `swmm_step(&elapsedTime)` until the value of `elapsedTime` is 0, thus signaling the end of the simulation. The `elapsedTime` variable can be used to update a progress meter while the simulation proceeds.

4. Call `swmm_end` to close down the processing modules.

5. Call `swmm_report` if one wishes to produce a pre-formatted report of computed results written to SWMM\'s report file.

6. Call `swmm_getError` to check if the run was halted due to a fatal error occurring.

7. Call `swmm_close` to close down the project.

Below is example code that illustrates these steps.

```C
char errMsg[241];
int  err;
double elapsedTime;

swmm_open("InpFile.inp", "RptFile.rpt", "");

swmm_start(1);
do {
    swmm_step(&elapsedTime);
} while (elapsedTime > 0)
swmm_end();

swmm_report();
err = swmm_getError(errMsg, 240);
swmm_close();

if (err > 0)
    printf("\nRun was unsuccessful. %s\n", errMsg);
else
    printf("\nRun was successful.\n");
```

<!---
  Examining Objects
-->

## Examining Object Properties {#examining}

The following API functions can be used to retrieve information about a project's objects and its simulation options:

- `swmm_getCount` - retrieves the number of subcatchments, nodes or links in a project.

- `swmm_getName` - retrieves the ID name of an object given its index in the list of all like objects.

- `swmm_getIndex` - retrieves the index of an object given its ID name.

- `swmm_getValue` - retrieves the value of an object's property or a project simulation option.

In the code below, after opening a SWMM input file, the name, length, slope, and full depth of each conduit link is printed to the console.

```C
char name[21];

swmm_open("InpFile.inp", "RptFile.rpt", "");

for (int i = 0; i < swmm_getCount(swmm_LINK); i++)
{
    if (swmm_getValue(swmm_LINKTYPE, i) == swmm_CONDUIT)
    {
        swmm_getName(swmm_LINK, i, name, 20);
        printf("\n%20s %10.2f  %10.4f  %10.2f",
            name,
            swmm_getValue(swmm_LENGTH, i),
            swmm_getValue(swmm_SLOPE, i),
            swmm_getValue(swmm_FULLDEPTH, i));
    }

}
swmm_close();
```

<!---
  Retreiving Results
-->

## Retrieving Simulation Results {#retrieving}

The `swmm_getValue` function can be used to retrieve the value of computed variables at the current time period while a simulation is proceeding, after each call to `swmm_step` (or `swmm_stride`). These variables include:

- rainfall, evaporation, infiltration and runoff rates for subcatchments

- water depth, total head, stored volume, lateral inflow, total inflow, and overflow (i.e., flooding) for nodes

- flow rate, velocity, flow depth, top width, and setting for links

- current date/time, elapsed time, flow routing time step and Courant routing time step (the largest time step that will satisfy stability criteria).

After a simulation has been completed (after `swmm_end` was called) the `swmm_getSavedValue` function can be used to retrieve the values of computed variables that were saved to SWMM's [binary output file](#files_used) at each uniform reporting time period. They include the same subcatchment, node and link variables as mentioned above.

The C++ code segment below illustrates how to use `swmm_getSavedValue` to populate a time series of computed flow values for a particular link:

```CXX
vector<double> linkTimeSeries;
double elapsedTime;

swmm_open("InpFile.inp", "RptFile.rpt", "");
int linkIndex = swmm_getIndex(swmm_LINK, "TheLink");

swmm_start(1);
do {
    swmm_step (&elapsedTime);
} while (elapsedTime > 0)
swmm_end();

for (int i = 1; i <= swmm_getValue(swmm_TOTALSTEPS, 0); i++)
    linkTimeSeries.push_back(swmm_getSavedValue(swmm_LINK_FLOW, linkIndex, i));

swmm_close();
```

<!---
  Controlling Simulation
-->

## Controlling Simulation Conditions {#controlling}

The `swmm_setValue` function allows one to control the following variables between time periods of a simulation:

- the rainfall rate generated by a specific Rain Gage (`swmm_GAGE_RAINFALL`)

- the fixed head maintained at an Outfall node (`swmm_NODE_HEAD`)

- the lateral inflow (or outflow) received at a Junction node (`swmm_NODE_LATFLOW`)

- the setting assigned to a link (e.g., pump speed, orifice opening,  weir opening) (`swmm_LINK_SETTING`)

- the routing time step to use (`swmm_ROUTE_STEP`).

The code below adjusts the opening of orifice O1 depending on the upstream flow rate in conduit C1 and the capacity available in diversion link C2:

```C
int  c1, c2, o1;
double elapsedTime, fullDepth;

swmm_open("InpFile.inp", "RptFile.rpt", "");

c1 = swmm_getIndex(swmm_LINK, "C1");
c2 = swmm_getIndex(swmm_LINK, "C2");
o1 = swmm_getIndex(swmm_LINK, "O1");
fullDepth = swmm_getValue(swmm_LINK_FULLDEPTH, c2);

swmm_start(1);
swmm_setValue(swmm_LINK_SETTING, o1, 0.0);
do {
    swmm_step(&elapsedTime);
    if (swmm_getValue(swmm_LINK_FLOW, c1) > 5 &&
        swmm_getValue(swmm_LINK_DEPTH, c2)/fullDepth < 0.5)
    {
        swmm_setValue(swmm_LINK_SETTING, o1, 1.0);
    }
    else
        swmm_setValue(swmm_LINK_SETTING, o1, 0.0);

} while (elapsedTime > 0);
swmm_end();

swmm_report();
swmm_close();
```

<!---
  Language Bindings
-->

## Language Bindings {#bindings}

The SWMM 5 API can be used with programs written in C/C++, Pascal, and Python. The files needed to build applications in each of these languages are included with this guide and are discussed below.

**swmm5.dll** <br>
This is the Windows 64-bit dynamic link library of the SWMM 5 engine that must be distributed with any Windows application that uses the engine API, no matter which language the application is written in. The API can be used on other operating systems as well if the engine source code is compiled to a shared object library. For example, on Linux the SWMM 5 engine source code would be compiled to shared library file named swmm5.so which would take the place of swmm5.dll.

**vcomp140.dll** <br>
This file contains the 64-bit OpenMP library for Windows that is used by swmm5.dll. It should be also be distributed with any Windows application that uses the SWMM 5 API. If the engine is compiled for a different platform then that platform's version of OpenMP should be used.

**swmm5.h** <br>
This file contains the C declarations of all of the API functions and their constants. It must appear in an include directive (i.e., \#include "swmm5.h") at the top of any C/C++ code module whose functions make calls to the API.

**swmm5.lib** <br>
When building a Windows application with the Visual Studio C/C++ compiler, this file must be added to the link command line (or to the Additional Dependencies linker options in the IDE). It tells the linker which functions can be imported and where their locations are within swmm5.dll.

**swmm5.pas** <br>
This file contains the declarations of the API functions and their constants for use with Delphi Pascal or with Lazarus/Free Pascal. It must appear in a **uses** clause at the top of any module that contains calls to the SWMM 5 API.

**swmm5.py** <br>
This file contains a Python binding for the SWMM 5 API. It uses Python's Ctypes module to build wrappers around each of the API's C-functions. Each API function retains its same name but without the `"swmm_"` prefix to avoid repeating the `"swmm"` qualifier each time a function is used (e.g., the original `swmm_getVersion` function in C/C++ would be called as `swmm5.getVersion` in Python). It must appear in an **imports** clause at the top of any module that contains calls to the  API functions and be distributed with the application.

To accommodate the Python language, the following API functions have their original definitions (as documented in [Appendix A](appendix_a.htm)) modified:

| Original C/C++                                                                                                       | Python Declaration              | Python Return Value                                          |
| :------------------------------------------------------------------------------------------------------------------- | :------------------------------ | :----------------------------------------------------------- |
| `swmm_open(char *f1, char *f2, char *f3)`                                                                            | `open(f1, f2)` if `f3` is empty | an error code (same as C/C++)                                |
| `swmm_step(double *elapsedTime)`                                                                                     | `step()`                        | elapsedTime                                                  |
| `swmm_stride(double *elapsedTime)`                                                                                   | `stride()`                      | elapsedTime                                                  |
| `swmm_getMassBalanceErr(char *errMsg, int msgLen)`                                                                   | `getMassBalErr()`               | `runoff`, `flow`, and `quality` continuity errors as a tuple |
| `swmm_getError(char *errMsg, int msgLen)`                                                                            | `getError()`                    | `errMsg`                                                     |
| `swmm_getName(int objType, int index, char *name, int size)`                                                         | `getName(objType, index, size)` | name                                                         |
| `swmm_decodeDate(double date, int *year, int *month, int *day, int *hour, int *minute, int *second, int *dayOfWeek)` | `decodeDate()`                  | a Python `DateTime` value                                    |

The following topics illustrate how a simple program that runs a SWMM
model from the command line would look in each of these three languages:

<!---
- @subpage c_lang
- @subpage pascal
- @subpage python
-->

<!---
  C/C++
-->

### C/C++ Example {#c_lang}

The code below uses C/C++ to run a SWMM model from the command line with
the names of the input and report file as arguments.

```C
    #include<stdio.h>
    #include "swmm5.h"
    void run_swmm(char *inpFile, char *rptFile)
    {
      double elapsedTime = 0;
      float  runoffErr, routeErr, qualErr;
      char errMsg[241];
      swmm_open(inpFile, rptFile, "");
      swmm_start(1); 
      do {
            swmm_step(&elapsedTime);
      } while elapsedTime > 0;
      swmm_end();
      swmm_report();
      swmm_getMassBalErr(&runoffErr, &routeErr, &qualErr);
      if (swmm_getError(errMsg, 240) > 0)
        printf("\n%s", errMsg);
      else {
        printf("Runoff Error:  %.4f\n", runoffErr);
        printf("Routing Error: %.4f\n", routeErr);
      }
      swmm_close();
    }

    int main(int argc, char* argv[])
    {
      run_swmm(argv[1], argv[2]);
    }
```

<!---
  Pascal
-->

### Pascal Example {#pascal}

The code below uses Pascal to run a SWMM model from the command line
with the names of the input and report file as arguments.

```Pascal
    program RunSWMM;
    uses
      SysUtils, swmm5;

      procedure Run_Swmm(InpFile: AnsiString; RptFile: AnsiString);
      var
        ElapsedTime : Double;
        RunoffErr, RouteErr, QualErr : Single;
        ErrMsg: array[0..240] of AnsiChar;
      begin
        swmm_open(PAnsiChar(InpFile), PAnsiChar(RptFile), '');
        ElapsedTime := 0;
        swmm_start(1);
        repeat
          swmm_step(ElapsedTime);
        until ElapsedTime = 0;
        swmm_end();
        swmm_report();
        swmm_getMassBalErr(RunoffErr, RouteErr, QualErr);
        if swmm_getError(ErrMsg, 240) > 0 then
          Writeln(ErrMsg)
        else begin
          Writeln(Format('Runoff Error:  %.4f', [RunoffErr]));]
          Writeln(Format('Routing Error: %.4f', [RouteErr]));
        end;
        swmm_close();
      end;

      begin
        Run_Swmm(ParamStr(1), ParamStr(2));
      end.
```

<!---
  Python
-->

### Python Example {#python}

The code below uses Python to run a SWMM model from the command line
with the names of the input and report file as arguments.

```Python
    import sys
    import swmm5
    def run_swmm(inpFile, rptFile):
      swmm5.open(inpFile, rptFile)
      swmm5.start(1)
      while True:
        if swmm5.step() == 0:
          break
      swmm5.end()
      swmm5.report()
      mass_bal = swmm5.getMassBalErr()
      errmsg = swmm5.getError()
      if len(errmsg) > 0:
        print(errmsg)
      else:
        print('Runoff Error: {0:8.4f}'.format(mass_bal[0]))
        print('Routing Error: {0:8.4f}'.format(mass_bal[1]))
      swmm5.close()

      if __name__ == '__main__':
        run_swmm(sys.argv[1], sys.argv[2])
```

---

## Appendix A. SWMM5's API Functions {#appendix-a}

The following functions make up the SWMM 5 API:

<!--
@subpage swmm_getversion

@subpage swmm_run

@subpage swmm_open

@subpage swmm_start

@subpage swmm_step

@subpage swmm_stride

@subpage swmm_end

@subpage swmm_getmassbalerr

@subpage swmm_report

@subpage swmm_close

@subpage swmm_getwarnings

@subpage swmm_geterror

@subpage swmm_getcount

@subpage swmm_getname

@subpage swmm_getindex

@subpage swmm_getvalue

@subpage swmm_getsavedvalue

@subpage swmm_setvalue

@subpage swmm_writeline

@subpage swmm_decodedate
-->

<!--
  swmm_getversion
-->

### swmm_getVersion {#swmm_getversion}

**Syntax**

`int swmm_getVersion(void);`

**Description**

Retrieves the current version number of the SWMM 5 engine.

**Arguments**

None.

**Return Value**

Returns a version number.

---

<!--
  swmm_run
-->

### swmm_run {#swmm_run}

**Syntax**

`int swmm_run(char* f1, char* f2, char* f3);`

**Description**

Runs a complete SWMM simulation.

**Arguments**

_f1_ name of input file.

_f2_ name of report file.

_f3_ name of binary output file.

**Return Value**

Returns a SWMM 5 error code if one or more errors were encountered, otherwise returns 0.

**Notes**

If _f3_ is an empty string then results will be written to the report file following the instructions contained in the `REPORT` section of the input
file.

---

<!--
  swmm_open
-->

### swmm_open {#swmm_open}

**Syntax**

`int swmm_open(char* f1, char* f2, char* f3);`

**Description**

Opens the SWMM engine to analyze a new project.

**Arguments**

_f1_ name of input file.

_f2_ name of report file.

_f3_ name of binary output file.

**Return Value**

Returns a SWMM 5 error code if one or more errors were encountered, otherwise returns 0.

**Notes**

If _f3_ is an empty string then a temporary file will be used to store results saved at each reporting time step. It will be  deleted when the `swmm_close` function is called.

**See Also**

[swmm_close](#swmm_close)

---

<!--
  swmm_start
-->

### swmm_start {#swmm_start}

**Syntax**

`int swmm_start(int saveFlag);`

**Description**

Initializes the SWMM engine prior to starting a new simulation.

**Arguments**

_saveFlag_ a flag which should be set to 1 if results are to be saved to the binary output file or to 0 if they are not.

**Return Value**

Returns a SWMM 5 error code if one or more errors were encountered, otherwise returns 0.

**Notes**

Must be called after `swmm_open` and before `swmm_step`.

**See Also**

[swmm_open](#swmm_open)

[swmm_step](#swmm_step)

---

<!--
  swmm_step
-->

### swmm_step {#swmm_step}

**Syntax**]

`int swmm_step(double* elapsedTime);`

**Description**

Advances the simulation by one routing time step.

**Arguments**

_elapsedTime_ returns the total elapsed time of the simulation (in decimal days) after the current time step has been processed

**Return Value**

Returns a SWMM 5 error code if one or more errors were encountered, otherwise returns 0.

**Notes**

When the end of the simulation period is reached the value of _elapsedTime_ will be set to 0.

After `swmm_start` is called, this function should be called in a loop until _elapsedTime_ becomes 0 (or an error condition occurs). For example:

```C
double elapsedTime;

swmm_start(1);
do {
    swmm_step(&elapsedTime);
} while (elapsedTime > 0.0);
swmm_end();
```

**See Also**

[swmm_start](#swmm_start)

[swmm_end](#swmm_end)

---

<!--
  swmm_stride
-->

### swmm_stride {#swmm_stride}

**Syntax**

`int swmm_stride(int strideStep, double* elapsedTime);`

**Description**

Advances the simulation by a specified time step.

**Arguments**

_strideStep_ the number of seconds to advance the simulation

_elapsedTime_ returns the total elapsed time of the simulation (in decimal days) after the _strideStep_ time step has been taken

**Return Value**

Returns a SWMM 5 error code if one or more errors were encountered, otherwise returns 0.

**Notes**

This function is similar to `swmm_step` except that it can execute multiple routing steps at a time and allow the system state to be examined at fixed intervals of time less frequent than the flow routing time step.

**See Also**

[swmm_start](#swmm_start)

[swmm_end](#swmm_end)

---

<!---
  swmm_end
-->

### swmm_end {#swmm_end}

**Syntax**

`int swmm_end(void);`

**Description**

Shuts down the SWMM engine after a simulation has been completed.

**Arguments**

None.

**Return Value**

Returns a SWMM 5 error code if one or more errors were encountered, otherwise returns 0.

**Notes**

Must be called after the `swmm_step` loop has been completed.

**See Also**

[swmm_open](#swmm_open)

[swmm_start](#swmm_start)

[swmm_step](#swmm_step)

---

<!---
  swmm_getmassbalerr
-->

### swmm_getMassBalErr {#swmm_getmassbalerr}

**Syntax**

`int swmm_getMassBalErr(float *runoff, float *flow, float *qual);`

**Description**

Retrieves the mass balance continuity errors after a SWMM simulation has
ended.

**Arguments**

_runoff_ percent continuity error for runoff flow.

_flow_ percent continuity error for flow routing.

_qual_ percent continuity error for water quality routing.

**Return Value**

Returns a SWMM 5 error code if one or more errors were encountered, otherwise returns 0.

**Notes**

The continuity error is defined as 100 \* (Total Mass Inflow  - Total Mass Outflow) / Total Mass Inflow where Total Mass Inflow includes the initial stored mass and Total Mass Outflow includes the final stored mass in the system.

This function should be called after a simulation has ended with a call
to [swmm_end](swmm_end.htm).

---

<!--
  swmm_report
-->

### swmm_report {#swmm_report}

**Syntax**

`int swmm_report(void);`

**Description**

Writes computed time series of results in a pre-set format to the SWMM report file after a simulation has been made.

**Arguments**

None.

**Return Value**

Returns a SWMM 5 error code if one or more errors were encountered, otherwise returns 0.

**Notes**

Should be called after `swmm_end` has been called.

The subcatchments, nodes, and links whose results are to be reported must be identified in the [REPORT](#report) section of the SWMM input file.

Instead of using _swmm_report_ an application can generate customized reports by writing code that makes use of the [swmm_getValue](#swmm_getvalue) [swmm_getSavedValue](#swmm_getsavedvalue) and [swmm_writeLine](#swmm_writeline) functions.

---

<!---
  swmm_close
-->

### swmm_close {#swmm_close}

**Syntax**

`int swmm_close(void);`

**Description**

Closes down the SWMM engine for the project just analyzed.

**Arguments**

None.

**Return Value**

Always returns 0.

**Notes**

`swmm_close` must be called before analyzing another project or before terminating the application.

---

<!--
  swmm_getwarnings
-->

### swmm_getWarnings {#swmm_getwarnings}

**Syntax**

`int swmm_getWarnings(void);`

**Description**

Retrieves the number of warning messages issued during an analysis. The text of any warning message issued will appear in the project's report file.

**Arguments**

None.

**Return Value**

Returns number of warnings issued.

<!---
  swmm_geterror
-->

### swmm_getError {#swmm_geterror}

**Syntax**

`int swmm_getError(char* errMsg, int msgLen);`

**Description**

Retrieves information about the last fatal error generated by the SWMM engine.

**Arguments**

_errMsg_ character array to hold error message text.

_msgLen_ maximum number of characters that _errMsg_ can hold.

**Return Value**

Returns the SWMM 5 error code of the last fatal error condition raised or 0 if no errors were generated.

**Notes**

The _errMsg_ array must be dimensioned to hold at least _msgLen_ + 1 characters by the calling program. A size of 240 characters should be sufficient.

---

<!---
  swmm_getcount
-->

### swmm_getCount {#swmm_getcount}

**Syntax**

`int swmm_getCount(int objType);`

**Description**

Retrieves the number of objects of a given type in a project.

**Arguments**

_objType_ type of SWMM object.

**Return Value**

Returns the number of objects of the given type in the project.

**Notes**

Valid object types are `swmm_GAGE`, `swmm_SUBCATCH`, `swmm_NODE`, and `swmm_LINK`.

---

<!--
  swmm_getname
-->

### swmm_getName {#swmm_getname}

**Syntax**

`void swmm_getName(int objType, int index, char* name, int size);`

**Description**

Retrieves the name of an object given its index.

**Arguments**

_objType_ type of SWMM object.

_index_ the position (from 0) of the object in the list of like objects.

_name_ a character array that holds the retrieved ID name.

_size_ the maximum number of characters that _name_ can store.

**Return Value**

None.

**Notes**

- Valid object types are `swmm_GAGE`, `swmm_SUBCATCH`, `swmm_NODE`, and `swmm_LINK`.

- The _name_ string must be dimensioned to _size+1_ or greater by the calling program.

- If the specified object does not exist then _name_ is returned as an empty string.

---

<!---
  swmm_getindex
-->

### swmm_getIndex {#swmm_getindex}

**Syntax**

`int swmm_getIndex(int objType, const char* name);`

**Description**

Retrieves the index of an object given its ID name.

**Arguments**

_objType_ type of SWMM object.

_name_ the ID name of a specific object type.

**Return Value**

Returns the object's position (from 0) in the array of like objects or -1 if the object does not exist.

**Notes**

Valid object types are `swmm_GAGE`, `swmm_SUBCATCH`, `swmm_NODE`, and `swmm_LINK`.

---

<!--
  swmm_getvalue
-->

### swmm_getValue {#swmm_getvalue}

**Syntax**

`double swmm_getValue(int property, int index);`

**Description**

Retrieves the value of an object's property.

**Arguments**

_property_ a property code associated with an object.

_index_ the position (from 0) of the object in the list of like objects.

**Return Value**

Returns the requested property value or 0 if the object or property does not exist.

**Notes**

The table below lists the property codes and the values they return for each type of object.

| Object Type   | Property Code            | Retrieved Value                                                                                                                                                                                                                                                                                        |
| :------------ | :----------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Rain Gages    | `swmm_GAGE_RAINFALL`     | current rainfall rate (in/hr or mm/hr)                                                                                                                                                                                                                                                                 |
| Subcatchments | `swmm_SUBCATCH_AREA`     | subcatchment area (acres or hectares)                                                                                                                                                                                                                                                                  |
| ^             | `swmm_SUBCATCH_RAINGAGE` | index of subcatchment's rain gage                                                                                                                                                                                                                                                                      |
| ^             | `swmm_SUBCATCH_RAINFALL` | current rainfall rate (in/hr or mm/hr)                                                                                                                                                                                                                                                                 |
| ^             | `swmm_SUBCATCH_EVAP`     | current evaporation rate (in/day or mm/day)                                                                                                                                                                                                                                                            |
| ^             | `swmm_SUBCATCH_INFIL`    | current infiltration rate (in/hr or mm/hr)                                                                                                                                                                                                                                                             |
| ^             | `swmm_SUBCATCH_RUNOFF`   | current runoff rate (flow units)                                                                                                                                                                                                                                                                       |
| ^             | `swmm_SUBCATCH_RPTFLAG`  | = 1 if results saved to output file, 0 if not                                                                                                                                                                                                                                                          |
| Nodes         | `swmm_NODE_TYPE`         | `swmm_JUNCTION` = 0 <br> `swmm_OUTFALL` = 1 <br> `swmm_STORAGE` = 2 <br> `swmm_DIVIDER` = 3                                                                                                                                                                                                            |
| ^             | `swmm_NODE_ELEV`         | node invert elevation (ft or m)                                                                                                                                                                                                                                                                        |
| ^             | `swmm_NODE_MAXDEPTH`     | node maximum depth (ft or m)                                                                                                                                                                                                                                                                           |
| ^             | `swmm_NODE_DEPTH`        | current computed water depth (ft or m)                                                                                                                                                                                                                                                                 |
| ^             | `swmm_NODE_HEAD`         | current computed water surface level (ft or m)                                                                                                                                                                                                                                                         |
| ^             | `swmm_NODE_VOLUME`       | current computed stored volume (ft<sup>3</sup> or m<sup>3</sup>)                                                                                                                                                                                                                                       |
| ^             | `swmm_NODE_LATFLOW`      | current lateral inflow (flow units)                                                                                                                                                                                                                                                                    |
| ^             | `swmm_NODE_INFLOW`       | current total inflow (flow units)                                                                                                                                                                                                                                                                      |
| ^             | `swmm_NODE_OVERFLOW`     | current overflow (flooding) (flowunits)                                                                                                                                                                                                                                                                |
| ^             | `swmm_NODE_RPTFLAG`      | = 1 if results saved to output file, 0 if not                                                                                                                                                                                                                                                          |
| Links         | `swmm_LINK_TYPE`         | `swmm_CONDUIT` = 0 <br> `swmm_PUMP` = 1 <br> `swmm_ORIFICE` = 2 <br> `swmm_WEIR` = 3 <br> `swmm_OUTLET` = 4                                                                                                                                                                                            |
| ^             | `swmm_LINK_NODE1`        | index of upstream node                                                                                                                                                                                                                                                                                 |
| ^             | `swmm_LINK_NONE2`        | index of downstream node                                                                                                                                                                                                                                                                               |
| ^             | `swmm_LINK_LENGTH`       | length of link (ft or m)                                                                                                                                                                                                                                                                               |
| ^             | `swmm_LINK_SLOPE`        | link slope (rise / run)                                                                                                                                                                                                                                                                                |
| ^             | `swmm_LINK_FULLDEPTH`    | full depth of flow (ft or m)                                                                                                                                                                                                                                                                           |
| ^             | `swmm_LINK_FULLFLOW`     | Manning flow rate when full (flow units)                                                                                                                                                                                                                                                               |
| ^             | `swmm_LINK_FLOW`         | current computed flow rate (flow units)                                                                                                                                                                                                                                                                |
| ^             | `swmm_LINK_VELOCITY`     | current computed flow velocity (ft/s or m/s)                                                                                                                                                                                                                                                           |
| ^             | `swmm_LINK_DEPTH`        | current computed flow depth (ft or m)                                                                                                                                                                                                                                                                  |
| ^             | `swmm_LINK_TOPWIDTH`     | current computed water surface width (ft or m)                                                                                                                                                                                                                                                         |
| ^             | `swmm_LINK_SETTING`      | current link setting (ratio)                                                                                                                                                                                                                                                                           |
| ^             | `swmm_LINK_TIMEOPEN`     | time since link was last opened (hours)                                                                                                                                                                                                                                                                |
| ^             | `swmm_LINK_TIMECLOSED`   | time since link was last closed (hours)                                                                                                                                                                                                                                                                |
| ^             | `swmm_LINK_RPTFLAG`      | = 1 if results saved to output file, 0 if not                                                                                                                                                                                                                                                          |
| System        | `swmm_STARTDATE`         | starting date and time of the simulation (a DateTime value)                                                                                                                                                                                                                                            |
| ^             | `swmm_CURRENTDATE`       | current simulation date and time (a DateTime value)                                                                                                                                                                                                                                                    |
| ^             | `swmm_ELAPSEDTIME`       | current elapsed simulation time (hours)                                                                                                                                                                                                                                                                |
| ^             | `swmm_ROUTESTEP`         | current flow routing time step (seconds)                                                                                                                                                                                                                                                               |
| ^             | `swmm_MAXROUTESTEP`      | largest routing time step that satisfies the Courant stability criteria at the current time period (seconds)                                                                                                                                                                                           |
| ^             | `swmm_REPORTSTEP`        | reporting time step (seconds)                                                                                                                                                                                                                                                                          |
| ^             | `swmm_TOTALSTEPS`        | current number of reporting time steps taken                                                                                                                                                                                                                                                           |
| ^             | `swmm_NOREPORT`          | 0 if SWMM's normal reporting of results has been disabled, 1 if not                                                                                                                                                                                                                                    |
| ^             | `swmm_FLOWUNIT`          | code number of flow units used by a project: <br> `swmm_CFS` = 0 (cubic feet / second) <br> `swmm_GPM` = 1 (gallons / minute) <br> `swmm_MGD` = 2 (million gallons / day) <br> `swmm_CMS` = 3 (cubic meters / second) <br> `swmm_LPS` = 4 (liters / second) <br> `swmm_MLD` = 5 (million liters / day) |

The meaning of `swmm_LINK_SETTING` depends on the type of link it refers to:

- Conduit - fraction of full flow volume utilized

- Pump - relative speed setting (0 means pump is off)

- Orifice - fraction of full opening available for flow (0 to 1)

- Weir - fraction of full opening available for flow (0 to 1)

- Outlet - fraction of normal flow rate utilized (0 to 1)

`swmm_LINKFLOW` is positive if the flow direction is from the link's upstream node to its downstream node or negative if reverse flow occurs.

SWMM's **DateTime** variable type encodes a calendar date and time of day as the number of decimal days since midnight of December 30, 1899 (the same as Excel uses). It can be decoded using the `swmm_decodeDate` function.

If `swmm_FLOWUNITS` is in metric units then all other project variables are expressed in metric units. Otherwise US standard units are used throughout.

---

<!--
  swmm_getsavedvalue
-->

### swmm_getSavedValue {#swmm_getsavedvalue}

**Syntax**

`double swmm_getSavedValue(int property, int index, int period);`

**Description**

Retrieves an object's computed value at a specified reporting time period.

**Arguments**

_property_ a property code associated with an object.

_index_ the position (from 0) of the object in the list of like objects.

_period_ a reporting time period index (starting from 1)

**Return Value**

Returns the requested property value or 0 if the object or property does not exist.

**Notes**

This function allows one to retrieve computed values saved to SWMM's output results file after a simulation has been completed only if the [swmm_start](#swmm_start) function was called with an argument equal to 1.

The table below lists the property codes and the values they return for each type of object.

| Object Type   | Property Code            | Retrieved Value                                                               |
| :------------ | :----------------------- | :---------------------------------------------------------------------------- |
| Subcatchments | `swmm_SUBCATCH_RAINFALL` | rainfall rate (in/hr or mm/hr)                                                |
| ^             | `swmm_SUBCATCH_EVAP`     | evaporation rate (in/day or mm/day)                                           |
| ^             | `swmm_SUBCATCH_INFIL`    | infiltration rate (in/hr or mm/hr)                                            |
| ^             | `swmm_SUBCATCH_RUNOFF`   | runoff rate (flow units)                                                      |
| Nodes         | `swmm_NODE_DEPTH`        | water depth (ft or m)                                                         |
| ^             | `swmm_NODE_HEAD`         | water surface level (ft or m)                                                 |
| ^             | `swmm_NODE_VOLUME`       | stored volume (ft<sup>3</sup> or m<sup>3</sup>)                               |
| ^             | `swmm_NODE_LATFLOW`      | lateral inflow (flow units)                                                   |
| ^             | `swmm_INFLOW`            | total inflow (flow units)                                                     |
| ^             | `swmm_OVERFLOW`          | overflow (flooding) (flow units)                                              |
| Links         | `swmm_LINK_FLOW`         | flow rate (flow units)                                                        |
| ^             | `swmm_LINK_VELOCITY`     | flow velocity (ft/s or m/s)                                                   |
| ^             | `swmm_LINK_DEPTH`        | flow depth (ft or m)                                                          |
| ^             | `swmm_LINK_SETTING`      | link setting (e.g., pump speed, orifice/weir opening, outlet flow adjustment) |
| System        | `swmm_CURRENTDATE`       | simulation date and time stored as a `DateTime` object                        |

SWMM's `DateTime` variable type encodes a calendar date and time of day as the number of decimal days since midnight of December 30, 1899 (the same as Excel uses). It can be decoded using the [swmm_decodeDate](#swmm_decodedate) function.

Only objects that were marked for reporting in the [REPORT](#report) section of the SWMM input file will have their computed results saved to SWMM's output file. One can see if this is the case by using the [swmm_getValue](#swmm_getvalue) function to check an object's `RPTFLAG` property. If the object was not marked for reporting then `swmm_getSavedValue` will return 0.

---

<!--
  swmm_setvalue
-->

### swmm_setValue {#swmm_setvalue}

**Syntax**

`void swmm_setValue(int property, int index, double value);`

**Description**

Sets the value of object properties that provide external inputs, boundary conditions and time steps for a simulation.

**Arguments**

_property_ a property code associated with an object

_index_ the position (from 0) of the object in the list of like objects.

_value_ the property's new value.

**Return Value**

None.

**Notes**

The table below lists the property codes and the quantities they
represent.

| Object Type  | Property Code           | Description                                                |
| :----------- | :---------------------- | :--------------------------------------------------------- |
| Rain Gage    | `swmm_GAGE_RAINFALL`    | rainfall rate (in/hr or mm/hr)                             |
| Subcatchment | `swmm_SUBCATCH_RPTFLAG` | = 1 if results saved to output file, 0 if not              |
| Node         | `swmm_NODE_HEAD`        | fixed head at an outfall node (ft or m)                    |
| ^            | `swmm_NODE_LATFLOW`     | lateral inflow (flow units)                                |
| ^            | `swmm_NODE_RPTFLAG`     | = 1 if results saved to output file, 0 if not              |
| Link         | `swmm_LINK_SETTING`     | link setting (see note below)                              |
| ^            | `swmm_LINK_RPTFLAG`     | = 1 if results saved to output file, 0 if not              |
| System       | `swmm_ROUTESTEP`        | flow routing time step (decimal seconds)                   |
| ^            | `swmm_REPORTSTEP`       | reporting time step (seconds)                              |
| ^            | `swmm_NOREPORT`         | enables/disables SWMM\'s status reporting (see note below) |

The meaning of `swmm_LINK_SETTING` depends on the type of link it refers to:

- Conduit - ignored

- Pump - relative speed setting (0 means pump is off)

- Orifice - fraction of full opening available for flow (0 to 1)

- Weir - fraction of full opening available for flow (0 to 1)

- Outlet - fraction of normal flow rate realized (0 to 1)

The `swmm_GAGE`, `swmm_NODE`, and `swmm_LINK` properties can only be set after an analysis has begun (after `swmm_start` has been called).

`swmm_REPORTSTEP`, `swmm_NOREPORT` and the `RPTFLAG`variables can only be set prior to an analysis having begun (before `swmm_start` has been called).

If `swmm_NOREPORT` is set to 1 then SWMM's normal writing of summary information to its report file is suppressed. This summary information can include a list of project analysis options, a summary of the project's data, a simulation's mass balance errors, some flow routing performance statistics, a list of control actions taken and tables of summary output results for all subcatchments, nodes and links.

`swmm_ROUTESTEP` can be set either before or during a simulation. During a simulation it can be assigned a value that satisfies the Courant stability criterion by using code that looks as follows:

```C
double elapsedTime, routeStep;

swmm_start(1);
do {
    swmm_step(&elapsedTime);
    routeStep = swmm_getValue(swmm_MAXROUTESTEP, 0);
    swmm_setValue(swmm_ROUTESTEP, 0, routeStep);
} while (elapsedTime > 0.0);

swmm_end();
```

---

<!--
  swmm_writeline
-->

### swmm_writeLine {#swmm_writeline}

**Syntax**

`void swmm_writeLine(const char *line);`

**Description**

Writes a line of text to SWMM's report file.

**Arguments**

_line_ a character string containing the text to be written.

**Return Value**

None.

<!---
  swmm_decodedate
-->

### swmm_decodeDate {#swmm_decodedate}

**Syntax**

```
void swmm_decodeDate(double date
                     int *year, int *month,  int *day,
                     int *hour, int *minute, int *second,
                     int *dayOfWeek);
```

**Description**

Decodes a SWMM `DateTime` value into a calendar date and clock time.

**Arguments**

_date_ a DateTime value (as decimal days since Dec 30, 1899) to be decoded

_year_ the year encoded in the _date_

_month_ the month of the year encoded in the _date_ ( 1 to 12)

_day_ the day of the month encoded in the _date_ (1 to 31)

_hour_ the hour encoded in the time portion of the _date_ (0 to 23)

_minute_ the minutes  encoded in the time portion of the _date_ (0 to 59)

_second_ the seconds encoded in the time portion of the _date_ (0 to 59)

_dayOfWeek_ the day of the week encoded in the _date_ (Sunday = 1)

**Return Value**

None.

**Notes**

The _date_ argument is an input parameter while the others are outputs.

---

## Appendix B. SWMM5's Input File Format {#appendix-b}

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

---

<!---
  title
-->

### [TITLE] {#title}

**Purpose:**

Attaches a descriptive title to the project being analyzed.

**Format:**

Any number of lines may be entered. The first line will be used as a page header in the output report.

---

<!---
  options
-->

### [OPTIONS] {#options}

**Purpose:**

Provides values for various analysis options.

**Format:**

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

**Remarks:**

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

---

<!---
  report
-->

### [REPORT] {#report}

**Purpose:**

Describes the contents of the report file that SWMM produces.

**Formats:**

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

**Remarks:**

**INPUT** specifies whether or not a summary of the input data should be provided in the output report. The default is **NO**.

**CONTINUITY** specifies whether continuity checks should be reported or not. The default is **YES**.

**FLOWSTATS** specifies whether summary flow statistics should be reported or not. The default is **YES**.

**CONTROLS** specifies whether all control actions taken during a simulation should be listed or not. The default is **NO**.

**SUBCATCHMENTS** gives a list of subcatchments whose results are to be reported. The default is **NONE**.

**NODES** gives a list of nodes whose results are to be reported. The default is **NONE**.

**LINKS** gives a list of links whose results are to be reported. The default is **NONE**.

**LID** specifies that the LID control Name in subcatchment _Subcatch_ should have a detailed performance report for it written to file Fname.

The SUBCATCHMENTS, NODES, LINKS, and LID lines can be repeated multiple times.

---

<!---
  files
-->

### [FILES] {#files}

**Purpose:**

Identifies optional interface files used or saved by a run.

**Formats:**

|                             |         |
| --------------------------- | ------- |
| **USE / SAVE** **RAINFALL** | _Fname_ |
| **USE / SAVE** **RUNOFF**   | _Fname_ |
| **USE / SAVE** **HOTSTART** | _Fname_ |
| **USE / SAVE** **RDII**     | _Fname_ |
| **USE** **INFLOWS**         | _Fname_ |
| **SAVE** **OUTFLOWS**       | _Fname_ |

**Parameters:**

_Fname_ -- the name of an interface file.

**Remarks:**

SWMM can use several different kinds of interface files that contain either externally imposed inputs (e.g., rainfall or infiltration/inflow hydrographs) or the results of previously run analyses (e.g., runoff or routing results). These files can help speed up simulations, simplify comparisons of different loading scenarios, and allow large study areas to be broken up into smaller areas that can be analyzed individually. The different types of interface files that are currently available include:

- rainfall interface file

- runoff interface file

- hot start file

- RDII interface file

- routing interface files

Rainfall, Runoff, and RDII files can either be used or saved in a run, but not both. A run can both use and save a Hot Start file (with different names).

Enclose the external file name in double quotes if it contains spaces and include its full path if it resides in a different directory than the SWMM input file.

---

<!---
  raingages
-->

### [RAINGAGES] {#raingages}

**Purpose:**

Identifies each rain gage that provides rainfall data for the study area.

**Formats:**

_Name_ _Form_ _Intvl_ _SCF_ **TIMESERIES** _Tseries_

_Name_ _Form_ _Intvl_ _SCF_ **FILE** _Fname_ (_Sta_ _Units_)

**Parameters:**

_Name_ -- name assigned to rain gage.

_Form_ -- form of recorded rainfall, either **INTENSITY**, **VOLUME** or **CUMULATIVE**.

_Intvl_ -- time interval between gage readings in decimal hours or hours:minutes format (e.g., 0:15 for 15-minute readings).

_SCF_ -- snow catch deficiency correction factor (use 1.0 for no adjustment).

_Tseries_ -- name of a time series in the [TIMESERIES] section with rainfall data.

_Fname_ -- name of an external file with rainfall data. Rainfall files are discussed in Section 11.3.

_Sta_ -- name of the recording station in a user-prepared formatted rain file.

_Units_ -- rain depth units for the data in a user-prepared formatted rain file, either **IN** (inches) or **MM** (millimeters).

**Remarks:**

Enclose the external file name in double quotes if it contains spaces and include its full path if it resides in a different directory than the SWMM input file.

The station name and depth units entries are only required when using a user-prepared formatted rainfall file.

---

<!---
  evaporation
-->

### [EVAPORATION] {#evaporation}

**Purpose:**

Specifies how daily potential evaporation rates vary with time for the study area.

**Formats:**

|                 |                                            |
| :-------------- | :----------------------------------------- |
| **CONSTANT**    | _evap_                                     |
| **MONTHLY**     | _e1 e2 e3 e4 e5 e6 e7 e8 e9 e10 e11 e12_   |
| **TIMESERIES**  | _Tseries_                                  |
| **TEMPERATURE** |                                            |
| **FILE**        | (_p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12_) |
| **RECOVERY**    | _patternID_                                |
| **DRY_ONLY**    | **NO / YES**                               |

**Parameters:**

_evap_ -- constant evaporation rate (in/day or mm/day).

_e1_ -- evaporation rate in January (in/day or mm/day).

...

_e12_ -- evaporation rate in December (in/day or mm/day).

_Tseries_ -- name of a time series in the [TIMESERIES] section with evaporation data.

_p1_ -- pan coefficient for January.

...

_p12_ -- pan coefficient for December.

_patID_ -- name of a monthly time pattern.

**Remarks:**

Use only one of the above formats (**CONSTANT**, **MONTHLY**, **TIMESERIES**, **TEMPERATURE**, or **FILE**). If no [EVAPORATION] section appears, then evaporation is assumed to be 0.

**TEMPERATURE** indicates that evaporation rates will be computed from the daily air temperatures contained in an external climate file whose name is provided in the [TEMPERATURE] section. This method also uses the site’s latitude, which can also be specified in the [TEMPERATURE] section.

**FILE** indicates that evaporation data will be read directly from the same external climate file used for air temperatures as specified in the [TEMPERATURE] section. Supplying monthly pan coefficients for these data is optional.

**RECOVERY** identifies an optional monthly time pattern of multipliers used to modify infiltration recovery rates during dry periods. For example, if the normal infiltration recovery rate was 1% during a specific time period and a pattern factor of 0.8 applied to this period, then the actual recovery rate would be 0.8%.

**DRY_ONLY** determines if evaporation only occurs during periods with no precipitation. The default is **NO**.

The evaporation rates provided in this section are potential rates. The actual amount of water evaporated will depend on the amount available as a simulation progresses.

---

<!---
  temperature
-->

### [TEMPERATURE] {#temperature}

**Purpose:**

Specifies daily air temperatures, monthly wind speed, and various snowmelt parameters for the study area. Required only when snowmelt is being modeled or when evaporation rates are computed from daily temperatures or are read from an external climate file.

**Formats:**

|                    |                                                      |
| :----------------- | :--------------------------------------------------- |
| **TIMESERIES**     | _Tseries_                                            |
| **FILE**           | _Fname_ (_Start_) (_Units_)                          |
| **WINDSPEED**      | **MONTHLY** _s1 s2 s3 s4 s5 s6 s7 s8 s9 s10 s11 s12_ |
| **WINDSPEED**      | **FILE**                                             |
| **SNOWMELT**       | _Stemp  ATIwt RNM Elev Lat DTLong_                   |
| **ADC IMPERVIOUS** | _f.0 f.1 f.2 f.3 f.4 f.5 f.6 f.7 f.8 f.9_            |
| **ADC PERVIOUS**   | _f.0 f.1 f.2 f.3 f.4 f.5 f.6 f.7 f.8 f.9_            |

**Parameters:**

_Tseries_ -- name of a time series in the [TIMESERIES] section with temperature data.

_Fname_ -- name of an external Climate file with temperature data.

_Start_ --date to begin reading from the file in month/day/year format (default is the beginning of the file).

_Units_ -- temperature units for GHCN files (**C10** for tenths of a degree C (the default), **C** for degrees C or **F** for degrees F.

_s1_ -- average wind speed in January (mph or km/hr).

...

_s12_ -- average wind speed in December (mph or km/hr).

_Stemp_ -- air temperature at which precipitation falls as snow (deg F or C).

_ATIwt_ -- antecedent temperature index weight (default is 0.5).

_RNM_ -- negative melt ratio (default is 0.6).

_Elev_ -- average elevation of study area above mean sea level (ft or m) (default is 0).

_Lat_ -- latitude of the study area in degrees North (default is 50).

_DTLong_ -- correction, in minutes of time, between true solar time and the standard clock time (default is 0).

_f.0_ -- fraction of area covered by snow when ratio of snow depth to depth at 100% cover is 0

...

_f.9_ -- fraction of area covered by snow when ratio of snow depth to depth at 100% cover is 0.9.

**Remarks:**

Use the **TIMESERIES** line to read air temperature from a time series or the **FILE** line to read it from an external Climate file. Climate files are discussed in Section 11.4. If neither format is used, then air temperature remains constant at 70 degrees F.

Enclose the Climate file name in double quotes if it contains spaces and include its full path if it resides in a different directory than the SWMM input file.

Temperatures supplied from NOAA's latest Climate Data Online GHCN files should have their units (**C** or **F**) specified. Older versions of these files listed temperatures in tenths of a degree C (**C10**). An asterisk can be entered for the Start date if it defaults to the beginning of the file.

Wind speed can be specified either by monthly average values or by the same Climate file used for air temperature. If neither option appears, then wind speed is assumed to be 0.

Separate Areal Depletion Curves (ADC) can be defined for impervious and pervious sub-areas. The ADC parameters will default to 1.0 (meaning no depletion) if no data are supplied for a particular type of sub-area.

---

<!---
  adjustments
-->

### [ADJUSTMENTS] {#adjustments}

**Purpose:**

Specifies optional monthly adjustments to be made to temperature,
evaporation rate, rainfall intensity and hydraulic conductivity in each
time period of a simulation.

**Formats:**

|                  |                                          |
| :--------------- | :--------------------------------------- |
| **TEMPERATURE**  | _t1 t2 t3 t4 t5 t6 t7 t8 t9 t10 t11 t12_ |
| **EVAPORATION**  | _e1 e2 e3 e4 e5 e6 e7 e8 e9 e10 e11 e12_ |
| **RAINFALL**     | _r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12_ |
| **CONDUCTIVITY** | _c1 c2 c3 c4 c5 c6 c7 c8 c9 c10 c11 c12_ |

**Parameters:**

_t1..t12_ -- adjustments to temperature in January, February, etc., as plus or minus degrees F (degrees C).

_e1..e12_ -- adjustments to evaporation rate in January, February, etc., as plus or minus in/day (mm/day).

_r1..r12_ -- multipliers applied to precipitation rate in January, February, etc.

_c1..c12_ -- multipliers applied to soil hydraulic conductivity in January, February, etc. used in either Horton or Green-Ampt infiltration.

**Remarks:**

The same adjustment is applied for each time period within a given month and is repeated for that month in each subsequent year being simulated.

---

<!---
  subcatchments
-->

### [SUBCATCHMENTS] {#subcatchments}

**Purpose:**
Identifies each subcatchment within the study area. Subcatchments are
land area units which generate runoff from rainfall.

**Format:**

_Name Rgage OutID Area %Imperv Width Slope Clength (Spack)_

**Parameters:**

_Name_ -- name assigned to the subcatchment.

_Rgage_ -- name of a rain gage in the [RAINGAGES] section assigned to the subcatchment.

_OutID_ -- name of the node or subcatchment that receives runoff from the subcatchment.

_Area_ -- area of the subcatchment (acres or hectares).

_%Imperv_ -- percentage of the subcatchment’s area that is impervious.

_Width_ -- characteristic width of the subcatchment (ft or meters).

_Slope_ -- the subcatchment’s slope (percent).

_Clength_ -- total curb length (any length units) used to describe pollutant buildup. Use 0 if not applicable.

_Spack_ -- optional name of a snow pack object (from the [SNOWPACKS] section) that characterizes snow accumulation and melting over the subcatchment.

---

<!---
  subareas
-->

### [SUBAREAS] {#subareas}

**Purpose:**

Supplies information about pervious and impervious areas for each subcatchment. Each subcatchment can consist of a pervious sub-area, an impervious sub-area with depression storage, and an impervious sub-area without depression storage.

**Format:**

_Subcat Nimp Nperv Simp Sperv %Zero RouteTo (%Routed)_

**Parameters:**

_Subcat_ -- subcatchment name.

_Nimp_ -- Manning's coefficient (n) for overland flow over the impervious sub-area.

_Nperv_ -- Manning's coefficient (n) for overland flow over the pervious sub-area.

_Simp_ -- depression storage for the impervious sub-area (inches or mm).

_Sperv_ -- depression storage for the pervious sub-area (inches or mm).

_%Zero_ -- percent of impervious area with no depression storage.

_RouteTo_ -- **IMPERVIOUS** if pervious area runoff runs onto impervious area, **PERVIOUS** if impervious runoff runs onto pervious area, or **OUTLET** if both areas drain to the subcatchment's outlet (default = **OUTLET**).

_%Routed_ -- percent of runoff routed from one type of area to another (default = 100).

---

<!---
  infiltration
-->

### [INFILTRATION] {#infiltration}

**Purpose:**

Supplies infiltration parameters for each subcatchment. Rainfall lost to infiltration only occurs over the pervious sub-area of a subcatchment.

**Format:**

_Subcat  p1  p2  p3  (p4  p5)  (Method)_

**Parameters:**

Subcat -- subcatchment name.

Method -- either **HORTON**, **MODIFIED_HORTON**, **GREEN_AMPT**, **MODIFIED_GREEN_AMPT**, or **CURVE_NUMBER**. If not specified then the infiltration method supplied in the [OPTIONS] section is used.

<u>For Horton and Modified Horton Infiltration:</u>

_p1_ -- maximum infiltration rate on the Horton curve (in/hr or mm/hr).

_p2_ -- minimum infiltration rate on the Horton curve (in/hr or mm/hr).

_p3_ -- decay rate constant of the Horton curve (1/hr).

_p4_ -- time it takes for a fully saturated soil to dry  (days).

_p5_ -- maximum infiltration volume possible (0 if not applicable) (in or mm).

<u>For Green-Ampt and Modified Green-Ampt Infiltration:</u>

_p1_ -- soil capillary suction (in or mm).

_p2_ -- soil saturated hydraulic conductivity (in/hr or mm/hr).

_p3_ -- initial soil moisture deficit (porosity minus moisture content) (fraction).

<u>For Curve-Number Infiltration:</u>

_p1_ -- SCS Curve Number.

_p2_ -- no longer used.

_p3_ -- time it takes for a fully saturated soil to dry (days).

---

<!---
  lid_controls
-->

### [LID_CONTROLS] {#lid_controls}

**Purpose:**

Defines scale-independent LID controls that can be deployed within subcatchments.

**Formats:**

|        |        |
| :----- | :----- |
| _Name_ | _Type_ |

followed by one or more of the following lines depending on Type:

|        |              |                                                |
| :----- | :----------- | :--------------------------------------------- |
| _Name_ | **SURFACE**  | _StorHt VegFrac Rough Slope Xslope_            |
| _Name_ | **SOIL**     | _Thick Por FC WP Ksat Kcoeff Suct_             |
| _Name_ | **PAVEMENT** | _Thick Vratio FracImp Perm Vclog (Treg Freg)_  |
| _Name_ | **STORAGE**  | _Height Vratio Seepage Vclog (Covrd)_          |
| _Name_ | **DRAIN**    | _Coeff Expon Offset Delay (Hopen Hclose Qcrv)_ |
| _Name_ | **DRAINMAT** | _Thick Vratio Rough_                           |
| _Name_ | **REMOVALS** | _Pollut Rmvl Pollut Rmvl ..._                  |

**Parameters:**

_Name_ -- name assigned to LID process.

_Type_ -- **BC** for bio-retention cell; **RG** for rain garden; **GR** for green roof; **IT** for infiltration trench; **PP** for permeable pavement; **RB** for rain barrel; **RD** for rooftop disconnection; **VS** for vegetative swale.

_Pollut_ -- name of a pollutant

_Rmvl_ -- the percent removal the LID achieves for the pollutant (several pollutant removals can be placed on the same line or specified in separate **REMOVALS** lines).

<u>For LIDs with Surface Layers:</u>

_StorHt_ -- when confining walls or berms are present this is the maximum depth to which water can pond above the surface of the unit before overflow occurs (in inches or mm). For LIDs that experience overland flow it is the height of any surface depression storage. For swales, it is the height of its trapezoidal cross-section.

_VegFrac_ -- fraction of the surface storage volume that is filled with vegetation.

_Rough_ -- Manning's coefficient (n) for overland flow over surface soil cover, pavement, roof surface or a vegetative swale. Use 0 for other types of LIDs.

_Slope_ -- slope of a roof surface, pavement surface or vegetative swale (percent). Use 0 for other types of LIDs.

_Xslope_ -- slope (run over rise) of the side walls of a vegetative swale's cross-section. Use 0 for other types of LIDs.

If either _Rough_ or _Slope_ values are 0 then any ponded water that exceeds
the surface storage depth is assumed to completely overflow the LID
control within a single time step.

<u>For LIDs with Pavement Layers:</u>

_Thick_ -- thickness of the pavement layer (inches or mm).

_Vratio_ -- void ratio (volume of void space relative to the volume of solids in the pavement for continuous systems or for the fill material used in modular systems). Note that porosity = void ratio / (1 + void ratio).

_FracImp_ -- ratio of impervious paver material to total area for modular systems; 0 for continuous porous pavement systems.

_Perm_ -- permeability of the concrete or asphalt used in continuous systems or hydraulic conductivity of the fill material (gravel or sand) used in modular systems (in/hr or mm/hr).

_Vclog_ -- the number of pavement layer void volumes of runoff treated it takes to completely clog the pavement. Use a value of 0 to ignore clogging.

_Treg_ -- the number of days that the pavement layer is allowed to clog before its permeability is restored, typically by vacuuming its surface. A value of 0 (the default) indicates that no permeability regeneration occurs.

_Freg_ -- The fractional degree to which the pavement's permeability is restored when a regeneration interval is reached. The default is 0 (no restoration) while a value of 1 indicates complete restoration to the original permeability value. Once regeneration occurs the pavement begins to clog once again at a rate determined by Vclog.

<u>For LIDs with Soil Layers:</u>

_Thick_ -- thickness of the soil layer (inches or mm).

_Por_ -- soil porosity (pore space volume / total volume).

_FC_ -- soil field capacity (moisture content of a fully drained soil).

_WP_ -- soil wilting point (moisture content of a fully dried soil).

_Ksat_ -- soil’s saturated hydraulic conductivity (in/hr or mm/hr).

_Kcoeff_ -- slope of the curve of log(conductivity) versus soil moisture deficit (porosity minus soil moisture) (dimensionless).

_Suct_ -- soil capillary suction (in or mm).

<u>For LIDs with Storage Layers:</u>

_Height_ -- thickness of the storage layer or height of a rain barrel (inches or mm).

_Vratio_ -- void ratio (volume of void space relative to the volume of solids in the layer). Note that porosity = void ratio / (1 + void ratio).

_Seepage_ -- the rate at which water seeps from the layer into the underlying native soil when first constructed (in/hr or mm/hr). If there is an impermeable floor or liner below the layer then use a value of 0.

_Vclog_ -- number of storage layer void volumes of runoff treated it takes to completely clog the layer. Use a value of 0 to ignore clogging.

_Covrd_ -- **YES** (the default) if a rain barrel is covered, **NO** if it is not.

Values for _Vratio_, _Seepage_, and _Vclog_ are ignored for rain barrels while _Covrd_ applies only to rain barrels.

<u>For LIDs with Drain Systems:</u>

_Coeff_ -- coefficient C that determines the rate of flow through the drain as a function of height of stored water above the drain bottom. For Rooftop Disconnection it is the maximum flow rate (in inches/hour or mm/hour) that the roof’s gutters and downspouts can handle before overflowing.

_Expon_ -- exponent n that determines the rate of flow through the drain as a function of height of stored water above the drain outlet.

_Offset_ -- height of the drain line above the bottom of the storage layer or rain barrel (inches or mm).

_Delay_ -- number of dry weather hours that must elapse before the drain line in a rain barrel is opened (the line is assumed to be closed once rainfall begins). A value of 0 signifies that the barrel's drain line is always open and drains continuously.  This parameter is ignored for other types of LIDs.

_Hopen_ -- The height of water  (in inches or mm) in the drain's Storage Layer that causes the drain to automatically open. Use 0 to disable this feature.

_Hclose_ -- The height of water (in inches or mm) in the drain's Storage Layer that causes the drain to automatically close. Use 0 to disable this feature.

_Qcurve_ -- The name of an optional Control Curve that adjusts the computed drain flow as a function of the head of water above the drain. Leave blank if not applicable.

<u>For Green Roof LIDs with Drainage Mats:</u>

_Thick_ -- thickness of the drainage mat (inches or mm).

_Vratio_ -- ratio of void volume to total volume in the mat.

_Rough_ -- Manning's coefficient (n) used to compute the horizontal flow rate of drained water through the mat.

**Remarks:**

The following table shows which layers are required (x) or are optional (o) for each type of LID process:

| LID Type              | Surface | Pavement | Soil | Storage | Drain | Drain Mat |
| :-------------------- | :-----: | :------: | :--: | :-----: | :---: | :-------: |
| Bio-Retention Cell    |    x    |          |   x  |    x    |  o    |           |
| Rain Garden           |    x    |          |  x   |         |       |           |
| Green Roof            |   x     |          | x    |         |       |     x     |
| Infiltration Trench   |   x     |          |      |   x     |   o   |           |
| Permeable Pavement    |    x    |    x     |  o   |    x    |  o    |           |
| Rain Barrel           |         |          |      |    x    |   x   |           |
| Rooftop Disconnection |    x    |          |      |         |   x   |           |
| Vegetative Swale      |    x    |          |      |         |       |           |

The equation used to compute flow rate out of the underdrain per unit area of the LID (in in/hr or mm/hr) is \f( q = C(h-H*{d})^{n} \f)  where \_q* is outflow, _h_ is height of stored water (inches or mm) and _H_<sub>_d_</sub> is the drain offset height. Note that the units of _C_ depend on the unit system being used as well as the value assigned to _n_.

The actual dimensions of an LID control are provided in the [LID_USAGE] section when it is placed in a particular subcatchment.

**Examples:**

```
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
```

---

<!---
  lid_usage
-->

### [LID_USAGE] {#lid_usage}

**Purpose:**

Deploys LID controls within specific subcatchment areas.

**Format:**

_Subcat LID Number Area Width InitSat FromImp ToPerv_

(_RptFile DrainTo FromPerv_)

**Parameters:**

_Subcat_ -- name of the subcatchment using the LID process.

_LID_ -- name of an LID process defined in the [LID_CONTROLS] section.

_Number_ -- number of replicate LID units deployed.

_Area_ -- area of each replicate unit (ft2 or m2)

_Width_ -- width of the outflow face of each identical LID unit (in ft or m).
This parameter applies to roofs, pavement, trenches, and swales that use overland flow to convey surface runoff off of the unit. It can be set to 0 for other LID processes, such as bio-retention cells, rain gardens, and rain barrels that simply spill any excess captured runoff over their berms.

_InitSat_ -- the percent to which the LID's soil, storage, and drain mat zones are initially filled with water. For soil zones 0 % saturation corresponds to the wilting point moisture content while 100 % saturation has the moisture content equal to the porosity.

_FromImp_ -- the percent of the impervious portion of the subcatchment’s non-LID area whose runoff is treated by the LID practice. (E.g., if rain barrels are used to capture roof runoff and roofs represent 60% of the impervious area, then the impervious area treated is 60%). If the LID unit treats only direct rainfall, such as with a green roof, then this value should be 0. If the LID takes up the entire subcatchment then this field is ignored.

_ToPerv_ -- a value of 1 indicates that the surface and drain flow from the LID unit should be routed back onto the pervious area of the subcatchment that contains it. This would be a common choice to make for rain barrels, rooftop disconnection, and possibly green roofs. The default value is 0.

_RptFile_ -- optional name of a file to which detailed time series results for the LID will be written. Enclose the name in double quotes if it contains spaces and include its full path if it resides in a different directory than the SWMM input file. Use ‘\*’ if not applicable and an entry for DrainTo or FromPerv follows

_DrainTo_ -- optional name of subcatchment or node that receives flow from the unit’s drain line, if different from the outlet of the subcatchment that the LID is placed in. Use ‘\*’ if not applicable and an entry for FromPerv follows.

_FromPerv_ -- optional percent of the pervious portion of the subcatchment’s non-LID area whose runoff is treated by the LID practice. The default value is 0.

**Remarks:**

If _ToPerv_ is set to 1 and _DrainTo_ set to some other outlet, then only the excess surface flow from the LID unit will be routed back to the subcatchment’s pervious area while the underdrain flow will be sent to _DrainTo_.

More than one type of LID process can be deployed within a subcatchment as long as their total area does not exceed that of the subcatchment and the total percent impervious area treated does not exceed 100.

**Examples:**

```
;34 rain barrels of 12 sq ft each are placed in
;subcatchment S1. They are initially empty and treat 17%
;of the runoff from the subcatchment’s impervious area.
;The outflow from the barrels is returned to the
;subcatchment’s pervious area.
S1  RB14  34  12  0  0  17  1

;Subcatchment S2 consists entirely of a single vegetative
;swale 200 ft long by 50 ft wide.
S2  Swale  1  10000  50  0  0  0  “swale.rpt”
```

---

<!---
  aquifers
-->

### [AQUIFERS] {#aquifers}

**Purpose:**

Supplies parameters for each unconfined groundwater aquifer in the study area. Aquifers consist of two zones – a lower saturated zone and an upper unsaturated zone with a moving boundary between the two.

**Format:**

_Name Por WP FC Ks Kslp Tslp ETu ETs Seep Ebot Egw Umc (Epat)_

**Parameters:**

_Name_ -- name assigned to aquifer.

_Por_ -- soil porosity (pore space volume / total volume).

_WP_ -- soil wilting point (moisture content of a fully dried soil).

_FC_ -- soil field capacity (moisture content of a fully drained soil).

_Ks_ -- saturated hydraulic conductivity (in/hr or mm/hr).

_Kslp_ -- slope of the logarithm of hydraulic conductivity versus moisture deficit (porosity minus moisture content) curve (dimensionless).

_Tslp_ -- slope of soil tension versus moisture content curve (inches or mm).

_ETu_ -- fraction of total evaporation available for evapotranspiration in the upper unsaturated zone.

_ETs_ -- maximum depth into the lower saturated zone over which evapotranspiration can occur (ft or m).

_Seep_ -- seepage rate from saturated zone to deep groundwater when water table is at ground surface (in/hr or mm/hr).

_Ebot_ -- elevation of the bottom of the aquifer (ft or m).

_Egw_ -- groundwater table elevation at start of simulation (ft or m).

_Umc_ -- unsaturated zone moisture content at start of simulation (volumetric fraction).

_Epat_ -- name of optional monthly time pattern used to adjust the upper zone evaporation fraction for different months of the year.

**Remarks:**

Local values for _Ebot_, _Egw_, and _Umc_ can be assigned to specific subcatchments in the [GROUNDWATER] section.

<!---
  groundwater
-->

### [GROUNDWATER] {#groundwater}

**Purpose:**

Supplies parameters that determine the rate of groundwater flow between the aquifer underneath a subcatchment and a node of the conveyance system.

**Format:**

_Subcat Aquifer Node Esurf A1 B1 A2 B2 A3 Dsw (Egwt Ebot Egw Umc)_

**Parameters:**

_Subcat_ -- subcatchment name.

_Aquifer_ -- name of groundwater aquifer underneath the subcatchment.

_Node_ -- name of a node in the conveyance system exchanging groundwater with the aquifer.

_Esurf_ -- surface elevation of the subcatchment (ft or m).

_A1_ -- groundwater flow coefficient (see below).

_B1_ -- groundwater flow exponent (see below).

_A2_ -- surface water flow coefficient (see below).

_B2_ -- surface water flow exponent (see below).

_A3_ -- surface water – groundwater interaction coefficient (see below).

_Dsw_ -- fixed depth of surface water at the receiving node (ft or m) (set to zero if surface water depth will vary as computed by flow routing).

_Egwt_ -- threshold groundwater table elevation which must be reached before any flow occurs (ft or m). Leave blank (or enter \*) to use the elevation of the receiving node's invert.

The following optional parameters can be used to override the values supplied for the subcatchment’s aquifer.

Ebot -- elevation of the bottom of the aquifer (ft or m).

Egw -- groundwater table elevation at the start of the simulation (ft or m).

Umc -- unsaturated zone moisture content at start of simulation (volumetric fraction).

**Remarks:**

The flow coefficients are used in the following equation that determines the lateral groundwater flow rate based on groundwater and surface water elevations:

\f[ Q_{L}  =  A1 (H_{gw}  –  H_{cb})^{B1}  –  A2 (H_{sw}  –  H_{cb})^{B2}  +  A3 H_{gw} H_{sw} \f]

where:

\f$ Q\_{L \f$ = lateral groundwater flow (cfs per acre or cms per hectare),

\f$ H\_{gw} \f$ = height of saturated zone above the bottom of the aquifer (ft or m),

\f$ H\_{sw} \f$ = height of surface water at the receiving node above the aquifer bottom (ft or m),

\f$ H\_{cb} \f$ = height of the channel bottom above the aquifer bottom (ft or m).

---

<!---
  gwf
-->

### [GWF] {#gwf}

**Purpose:**

Defines custom groundwater flow equations for specific subcatchments.

**Format:**

_Subcat LATERAL/DEEP  Expr_

**Parameters:**

_Subcat_ -- subcatchment name.

_Expr_ -- a math formula expressing the rate of groundwater flow (in cfs per acre or cms per hectare for lateral flow or in/hr or mm/hr for deep flow) as a function of the following variables:

- **Hgw** (for height of the groundwater table)
- **Hsw** (for height of the surface water)
- **Hcb** (for height of the channel bottom)
- **Hgs** (for height of ground surface)

where all heights are relative to the aquifer bottom and have units of either feet or meters;

- **Ks** (for saturated hydraulic conductivity in in/hr or mm/hr)
- **K** (for unsaturated hydraulic conductivity in in/hr or mm/hr)
- **Theta** (for moisture content of the unsaturated zone)
- **Phi** (for aquifer soil porosity)
- **Fi** (for infiltration rate from the ground surface in in/hr or mm/hr)
- **Fu** (for percolation rate from the upper unsaturated zone in in/hr or mm/hr)
- **A** (for subcatchment area in acres or hectares)

**Remarks:**

Use **LATERAL** to designate an expression for lateral groundwater flow (to a node of the conveyance network) and **DEEP** for vertical loss to deep groundwater.

See the [TREATMENT] section for a list of built-in math functions that can be used in Expr. In particular, the STEP(x) function is 1 when x > 0 and is 0 otherwise.

**Examples:**

```
;Two-stage linear reservoir for lateral flow
Subcatch1 LATERAL 0.001*Hgw + 0.05*(Hgw–5)*STEP(Hgw–5)

;Constant seepage rate to deep aquifer
Subactch1  DEEP  0.002
```

---

<!---
  snowpacks
-->

### [SNOWPACKS] {#snowpacks}

**Purpose:**

Specifies parameters that govern how snowfall accumulates and melts on the plowable, impervious and pervious surfaces of subcatchments.

**Formats:**

|        |                |                                              |
| ------ | -------------- | -------------------------------------------- |
| _Name_ | **PLOWABLE**   | _Cmin  Cmax  Tbase  FWF  SD0  FW0  SNN0_     |
| _Name_ | **IMPERVIOUS** | _Cmin  Cmax  Tbase  FWF  SD0  FW0  SD100_    |
| _Name_ | **PERVIOUS**   | _Cmin  Cmax  Tbase  FWF  SD0  FW0  SD100_    |
| _Name_ | **REMOVAL**    | _Dplow Fout Fimp Fperv Fimelt (Fsub Scatch)_ |

**Parameters:**

_Name_ -- name assigned to snowpack parameter set .

_Cmin_ -- minimum melt coefficient (in/hr-deg F or mm/hr-deg C).

_Cmax_ -- maximum melt coefficient (in/hr-deg F or mm/hr-deg C).

_Tbase_ -- snow melt base temperature (deg F or deg C).

_FWF_ -- ratio of free water holding capacity to snow depth (fraction).

_SD0_ -- initial snow depth (in or mm water equivalent).

_FW0_ -- initial free water in pack (in or mm).

_SNN0_ -- fraction of impervious area that can be plowed.

_SD100_ -- snow depth above which there is 100% cover (in or mm water equivalent).

_Dplow_ -- depth of snow on plowable areas at which snow removal begins (in or mm).

_Fout_ -- fraction of snow on plowable area transferred out of watershed.

_Fimp_ -- fraction of snow on plowable area transferred to impervious area by plowing.

_Fperv_ -- fraction of snow on plowable area transferred to pervious area by plowing.

_Fimelt_ -- fraction of snow on plowable area converted into immediate melt.

_Fsub_ -- fraction of snow on plowable area transferred to pervious area in another subcatchment.

_Scatch_ -- name of subcatchment receiving the Fsub fraction of transferred snow.

**Remarks:**

Use one set of **PLOWABLE**, **IMPERVIOUS**, and **PERVIOUS** lines for each snow pack parameter set created. Snow pack parameter sets are assigned to specific subcatchments in the [SUBCATCHMENTS] section. Multiple subcatchments can share the same set of snow pack parameters.

The **PLOWABLE** line contains parameters for the impervious area of a subcatchment that is subject to snow removal by plowing but not to areal depletion. This area is the fraction _SNN0_ of the total impervious area. The **IMPERVIOUS** line contains parameter values for the remaining impervious area and the **PERVIOUS** line does the same for the entire pervious area. Both of the latter two areas are subject to areal depletion.

The **REMOVAL** line describes how snow removed from the plowable area is transferred onto other areas. The various transfer fractions should sum to no more than 1.0. If the line is omitted then no snow removal takes place.

---

<!---
  junctions
-->

### [JUNCTIONS] {#junctions}

**Purpose:**

Identifies each junction node of the drainage system.  Junctions are points in space where channels and pipes connect together. For sewer systems they can be either connection fittings or manholes.

**Format:**

_Name  Elev  (Ymax  Y0  Ysur  Apond)_

**Parameters:**

_Name_ -- name assigned to junction node.

_Elev_ -- elevation of the junction’s invert (ft or m).

_Ymax_ -- depth from ground to invert elevation (ft or m) (default is 0).

_Y0_ -- water depth at the start of the simulation (ft or m) (default is 0).

_Ysur_ -- maximum additional pressure head above the ground elevation that the junction can sustain under surcharge conditions (ft or m) (default is 0).

_Apond_ -- area subjected to surface ponding once water depth exceeds Ymax + Ysur (ft<sup>2</sup> or m<sup>2</sup>) (default is 0).

**Remarks:**

If _Ymax_ is 0 then SWMM sets the junction’s maximum depth to the distance from its invert to the top of the highest connecting link.

If the junction is part of a force main section of the system then set Ysur to the maximum pressure that the system can sustain.

Surface ponding can only occur when Apond is non-zero and the **ALLOW_PONDING** analysis option is turned on.

---

<!---
  outfalls
-->

### [OUTFALLS] {#outfalls}

**Purpose:**
Identifies each outfall node (i.e., final downstream boundary) of the drainage system and the corresponding water stage elevation.  Only one link can be incident on an outfall node.

**Formats:**

|        |        |                |           |             |             |
| :----- | :----- | :------------- | :-------- | :---------- | :---------- |
| _Name_ | _Elev_ | **FREE**       | (_Gated_) | (_RouteTo_) |             |
| _Name_ | _Elev_ | **NORMAL**     | (_Gated_) | (_RouteTo_) |             |
| _Name_ | _Elev_ | **FIXED**      | _Stage_   | (_Gated_)   | (_RouteTo_) |
| _Name_ | _Elev_ | **TIDAL**      | _Tcurve_  | (_Gated_)   | (_RouteTo_) |
| _Name_ | _Elev_ | **TIMESERIES** | _Tseries_ | (_Gated_)   | (_RouteTo_) |

**Parameters:**

_Name_ -- name assigned to outfall node.

_Elev_ -- node’s invert elevation (ft or m).

_Stage_ -- elevation of a fixed stage outfall (ft or m).

_Tcurve_ -- name of a curve in the [CURVES] section containing tidal height (i.e., outfall stage) v. hour of day over a complete tidal cycle.

_Tseries_ -- name of a time series in [TIMESERIES] section that describes how outfall stage varies with time.

_Gated_ -- **YES** or **NO** depending on whether a flap gate is present that prevents reverse flow. The default is **NO**.

_RouteTo_ -- optional name of a subcatchment that receives the outfall's discharge. The default is not to route the outfall’s discharge.

---

<!---
  dividers
-->

### [DIVIDERS] {#dividers}

**Purpose:**

Identifies each flow divider node of the drainage system. Flow dividers are junctions with exactly two outflow conduits where the total outflow is divided between the two in a prescribed manner.

**Formats:**

|        |        |           |              |            |                        |
| :----- | :----- | :-------- | :----------- | :--------- | :--------------------- |
| _Name_ | _Elev_ | _DivLink_ | **OVERFLOW** |            | (_Ymax Y0 Ysur Apond_) |
| _Name_ | _Elev_ | _DivLink_ | **CUTOFF**   | Qmin       | (_Ymax Y0 Ysur Apond)_ |
| _Name_ | _Elev_ | _DivLink_ | **TABULAR**  | Dcurve     | (_Ymax Y0 Ysur Apond_) |
| _Name_ | _Elev_ | _DivLink_ | **WEIR**     | Qmin Ht Cd | (_Ymax Y0 Ysur Apond_) |

**Parameters:**

_Name_ -- name assigned to divider node.

_Elev_ -- node’s invert elevation (ft or m).

_DivLink_ -- name of the link to which flow is diverted.

_Qmin_ -- flow at which diversion begins for either a CUTOFF or WEIR divider (flow units).

_Dcurve_ -- name of a curve for a TABULAR divider that relates diverted flow to total flow.

_Ht_ -- height of a WEIR divider (ft or m).

_Cd_ -- discharge coefficient for a WEIR divider.

_Ymax_ -- depth from the ground to the node’s invert elevation (ft or m) (default is 0).

_Y0_ -- water depth at the start of the simulation (ft or m) (default is 0).

_Ysur_ -- maximum additional pressure head above the ground elevation that the node can sustain under surcharge conditions (ft or m) (default is 0).

_Apond_ -- area subjected to surface ponding once water depth exceeds Ymax + Ysur (ft<sup>2</sup> or m<sup>2</sup>) (default is 0).

---

<!---
  storage
-->

### [STORAGE] {#storage}

**Purpose:**

Identifies each storage node of the drainage system. Storage nodes can have any shape as specified by a surface area versus water depth relation.

**Formats:**

|        |        |        |      |                |             |                             |
| :----- | :----- | :----- | :--- | :------------- | :---------- | :-------------------------- |
| _Name_ | _Elev_ | _Ymax_ | _Y0_ | **TABULAR**    | _Acurve_    | (_Ysur Fevap Psi Ksat IMD_) |
| _Name_ | _Elev_ | _Ymax_ | _Y0_ | **FUNCTIONAL** | _A1 A2 A0_  | (_Ysur Fevap Psi Ksat IMD_) |
| _Name_ | _Elev_ | _Ymax_ | _Y0_ | _Shape_        | _L  W  Z_   | (_Ysur Fevap Psi Ksat IMD_) |

**Parameters:**

_Name_ -- name assigned to storage node.

_Elev_ -- node’s invert elevation (ft or m).

_Ymax_ -- water depth when the storage node is full (ft or m).

_Y0_ -- water depth at the start of the simulation (ft or m).

_Acurve_ -- name of a curve in the [CURVES] section that relates surface area (ft<sup>2</sup> or m<sup>2</sup>) to depth (ft or m) for **TABULAR** geometry.

_A1_ -- coefficient of a **FUNCTIONAL** relation between surface area and depth.

_A2_ -- exponent of a **FUNCTIONAL** relation between surface area and depth.

_A0_ -- constant of a **FUNCTIONAL** relation between surface area and depth.

_Shape_ -- shape used to relate surface area to depth; choices are **CYLINDRICAL**, **CONICAL**, **PARABOLOID**, or **PYRAMIDAL**.

_Ysur_ -- maximum additional pressure head above full depth that a closed storage unit can sustain under surcharge conditions (ft or m) (default is 0).

_L, W, Z_ -- dimensions of the storage unit's shape (see table below).

_Fevap_ -- fraction of potential evaporation from the storage unit’s water surface realized (default is 0).

Optional seepage parameters for soil surrounding the storage unit:

_Psi_ -- suction head (inches or mm).

_Ksat_ -- saturated hydraulic conductivity  (in/hr or mm/hr).

_IMD_ -- initial moisture deficit (porosity minus moisture content) (fraction).

**Remarks:**

_A1_, _A2_, and _A0_ are used in the following expression that relates surface area (ft2 or m2) to water depth (ft or m) for a storage unit with
**FUNCTIONAL** geometry:

\f[ Area = A0 + A1 Depth^{A2} \f]

For **TABULAR** geometry, the surface area curve will be extrapolated outwards to meet the unit's maximum depth if need be.

The dimensions of storage units with other shapes are defined as follows:

|                Shape                 |                L                 |                W                |           Z           |
| :----------------------------------: | :------------------------------: | :-----------------------------: | :-------------------: |
| **CYLINDRICAL** <br> ![](embim3.jpg) |        major axis length         |        minor axis width         |       not used        |
|   **CONICAL** <br> ![](embim4.jpg)   |    major axis length of base     |    minor axis width of base     | side slope (run/rise) |
| **PARABOLOID** <br> ![](embim5.jpg)  | major axis length at full height | minor axis width at full height |      full height      |
|  **PYRAMIDAL** <br> ![](embim6.jpg)  |           base length            |           base width            | side slope (run/rise) |

The parameters _Psi_, _Ksat_, and _IMD_ need only be supplied if seepage loss through the soil at the bottom and sloped sides of the storage unit should be considered. They are the same Green-Ampt infiltration parameters described in the [INFILTRATION] section. If _Ksat_ is zero then no seepage occurs while if _IMD_ is zero then seepage occurs at a constant rate equal to _Ksat_. Otherwise seepage rate will vary with storage depth.

---

<!---
  conduits
-->

### [CONDUITS] {#conduits}

**Purpose:**

Identifies each conduit link of the drainage system. Conduits are pipes or channels that convey water from one node to another.

**Format:**

_Name  Node1  Node2  Length  N  Z1  Z2_  (_Q0  Qmax_)

**Parameters:**

_Name_ -- name assigned to conduit link.

_Node1_ -- name of the conduit’s upstream node.

_Node2_ -- name of the conduit’s downstream node.

_Length_ -- conduit length (ft or m).

_N_ -- Manning’s roughness coefficient (n).

_Z1_ -- offset of the conduit’s upstream end above the invert of its upstream node (ft or m).

_Z2_ -- offset of the conduit’s downstream end above the invert of its downstream node (ft or m).

_Q0_ -- flow in the conduit at the start of the simulation (flow units) (default is 0).

_Qmax_ -- maximum flow allowed in the conduit (flow units) (default is no limit).

**Remarks:**

The figure below illustrates the meaning of the _Z1_ and _Z2_ parameters.

![Link_offset](link_offset.png)

These offsets are expressed as a relative distance above the node invert if the **LINK_OFFSETS** option is set to **DEPTH** (the default) or as an absolute elevation if it is set to **ELEVATION**.

---

<!---
  pumps
-->

### [PUMPS] {#pumps}

**Purpose:**

Identifies each pump link of the drainage system.

**Format:**

_Name  Node1  Node2  Pcurve_  (_Status  Startup  Shutoff_)

**Parameters:**

_Name_ -- name assigned to pump link.

_Node1_ -- name of the pump’s inlet node.

_Node2_ -- name of the pump’s outlet node.

_Pcurve_ -- name of a pump curve listed in the [CURVES] section of the input.

_Status_ -- pump’s status at the start of the simulation (either **ON** or **OFF**; default is **ON**).

_Startup_ -- depth at the inlet node when the pump turns on (ft or m) (default is 0).

_Shutoff_ -- depth at inlet node when the pump shuts off (ft or m) (default is 0).

**Remarks:**

A pump curve describes the relation between a pump's flow rate and conditions at its inlet and outlet nodes. The following types of pump curves are supported:

_Type1_ -- An off-line pump with a wet well where flow increases incrementally with available wet well volume.

_Type2_ -- An in-line pump where flow increases incrementally with inlet node depth.

_Type3_ -- An in-line pump where flow varies continuously with head difference between the inlet and outlet nodes.

_Type4_ -- A variable speed in-line pump where flow varies continuously with inlet node depth.

_Type5_ -- A variable speed version of the Type3 pump where the head v. flow curve shifts position as the speed setting changes.

_Ideal_ -- A transfer pump that does not require a pump curve and is used mainly for preliminary design. Its flow rate equals the inflow rate to its inlet node no matter what the head difference is between its inlet and outlet nodes. Use an asterisk (\*) as the value for Pcurve.

---

<!---
  orifices
-->

### [ORIFICES] {#orifices}

**Purpose:**

Identifies each orifice link of the drainage system. An orifice link serves to limit the flow exiting a node and is often used to model flow diversions and storage node outlets.

**Format:**

_Name  Node1  Node2  Type  Offset  Cd_  (_Gated  Orate_)

**Parameters:**

_Name_ -- name assigned to orifice link.

_Node1_ -- name of the orifice’s inlet node.

_Node2_ -- name of the orifice’s outlet node.

_Type_ -- the type of orifice - either **SIDE** if oriented in a vertical plane or **BOTTOM** if oriented in a horizontal plane.

_Offset_ -- amount that a Side Orifice’s bottom or the position of a Bottom Orifice is offset above the invert of inlet node (ft or m, expressed as either a depth or as an elevation, depending on the **LINK_OFFSETS** option setting).

_Cd_ -- discharge coefficient (unitless).

_Flap_ -- **YES** if a flap gate prevents reverse flow, **NO** if not (default is **NO**).

_Orate_ -- time in decimal hours to open a fully closed orifice (or close a fully open one). Use 0 if the orifice can open/close instantaneously.

**Remarks:**

The geometry of an orifice’s opening must be described in the [XSECTIONS] section. The only allowable shapes are **CIRCULAR** and **RECT_CLOSED** (closed rectangular).

![](embim7.jpg)

---

<!---
  weirs
-->

### [WEIRS] {#weirs}

**Purpose:**

Identifies each weir link of the drainage system. Weirs are used to model flow diversions and storage node outlets.

**Format:**

_Name Node1 Node2 Type CrstHt Cd_ (_Gated EC Cd2 Sur_ (_Width Surf_))

**Parameters:**

_Name_ -- name assigned to weir link.

_Node1_ -- name of the weir’s inlet node.

_Node2_ -- name of the weir’s outlet node.

_Type_ -- **TRANSVERSE**, **SIDEFLOW**, **V-NOTCH**, **TRAPEZOIDAL** or **ROADWAY**.

_CrstHt_ -- amount that the weir’s opening is offset above the invert of inlet node (ft or m, expressed as either a depth or as an elevation, depending on the **LINK_OFFSETS** option setting).

_Cd_ -- weir discharge coefficient (for CFS if using US flow units or CMS if using metric flow units).

_Gated_ -- **YES** if a flap gate prevents reverse flow, **NO** if not (default is **NO**).

_EC_ -- number of end contractions for a **TRANSVERSE** or **TRAPEZOIDAL** weir (default is 0).

_Cd2_ -- discharge coefficient for the triangular ends of a **TRAPEZOIDAL** weir (for CFS if using US flow units or CMS if using metric flow units) (default is the value of _Cd_).

_Sur_ -- **YES** if the weir can surcharge (have an upstream water level higher than the height of the weir’s opening); **NO** if it cannot (default is **YES**).

The following parameters apply only to **ROADWAY** weirs:

_Width_ -- width of road lanes and shoulders for a **ROADWAY** weir (ft or m).

_Surf_ -- type of road surface for a **ROADWAY** weir: **PAVED** or **GRAVEL**.

**Remarks:**

The geometry of a weir’s opening is described in the [XSECTIONS] section. The following shapes must be used with each type of weir:

| Weir Type   | Cross-Section Shape |
| :---------- | :-----------------: |
| Transverse  |      RECT_OPEN      |
| Sideflow    |      RECT_OPEN      |
| V-Notch     |     TRIANGULAR      |
| Trapezoidal |     TRAPEZOIDAL     |
| Roadway     |      RECT_OPEN      |

The **ROADWAY** weir is a broad crested rectangular weir used model roadway crossings usually in conjunction with culvert-type conduits. It uses the FHWA HDS-5 method to determine a discharge coefficient as a function of flow depth and roadway width and surface. If no roadway data are provided then the weir behaves as a **TRANSVERSE** weir with Cd as its discharge coefficient. Note that if roadway data are provided, then values for the other optional weir parameters (**NO** for _Gated_, **0** for _EC_, **0** for _Cd2_, and **NO** for _Sur_) must be entered even though they do not apply to **ROADWAY** weirs.

---

<!---
outlets
-->

### [OUTLETS] {#outlets}

**Purpose:**

Identifies each outlet flow control device of the drainage system. These are devices used to model outflows from storage units or flow diversions that have a user-defined relation between flow rate and water depth.

**Formats:**

|        |         |         |          |                      |          |           |
| :----- | :------ | :------ | :------- | :------------------- | -------- | --------- |
| _Name_ | _Node1_ | _Node2_ | _Offset_ | **TABULAR/DEPTH**    | _Qcurve_ | (_Gated_) |
| _Name_ | _Node1_ | _Node2_ | _Offset_ | **TABULAR/HEAD**     | _Qcurve_ | (_Gated_) |
| _Name_ | _Node1_ | _Node2_ | _Offset_ | **FUNCTIONAL/DEPTH** | _C1 C2_  | (_Gated_) |
| _Name_ | _Node1_ | _Node2_ | _Offset_ | **FUNCTIONAL/HEAD**  | _C1 C2_  | (_Gated_) |

**Parameters:**

_Name_ -- name assigned to outlet link.

_Node1_ -- name of the outlet’s inlet node.

_Node2_ -- name of the outlet’s outlet node.

_Offset_ -- amount that the outlet is offset above the invert of its inlet node (ft or m, expressed as either a depth or as an elevation, depending on the **LINK_OFFSETS** option setting).

_Qcurve_ -- name of the rating curve listed in the [CURVES] section that describes outflow rate (flow units) as a function of:

- water depth above the offset elevation at the inlet node (ft or m) for a **TABULAR/DEPTH** outlet

- head difference (ft or m) between the inlet and outflow nodes for a **TABULAR/HEAD** outlet.

_C1, C2_ -- coefficient and exponent, respectively, of a power function that relates outflow to:

- water depth (ft or m) above the offset elevation at the inlet node for a **FUNCTIONAL/DEPTH** outlet

- head difference  (ft or m) between the inlet and outflow nodes for a **FUNCTIONAL/HEAD** outlet.

_Gated_ -- **YES** if a flap gate prevents reverse flow, **NO** if not (default is **NO**).

---

<!---
  xsections
-->

### [XSECTIONS] {#xsections}

**Purpose:**

Provides cross-section geometric data for conduit and regulator links of the drainage system.

**Formats:**

|        |               |                                               |
| :----- | :------------ | :-------------------------------------------- |
| _Link_ | _Shape_       | _Geom1 Geom2 Geom3 Geom4_ (_Barrels Culvert_) |
| _Link_ | **IRREGULAR** | _Tsect_                                       |
| _Link_ | **STREET**    | _Street_                                      |

**Parameters:**

_Link_ -- name of a conduit, orifice, or weir.

_Shape_ -- a cross-section shape (see Tables D-1 below for available shapes).

_Geom1_ -- full height of the cross-section (ft or m).

_Geom2-4_ -- auxiliary parameters (width, side slopes, etc.) as listed in Table D-1.

_Barrels_ -- number of barrels (i.e., number of parallel pipes of equal size, slope, and roughness) associated with a conduit (default is 1).

_Culvert_ -- code number from Table A.10 for the conduit’s inlet geometry if it is a culvert subject to possible inlet flow control (leave blank otherwise).

_Curve_ -- name of a Shape Curve in the [CURVES] section that defines how cross-section width varies with depth.

_Tsect_ -- name of an entry in the [TRANSECTS] section that describes the cross-section geometry of an irregular channel.

_Street_ -- name of an entry in the [STREETS] section that describes the cross-section geometry of a street.

**Remarks:**

The standard conduit shapes and their geometric parameters are listed in the following table:

| Shape               | Geom1           | Geom2          | Geom3           | Geom4           |
| :------------------ | :-------------- | :------------- | :-------------- | :-------------- |
| CIRCULAR            | Diameter        |                |                 |                 |
| FORCE_MAIN          | Diameter        | Roughness[^1]  |                 |                 |
| FILLED_CIRCULAR[^2] | Diameter        | Sediment Depth |                 |                 |
| RECT_CLOSED         | Full Height     | Top Width      |                 |                 |
| RECT_OPEN           | Full Height     | Top Width      |                 |                 |
| TRAPEZOIDAL         | Full Height     | Base Width     | Left  Slope[^3] | Right Slope[^3] |
| TRIANGULAR          | Full Height     | Top Width      |                 |                 |
| HORIZ_ELLIPSE       | Full Height     | Max. Width     | Size Code[^4]   |                 |
| VERT_ELLIPSE        | Full Height     | Max. Width     | Size Code[^4]   |                 |
| ARCH                | Full Height     | Max. Width     | Size Code[^5]   |                 |
| PARABOLIC           | Full Height     | Top Width      |                 |                 |
| POWER               | Full Height     | Top Width      | Exponent        |                 |
| RECT_TRIANGULAR     | Full Height     | Top Width      | Triangle Height |                 |
| RECT_ROUND          | Full Height     | Top Width      | Bottom Radius   |                 |
| MODBASKETHANDLE     | Full Height     | Base Width     | Top Radius[^6]  |                 |
| EGG                 | Full Height     |                |                 |                 |
| HORSESHOE           | Full Height     |                |                 |                 |
| GOTHIC              | Full Height     |                |                 |                 |
| CATENARY            | Full Height     |                |                 |                 |
| SEMIELLIPTICAL      | Full Height     |                |                 |                 |
| BASKETHANDLE        | Full Height     |                |                 |                 |
| SEMICIRCULAR        | Full Height     |                |                 |                 |
| CUSTOM              | Full Height     | Shape Curve    |                 |                 |

[^1]: C-factors are used when **H-W** is the **FORCE_MAIN_EQUATION** choice in the [OPTIONS] section while roughness heights (in inches or mm) are used for **D-W**.
[^2]: A circular conduit partially filled with sediment to a specified depth.
[^3]: Slopes are horizontal run / vertical rise.
[^4]: Size code of a standard shaped elliptical pipe. Leave blank (or 0) if the pipe has a custom dimensions.
[^5]: Size code of a standard arch pipe. Leave blank (or 0) if the pipe has custom dimensions).
[^6]: Set to zero to use a standard modified baskethandle shape whose top radius is half the base width.

Size codes for standard elliptical and arch pipes and for culverts can be found in the SWMM Users Manual.

The **CUSTOM** shape is a closed conduit whose width versus height is described by a user-supplied Shape Curve.

An **IRREGULAR** cross-section is used to model an open channel whose geometry is described by a Transect object.

A **STREET** cross-section is used to model street conduits and inlet flow capture (see the [INLETS] and [INLETS_USAGE] sections).

The _Culvert_ code number is used only for closed conduits acting as culverts that should be analyzed for inlet control conditions using the FHWA HDS-5 methodology.

---

<!---
  transects
-->

### [TRANSECTS] {#transects}

**Purpose:**

Describes the cross-section geometry of natural channels or conduits with irregular shapes following the HEC-2 data format.

**Formats:**

|        |          |                                                     |
| :----- | :------- | :-------------------------------------------------- |
| **NC** | _Nleft_  | _Nright Nchanl_                                     |
| **X1** | _Name_   | _Nsta_ _Xleft Xright 0 0 0 Lfactor Wfactor Eoffset_ |
| **GR** | _Elev_   | _Station_ ... _Elev  Station_                       |

**Parameters:**

_Nleft_ -- Manning’s roughness coefficient (n) of right overbank portion of channel (use 0 if no change from previous **NC** line).

_Nright_ -- Manning’s roughness coefficient (n) of right overbank portion of channel (use 0 if no change from previous **NC** line.

_Nchanl_ -- Manning’s roughness coefficient (n) of main channel portion of channel (use 0 if no change from previous **NC** line.

_Name_ -- name assigned to the transect.

_Nsta_ -- number of stations across the cross-section’s width at which elevation data is supplied.

_Xleft_ -- station position which ends the left overbank portion of the channel (ft or m).

_Xright_ -- station position which begins the right overbank portion of the channel (ft or m).

_Lfactor_ -- meander modifier that represents the ratio of the length of a meandering main channel to the length of the overbank area that surrounds it (use 0 if not applicable).

_Wfactor_ -- factor by which distances between stations should be multiplied to increase (or decrease) the width of the channel (enter 0 if not applicable).

_Eoffset_ -- amount to be added (or subtracted) from the elevation of each station (ft or m).

_Elev_ -- elevation of the channel bottom at a cross-section station relative to some fixed reference (ft or m).

_Station_ -- distance of a cross-section station from some fixed reference (ft or m).

**Remarks:**

Transect geometry is described as shown below, assuming that one is looking in a downstream direction:

![](embim8.gif)

The first line in this section must always be a **NC** line. After that, the **NC** line is only needed when a transect has different Manning’s n values than the previous one.

The Manning’s n values on the **NC** line will supersede any roughness value entered for the conduit which uses the irregular cross-section.

There should be one **X1** line for each transect. Any number of **GR** lines may follow, and each **GR** line can have any number of Elevation-Station data pairs. (In HEC-2 the **GR** line is limited to 5 stations.)

The station that defines the left overbank boundary on the **X1** line must correspond to one of the station entries on the **GR** lines that follow. The same holds true for the right overbank boundary. If there is no match, a warning will be issued and the program will assume that no overbank area exists.

The meander modifier is applied to all conduits that use this particular transect for their cross section. It assumes that the length supplied for these conduits is that of the longer main channel. SWMM will use the shorter overbank length in its calculations while increasing the main channel roughness to account for its longer length.

---

<!---
  streets
-->

### [STREETS] {#streets}

**Purpose:**

Describes the cross-section geometry of conduits that represent streets.

**Format:**

_Name Tcrown Hcurb Sx nRoad_ (_a W_)(_Sides Tback Sback nBack_)

**Parameters:**

_Name_ -- name assigned to the street cross-section

_Tcrown_ -- distance from street’s curb to its crown (ft or m)

_Hcurb_ -- curb height (ft or m)

_Sx_ -- street cross slope (%)

_nRoad_ -- Manning’s roughness coefficient (n) of the road surface

_a_ -- gutter depression height (in or mm) (default = 0)

_W_ -- depressed gutter width (ft or m) (default = 0)

_Sides_ -- 1 for single sided street or 2 for two-sided street (default = 2)

_Tback_ -- street backing width (ft or m) (default = 0)

_Sback_ -- street backing slope (%) (default = 0)

_nBack_ -- street backing Manning’s roughness coefficient (n) (default = 0)

**Remarks:**

![](embim9.gif)

If the street has no depressed gutter (a = 0) then the gutter width entry is ignored. If the street has no backing then the three backing parameters can be omitted.

---

<!---
  inlets
-->

### [INLETS] {#inlets}

**Purpose:**

Defines inlet structure designs used to capture street and channel flow that are sent to below ground sewers.

**Format:**

|        |                      |                                       |
| :----- | :------------------- | :------------------------------------ |
| _Name_ | **GRATE/DROP_GRATE** | _Length Width Type_ (_Aopen Vsplash_) |
| _Name_ | **CURB/DROP_CURB**   | _Length Height_ (_Throat_)            |
| _Name_ | **SLOTTED**          | _Length Width_                        |
| _Name_ | **CUSTOM**           | _Dcurve/Rcurve_                       |

**Parameters:**

_Name_ -- name assigned to the inlet structure.

_Length_ -- length of the inlet parallel to the street curb (ft or m).

_Width_ -- width of a **GRATE** or **SLOTTED** inlet (ft or m).

_Height_ -- height of a **CURB** opening inlet (ft or m).

_Type_ -- type of **GRATE** used (see below).

_Aopen_ -- fraction of a **GENERIC** grate’s area that is open.

_Vsplash_ -- splash over velocity for a **GENERIC** grate (ft/s or m/s).

_Throat_ -- the throat angle of a **CURB** opening inlet (**HORIZONTAL, INCLINED or VERTICAL**).

_Dcurve_ -- name of a Diversion-type curve (captured flow v. approach flow) for a **CUSTOM** inlet.

_Rcurve_ -- name of a Rating-type curve (captured flow v. water depth) for a **CUSTOM** inlet.

**Remarks:**

These are the different types of standard inlets that SWMM can model:

![](embim10.jpg)

Use one line for each inlet design except for a combination inlet where one **GRATE** line describes its grated inlet and a second **CURB** line (with the same inlet name) describes its curb opening inlet.

**GRATE**, **CURB**, and **SLOTTED** inlets are used with **STREET** conduits, **DROP_GRATE** and **DROP_CURB** inlets with open channels, and a **CUSTOM** inlet with any conduit.

**GRATE** and **DROP_GRATE** types can be any of the following:

| Grate Type       | Sketch           | Description                                                                                                      |
| :--------------- | :--------------- | :--------------------------------------------------------------------------------------------------------------- |
| **P_BAR-50**     | ![](embim11.gif) | Parallel bar grate with bar spacing 1-7/8-in on center                                                           |
| **P_BAR-50X100** | ![](embim12.gif) | Parallel bar grate with bar spacing 1-7/8-in on center and 3/8-in diameter lateral rods spaced at 4-in on center |
| **P_BAR-30**     | ![](embim13.gif) | Parallel bar grate with 1-1/8-in on center bar spacing                                                           |
| **CURVED_VANE**  | ![](embim14.jpg) | Curved vane grate with 3-1/4-in longitudinal bar and 4-1/4-in transverse bar spacing on center                   |
| **TILT_BAR-45**  | ![](embim15.jpg) | 45 degree tilt bar grate with 2-1/4-in longitudinal bar and4-in transverse bar spacing on center                 |
| **TILT_BAR-30**  | ![](embim16.jpg) | 30 degree tilt bar grate with 3-1/4-in and 4-in on center longitudinal and lateral bar spacing respectively      |
| **RETICULINE**   | ![](embim17.gif) | "Honeycomb" pattern of lateral bars and longitudinal bearing bars                                                |
| **GENERIC**      |                  | A generic grate design.                                                                                          |

Only a **GENERIC** type grate requires that Aopen and Vsplash values be provided. The other standard grate types have predetermined values of these parameters. (Splash over velocity is the minimum velocity that will cause some water to shoot over the inlet thus reducing its capture efficiency).

A **CUSTOM** inlet takes the name of either a Diversion curve or a Rating curve as its only parameter (see the [CURVES] section).  Diversion curves are best suited for on-grade inlets and Rating curves for on-sag inlets.

**Examples:**

```
; A 2-ft x 2-ft parallel bar grate
InletType1  GRATE  2  2  P-BAR-30

; A combination inlet
InletType2  GRATE  2  2  CURVED_VANE
InletType2  CURB   4  0.5  HORIZONTAL

; A custom inlet using Curve1 as its capture curve
InletType3  CUSTOM  Curve1
```

---

<!---
  inlet_usage
-->

### [INLET_USAGE] {#inlet_usage}

**Purpose:**

Assigns inlet structures to specific street and open channel conduits.

**Format:**

_Conduit Inlet Node_ (_Number %Clogged Qmax aLocal wLocal Placement_)

**Parameters:**

_Conduit_ -- name of a street or open channel conduit containing the inlet.

_Inlet_ -- name of an inlet structure (from the [INLETS] section) to use.

_Node_ -- name of the sewer node receiving flow captured by the inlet.

_Number_ -- number of replicate inlets placed on each side of the street.

_%Clogged_ -- degree to which inlet capacity is reduced due to clogging (%).

_Qmax_ -- maximum flow that the inlet can capture (flow units).

_aLocal_ -- height of local gutter depression (in or mm).

_wLocal_ -- width of local gutter depression (ft or m).

_Placement_ -- **AUTOMATIC**, **ON_GRADE**, or **ON_SAG**.

**Remarks:**

Only conduits with a **STREET** cross section can be assigned a curb and gutter inlet while drop inlets can only be assigned to conduits with a **RECT_OPEN** or **TRAPEZOIDAL** cross section.

Only the first three parameters are required. The default number of inlets is 1 (for each side of a two-sided street) while the remaining parameters have default values of 0.

A Qmax value of 0 indicates that the inlet has no flow restriction.

The local gutter depression applies only over the length of the inlet unlike the continuous depression for a **STREET** cross section which exists over the full curb length.

The default inlet placement is **AUTOMATIC**, meaning that the program uses the network topography to determine whether an inlet operates on-grade or on-sag. On-grade means the inlet is located on a continuous grade. On-sag means the inlet is located at a sag or sump point where all adjacent conduits slope towards the inlet leaving no place for water to flow except into the inlet.

---

<!---
  losses
-->

### [LOSSES] {#inlet_usage}

**Purpose:**

Specifies minor head loss coefficients, flap gates, and seepage rates for conduits.

**Format:**

_Conduit  Kentry  Kexit  Kavg_  (_Flap  Seepage_)

**Parameters:**

_Conduit_ -- name of a conduit.

_Kentry_ -- minor head loss coefficient at the conduit’s entrance.

_Kexit_ -- minor head loss coefficient at the conduit’s exit.

_Kavg_ -- average minor head loss coefficient across the length of the conduit.

_Flap_ -- **YES** if the conduit has a flap valve that prevents back flow, **NO** otherwise. (Default is **NO**).

_Seepage_ -- Rate of seepage loss into the surrounding soil (in/hr or mm/hr). (Default is 0.)

**Remarks:**

Minor losses are only computed for the Dynamic Wave flow routing option (see the [OPTIONS] section). They are computed as \f$ Kv^{2}/2g \f$ where K = minor loss coefficient, v = velocity, and g = acceleration of gravity. Entrance losses are based on the velocity at the entrance of the conduit, exit losses on the exit velocity, and average losses on the average velocity.

Only enter data for conduits that actually have minor losses, flap valves, or seepage losses.

---

<!---
  controls
-->

### [CONTROLS] {#controls}

**Purpose:**

Determines how pumps and regulators will be adjusted based on simulation time or conditions at specific nodes and links.

**Formats:**

Each control rule is a series of statements of the form:

**RULE** _ruleID_

**IF** _condition_1_

**AND** _condition_2_

**OR** _condition_3_

**AND** _condition_4_

Etc.

**THEN** _action_1_

**AND** _action_2_

Etc.

**ELSE** _action_3_

**AND** _action_4_

Etc.

**PRIORITY** _value_

**Parameters:**

_ruleID_ -- an ID label assigned to the rule.

_condition_n_ -- a condition clause.

_action_n_ -- an action clause.

_value_ -- a priority value (e.g., a number from 1 to 5).

**Remarks:**

Keywords are shown in boldface and _ruleID_ is an ID label assigned to the rule, _condition_n_ is a Condition Clause, _action_n_ is an Action Clause, and value is a priority value (e.g., a number from 1 to 5). The formats used for Condition and Action clauses are discussed below.

Only the RULE, IF and THEN portions of a rule are required; the ELSE and PRIORITY portions are optional.

Blank lines between clauses are permitted and any text to the right of a semicolon is considered a comment.

When mixing AND and OR clauses, the OR operator has higher precedence than AND, i.e.,

    IF A or B and C

is equivalent to

    IF (A or B) and C.

If the interpretation was meant to be

    IF A or (B and C)

then this can be expressed using two rules as in

    IF A THEN ...
    IF B and C THEN ...

The PRIORITY value is used to determine which rule applies when two or more rules require that conflicting actions be taken on a link. A conflicting rule with a higher priority value has precedence over one with a lower value (e.g., PRIORITY 5 outranks PRIORITY 1). A rule without a priority value always has a lower priority than one with a value. For two rules with the same priority value, the rule that appears first is given the higher priority.

<!---
  controls_subpages
-->

@subpage conditional_clauses
@subpage action_clauses
@subpage modulated_controls
@subpage pid_controllers
@subpage named_variables
@subpage arithmetic_expressions

<!---
  conditional_clauses
-->

#### Condition Clauses {#conditional_clauses}

A Condition Clause of a control rule has the following formats:

    object id attribute relation value
    object id attribute relation object id attribute

where:

`object` -- is a category of object

`id` -- is the object's ID name

`attribute` -- is an attribute or property of the object

`relation` -- is a relational operator (=, <>, <, <=, >, >=)

`value` -- is an attribute value

Some examples of condition clauses are:

    GAGE  G1   6-HR_DEPTH > 0.5
    NODE  N23  DEPTH  >  10
    NODE  N23  DEPTH  >  NODE N25 DEPTH
    PUMP  P45  STATUS =  OFF
    SIMULATION CLOCKTIME = 22:45:00

The objects and attributes that can appear in a condition clause are as
follows:

| Object                   | Attributes                                                                                     | Value                                       |
| :----------------------- | :--------------------------------------------------------------------------------------------- | :------------------------------------------ |
| `GAGE`                   | `INTENSITY` <br> `n-HR_DEPTH`                                                                  | numerical value                             |
| `NODE`                   | `DEPTH` <br> `MAXDEPTH` <br> `HEAD` <br> `VOLUME` <br> `INFLOW`                                | numerical value                             |
| `LINK` or <br> `CONDUIT` | `FLOW` <br> `FULLFLOW` <br> `DEPTH` <br> `MAXDEPTH` <br> `VELOCITY` <br> `LENGTH` <br> `SLOPE` | numerical value                             |
| ^                        | `STATUS`                                                                                       | `OPEN` or `CLOSED`                          |
| ^                        | `TIMEOPEN` <br> `TIMECLOSED`                                                                   | decimal hours or hr:min                     |
| `PUMP`                   | `STATUS`                                                                                       | `ON` or `OFF`                               |
| ^                        | `SETTING`                                                                                      | pump curve multiplier                       |
| ^                        | `FLOW`                                                                                         | numerical value                             |
| `ORIFICE`                | `SETTING`                                                                                      | fraction open                               |
| `WEIR`                   | `SETTING`                                                                                      | fraction open                               |
| `OUTLET`                 | `SETTING`                                                                                      | rating curve multiplier                     |
| `SIMULATION`             | `TIME`                                                                                         | elapsed time in decimal hours or hr:min:sec |
| ^                        | `DATE`                                                                                         | month/day/year                              |
| ^                        | `MONTH`                                                                                        | month of year (January = 1)                 |
| ^                        | `DAY`                                                                                          | day of week (Sunday = 1)                    |
| ^                        | `CLOCKTIME`                                                                                    | time of day in hr:min:sec                   |

Gage INTENSITY is the rainfall intensity for a specific rain gage in the current simulation time period. Gage n-HR_DEPTH is a gage's total rainfall depth over the past n hours where n is a number between 1 and 48.

TIMEOPEN is the duration a link has been in an OPEN or ON state or have its SETTING be greater than zero; TIMECLOSED is the duration it has remained in a CLOSED or OFF state or have its SETTING be zero.

<!---
  action_clauses
-->

#### Action Clauses {#action_clauses}

An Action Clause of a control rule can have one of the following formats:

    CONDUIT id STATUS = OPEN/CLOSED
    PUMP id STATUS = ON/OFF
    PUMP/ORIFICE/WEIR/OUTLET id SETTING = value

where the meaning of SETTING depends on the object being controlled:

- for Pumps it is a multiplier applied to the flow computed from the pump curve (for a Type5 pump curve it is a relative speed setting that shifts the curve up or down),

- for Orifices it is the fractional amount that the orifice is fully open,

- for Weirs it is the fractional amount of the original freeboard that exists (i.e., weir control is accomplished by moving the crest height up or down),

- for Outlets it is a multiplier applied to the flow computed from the outlet's rating curve.

Some examples of action clauses are:

    PUMP P67 STATUS = OFF
    ORIFICE O212 SETTING = 0.5

<!---
  modulated_controls
-->

#### Modulated Controls {#modulated_controls}

Modulated controls are control rules that provide for a continuous degree of control applied to a pump or flow regulator as determined by the value of some controller variable, such as water depth at a node, or by time. The functional relation between the control setting and the controller variable can be specified by using a Control Curve, a Time Series, or a PID Controller. Some examples of modulated control rules are:

    RULE MC1
    IF NODE N2 DEPTH >= 0
    THEN WEIR W25 SETTING = CURVE C25

    RULE MC2
    IF SIMULATION TIME > 0
    THEN PUMP P12 SETTING = TIMESERIES TS101

    RULE MC3
    IF LINK L33 FLOW <> 1.6
    THEN ORIFICE O12 SETTING = PID 0.1 0.0 0.0

Note how a modified form of the action clause is used to specify the name of the control curve, time series or PID parameter set that defines the degree of control. A PID parameter set contains three values -- a proportional gain coefficient, an integral time (in minutes), and a derivative time (in minutes). Also, by convention the controller variable used in a Control Curve or PID Controller will always be the object and attribute named in the last condition clause of the rule. As an example, in rule MC1 above Curve C25 would define how the fractional setting at Weir W25 varied with the water depth at Node N2. In rule MC3, the PID controller adjusts the opening of Orifice O12 to maintain a flow of 1.6 in Link L33.

<!---
  pid_controllers
-->

#### PID Controllers {#pid_controllers}

A PID (Proportional-Integral-Derivative) Controller is a generic closed-loop control scheme that tries to maintain a desired set-point on some process variable by computing and applying a corrective action that adjusts the process accordingly. In the context of a hydraulic conveyance system a PID controller might be used to adjust the opening on a gated orifice to maintain a target flow rate in a specific conduit or to adjust a variable speed pump to maintain a desired depth in a storage unit. The classical PID controller has the form:

\f[ m(t) = K_{p} \[ e(t) + \frac{1}{T_{i}} \int e(\tau) d\tau + T_{d} \frac{de(t)}{dt} \] \f]

where m(t) = controller output, Kp = proportional coefficient (gain), Ti = integral time, Td = derivative time, e(t) = error (difference between setpoint and observed variable value), and t = time.

The controller output m(t) has the same meaning as a link setting used in a rule's Action Clause while dt is the current flow routing time step in minutes. Because link settings are relative values (with respect to either a pump's standard operating curve or to the full opening height of an orifice or weir) the error e(t) used by the controller is also a relative value. It is defined as the difference between the control variable setpoint x* and its value at time t, x(t), normalized to the setpoint value: \f$ e(t) = (x* - x(t)) / x\* \f$.

Note that for direct action control, where an increase in the link setting causes an increase in the controlled variable, the sign of Kp must be positive. For reverse action control, where the controlled variable decreases as the link setting increases, the sign of Kp must be negative. The user must recognize whether the control is direct or reverse action and use the proper sign on Kp accordingly. For example, adjusting an orifice opening to maintain a desired downstream flow is direct action. Adjusting it to maintain an upstream water level is reverse action. Controlling a pump to maintain a fixed wet well water level would be reverse action while using it to maintain a fixed downstream flow is direct action.

<!---
  named_variables
-->

#### Named Variables {#named_variables}

Named Variables are aliases used to represent the triplet of <object type | object id | object attribute> (or a doublet for Simulation times) that appear in the condition clauses of control rules. They allow condition clauses to be written as:

    variable relation value
    variable relation variable

where variable is defined on a separate line before its first use in a
rule using the format:

    VARIABLE  name = object id attribute

Here is an example of using this feature:

    VARIABLE  N123_Depth = NODE N123 DEPTH
    VARIABLE  N456_Depth = NODE N456 DEPTH
    VARIABLE  P45 = PUMP 45 STATUS

    RULE 1
    IF    N123_Depth > N456_Depth
    AND   P45 = OFF
    THEN  PUMP 45 STATUS = ON

    RULE 2
    IF   N123_Depth < 1
    THEN PUMP 45 STATUS = OFF

A variable is not allowed to have the same name as an object attribute.

Aside from saving some typing, named variables are required when using arithmetic expressions in rule condition clauses.

<!---
arithmetic_expressions
-->

#### Arithmetic Expressions {#arithmetic_expressions}

In addition to a simple condition placed on a single variable, a control condition clause can also contain an arithmetic expression formed from several variables whose value is compared against. Thus the format of a condition clause can be extended as follows:

    expression  relation  value
    expression  relation  variable

where expression is defined on a separate line before its first use in a rule using the format:

    EXPRESSION  name = f(variable1, variable2, ...)

The function f(...) can be any well-formed mathematical expression containing one or more named variables as well as any of the following math functions (which are case insensitive) and operators:

- abs(x) for absolute value of x

- sgn(x) which is +1 for x >= 0 or -1 otherwise

- step(x) which is 0 for x <= 0 and 1 otherwise

- sqrt(x) for the square root of x

- log(x) for logarithm base e of x

- log10(x) for logarithm base 10 of x

- exp(x) for e raised to the x power

- the standard trig functions (sin, cos, tan, and cot)

- the inverse trig functions (asin, acos, atan, and acot)

- the hyperbolic trig functions (sinh, cosh, tanh, and coth)

- the standard operators  +, -, \*, /, ^ (for exponentiation ) and any level of nested parentheses.

Here is an example of using this feature:

    VARIABLE  P1_flow = LINK 1 FLOW
    VARIABLE  P2_flow = LINK 2 FLOW
    VARIABLE  O3_flow = Link 3 FLOW
    EXPRESSION Net_Inflow = (P1_flow + P2_flow)/2 - O3_flow

    RULE 1
    IF   Net_Inflow > 0.1
    THEN ORIFICE 3 SETTING = 1
    ELSE ORIFICE 3 SETTING = 0.5

---

<!---
  pollutants
-->

### [POLLUTANTS] {#pollutants}

**Purpose:**

Identifies the pollutants being analyzed.

**Format:**

_Name Units Crain Cgw Cii Kd_ (_Sflag CoPoll CoFract Cdwf Cinit_)

**Parameters:**

_Name_ -- name assigned to a pollutant.

_Units_ -- concentration units (**MG/L** for milligrams per liter, **UG/L** for micrograms per liter, or **#/L** for direct count per liter).

_Crain_ -- concentration of the pollutant in rainfall (concentration units).

_Cgw_ -- concentration of the pollutant in groundwater (concentration units).

_Cii_ -- concentration of the pollutant in inflow/infiltration (concentration units).

_Kdecay_ -- first-order decay coefficient (1/days).

_Sflag_ -- **YES** if pollutant buildup occurs only when there is snow cover, **NO** otherwise (default is **NO**).

_CoPoll_ -- name of a co-pollutant (default is no co-pollutant designated by a \*).

_CoFract_ -- fraction of the co-pollutant’s concentration (default is 0).

_Cdwf_ -- pollutant concentration in dry weather flow (default is 0).

_Cinit_ -- pollutant concentration throughout the conveyance system at the start of the simulation (default is 0).

**Remarks:**

**FLOW** is a reserved word and cannot be used to name a pollutant.

Parameters _Sflag_ through _Cinit_ can be omitted if they assume their default values. If there is no co-pollutant but non-default values for _Cdwf_ or _Cinit_, then enter an asterisk (\*) for the co-pollutant name.

When pollutant X has a co-pollutant Y, it means that fraction _CoFract_ of pollutant Y’s runoff concentration is added to pollutant X’s runoff concentration when wash off from a subcatchment is computed.

The dry weather flow concentration can be overridden for any specific node of the conveyance system by editing the node’s Inflows property (see the [INFLOWS] section).

---

<!---
  landuses
-->

### [LANDUSES] {#landuses}

**Purpose:**

Identifies the various categories of land uses within the drainage area. Each subcatchment area can be assigned a different mix of land uses. Each land use can be subjected to a different street sweeping schedule. Land uses are only used in conjunction with pollutant buildup and wash off.

**Format:**

_Name_  (_SweepInterval  Availability  LastSweep_)

**Parameters:**

_Name_ -- land use name.

_SweepInterval_ -- days between street sweeping.

_Availability_ -- fraction of pollutant buildup available for removal by street sweeping.

_LastSweep_ -- days since last sweeping at the start of the simulation.

---

<!---
  coverages
-->

### [COVERAGES] {#coverages}

**Purpose:**

Specifies the percentage of a subcatchment’s area that is covered by each category of land use.

**Format:**

_Subcat  Landuse  Percent  Landuse  Percent_  ...

**Parameters:**

_Subcat_ -- subcatchment name.

_Landuse_ -- land use name.

_Percent_ -- percent of the subcatchment’s area covered by the land use.

**Remarks:**

More than one pair of land use - percentage values can be entered per line. If more than one line is needed, then the subcatchment name must still be entered first on the succeeding lines.

If a land use does not pertain to a subcatchment, then it does not have to be entered.

If no land uses are associated with a subcatchment then no pollutants will appear in the runoff from the subcatchment.

---

<!---
  loadings
-->

### [LOADINGS] {#loadings}

**Purpose:**

Specifies the pollutant buildup that exists on each subcatchment at the start of a simulation.

**Format:**

_Subcat  Pollut  InitBuildup  Pollut  InitBuildup_ ...

**Parameters:**

_Subcat_ -- name of a subcatchment.

_Pollut_ -- name of a pollutant.

_InitBuildup_ -- initial buildup of the pollutant (lbs/acre or kg/hectare).

**Remarks:**

More than one pair of pollutant - buildup values can be entered per line. If more than one line is needed, then the subcatchment name must still be entered first on the succeeding lines.

If an initial buildup is not specified for a pollutant, then its initial buildup is computed by applying the DRY_DAYS option (specified in the [OPTIONS] section) to the pollutant’s buildup function for each land use in the subcatchment.

---

<!---
  buildup
-->

### [BUILDUP] {#buildup}

**Purpose:**

Specifies the rate at which pollutants build up over different land uses between rain events.

**Format:**

_Landuse  Pollutant  FuncType  C1  C2  C3  PerUnit_

**Parameters:**

_Landuse_ -- land use name.

_Pollutant_ -- pollutant name.

_FuncType_ -- buildup function type: ( **POW / EXP / SAT / EXT** ).

_C1, C2, C3_ -- buildup function parameters (see table below).

_PerUnit_ -- **AREA** if buildup is per unit area, **CURBLENGTH** if per length of curb.

**Remarks:**

Buildup is measured in pounds (kilograms) per unit of area (or curb length) for pollutants whose concentration units are either mg/L or ug/L. If the concentration units are counts/L, then buildup is expressed as counts per unit of area (or curb length).

The buildup functions associated with each function type are:

| Type    | Function    | Equation[^*]                |
| ------- | ----------- | --------------------------- |
| **POW** | Power       | Min \f$ (C1, C2 t^{C3}) \f$ |
| **EXP** | Exponential | \f$ C1 (1 – exp(-C2 t)) \f$ |
| **SAT** | Saturation  | \f$ C1 t / (C3 + t) \f$     |
| **EXT** | External    | See below                   |

[^*] t is antecedent dry days.

For the **EXT** buildup function, C1 is the maximum possible buildup (mass per area or curb length), C2 is a scaling factor, and C3 is the name of a Time Series that contains buildup rates (as mass per area or curb length per day) as a function of time.

---

<!---
  washoff
-->

### [WASHOFF] {#washoff}

**Purpose:**

Specifies the rate at which pollutants are washed off from different land uses during rain events.

**Format:**

_Landuse  Pollutant  FuncType  C1  C2  SweepRmvl BmpRmvl_

**Parameters:**

_Landuse_ -- land use name.

_Pollutant_ -- pollutant name.

_FuncType_ -- washoff function type: **EXP / RC / EMC**.

_C1, C2_ -- washoff function coefficients(see table below).

_SweepRmvl_ -- street sweeping removal efficiency (percent).

_BmpRmvl_ -- BMP removal efficiency (percent).

**Remarks:**

The equations used for each type of washoff function are as follows:

| Type | Function                 | Equation                                | Units      |
| :--- | :----------------------- | :-------------------------------------- | :--------- |
| EXP  | Exponential              | \f$ C1 (runoff)^{C2} \f$ <br> (buildup) | Mass/hour  |
| RC   | Rating Curve             | \f$ C1 (runoff)^{C2} \f$                | Mass/sec   |
| EMC  | Event Mean Concentration | \f$ C1 \f$                              | Mass/Liter |

Each washoff function expresses its results in different units.

For the **Exponential** function the **runoff** variable is expressed in catchment depth per unit of time (inches per hour or millimeters per hour), while for the **Rating Curve** function it is in whatever flow units were specified in the [OPTIONS] section of the input file (e.g., CFS, CMS, etc.).

The buildup parameter in the **Exponential** function is the current total buildup over the subcatchment’s land use area in mass units. The units of C1 in the Exponential function are (in/hr)<sup>-C2</sup> per hour (or (mm/hr)<sup>-C2</sup> per hour). For the **Rating Curve** function, the units of _C1_ depend on the flow units employed. For the **EMC** (event mean concentration) function, _C1_ is always in concentration units.

---

<!---
  treatment
-->

### [TREATMENT] {#washoff}

**Purpose:**

Specifies the degree of treatment received by pollutants at specific nodes of the drainage system.

**Format:**

_Node  Pollut  Result_ = _Func_

**Parameters:**

_Node_ -- Name of the node where treatment occurs.

_Pollut_ -- Name of pollutant receiving treatment.

_Result_ -- Result computed by treatment function. Choices are:

_C_ -- (function computes effluent concentration)

_R_ -- (function computes fractional removal).

_Func_ -- mathematical function expressing treatment result in terms of pollutant concentrations, pollutant removals, and other standard variables (see below).

**Remarks:**

Treatment functions can be any well-formed mathematical expression involving:

- inlet pollutant concentrations (use the pollutant name to represent a concentration)

- removal of other pollutants (use R\_ pre-pended to the pollutant name to represent removal)

- process variables which include:
  - **FLOW** for flow rate into node (user’s flow units)
  - **DEPTH** for water depth above node invert (ft or m)
  - **AREA** for node surface area (ft2 or m2)
  - **DT** for routing time step (seconds)
  - **HRT** for hydraulic residence time (hours)

Any of the following math functions can be used in a treatment function:

- abs(x) for absolute value of x
- sgn(x) which is +1 for x >= 0 or -1 otherwise
- step(x) which is 0 for x <= 0 and 1 otherwise
- sqrt(x) for the square root of x
- log(x) for logarithm base e of x
- log10(x) for logarithm base 10 of x
- exp(x) for e raised to the x power
- the standard trig functions (sin, cos, tan, and cot)
- the inverse trig functions (asin, acos, atan, and acot)
- the hyperbolic trig functions (sinh, cosh, tanh, and coth)

along with the standard operators +, -, \*, /, ^ (for exponentiation ) and any level of nested parentheses.

Examples:

    ; 1-st order decay of BOD
    Node23  BOD   C = BOD * exp(-0.05*HRT)

    ; lead removal is 20% of TSS removal
    Node23  Lead  R = 0.2 \* R_TSS

---

<!---
  inflows
-->

### [INFLOWS] {#inflows}

**Purpose:**

Specifies external hydrographs and pollutographs that enter the drainage system at specific nodes.

**Formats:**

|        |          |           |                                       |
| :----- | :------- | :-------- | :------------------------------------ |
| _Node_ | **FLOW** | _Tseries_ | (**FLOW** (_1.0 Sfactor Base Pat_))   |
| _Node_ | _Pollut_ | _Tseries_ | (_Type_ (_Mfactor Sfactor Base Pat_)) |

**Parameters:**

Node name of the node where external inflow enters.

Pollut name of a pollutant.

Tseries name of a time series in the [TIMESERIES] section describing how external flow or pollutant loading varies with time.

Type **CONCEN** if pollutant inflow is described as a concentration, **MASS** if it is described as a mass flow rate (default is **CONCEN**).

Mfactor the factor that converts the inflow’s mass flow rate units into the project’s mass units per second, where the project’s mass units are those specified for the pollutant in the [POLLUTANTS] section (default is 1.0 - see example below).

Sfactor a scaling factor that multiplies the recorded time series values (default is 1.0).

Base a constant baseline value added to the time series value (default is 0.0).

Pat name of an optional time pattern in the [PATTERNS] section used to adjust the baseline value on a periodic basis.

**Remarks:**

External inflows are represented by both a constant and time varying
component as follows:

_Inflow_ = (_Baseline value_)\*(_Pattern factor_) + (_Scaling factor_)\*(_Time series value_)

If an external inflow of a pollutant concentration is specified for a node, then there must also be an external inflow of **FLOW** provided for the same node, unless the node is an Outfall. In that case a pollutant can enter the system during periods when the outfall is submerged and reverse flow occurs. External pollutant mass inflows do not require a **FLOW** inflow.

**Examples:**

    ; NODE2 receives flow inflow from time series N2FLOW
    ; and TSS concentration from time series N2TSS
    NODE2   FLOW  N2FLOW
    NODE2   TSS   N33TSS  CONCEN

    ; NODE65 has a mass inflow of BOD from time series N65BOD\
    ; listed in lbs/hr (126 converts lbs/hr to mg/sec)
    NODE65  BOD  N65BOD  MASS  126

    ; Flow inflow to Node N176 consists of the flow time series
    ; FLOW_176 scaled at 0.5 plus a baseline flow of 12.7
    ; adjusted by pattern FlowPat
    N176  FLOW  FLOW_176  FLOW  1.0  0.5  12.7  FlowPat

---

<!---
  dwf
-->

### [DWF] {#dwf}

**Purpose:**

Specifies dry weather flow and its quality entering the drainage system at specific nodes.

**Format:**

_Node  Type  Base_  (_Pat1  Pat2  Pat3  Pat4_)

**Parameters:**

_Node_ -- name of a node where dry weather flow enters.

_Type_ -- keyword **FLOW** for flow or a pollutant name for a quality constituent.

_Base_ -- average baseline value for corresponding constituent  (flow or concentration units).

_Pat1_, _Pat2_, etc. -- names of up to four time patterns appearing in the [PATTERNS] section.

**Remarks:**

The actual dry weather input will equal the product of the baseline value and any adjustment factors supplied by the specified patterns. (If not supplied, an adjustment factor defaults to 1.0.)

The patterns can be any combination of monthly, daily, hourly and weekend hourly patterns, listed in any order. See the [PATTERNS] section for more details.

---

<!---
  rdii
-->

### [RDII] {#rdii}

**Purpose:**

Specifies the parameters that describe rainfall-dependent infiltration/inflow (RDII) entering the drainage system at specific nodes.

**Format:**

_Node  UHgroup  SewerArea_

**Parameters:**

_Node_ -- name of a node receiving RDII flow.

_UHgroup_ -- name of an RDII unit hydrograph group appearing in the [HYDROGRAPHS] section.

_SewerArea_ -- area of the sewershed that contributes RDII to the node (acres or hectares).

---

<!---
  hydrographs
-->

### [HYDROGRAPHS] {#hydrographs}

**Purpose:**

Specifies the shapes of the triangular unit hydrographs that determine the amount of rainfall-dependent infiltration/inflow (RDII) entering the\ drainage system.

**Format:**

_Name_ _Raingage_

_Name_  *Month* **SHORT/MEDIUM/LONG** _R  T  K_ (_Dmax Drec D0_)

**Remarks:**

_Name_ -- name assigned to a unit hydrograph group.

_Raingage_ -- name of the rain gage used by the unit hydrograph group.

_Month_ -- month of the year (e.g., **JAN**, **FEB**, etc. or **ALL** for all months).

_R_ -- response ratio for the unit hydrograph.

_T_ -- time to peak (hours) for the unit hydrograph.

_K_ -- recession limb ratio for the unit hydrograph.

_Dmax_ -- maximum initial abstraction depth available (in rain depth units).

_Drec_ -- initial abstraction recovery rate (in rain depth units per day)

_D0_ -- initial abstraction depth already filled at the start of the simulation (in rain depth units).

**Remarks:**

For each group of unit hydrographs, use one line to specify its rain gage followed by as many lines as are needed to define each unit hydrograph used by the group throughout the year. Three separate unit hydrographs, that represent the short-term, medium-term, and long-term RDII responses, can be defined for each month (or all months taken together). Months not listed are assumed to have no RDII.

The response ratio (R) is the fraction of a unit of rainfall depth that becomes RDII. The sum of the ratios for a set of three hydrographs does not have to equal 1.0.

The recession limb ratio (K) is the ratio of the duration of the hydrograph’s recession limb to the time to peak (T) making the hydrograph time base equal to \f$ T\*(1+K) \f$ hours. The area under each unit hydrograph is 1 inch (or mm).

The optional initial abstraction parameters determine how much rainfall is lost at the start of a storm to interception and depression storage. If not supplied then the default is no initial abstraction.

**Example:**

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

---

<!---
  curves
-->

### [CURVES] {#curves}

**Purpose:**

Describes a relationship between two variables in tabular format.

**Format:**

_Name_ _Type_

_Name_ _X-value_ _Y-value_  ...

**Parameters:**

|           |                                                                                                                                                                    |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| _Name_    | name assigned to the curve.                                                                                                                                        |
| _Type_    | the type of curve being defined: <br> **STORAGE / SHAPE / DIVERSION / TIDAL /** <br> **PUMP1 / PUMP2 / PUMP3 / PUMP4 / PUMP5 /** <br> **RATING / CONTROL / WEIR**. |
| _X-value_ | an X (independent variable) value.                                                                                                                                 |
| _Y-value_ | the Y (dependent variable) value corresponding to X.                                                                                                               |

**Remarks:**

Each curve should have its name and type on the first line with its data points entered on subsequent lines.

Multiple pairs of x-y values can appear on a line. If more than one line is needed, repeat the curve's name on subsequent lines.

X-values must be entered in increasing order.

Choices for curve type have the following meanings (flows are expressed in the user’s choice of flow units set in the [OPTIONS] section):

- **STORAGE** surface area in ft<sup>2</sup> (m<sup>2</sup>) v. depth in ft (m) for a storage unit node
- **SHAPE** width v. depth for a custom closed cross-section, both normalized with respect to full depth
- **DIVERSION** diverted outflow v. total inflow for a flow divider node or a Custom inlet
- **TIDAL** water surface elevation in ft (m) v. hour of the day for an outfall node
- **PUMP1** pump outflow v. increment of inlet node volume in ft<sup>3</sup> (m<sup>3</sup>)
- **PUMP2** pump outflow v. increment of inlet node depth in ft (m)
- **PUMP3** pump outflow v. head difference between outlet and inlet nodes in ft (m) that has decreasing flow with increasing head
- **PUMP4** pump outflow v. continuous inlet node depth in ft (m)
- **PUMP5** pump outflow v. head difference between outlet and inlet nodes in ft (m) that has decreasing flow with increasing head
- **RATING** flow v. head in ft (m) for an Outlet link or a Custom inlet
- **CONTROL** control setting for a pump or flow regulator v. a controller variable (such as a node water level) in a modulated control; flow adjustment setting v. head for an LID unit’s underdrain
- **WEIR** discharge coefficient for flow in CFS (CMS) v. head in ft (m)

**Examples:**

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

---

<!---
  timeseries
-->

### [TIMESERIES] {#timeseries}

**Purpose:**

Describes how a quantity varies over time.

**Formats:**

_Name_ ( _Date_ ) _Hour_ _Value_ ...

_Name_ _Time_ _Value_ ...

_Name_ _FILE_ _Fname_

**Parameters:**

_Name_ -- name assigned to the time series.

_Date_ -- date in Month/Day/Year format (e.g., June 15, 2001 would be 6/15/2001).

_Hour_ -- 24-hour military time (e.g., 8:40 pm would be 20:40) relative to the last date specified (or to midnight of the starting date of the simulation if no previous date was specified).

_Time_ -- hours since the start of the simulation, expressed as a decimal number or as hours:minutes (where hours can be greater than 24).

_Value_ -- a value corresponding to the specified date and time.

_Fname_ -- the name of a file in which the time series data are stored

**Remarks:**

There are two options for supplying the data for a time series:

1. directly within this input file section as described by the first two formats

2. through an external data file named with the third format.

When direct data entry is used, multiple date-time-value or time-value entries can appear on a line. If more than one line is needed, the table's name must be repeated as the first entry on subsequent lines.

When an external file is used, each line in the file must use the same formats listed above, except that only one date-time-value (or time-value) entry is allowed per line. Any line that begins with a semicolon is considered a comment line and is ignored. Blank lines are also permitted. Enclose the external file name in double quotes if it contains spaces and include its full path if it resides in a different directory than the SWMM input file.

There are two options for describing the occurrence time of time series data:

- as calendar date plus time of day (which requires that at least one date, at the start of the series, be entered)

- as elapsed hours since the start of the simulation.

For the first method, dates need only be entered at points in time when a new day occurs.

For rainfall time series, it is only necessary to enter periods with non-zero rainfall amounts. SWMM interprets the rainfall value as a constant value lasting over the recording interval specified for the rain gage which utilizes the time series. For all other types of time series, SWMM uses interpolation to estimate values at times that fall in between the recorded values.

**Examples:**

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

---

<!---
  patterns
-->

### [PATTERNS] {#patterns}

**Purpose:**

Specifies time patterns of dry weather flow or quality in the form of adjustment factors applied as multipliers to baseline values.

**Format:**

|        |             |                                    |
| :----- | :---------- | :--------------------------------- |
| _Name_ | **MONTHLY** | _Factor1_ _Factor2_ ... _Factor12_ |
| _Name_ | **DAILY**   | _Factor1_ _Factor2_ ... _Factor7_  |
| _Name_ | **HOURLY**  | _Factor1_ _Factor2_ ... _Factor24_ |
| _Name_ | **WEEKEND** | _Factor1_ _Factor2_ ... _Factor24_ |

**Parameters:**

_Name_ -- name used to identify the pattern.

_Factor1_,

_Factor2_,

etc. multiplier values.

**Remarks:**

The **MONTHLY** format is used to set monthly pattern factors for dry weather flow constituents.

The **DAILY** format is used to set dry weather pattern factors for each day of the week, where Sunday is day 1.

The **HOURLY** format is used to set dry weather factors for each hour of the day starting from midnight. If these factors are different for weekend days than for weekday days then the **WEEKEND** format can be used to specify hourly adjustment factors just for weekends.

More than one line can be used to enter a pattern’s factors by repeating the pattern’s name (but not the pattern type) at the beginning of each additional line.

The pattern factors are applied as multipliers to any baseline dry weather flows or quality concentrations supplied in the [DWF] section.

**Examples:**

    ; Day of week adjustment factors
    D1  DAILY  0.5  1.0  1.0  1.0  1.0  1.0  0.5
    D2  DAILY  0.8  0.9  1.0  1.1  1.0  0.9  0.8

    ; Hourly adjustment factors
    H1 HOURLY  0.5 0.6 0.7 0.8 0.8 0.9
    H1         1.1 1.2 1.3 1.5 1.1 1.0
    H1         0.9 0.8 0.7 0.6 0.5 0.5
    H1         0.5 0.5 0.5 0.5 0.5 0.5

---

## Appendix C. SWMM5's Output File Format {#appendix-c}

The **Binary Output File** produced by the SWMM 5 computational engine consists of the following sections:

@subpage opening_records

@subpage object_id_names

@subpage object_properties

@subpage reporting_variables

@subpage reporting_interval

@subpage computed_results

@subpage closing_records

<!---
  opening_records
-->

### Opening Records {#opening_records}

The following basic information is written at the beginning of the binary output file produced by a SWMM 5 simulation:

- an identifying number equal to 516114522

- the version number of the engine (currently 52001)

- a code number for the flow units that are in effect where 0 = CFS, 1 = GPM, 2 = MGD, 3 = CMS, 4 = LPS, and 5 = LPD

- the number of subcatchments in the project reported on

- the number of nodes in the project reported on

- the number of links in the project reported on

- the number of pollutants in the project.

Each of these values is written to the file as a 4-byte integer.

<!---
  object_id_names
-->

### Object ID Names {#object_id_names}

Following the [Opening Records](#opening_records) section of SWMM 5's binary output file comes a section containing the ID names of each subcatchment, node, link, and pollutant defined in the SWMM 5 input file whose results are being reported. Pollutant concentration codes are also contained in this section of the file.

Each ID name consists of two records written to the output file:

- the number of characters in the name (expressed as a 4-byte integer)

- the string of characters that comprise the name (each character is 1 byte).

The names are written first for each reported subcatchment, then for each reported node, then for each reported link, and finally for each pollutant. Within each category of object the names are written in the same order in which the object appeared in the input file. This same order is maintained when computed results are written to the file in the [Computed Results](#computed_results) section.

Following the listing of pollutant names, the code numbers that represent the concentration units of each pollutant are written to the file as 4-byte integers. These codes are: 0 for mg/L, 1 for ug/L, and 2 for counts/L.

To summarize, the overall layout of this section of the file is:

- subcatchment ID names

- node ID names

- link ID names

- pollutant names

- pollutant concentration units codes

The byte position of the file where this section begins and the byte position where the next section begins are located in the [Closing Records](#closing_records) section of the file.

<!---
  object_properties
-->

### Object Properties {#object_properties}

The **Object Properties** section of the binary output file produced by the SWMM 5 engine contains a limited collection of input properties associated with each object (not the entire set of properties). The format of this section is as follows:

| Item                                            | Type  | Description                                                                                                                                                         |
| :---------------------------------------------- | :---- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Number of subcatchment properties saved         | INT4  | Currently equal to 1                                                                                                                                                |
| Code number of each subcatchment property saved | INT4  | Currently equal to 1 for subcatchment area                                                                                                                          |
| Value of each property for each subcatchment    | REAL4 | Subcatchment area (ac or ha) for each subcatchment                                                                                                                  |
| Number of node properties saved                 | INT4  | Currently equal to 3                                                                                                                                                |
| Code number of each node property saved         | INT4  | Currently equal to: <br> 0 (node type code), <br> 2 (node invert elevation), and <br> 3 (node max. depth)                                                           |
| Value of each property for each node            | REAL4 | Node type code, invert elevation (ft or m), and max. depth (ft. or m) for each node                                                                                 |
| Number of link properties saved                 | INT4  | Currently equal to 5                                                                                                                                                |
| Code number of each link property saved         | INT4  | Currently equal to: <br> 0 (link type code), <br> 4 (upstream invert offset), <br> 4 (downstream invert offset), <br> 3 (link max. depth), and <br> 5 (link length) |
| Value of each property for each link            | REAL4 | Link type code, upstream invert offset (ft or m), downstream invert offset (ft or m), max. depth (ft or m), and length (ft or m) for each link.                     |

**Notes:**

1.  INT4 = 4-byte integer, REAL4 = 4-byte float

2.  Node type codes are:

    - 0 = Junction
    - 1 = Outfall
    - 2 = Storage
    - 3 = Divider

3.  Link type codes are:

    - 0 = Conduit
    - 1 = Pump
    - 2 = Orifice
    - 3 = Weir
    - 4 = Outlet

4.  The units of each property (US or metric) depend on the unit system used for flow.

<!---
  reporting_variables
-->

### Reporting Variables {#object_properties}

The **Reporting Variables** section of the SWMM 5 binary output file contains the number and codes of each variable reported on. The items written to this section, all as 4-byte integers, are as follows:

- Number of subcatchment variables (currently 8 + number of pollutants).

- Code number of each subcatchment variable:

  - 0 for rainfall (in/hr or mm/hr),
  - 1 for snow depth (in or mm),
  - 2 for evaporation loss (in/day or mm/day),
  - 3 for infiltration losses (in/hr or mm/hr),
  - 4 for runoff rate (flow units),
  - 5 for groundwater outflow rate (flow units),
  - 6 for groundwater water table elevation (ft or m),
  - 7 for unsaturated zone moisture content (fraction)
  - 8 for runoff concentration of first pollutant,

    ...

  - 7 + N for runoff concentration of N-th pollutant.

- Number of node variables (currently 6 + number of pollutants)

- Code number of each node variable:

  - 0 for depth of water above invert (ft or m),
  - 1 for hydraulic head (ft or m),
  - 2 for volume of stored + ponded water (ft3 or m3),
  - 3 for lateral inflow (flow units),
  - 4 for total inflow (lateral + upstream) (flow units),
  - 5 for flow lost to flooding (flow units),
  - 6 for concentration of first pollutant,

    ...

  - 5 + N for concentration of N-th pollutant.

- Number of link variables (currently 5 + number of pollutants)

- Code number of each link variable:

  - 0 for flow rate (flow units),
  - 1 for flow depth (ft or m),
  - 2 for flow velocity (ft/s or m/s),
  - 3 for flow volume (ft3 or m3)
  - 4 for fraction of conduit's area filled or setting for non-conduits
  - 5 for concentration of first pollutant,

    ...

  - 4 + N for concentration of N-th pollutant.

- Number of system-wide variables (currently 15)

- Code number of each system-wide variable:

  - 0 for air temperature (deg. F or deg. C),
  - 1 for rainfall (in/hr or mm/hr),
  - 2 for snow depth (in or mm),
  - 3 for evaporation + infiltration loss rate (in/hr or mm/hr),
  - 4 for runoff flow (flow units),
  - 5 for dry weather inflow (flow units),
  - 6 for groundwater inflow (flow units),
  - 7 for RDII inflow (flow units),
  - 8 for user supplied direct inflow (flow units),
  - 9 for total lateral inflow (sum of variables 4 to 8) (flow units),
  - 10 for flow lost to flooding (flow units),
  - 11 for flow leaving through outfalls (flow units),
  - 12 for volume of stored water (ft3 or m3),
  - 13 for actual evaporation rate (in/day or mm/day)
  - 14 for potential evaporation rate (PET) (in/day or mm/day

<!---
  reporting_interval
-->

### Reporting Interval {#object_properties}

The **Reporting Interval** portion of the SWMM 5 binary output file consists of only two records:

- The start date and time of the simulation, expressed as an 8-byte double precision number representing the number of decimal days since 12:00 am of December 30, 1899.

- The time interval between reporting periods in seconds, expressed as a 4-byte integer number.

<!---
  computed_results
-->

### Computed Results {#computed_results}

The **Computed Results** portion of the SWMM 5 binary output file is where the computed values for each subcatchment, node, link and system reporting variable are stored for each reporting period of the simulation. The number and type of each reporting variable was supplied in the [Reporting Variables](#reporting_variables) portion of the file. The following information is written to the binary file for each reporting period, starting from the first period (the initial conditions at time 0 of the simulation do not appear in the file):

- the date and time of the reporting period, expressed as an 8-byte double precision number representing the number of decimal days since 12/30/1899,

- the value of each subcatchment variable for each subcatchment, as 4-byte floats,

- the value of each node variable for each node, as 4-byte floats

- the value of each link variable for each link, as 4-byte floats

- the value of each system-wide variable, as 4-byte floats.

Note that the layout of these data is as follows:

- Date/time value
- Variable 1, Variable 2, etc. for first subcatchment

  ...

- Variable 1, Variable 2, etc. for last subcacthment
- Variable 1, Variable 2, etc. for first node

  ...

- Variable 1, Variable 2, etc. for last node
- Variable 1, Variable 2, etc. for first link

  ...

- Variable 1, Variable 2, etc. for last link
- First system-wide variable, ...., last system-wide variable.

The order in which each variable is written for a specific object follows the order in which the variables were listed in the Reporting Variables section of the file. The order in which each subcatchment, node, and link appear follows the same order in which they appear in the Object ID Names section of the file, and is the same order they appear in the SWMM 5 input file. Again remember that only a subset of all subcatchments, nodes, and links may be reported on, depending on the options that were specified in the [REPORT] section of the project's input file.

This collection of data is repeated for each reporting period. The actual number of reporting periods can be read from the [Closing Records](#closing_records) portion of the file.

<!---
  closing_records
-->

### Closing Records {#closing_records}

The following records are written to the SWMM 5 binary results file following the data that appear in the [Computed Results](#computed_results) section of the file:

- the byte position where the [Object ID Names](#object_id_names) section of the file begins (4-byte integer)

- the byte position where the [Object Properties](#object_properties) section of the file begins (4-byte integer)

- the byte position where the [Computed Results](#computed_results) section of the file begins (4-byte integer)

- the total number of reporting periods contained in the [Computed Results](#computed_results) section of the file (4-byte integer)

- the error code status of the simulation, where 0 indicates no errors (4-byte integer)

- the same identifying number, 516114522, that appears as the very first record in the file (4-byte integer).

Note that byte positions are expressed as offsets from the beginning of the file, whose byte position is 0.
