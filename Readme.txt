CONTENTS OF SWMM522_ENGINE.ZIP
==============================

The 'src' folder of this archive contains the C source code for
version 5.2.2 of the Storm Water Management Model's computational
engine. Consult the included 'Roadmap.txt' file for an overview of
the various code modules. The code can be compiled into both a shared
object library and a command line executable. Under Windows, the 
library file (swmm5.dll) is used to power SWMM's graphical user
interface.

The 'CMakeLists.txt' file is a script used by CMake (https://cmake.org/)
to build the SWMM 5.2 binaries. CMake is a cross-platform build tool
that generates platform native build systems for many compilers. To
check if the required version is installed on your system, enter

    cmake --version

from a console window and check that the version is 3.5 or higher.

To build the SWMM 5.2 engine library and its command line executable
using CMake and the Microsoft Visual Studio C compiler on Windows:

1. Open a console window and navigate to the directory where this
   Readme file resides (which should have 'src' as a sub-directory
   underneath it).

2. Issue the following commands:
     mkdir build
     cd build

3. Then enter the following CMake commands:
     cmake -G <compiler> .. -A <platform>
     cmake --build . --config Release

where <compiler> is the name of the Visual Studio compiler being used
in double quotes (e.g., "Visual Studio 15 2017" or "Visual Studio 16 2019")
and <platform> is Win32 for a 32-bit build or x64 for a 64-bit build.
The resulting engine DLL (swmm5.dll) and command line executable
(runswmm.exe) will appear in the build\Release directory.

For other platforms, such as Linux or MacOS, Step 3 can be replaced with:
      cmake ..
      cmake --build .

The resulting shared object library (libswmm5.so) and command line executable
(runswmm) will appear in the build directory. 