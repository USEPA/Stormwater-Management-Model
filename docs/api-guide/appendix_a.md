## Appendix A. SWMM5's API Functions {#appendix_a}

The following functions make up the SWMM 5 API:

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

<!--
  swmm_getversion
-->

@page swmm_getversion swmm_getVersion

**Syntax**

`int swmm_getVersion(void);`

**Description**

Retrieves the current version number of the SWMM 5 engine.

**Arguments**

None.

**Return Value**

Returns a version number.

<!--
  swmm_run
-->

@page swmm_run swmm_run

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

<!--
  swmm_open
-->

@page swmm_open swmm_open

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

<!--
  swmm_start
-->

@page swmm_start swmm_start

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

<!--
  swmm_step
-->

@page swmm_step swmm_step

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

<!--
  swmm_stride
-->

@page swmm_stride swmm_stride

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

<!---
  swmm_end
-->

@page swmm_end swmm_end

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

<!---
  swmm_getmassbalerr
-->

@page swmm_getmassbalerr swmm_getMassBalErr

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

<!--
  swmm_report
-->

@page swmm_report swmm_report

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

<!---
  swmm_close
-->

@page swmm_close swmm_close

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

<!--
  swmm_getwarnings
-->

@page swmm_getwarnings swmm_getWarnings

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

@page swmm_geterror swmm_getError

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

<!---
  swmm_getcount
-->

@page swmm_getcount swmm_getCount

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

<!--
  swmm_getname
-->

@page swmm_getname swmm_getName

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

<!---
  swmm_getindex
-->

@page swmm_getindex swmm_getIndex

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

<!--
  swmm_getvalue
-->

@page swmm_getvalue swmm_getValue

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

<!--
  swmm_getsavedvalue
-->

@page swmm_getsavedvalue swmm_getSavedValue

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

<!--
  swmm_setvalue
-->

@page swmm_setvalue swmm_setValue

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

<!--
  swmm_writeline
-->

@page swmm_writeline swmm_writeLine

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

@page swmm_decodedate swmm_decodeDate

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
