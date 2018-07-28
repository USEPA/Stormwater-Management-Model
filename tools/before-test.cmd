::
::  before-test.cmd - Prepares AppVeyor CI worker to run swmm regression tests
::  
::  Date Created: 4/5/2018
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
::  Arguments: 
::    1 - relative path regression test file staging location 
::    2 - absolute path to location of software under test
::    3 - build identifier for software under test
::  
::  Note: 
::    Tests and benchmark files are stored in the swmm-example-networks repo.
::    This script retreives them using a stable URL associated with a release on 
::    GitHub and stages the files for nrtest to run. The script assumes that 
::    before-test.cmd and gen-config.cmd are located together in the same folder. 
::

@echo off
setlocal

set SCRIPT_HOME=%~dp0
set TEST_HOME=%~1

set EXAMPLES_VER=1.0.0
set BENCHMARK_VER=5112

set TESTFILES_URL=https://github.com/OpenWaterAnalytics/swmm-example-networks/archive/v%EXAMPLES_VER%.zip
set BENCHFILES_URL=https://github.com/OpenWaterAnalytics/swmm-example-networks/releases/download/v%EXAMPLES_VER%/swmm-benchmark-%BENCHMARK_VER%.zip


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
mklink /D .\tests .\swmm-example-networks-%EXAMPLES_VER%\swmm-tests


:: generate json configuration file for software under test
mkdir apps
%SCRIPT_HOME%\gen-config.cmd %~2 > apps\swmm-%~3.json
