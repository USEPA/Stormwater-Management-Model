setlocal EnableDelayedExpansion

:: Make build directory and change to it
mkdir build
cd build

:: Configure using the CMakeFiles
%LIBRARY_BIN%\cmake -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX:PATH="%LIBRARY_PREFIX%" -DCMAKE_BUILD_TYPE:STRING=Release ..
if errorlevel 1 exit 1

:: Build and install
cmake --build . --config Release --target install
if errorlevel 1 exit 1
