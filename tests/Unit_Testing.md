<!---
   Unit_Testing.md

   Created: Dec 2, 2019
   Updated:

   Author: Michael E. Tryby
           US EPA - ORD/CESER
--->


## Unit Testing SWMM locally on Windows

### Dependencies

Before the project can be built and tested the required dependencies must be installed.

**Summary of Build Dependencies: Windows**

  - Build
      - Build Tools for Visual Studio 2017
      - CMake 3.13

  - Unit Test
      - Boost 1.67.0 (installed in default location "C:\\local")



### Build and Unit Test

SWMM can be built and unit tests run with one simple command.
```
\> cd swmm
\swmm>tools\make.cmd /t
```
