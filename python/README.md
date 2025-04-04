EPA ORD Stormwater Management Model (SWMM)
==========================================

Stormwater Management Model (SWMM) computational engine and output post-processing codebase

## Build Status
[![Build and Unit Testing](https://github.com/USEPA/Stormwater-Management-Model/actions/workflows/unit_testing.yml/badge.svg)](https://github.com/USEPA/Stormwater-Management-Model/actions/workflows/unit_testing.yml)
[![Build and Regression Testing](https://github.com/USEPA/Stormwater-Management-Model/actions/workflows/regression_testing.yml/badge.svg)](https://github.com/USEPA/Stormwater-Management-Model/actions/workflows/regression_testing.yml)
[![Docs](https://github.com/USEPA/Stormwater-Management-Model/actions/workflows/build_docs.yml/badge.svg)](https://github.com/USEPA/Stormwater-Management-Model/actions/workflows/build_docs.yml)
[![Deployment](https://github.com/USEPA/Stormwater-Management-Model/actions/workflows/deploy.yml/badge.svg)](https://github.com/USEPA/Stormwater-Management-Model/actions/workflows/deploy.yml)
[![Documentation](https://github.com/USEPA/Stormwater-Management-Model/actions/workflows/build-and-test.yml/badge.svg?branch=docs)](https://github.com/USEPA/Stormwater-Management-Model/actions/workflows/build-and-test.yml)
[![Issues](https://img.shields.io/github/issues/USEPA/Stormwater-Management-Model)](https://github.com/USEPA/Stormwater-Management-Model/issues)

## Python Binding
[![PyPi](https://img.shields.io/pypi/v/epaswmm.svg)](https://pypi.org/project/epaswmm)
[![PythonVersion](https://img.shields.io/pypi/pyversions/epaswmm.svg)](https://pypi.org/project/epaswmm)
[![Wheel](https://img.shields.io/pypi/wheel/epaswmm.svg)](https://pypi.org/project/epaswmm)
[![Downloads](https://pepy.tech/badge/epaswmm)](https://pepy.tech/project/epaswmm)
[![Downloads](https://pepy.tech/badge/epaswmm/month)](https://pepy.tech/project/epaswmm)
[![Downloads](https://pepy.tech/badge/epaswmm/week)](https://pepy.tech/project/epaswmm)

## Introduction
This is the official SWMM source code repository maintained by US EPA Office of Research and Development, Center For Environmental Solutions & Emergency Response, Water Infrastructure Division located in Cincinnati, Ohio.

SWMM is a dynamic hydrology-hydraulic water quality simulation model. It is used for single event or long-term (continuous) simulation of runoff quantity and quality from primarily urban areas. SWMM source code is written in the C Programming Language and released in the Public Domain.

## Build Instructions

The 'src' folder of this repository contains the C source code for
version of Storm Water Management Model's computational
engine. Consult the included 'Roadmap.txt' file for an overview of
the various code modules. The code can be compiled into both a shared
object library and a command line executable. Under Windows, the 
library file (swmm5.dll) is used to power SWMM's graphical user
interface.

Also included is a python interface for the SWMM computational engine and output 
post-processing application programming interfaces located in the python folder.

The 'CMakeLists.txt' file is a script used by CMake (https://cmake.org/)
to build the SWMM binaries. CMake is a cross-platform build tool
that generates platform native build systems for many compilers. To
check if the required version is installed on your system, enter from 
a console window and check that the version is 3.5 or higher.

```bash
cmake --version
```

To build the SWMM engine library and its command line executable
using CMake and the Microsoft Visual Studio C compiler on Windows:

1. Open a console window and navigate to the directory where this
   Readme file resides (which should have 'src' as a sub-directory
   underneath it).

2. Use the following command to create the directory for storing the built binaries:

```bash
mkdir build
```

3. Then the following CMake commands to build the binaries:

``` bash
cmake -G <compiler> -B build
cmake --build ./build --config Release
```

where `<compiler>` is the name of the compiler being used
in double quotes (e.g., "Visual Studio 17 2022" for windows, "Ninja" for linux, or "Xcode" for macos). The resulting engine shared libraries (i.e., swmm5.dll), command line executable (i.e., runswmm.exe), and output processing libraries (i.e., swmm-output.dll)
will appear in the build\Release directory.

### Python Bindings (Experimental)

Experimental python bindings for the SWMM API are being developed to support regression and benchmark testing as well as for other applications. _**These bindings are still under development and testing and has yet to be cleared through US EPA ORD's official quality assurance review process**_. The exprimental python bindings can be built and installed locally using the following command.

```bash
cd python
python -m pip install -r requirements.txt
python -m pip install . 
```
Users may also build python wheels for installation or distribution. Once the python bindings
have been validated and cleared through EPA's quality assuracnce clearance process, they will be available for installation via package indexing repositories such as pypi.

Example usage of python bindings can be found below. More extensive documentation will be provided once cleared.

```python

from epaswmm import solver
from epaswmm.solver import Solver 
from epaswmm.output import Output

# Alternative 1 to run SWMM

with Solver(inp_file="input_file.inp") as swmm_solver:
   
   # Open swmm file and starts the simulation
   swmm_solver.start()

   # Set initialization parameters e.g., time step stride, start date, end date etc.
   swmm_solver.time_stride = 600 

   for elapsed_time, current_datetime in swmm_solver:

      # Get and set attributes per timestep
      print(current_datetime)

      swmm_solver.set_value(
         object_type=solver.SWMMObjects.RAIN_GAGE,
         property_type=solver.SWMMRainGageProperties.GAGE_RAINFALL,
         index="RG1",
         value=3.6
      )

# Alternative 2 to run SWMM
swmm_solver = Solver(inp_file="input_file.inp")
swmm_solver.initialize()

for elapsed_time, current_datetime in swmm_solver:
   # Get and set attributes per timestep
   print(current_datetime)

swmm_solver.finalize()
# or
# swmm_solver.end()
# swmm_solver.report()
# swmm_solver.close()

# Alternative 3 to run SWMM
swmm_solver = Solver(inp_file="input_file.inp")
swmm_solver.execute()

# To read output file

swmm_output = Output(output_file='output_file.out')

# Dict[datetime, float]
link_timeseries = swmm_output.get_link_timeseries(
   element_index="C1",
   attribute=output.LinkAttribute.FLOW_RATE,
)

```

## Unit and Regression Testing

Unit tests and regression tests have been developed for both the natively compiled SWMM computational engine and output toolkit as well as their respective python bindings. Unit tests for the natively compiled toolkits use the Boost 1.67.0 library and can be compiled by adding DBUILD_TESTS=ON flag during the cmake build phase as shown below:

```bash
ctest --test-dir .  -DBUILD_TESTS=ON --config Debug --output-on-failure
```

Unit testing on the python bindings may be executed using the following command after installation.

```bash
cd python\tests
pytest .
```

Regression tests are executed using the python bindings using the pytest and pytest-regressions extension using the following commands.

```bash
cd ci
pytest --data-dir <path-to-regression-testing-files> --atol <absolute-tolerance> --rtol <relative-tolerance> --benchmark-compare --benchmark-json=PATH
```

## Find Out More
The source code distributed here is identical to the code found at the official [SWMM website](https://www.epa.gov/water-research/storm-water-management-model-swmm).
The SWMM website also hosts the official manuals and installation binaries for the SWMM software. 

A live web version of the SWMM documentation of the API and user manuals can be found on the [SWMM GitHub Pages website](https://usepa.github.io/Stormwater-Management-Model). Note that this is an experimental version that is still under development and has yet to go through EPA'S official quality assurance review process.

## Disclaimer 
The United States Environmental Protection Agency (EPA) GitHub project code is provided on an "as is" basis and the user assumes responsibility for its use. EPA has relinquished control of the information and no longer has responsibility to protect the integrity, confidentiality, or availability of the information. Any reference to specific commercial products, processes, or services by service mark, trademark, manufacturer, or otherwise, does not constitute or imply their endorsement, recommendation or favoring by EPA. The EPA seal and logo shall not be used in any manner to imply endorsement of any commercial product or activity by EPA or the United States Government.

