::
::  before-test.cmd - Prepares AppVeyor CI worker to run swmm regression tests
::
::  Date Created: 4/5/2018
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
::  Arguments:
::    1 - (build identifier for software under test)
::    2 - (version identifier for software under test)
::    3 - (relative path regression test file staging location)
::
::  Note:
::    Tests and benchmark files are stored in the swmm-example-networks repo.
::    This script retreives them using a stable URL associated with a release on
::    GitHub and stages the files for nrtest to run. The script assumes that
::    before-test.cmd and gen-config.cmd are located together in the same folder.
::

@echo off
setlocal


IF [%1]==[] ( set "SUT_BUILD_ID=local"
) ELSE ( set "SUT_BUILD_ID=%~1" )

IF [%2]==[] (set SUT_VERSION=
) ELSE ( set "SUT_VERSION=%~2" )

IF [%3]==[] ( set "TEST_HOME=nrtestsuite"
) ELSE ( set "TEST_HOME=%~3" )


:: determine latest tag in swmm-example-networks repo
set LATEST_URL=https://github.com/OpenWaterAnalytics/swmm-example-networks/releases/latest
FOR /F delims^=^"^ tokens^=2 %g IN ('curl --silent %LATEST_URL%') DO ( set "LATEST_TAG=%~nxg" )

set TESTFILES_URL=https://github.com/OpenWaterAnalytics/swmm-example-networks/archive/%LATEST_TAG%.zip
set BENCHFILES_URL=https://github.com/OpenWaterAnalytics/swmm-example-networks/releases/download/%LATEST_TAG%/benchmark-%PLATFORM%.zip


echo INFO: Staging files for regression testing

:: create a clean directory for staging regression tests
if exist %TEST_HOME% (
  rmdir /s /q %TEST_HOME%
)
mkdir %TEST_HOME%
cd %TEST_HOME%


:: retrieve swmm-examples for regression testing
curl -fsSL -o examples.zip %TESTFILES_URL%

:: retrieve swmm benchmark results
curl -fsSL -o benchmark.zip %BENCHFILES_URL%

:: extract tests and benchmarks
7z x examples.zip *\swmm-tests\* > nul
7z x benchmark.zip -obenchmark\ > nul


:: set up symlink for tests directory
mklink /D .\tests .\swmm-example-networks-%EXAMPLES_VER%\swmm-tests > nul


:: Determine SUT executable path
set "SCRIPT_HOME=%~dp0"
:: TODO: This may fail when there is more than one cmake buildprod folder
for /d /r "%SCRIPT_HOME%..\" %%a in (*) do if /i "%%~nxa"=="bin" set "BUILD_HOME=%%a"
set "SUT_PATH=%BUILD_HOME%\Release"


:: generate json configuration file for software under test
mkdir apps
%SCRIPT_HOME%\gen-config.cmd %SUT_PATH% %SUT_VERSION% %PLATFORM% > apps\swmm-%SUT_BUILD_ID%.json
