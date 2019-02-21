::
::  gen-config.cmd - Generated nrtest app configuration file for test executable
::
::  Date Created: 1/12/2018
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
::  Arguments:
::    1 - absolute path to test executable (valid path seperator for nrtest is "/")
::    2 - (version tag or commit hash string)
::    3 - (description build configuration)

@echo off
setlocal

:: swmm target created by the cmake build script
set TEST_CMD=run-swmm.exe

:: remove quotes from path and convert backward to forward slash
set ABS_BUILD_PATH=%~1
set ABS_BUILD_PATH=%ABS_BUILD_PATH:\=/%

IF [%2]==[] ( set "VERSION=unknown"
) ELSE ( set "VERSION=%~2" )

IF [%3]==[] ( set "DESCRIPTION=unknown"
) ELSE ( set "DESCRIPTION=%~3" )


echo {
echo     "name" : "swmm",
echo     "version" : "Commit ID %VERSION%",
echo     "description" : "Build configuration %DESCRIPTION%",
echo     "setup_script" : "",
echo     "exe" : "%ABS_BUILD_PATH%/%TEST_CMD%"
echo }
