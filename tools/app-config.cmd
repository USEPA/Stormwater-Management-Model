::
::  app-config.cmd - Generates nrtest app configuration file for SUT executable
::
::  Date Created: 10/16/2019
::
::  Author: Michael E. Tryby
::          US EPA - ORD/CESER
::
::  Arguments:
::    1 - absolute path to test executable (valid path seperator for nrtest is "/")
::    2 - (platform)
::    3 - (build identifier for SUT)
::    4 - (commit hash string)

@echo off
setlocal

:: swmm target created by the cmake build script
set TEST_CMD=run%PROJECT%.exe

:: remove quotes from path and convert backward to forward slash
set ABS_BUILD_PATH=%~1
set ABS_BUILD_PATH=%ABS_BUILD_PATH:\=/%

IF [%2]==[] ( set "PLATFORM=unknown"
) ELSE ( set "PLATFORM=%~2" )

IF [%3]==[] ( set "BUILD_ID=unknown"
) ELSE ( set "BUILD_ID=%~3" )

IF [%4]==[] ( set "VERSION=unknown"
) ELSE ( set "VERSION=%~4" )


echo {
echo     "name" : "%PROJECT%",
echo     "version" : "%VERSION%",
echo     "description" : "%PLATFORM% %BUILD_ID%",
echo     "setup_script" : "",
echo     "exe" : "%ABS_BUILD_PATH%/%TEST_CMD%"
echo }
