
SWMM-NRTestSuite
================

Installation
------------

Satisfying dependencies?
nrtest swmm was develop on Windows using Python 2.7 64 bit.  

Third party package dependencies:
header_detail_footer
nrtest 
numpy 

Project packages: 
nrtest_swmm
swmm-output


pip can be used to install local packages. 

	$ pip install -r tools/requirements.txt


Running Tests 
-------------

working directory: 
Change working directory to current location of 'swmm-nrtestsuite'

path to nrtest script:
On Windows scripts installed in a python egg are not on the python path. Therefore the 
absolute path to the nrtest script must be passed to python at the command prompt. 
 

python <install path\>nrtest execute apps\<app.json> tests\<test.json> -o output\
python <install path\>nrtest compare test_benchmark\ ref_benchmark\ --rtol --atol
 

To what values should rtol and atol be set? 

What appears to be a simple question on the surface turns out to be more difficult upon deeper 
examination. The numpy testing assert allclose comparison criteria leaves a lot to be desired from 
a theoretical perspective, however, I have found it useful when evaluating differences between 
versions of SWMM.  

For those wishing to dig into the details I found this blog post to be a useful starting point:

https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/


System return values

For both nrtest execute and nrtest compare non-zero exit codes are returned on failure. 


Extending the Testsuite
-----------------------

How to add a new or different version of SWMM? 

nrtest runs the command line execuatable version of SWMM for testing purposes. To add a new 
or different version of SWMM for benchmarking a json file that contains the absolute path 
to the executable location and some meta-data describing it needs to be created. See the nrtest
documentation for details. The executable and the json file need to be copied to their 
designated locations. 


How to add a test? 

To add a new test the [REPORTS] section of the input file should be configured to write 
all results out to the binary file. A json file also needs to be created that describes how 
to execute the test, what files are needed, what files get generated, what comparison 
routines to use, and meta-data describing the test. The format and data found in the json 
files is resonably well described in the nrtest documentation. The input file, any auxiliary
file need to run the test, and the json file describing the test should be added to the apps\
in the swmm-nrtestsuite folder.


How to add a new comparison routine? 

New comparison criteria can easily be implemented in the nrtest_swmm package. The package
nrtest uses setup entrypoints as a simple plugin mechanism for finding comparison routines
at runtime. New comparison routines can be added as a function with the swmm_xxxxxxx_compare() 
name pattern. The function also needs to be declared as an entry point in the nrtest_swmm 
setup.py file. 

A basic comparison function is also provided for checking the contents of report files. It can
easily be adapted for testing other types of text based files.  


Common Problems
---------------

When adding new apps and tests the json format can be a bit finicky and json parsing errors 
aren't being reported in a user friendly manner. 

Sometimes the outputapi.dll can not be found at run time. The dll can not be accessed when 
swmm_reader package is installed as a compressed python egg. The egg needs to be uncompressed. 

Another reason the dll may not be found is that the absolute path to the dll location is 
hard coded on line 569 the outputapi.py file in the swmm_reader package. On Windows systems 
the path needs to point to the installed location of the package for the dll to load properly. 

The absolute path to the executable must be specified in the json file describing the application
for testing. 

SWMM requires the absolute path of data files referenced in a SWMM input file (e.g. rain gauge data). 

The packages swmm_reader and nrtest_swmm were developed and run on Windows with 64 bit version 
of Python 2.7. They have not been tested on Mac OS or Linux systems and have not been run using 
Python 3. 

Unfortunately, nrtest_swmm is slow. For example comparing two 500 MB files takes on the order 
of 20 minutes. Therefore, tests when added should generate small output files. If your system 
is running a test and appears to be doing nothing it might be bogged down comparing large 
binary files. 
