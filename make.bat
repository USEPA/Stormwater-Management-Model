::
::  makefile.bat - build swmm-solver
::
::  Date Created: 7/11/2019
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
:: Requires:
::    Build Tools for Visual Studio
::      https://visualstudio.microsoft.com/downloads/
::
::    CMake
::      https://cmake.org/download/
::
:: Note:
::    This script must be located at the root of the project folder
::    in order to work properly.
::

@echo off
echo INFO: Building swmm-solver  ...

set GENERATOR=Visual Studio 15 2017

:: Determine project path and strip trailing \ from path
set "PROJECT_PATH=%~dp0"
IF %PROJECT_PATH:~-1%==\ set "PROJECT_PATH=%PROJECT_PATH:~0,-1%"

:: check for requirements
WHERE cmake
IF %ERRORLEVEL% NEQ 0 ECHO cmake not installed & EXIT /B 1

:: generate build system
IF exist buildprod_win32 ( cd buildprod_win32 ) ELSE ( mkdir buildprod_win32 & cd buildprod_win32 )
cmake -G"%GENERATOR%" ..

cd ..
IF exist buildprod_win64 ( cd buildprod_win64 ) ELSE ( mkdir buildprod_win64 & cd buildprod_win64 )
cmake -G"%GENERATOR% Win64" ..

:: perform build
cmake --build . --config Release --target install
cpack

cd ..\buildprod_win32
cmake --build . --config Release --target install
cpack

:: return to project root
cd %PROJECT_PATH%
