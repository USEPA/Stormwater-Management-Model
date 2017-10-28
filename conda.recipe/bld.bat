setlocal EnableDelayedExpansion

:: Make a build folder and change to it.
mkdir build
cd build

# GCC Compatibiliy Issues
conda uninstall gcc
conda install -c serge-sans-paille gcc_49

:: Configure using the CMakeFiles
%LIBRARY_BIN%\cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX:PATH="%LIBRARY_PREFIX%" -DCMAKE_BUILD_TYPE:STRING=Release ..
if errorlevel 1 exit 1

:: Build!
nmake
if errorlevel 1 exit 1

:: Install!
nmake install
if errorlevel 1 exit 1
