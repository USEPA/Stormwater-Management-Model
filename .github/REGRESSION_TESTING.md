
# SWMM Regression Testing

## Prerequisits

Running SWMM's regression test suite `swmm-nrtestsuite` requires installation of the following software. 
* git 
* C compiler - MSVC, gcc, xcode
* cmake
* python 2.7 including setup tools 
* swig


## Step by Step Guide for Linux and MacOS

The following are step by step instructions to compare the current SWMM OWA build against the SWMM 5.1.12 MSVC 32 bit benchmark. 

1. Clone the swmm github repository. 
```
$ git clone --branch=develop https://github.com/OpenWaterAnalytics/Stormwater-Management-Model.git
```

2. Make repository root the current working directory
```
$ cd Stormwater-Management-Model
```

3. Build swmm using cmake. 
```
$ cd buildprod
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --build . --config Release
```

4. Install the required python packages. 
```
$ pip install -r tools/requirements.txt 
```

5. Configure and run the regression tests: where <build id> - is the build identifier (i.e. swmm version number).
```
$ cd ..
$ tools/before-test.sh nrtestsuite `pwd`/buildprod/bin <build id>
$ tools/run-nrtest.sh nrtestsuite <build id>
```

## Step by Step Guide for Windows 

Coming soon ... 


## Working with nrtest 

Using `nrtest` it is possible to compare any two versions of SWMM as long as 
they share the same binary file format. `nrtest` comes with a python scripts 
for running its `execute` and `compare` commands. 
```
$ python nrtest execute apps/<app.json> tests/<test.json> -o benchmark/
$ python nrtest compare test_benchmark/ ref_benchmark/ --rtol --atol
``` 

**To what values should rtol and atol be set?** 

What appears to be a simple question on the surface turns out to be more difficult upon deeper 
examination. The numpy testing assert allclose comparison criteria leaves a lot to be desired from 
a theoretical perspective, however, I have found it useful when evaluating differences between 
versions of SWMM.  

For those wishing to dig into the details I found the linked [blog post](https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/) to be a useful starting point. 


For both nrtest execute and nrtest compare non-zero exit codes are returned on failure. 

More information on nrtest is available in the nrtest [docs](https://nrtest.readthedocs.io/en/latest/).


## Extending the Testsuite

**Where did the test and benchmark files go?**

The test and benchmark files are kept in a seperate 
[repo](https://github.com/OpenWaterAnalytics/swmm-example-networks). The script 
before-test.sh has been provided to retreive and stage the needed files for testing. 

**How to add a new or different version of SWMM?** 

nrtest runs the command line execuatable version of SWMM for testing purposes. To add a new 
or different version of SWMM for benchmarking a json file that contains the absolute path 
to the executable location and some meta-data describing it needs to be created. See the nrtest
documentation for details. The executable and the json file need to be copied to their 
designated locations. 


**How to add a test?** 

To add a new test the [REPORTS] section of the input file should be configured 
to write all results out to the binary file. A json file also needs to be 
created that describes how to execute the test, what files are needed, what 
files get generated, what comparison routines to use, and meta-data describing 
the test. The format and data found in the json files is resonably well 
described in the nrtest documentation. The input file, any auxiliary file need 
to run the test, and the json file describing the test should be added to the 
swmm-tests folder found in the swmm-example-networks repo. Finally, the test 
needs to be added to run-nrtest.sh so it gets called and run. 


**How to add a new comparison routine?** 

New comparison criteria can easily be implemented in the nrtest_swmm package. The package
nrtest uses setup entrypoints as a simple plugin mechanism for finding comparison routines
at runtime. New comparison routines can be added as a function with the swmm_xxxxxxx_compare() 
name pattern. The function also needs to be declared as an entry point in the nrtest_swmm 
setup.py file. 

A basic comparison function is also provided for checking the contents of report files. It can
easily be adapted for testing other types of text based files.  


## Common Problems

When adding new apps and tests the json format can be a bit finicky and json parsing errors 
aren't being reported in a user friendly manner. 

The absolute path to the executable must be specified in the json file describing the application
for testing. 

SWMM requires the absolute path of data files referenced in a SWMM input file (e.g. rain gauge data). 

The packages swmm_output and nrtest_swmm were developed and run on Windows with 64 bit version 
of Python 2.7. They have not been tested on Mac OS or Linux systems and have not been run using 
Python 3. 

Unfortunately, nrtest_swmm is slow. For example comparing two 500 MB files takes on the order 
of 20 minutes. Therefore, tests when added should generate small output files. If your system 
is running a test and appears to be doing nothing it might be bogged down comparing large 
binary files. 
