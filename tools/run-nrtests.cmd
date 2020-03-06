::
::  run_nrtest.cmd - Runs numerical regression test
::
::  Date Created: 10/16/2019
::  Date Updated:
::
::  Author: Michael E. Tryby
::          US EPA - ORD/CESER
::
::  Dependencies:
::    python -m pip install -r requirements.txt
::
::  Environment Variables:
::    PROJECT
::    BUILD_HOME - relative path
::    TEST_HOME  - relative path
::    PLATFORM
::    REF_BUILD_ID
::
::  Arguments:
::    1 - (SUT_VERSION)  - optional argument
::    2 - (SUT_BUILD_ID) - optional argument
::

::@echo off
setlocal EnableDelayedExpansion


:: Check that required environment variables are set
if not defined PROJECT ( echo "ERROR: PROJECT must be defined" & exit /B 1 )
if not defined BUILD_HOME ( echo "ERROR: BUILD_HOME must be defined" & exit /B 1 )
if not defined TEST_HOME ( echo "ERROR: TEST_HOME must be defined" & exit /B 1 )
if not defined PLATFORM ( echo "ERROR: PLATFORM must be defined" & exit /B 1 )
if not defined REF_BUILD_ID ( echo "ERROR: REF_BUILD_ID must be defined" & exit /B 1 )


:: determine project directory
set "CUR_DIR=%CD%"
set "SCRIPT_HOME=%~dp0"
cd %SCRIPT_HOME%
pushd ..
set PROJ_DIR=%CD%
popd


cd %PROJ_DIR%\%TEST_HOME%

:: Process optional arguments
if [%1]==[] (set "SUT_VERSION=unknown"
) else ( set "SUT_VERSION=%~1" )

if [%2]==[] ( set "SUT_BUILD_ID=local"
) else ( set "SUT_BUILD_ID=%~2" )


:: check if app config file exists
if not exist apps\%PROJECT%-%SUT_BUILD_ID%.json (
  mkdir apps
  call %SCRIPT_HOME%\app-config.cmd %PROJ_DIR%\%BUILD_HOME%\bin\Release^
    %PLATFORM% %SUT_BUILD_ID% %SUT_VERSION% > apps\%PROJECT%-%SUT_BUILD_ID%.json
)


:: recursively build test list
set TESTS=
for /F "tokens=*" %%T in ('dir /b /s /a:d tests') do (
  set FULL_PATH=%%T
  set TESTS=!TESTS! !FULL_PATH:*%TEST_HOME%\=!
)


:: determine location of python Scripts folder
for /F "tokens=*" %%G in ('where python.exe') do (
  set PYTHON_DIR=%%~dpG
  goto break_loop_1
)
:break_loop_1
set "NRTEST_SCRIPT_PATH=%PYTHON_DIR%Scripts"


:: build nrtest execute command
set NRTEST_EXECUTE_CMD=python.exe %NRTEST_SCRIPT_PATH%\nrtest execute
set TEST_APP_PATH=apps\%PROJECT%-%SUT_BUILD_ID%.json
set TEST_OUTPUT_PATH=benchmark\%PROJECT%-%SUT_BUILD_ID%

:: build nrtest compare command
set NRTEST_COMPARE_CMD=python.exe %NRTEST_SCRIPT_PATH%\nrtest compare
set REF_OUTPUT_PATH=benchmark\%PROJECT%-%REF_BUILD_ID%
set RTOL_VALUE=0.01
set ATOL_VALUE=1.E-6

:: change current directory to test suite
::cd %TEST_HOME%

:: if present clean test benchmark results
if exist %TEST_OUTPUT_PATH% (
  rmdir /s /q %TEST_OUTPUT_PATH%
)

:: perform nrtest execute
echo INFO: Creating SUT %SUT_BUILD_ID% artifacts
set NRTEST_COMMAND=%NRTEST_EXECUTE_CMD% %TEST_APP_PATH% %TESTS% -o %TEST_OUTPUT_PATH%
:: if there is an error exit the script with error value 1
%NRTEST_COMMAND% || exit /B 1

echo.

:: perform nrtest compare
echo INFO: Comparing SUT artifacts to REF %REF_BUILD_ID%
set NRTEST_COMMAND=%NRTEST_COMPARE_CMD% %TEST_OUTPUT_PATH% %REF_OUTPUT_PATH% --rtol %RTOL_VALUE% --atol %ATOL_VALUE% -o benchmark\receipt.json
%NRTEST_COMMAND%

:: Return user to their current dir
cd %CUR_DIR%
