# Appendix C. SWMM 5's Output File Format {#appendix_c}

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

@page opening_records Opening Records

The following basic information is written at the beginning of the
binary output file produced by a SWMM 5 simulation:

---

· an identifying number equal to 516114522

---

---

· the version number of the engine (currently 52001)

---

---

· a code number for the flow units that are in effect where 0 = CFS, 1 = GPM, 2 = MGD, 3 = CMS, 4 = LPS, and 5 = LPD

---

---

· the number of subcatchments in the project reported on

---

---

· the number of nodes in the project reported on

---

---

· the number of links in the project reported on

---

---

· the number of pollutants in the project.

---

Each of these values is written to the file as a 4-byte integer.

<!---
  object_id_names
-->

@page object_id_names Object ID Names

Following the Opening Records section of SWMM 5's binary output file
comes a section containing the ID names of each subcatchment, node,
link, and pollutant defined in the SWMM 5 input file whose results are
being reported. Pollutant concentration codes are also contained in this
section of the file.

Each ID name consists of two records written to the output file:

---

· the number of characters in the name (expressed as a 4-byte integer)

---

---

· the string of characters that comprise the name (each character is 1 byte).

---

The names are written first for each reported subcatchment, then for
each reported node, then for each reported link, and finally for each
pollutant. Within each category of object the names are written in the
same order in which the object appeared in the input file. This same
order is maintained when computed results are written to the file in the
Computed Results section.

Following the listing of pollutant names, the code numbers that
represent the concentration units of each pollutant are written to the
file as 4-byte integers. These codes are: 0 for mg/L, 1 for ug/L, and 2
for counts/L.

To summarize, the overall layout of this section of the file is:

---

· subcatchment ID names

---

---

· node ID names

---

---

· link ID names

---

---

· pollutant names

---

---

· pollutant concentration units codes

---

The byte position of the file where this section begins and the byte
position where the next section begins are located in the Closing
Records section of the file.

<!---
  object_properties
-->

@page object_properties Object Properties

The Object Properties section of the binary output file produced by the
SWMM 5 engine contains a limited collection of input properties
associated with each object (not the entire set of properties). The
format of this section is as follows:

+-----------------------+-----------------------+-----------------------+
| Item | Type | Description |
+-----------------------+-----------------------+-----------------------+
| Number of | INT4 | Currently equal to 1 |
| subcatchment | | |
| properties saved | | |
| | | |
|   | | |
+-----------------------+-----------------------+-----------------------+
| Code number of each | INT4 | Currently equal to 1 |
| subcatchment property | | for subcatchment area |
| saved | | |
| | | |
|   | | |
+-----------------------+-----------------------+-----------------------+
| Value of each | REAL4 | Subcatchment area (ac |
| property for each | | or ha) for each |
| subcatchment | | subcatchment |
| | | |
|   | | |
+-----------------------+-----------------------+-----------------------+
| Number of node | INT4 | Currently equal to 3 |
| properties saved | | |
| | | |
|   | | |
+-----------------------+-----------------------+-----------------------+
| Code number of each | INT4 | Currently equal to: |
| node property saved | | |
| | | 0 (node type code), |
| | | |
| | | 2 (node invert |
| | | elevation), and |
| | | |
| | | 3 (node max. depth) |
| | | |
| | |   |
+-----------------------+-----------------------+-----------------------+
| Value of each | REAL4 | Node type code, |
| property for each | | invert elevation (ft |
| node | | or m), and max. depth |
| | | (ft. or m) for each |
| | | node |
| | | |
| | |   |
+-----------------------+-----------------------+-----------------------+
| Number of link | INT4 | Currently equal to 5 |
| properties saved | | |
| | | |
|   | | |
+-----------------------+-----------------------+-----------------------+
| Code number of each | INT4 | Currently equal to: |
| link property saved | | |
| | | 0 (link type code), |
| | | |
| | | 4 (upstream invert |
| | | offset), |
| | | |
| | | 4 (downstream invert |
| | | offset), |
| | | |
| | | 3 (link max. depth), |
| | | and |
| | | |
| | | 5 (link length) |
| | | |
| | |   |
+-----------------------+-----------------------+-----------------------+
| Value of each | REAL4 | Link type code, |
| property for each | | upstream invert |
| link | | offset (ft or m), |
| | | downstream invert |
| | | offset (ft or m), |
| | | max. depth (ft or m), |
| | | and length (ft or m) |
| | | for each link. |
+-----------------------+-----------------------+-----------------------+

NOTES:

---

1.  INT4 = 4-byte integer, REAL4 = 4-byte float

---

---

2.  Node type codes are:

---

0 = Junction

1 = Outfall

2 = Storage

3 = Divider

---

3.  Link type codes are:

---

0 = Conduit

1 = Pump

2 = Orifice

3 = Weir

4 = Outlet

---

4.  The units of each property (US or metric) depend on the unit system used for flow.

<!---
  reporting_variables
-->

@page reporting_variables Reporting Variables

The Reporting Variables section of the SWMM 5 binary output file
contains the number and codes of each variable reported on. The items
written to this section, all as 4-byte integers, are as follows:

---

· Number of subcatchment variables (currently 8 + number of pollutants).

---

---

· Code number of each subcatchment variable:

---

0 for rainfall (in/hr or mm/hr),

1 for snow depth (in or mm),

2 for evaporation loss (in/day or mm/day),

3 for infiltration losses (in/hr or mm/hr),

4 for runoff rate (flow units),

5 for groundwater outflow rate (flow units),

6 for groundwater water table elevation (ft or m),

7 for unsaturated zone moisture content (fraction)

8 for runoff concentration of first pollutant,

...

7 + N for runoff concentration of N-th pollutant.

---

· Number of node variables (currently 6 + number of pollutants)

---

---

· Code number of each node variable:

---

0 for depth of water above invert (ft or m),

1 for hydraulic head (ft or m),

2 for volume of stored + ponded water (ft3 or m3),

3 for lateral inflow (flow units),

4 for total inflow (lateral + upstream) (flow units),

5 for flow lost to flooding (flow units),

6 for concentration of first pollutant,

...

5 + N for concentration of N-th pollutant.

---

· Number of link variables (currently 5 + number of pollutants)

---

---

· Code number of each link variable:

---

0 for flow rate (flow units),

1 for flow depth (ft or m),

2 for flow velocity (ft/s or m/s),

3 for flow volume (ft3 or m3)

4 for fraction of conduit's area filled or setting for non-conduits

5 for concentration of first pollutant,

...

4 + N for concentration of N-th pollutant.

---

· Number of system-wide variables (currently 15)

---

---

· Code number of each system-wide variable:

---

0 for air temperature (deg. F or deg. C),

1 for rainfall (in/hr or mm/hr),

2 for snow depth (in or mm),

3 for evaporation + infiltration loss rate (in/hr or mm/hr),

4 for runoff flow (flow units),

5 for dry weather inflow (flow units),

6 for groundwater inflow (flow units),

7 for RDII inflow (flow units),

8 for user supplied direct inflow (flow units),

9 for total lateral inflow (sum of variables 4 to 8) (flow units),

10 for flow lost to flooding (flow units),

11 for flow leaving through outfalls (flow units),

12 for volume of stored water (ft3 or m3),

13 for actual evaporation rate (in/day or mm/day)

14 for potential evaporation rate (PET) (in/day or mm/day

<!---
  reporting_interval
-->

@page reporting_interval Reporting Interval

The Reporting Interval portion of the SWMM 5 binary output file consists
of only two records:

---

· The start date and time of the simulation, expressed as an 8-byte double precision number representing the number of decimal days since 12:00 am of December 30, 1899.

---

---

· The time interval between reporting periods in seconds, expressed as a 4-byte integer number.

---

<!---
  computed_results
-->

@page computed_results Computed Results

The Computed Results portion of the SWMM 5 binary output file is where
the computed values for each subcatchment, node, link and system
reporting variable are stored for each reporting period of the
simulation. The number and type of each reporting variable was supplied
in the Reporting Variables portion of the file. The following
information is written to the binary file for each reporting period,
starting from the first period (the initial conditions at time 0 of the
simulation do not appear in the file):

---

· the date and time of the reporting period, expressed as an 8-byte double precision number representing the number of decimal days since 12/30/1899,

---

---

· the value of each subcatchment variable for each subcatchment, as 4-byte floats,

---

---

· the value of each node variable for each node, as 4-byte floats

---

---

· the value of each link variable for each link, as 4-byte floats

---

---

· the value of each system-wide variable, as 4-byte floats.

---

Note that the layout of these data is as follows:

Date/time value

Variable 1, Variable 2, etc. for first subcatchment

...

Variable 1, Variable 2, etc. for last subcacthment

Variable 1, Variable 2, etc. for first node

...

Variable 1, Variable 2, etc. for last node

Variable 1, Variable 2, etc. for first link

...

Variable 1, Variable 2, etc. for last link

First system-wide variable, ...., last system-wide variable.

The order in which each variable is written for a specific object
follows the order in which the variables were listed in the Reporting
Variables section of the file. The order in which each subcatchment,
node, and link appear follows the same order in which they appear in the
Object ID Names section of the file, and is the same order they appear
in the SWMM 5 input file. Again remember that only a subset of all
subcatchments, nodes, and links may be reported on, depending on the
options that were specified in the [REPORT] section of the project's
input file.

This collection of data is repeated for each reporting period. The
actual number of reporting periods can be read from the Closing Records
portion of the file.

<!---
  closing_records
-->

@page closing_records Closing Records

The following records are written to the SWMM 5 binary results file
following the data that appear in the Computed Results section of the
file:

---

· the byte position where the Object ID Names section of the file begins (4-byte integer)

---

---

· the byte position where the Object Properties section of the file begins (4-byte integer)

---

---

· the byte position where the Computed Results section of the file begins (4-byte integer)

---

---

· the total number of reporting periods contained in the Computed Results section of the file (4-byte integer)

---

---

· the error code status of the simulation, where 0 indicates no errors (4-byte integer)

---

---

· the same identifying number, 516114522, that appears as the very first record in the file (4-byte integer).

---

Note that byte positions are expressed as offsets from the beginning of
the file, whose byte position is 0.
