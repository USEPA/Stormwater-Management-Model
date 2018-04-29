/*
 *   test_swmm.cpp
 *
 *   Created: 03/10/2018
 *   Author: Bryant E. McDonnell
 *           EmNet LLC
 *
 *   Unit testing for SWMM using Boost Test.
 */

// NOTE: Travis installs libboost test version 1.5.4
//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "swmm5"
#include <boost/test/included/unit_test.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>

#include "swmm5.h"


// NOTE: Test Input File
#define DATA_PATH_INP "./swmm_api_test.inp"
#define DATA_PATH_RPT "./swmm_api_test.rpt"
#define DATA_PATH_OUT "./swmm_api_test.out"

#define DATA_PATH_INP2 "./swmm_api_test2.inp"
#define DATA_PATH_RPT2 "./swmm_api_test2.rpt"
#define DATA_PATH_OUT2 "./swmm_api_test2.out"

using namespace std;

// Custom test to check the minimum number of correct decimal digits between 
// the test and the ref vectors.  
bool tolCheck(float a, float b, float tol)
{
    return abs(a - b) <= tol;
}

BOOST_AUTO_TEST_SUITE (test_swmm_auto)

BOOST_AUTO_TEST_CASE(RunTest) {
    int error = swmm_run( (char *)DATA_PATH_INP, (char *)DATA_PATH_RPT, (char *)DATA_PATH_OUT);
    BOOST_CHECK_EQUAL(0, error);
}

BOOST_AUTO_TEST_CASE(OpenTest) {
    int error = swmm_open((char *)DATA_PATH_INP, (char *)DATA_PATH_RPT, (char *)DATA_PATH_OUT);
    BOOST_CHECK_EQUAL(0, error);
    swmm_close();
}

// Try to open valid file but not an input file - Should Fail at 200?
BOOST_AUTO_TEST_CASE(FailOpen191) {
    int error = swmm_open((char *)DATA_PATH_RPT, (char *)DATA_PATH_OUT, (char *)"");
    BOOST_CHECK_EQUAL(191, error);
    swmm_close();
}

BOOST_AUTO_TEST_CASE(FailOpen301) {
    int error = swmm_open((char *)DATA_PATH_INP, (char *)DATA_PATH_INP, (char *)DATA_PATH_INP);
    BOOST_CHECK_EQUAL(301, error);
    swmm_close();
}

BOOST_AUTO_TEST_CASE(FailOpen303) {
    int error = swmm_open((char *)"", (char *)DATA_PATH_INP, (char *)DATA_PATH_OUT);
    BOOST_CHECK_EQUAL(303, error);
    swmm_close();
}

BOOST_AUTO_TEST_CASE(FailOpen305) {
    int error = swmm_open((char *)DATA_PATH_INP, (char *)"", (char *)DATA_PATH_OUT);
    BOOST_CHECK_EQUAL(305, error);
    swmm_close();
}

BOOST_AUTO_TEST_CASE(FailOpen307) {
    FILE * fp;
    fp = fopen(DATA_PATH_OUT, "r");
    int error = swmm_open((char *)DATA_PATH_INP, (char *)DATA_PATH_RPT, (char *)DATA_PATH_OUT);
    BOOST_CHECK_EQUAL(0, error);// Should be 307
    swmm_close();
    fclose(fp);
}

BOOST_AUTO_TEST_SUITE_END()

// Adding Fixtures
struct FixtureStep{
    FixtureStep() {
        swmm_open((char *)DATA_PATH_INP, (char *)DATA_PATH_RPT, (char *)DATA_PATH_OUT);
    }
    ~FixtureStep() {
        swmm_end();
        swmm_close();
    }
};

struct FixtureBeforeClose{
    FixtureBeforeClose() {
        swmm_open((char *)DATA_PATH_INP, (char *)DATA_PATH_RPT, (char *)DATA_PATH_OUT);
        swmm_start(0);

        int error;
        double elapsedTime = 0.0;
        do
        {
            error = swmm_step(&elapsedTime);
        }while (elapsedTime != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
        swmm_end();
    }
    ~FixtureBeforeClose() {
        swmm_close();
    }
};

struct FixtureBeforeStart{
    FixtureBeforeStart() {
        swmm_open((char *)DATA_PATH_INP, (char *)DATA_PATH_RPT, (char *)DATA_PATH_OUT);
    }
    ~FixtureBeforeStart() {
        int error;
        double elapsedTime = 0.0;
        do
        {
            error = swmm_step(&elapsedTime);
        }while (elapsedTime != 0 && !error);
        if (!error) swmm_end();
        if (!error) swmm_report();

        swmm_close();
    }
};

BOOST_AUTO_TEST_SUITE(test_swmm_fixture)

BOOST_FIXTURE_TEST_CASE(StepTest, FixtureStep) {
    swmm_start(0);
    int error;
    double elapsedTime = 0.0;
    do
    {
        error = swmm_step(&elapsedTime);
    }while (elapsedTime != 0 && !error);
    BOOST_CHECK_EQUAL(0, error);
}

BOOST_FIXTURE_TEST_CASE(ReportTest, FixtureBeforeClose) {
    int error = swmm_report();
    BOOST_REQUIRE(error == 0);
}

BOOST_FIXTURE_TEST_CASE(TStoRpt, FixtureBeforeStart) {
    int error = swmm_start(1);
    BOOST_REQUIRE(error == 0);
}

// swmm_step called before swmm_start
BOOST_FIXTURE_TEST_CASE(FailStart403, FixtureBeforeStart) {
    double elapsedTime = 0.0;
    int error = swmm_step(&elapsedTime);
    BOOST_CHECK_EQUAL(403, error);
    BOOST_REQUIRE(error == 403);
}


BOOST_AUTO_TEST_SUITE_END()