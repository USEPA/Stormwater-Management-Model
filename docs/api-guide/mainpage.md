@mainpage Introduction

The EPA Storm Water Management Model Version 5 (SWMM 5) is a dynamic rainfall-runoff simulation model used for single event or long-term (continuous) simulation of runoff quantity and quality from primarily urban areas.

![Urban Wet Weather Flows](urbanwetweatherflows.zoom75.jpg)

SWMM's computational engine contains a library of functions (or Application Programming Interface) written in C that allow programmers to customize its use within their own programs. Some typical use cases for the API might include:

- supplying externally obtained rainfall and lateral inflows to a system
- providing real time control of pumps and regulators based on monitoring data
- linking SWMM with other models, such as a 2-D surface flow or a groundwater flow model
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

@page data_model SWMM's Data Model

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

@page files_used Files Used By SWMM

The following files are used when linking the SWMM 5 engine with other applications:

**A SWMM 5 input file.** This is a plain text file that contains all of the information needed to run a simulation for a given study area. The contents and format of this file are described in detail in [Appendix B](#appendix_b). The application using the SWMM API must either identify the name of an existing input file or create the file itself and save it to disk. Projects created and saved when running the Windows version of SWMM can be used as input files to the SWMM API.

**A SWMM 5 report file.** This is a plain text file which can contain a run's status report, including any error messages, as well as summary results tables. The SWMM 5 engine will open and write the contents of this file, but the interfacing application must provide a valid file name to the engine. The API allows one to control how much information will appear in the file. The application can decide whether or not to display the report file and whether to save or delete it.

**A SWMM 5 output file.** This is an optional binary file that will contain computed results at uniform reporting time steps of a simulation for all of the elements that were marked for reporting in the project's input file. The SWMM engine will open and write the contents of this file, but the interfacing application must provide a valid file name to the engine if the file will be saved to disk once the SWMM engine is closed. If no file name is supplied then
SWMM will use a temporary file instead.  After a SWMM run is completed, the application can use the API to access this file to retrieve simulation results. The format of this file is described in [Appendix C](#appendix_c) in case one wishes to read its contents after a SWMM 5 project has been closed.

<!---
  Units
-->

@page units Units of Measurement

SWMM can use either US customary units or SI metric units for its design and computed variables. The choice of flow units set in SWMM\'s input file determines what unit system is in effect:

- selecting CFS (cubic feet per second), GPM (gallons per minutes), or MGD (million gallons per day) for flow units implies that US units will be used throughout

- selecting CMS (cubic meters per second), LPS (liters per second), or MLD (million liters per day) as flow units implies that SI units will be used throughout.

- pollutant concentration and Manning's roughness coefficient (n) are always expressed in metric units.

The API function `swmm_getValue(swmm_FLOWUNITS, 0)` can be used to determine which choice of flow units was specified in a project\'s input file.

@page units Units of Measurement

SWMM can use either US customary units or SI metric units for its design and computed variables. The choice of flow units set in SWMM\'s input file determines what unit system is in effect:

- selecting CFS (cubic feet per second), GPM (gallons per minutes), or MGD (million gallons per day) for flow units implies that US units will be used throughout

- selecting CMS (cubic meters per second), LPS (liters per second), or MLD (million liters per day) as flow units implies that SI units will be used throughout.

- pollutant concentration and Manning's roughness coefficient (n) are always expressed in metric units.

The API function `swmm_getValue(swmm_FLOWUNITS, 0)` can be used to determine which choice of flow units was specified in a project\'s input file.

<!---
  Running SWMM
-->

@page running Running the SWMM Engine

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

@page examining Examining Object Properties

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

@page retrieving Retrieving Simulation Results

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

@page controlling Controlling Simulation Conditions

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

@page bindings Language Bindings

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

- @subpage c_lang
- @subpage pascal
- @subpage python

<!---
  C/C++
-->

@page c_lang C/C++ Example

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

@page pascal Pascal Example

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

@page python Python Example

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
