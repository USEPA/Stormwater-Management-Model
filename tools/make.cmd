::
::  make.cmd - builds project
::
::  Date Created: 10/15/2019
::  Date Updated:
::
::  Author: Michael E. Tryby
::          US EPA - ORD/CESER
::
::  Requires:
::    Build Tools for Visual Studio download:
::      https://visualstudio.microsoft.com/downloads/
::
::    CMake download:
::      https://cmake.org/download/
::
::  Optional Arguments:
::    /g ("GENERATOR") defaults to "Visual Studio 15 2017"
::    /t builds and runs unit tests (requires Boost)
::


::echo off


:: set global defaults
set "PROJECT=swmm"
set "BUILD_HOME=build"
set "PLATFORM=win32"

:: determine project directory
set "CUR_DIR=%CD%"
set "SCRIPT_HOME=%~dp0"
cd %SCRIPT_HOME%
cd ..

:: check for requirements
where cmake > nul
if %ERRORLEVEL% NEQ 0 ( echo "ERROR: cmake not installed" & exit /B 1 )


setlocal EnableDelayedExpansion


echo INFO: Building %PROJECT%  ...


:: set local defaults
set "GENERATOR=Visual Studio 15 2017"
set "TESTING=0"

:: process arguments
:loop
if NOT [%1]==[] (
  if "%1"=="/g" (
    set "GENERATOR=%~2"
    shift
  )
  if "%1"=="/t" (
    set "TESTING=1"
  )
  shift
  goto :loop
)


:: if generator has changed delete the build folder
if exist %BUILD_HOME% (
  for /F "tokens=*" %%f in ( 'findstr CMAKE_GENERATOR:INTERNAL %BUILD_HOME%\CmakeCache.txt' ) do (
    for /F "delims=:= tokens=3" %%m in ( 'echo %%f' ) do (
      set CACHE_GEN=%%m
      if not "!CACHE_GEN!" == "!GENERATOR!" ( rmdir /s /q %BUILD_HOME% & mkdir %BUILD_HOME% )
    )
  )
) else (
  mkdir %BUILD_HOME%^
  & if %ERRORLEVEL% NEQ 0 ( echo "ERROR: unable to make %BUILD_HOME% dir" & exit /B 1 )
)


:: perform the build
cd %BUILD_HOME%
if %TESTING% EQU 1 (
  cmake -G"%GENERATOR%" -DBUILD_TESTS=ON -DBOOST_ROOT=C:\local\boost_1_67_0 ..^
  && cmake --build . --config Debug^
  & echo. && ctest -C Debug --output-on-failure
) else (
  cmake -G"%GENERATOR%" -DBUILD_TESTS=OFF ..^
  && cmake --build . --config Release --target install
)


endlocal


:: determine platform from CmakeCache.txt file
for /F "tokens=*" %%f in ( 'findstr CMAKE_SHARED_LINKER_FLAGS:STRING %BUILD_HOME%\CmakeCache.txt' ) do (
  for /F "delims=: tokens=3" %%m in ( 'echo %%f' ) do (
    if "%%m" == "X86" ( set "PLATFORM=win32" ) else if "%%m" == "x64" ( set "PLATFORM=win64" )
  )
)
if not defined PLATFORM ( echo "ERROR: PLATFORM could not be determined" & exit /B 1 )


:: return to users current dir
cd %CUR_DIR%
