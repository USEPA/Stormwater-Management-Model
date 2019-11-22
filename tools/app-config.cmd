::
::  app-config.cmd - Generates nrtest app configuration file for SUT executable
::
::  Date Created: 10/16/2019
::  Date Modified: 10/17/2019
::
::  Author: Michael E. Tryby
::          US EPA - ORD/CESER
::
::  Environment Variables:
::    PROJECT
::
::  Arguments:
::    1 - absolute path to test executable (valid path seperator for nrtest is "/")
::    2 - (platform)
::    3 - (build identifier for SUT)
::

@echo off
setlocal

if not defined PROJECT ( echo "ERROR: PROJECT must be defined" & exit /B 1 )


:: swmm target created by the cmake build script
set TEST_CMD=run%PROJECT%.exe

:: remove quotes from path and convert backward to forward slash
set ABS_BUILD_PATH=%~1
set ABS_BUILD_PATH=%ABS_BUILD_PATH:\=/%

if [%2]==[] ( set "PLATFORM=unknown"
) else ( set "PLATFORM=%~2" )

if [%3]==[] ( set "BUILD_ID=unknown"
) else ( set "BUILD_ID=%~3" )

:: determine version
for /F "tokens=1" %%v in ( 'git rev-parse --short HEAD' ) do ( set "VERSION=%%v" )
if not defined VERSION ( echo "ERROR: VERSION could not be determined" & exit /B 1 )


echo {
echo     "name" : "%PROJECT%",
echo     "version" : "%VERSION%",
echo     "description" : "%PLATFORM% %BUILD_ID%",
echo     "setup_script" : "",
echo     "exe" : "%ABS_BUILD_PATH%/%TEST_CMD%"
echo }
