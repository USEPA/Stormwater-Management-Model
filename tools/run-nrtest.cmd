::
::  run_nrtest.cmd - Runs numerical regression test
::
::  Date Created: 1/12/2018
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
::  Arguments:
::    1 - (SUT build identifier)
::    2 - (test suite path)
::

@echo off
setlocal


:: Check existence and apply default arguments
IF [%1]==[] ( set "SUT_BUILD_ID=local"
) ELSE ( set "SUT_BUILD_ID=%~1" )

IF [%2]==[] ( set "TEST_SUITE_PATH=nrtestsuite"
) ELSE ( set "TEST_SUITE_PATH=%~2" )


:: Determine location of python Scripts folder
FOR /F "tokens=*" %%G IN ('where python') DO (
  set PYTHON_DIR=%%~dpG
)
set "NRTEST_SCRIPT_PATH=%PYTHON_DIR%Scripts"


:: determine REF_BUILD_ID from benchmark manifest
7z e .\%TEST_SUITE_PATH%\benchmark.zip -o.\%TEST_SUITE_PATH% manifest.json -r -y > nul
FOR /F delims^=^"^ tokens^=4 %%i IN ( 'findstr /l win .\%TEST_SUITE_PATH%\manifest.json' ) DO ( set "TEMP=%%i" )
FOR /F "tokens=2" %%d IN ( 'echo %TEMP%' ) DO ( set "REF_BUILD_ID=%%d" )


set NRTEST_EXECUTE_CMD=python %NRTEST_SCRIPT_PATH%\nrtest execute
set TEST_APP_PATH=apps\swmm-%SUT_BUILD_ID%.json
set TESTS=tests\examples tests\extran tests\routing tests\user
set TEST_OUTPUT_PATH=benchmark\swmm-%SUT_BUILD_ID%

set NRTEST_COMPARE_CMD=python %NRTEST_SCRIPT_PATH%\nrtest compare
set REF_OUTPUT_PATH=benchmark\swmm-%REF_BUILD_ID%
set RTOL_VALUE=0.01
set ATOL_VALUE=0.00

:: change current directory to test suite
cd %TEST_SUITE_PATH%

:: if present clean test benchmark results
if exist %TEST_OUTPUT_PATH% (
  rmdir /s /q %TEST_OUTPUT_PATH%
)

echo INFO: Creating SUT %SUT_BUILD_ID% artifacts
set NRTEST_COMMAND=%NRTEST_EXECUTE_CMD% %TEST_APP_PATH% %TESTS% -o %TEST_OUTPUT_PATH%
:: if there is an error exit the script with error value 1
%NRTEST_COMMAND% || exit /B 1

echo INFO: Comparing SUT artifacts to REF %REF_BUILD_ID%
set NRTEST_COMMAND=%NRTEST_COMPARE_CMD% %TEST_OUTPUT_PATH% %REF_OUTPUT_PATH% --rtol %RTOL_VALUE% --atol %ATOL_VALUE% --output benchmark\receipt.json
%NRTEST_COMMAND%
