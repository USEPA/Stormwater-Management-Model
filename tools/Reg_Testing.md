<!---
   Reg_Testing.md

   Created: Dec 2, 2019
   Updated:

   Author: Michael E. Tryby
           US EPA - ORD/CESER
--->

## Regression Testing SWMM locally on Windows


### Dependencies

Before the project can be built and tested the required dependencies must be installed.

**Summary of Build Dependencies: Windows**

  - Build
      - Build Tools for Visual Studio 2017
      - CMake 3.13

  - Regression Test
      - Python 3.6 64 bit
      - curl
      - git
      - 7z

Once Python is present, the following command installs the required packages for regression testing.
```
\> cd swmm
\swmm>pip install -r tools\requirements-win.txt
```


### Build

EPANET can be built with one simple command.
```
\swmm>tools\make.cmd
```


### Regression Test

This command runs regression tests for the local build and compares them to the latest benchmark.
```
\swmm>tools\before-nrtest.cmd && tools\run-nrtest.cmd
```
