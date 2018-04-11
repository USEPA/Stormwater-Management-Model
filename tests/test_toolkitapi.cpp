/*
 *   test_toolkitAPI.cpp
 *
 *   Created: 03/10/2018
 *   Author: Bryant E. McDonnell
 *           EmNet LLC
 *
 *   Unit testing for SWMM-ToolkitAPI using Boost Test.
 
 
 
* TABLE OF CONTENTS AND STRUCTURE

  ->  Numerical Diff Functions
  ->  Non-Fixuture Unit Tests
  ->  Defining the different testing fixtures
  ->  Unit tests using the fixtures
 */

// NOTE: Travis installs libboost test version 1.5.4
//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "toolkitAPI"
#include <boost/test/included/unit_test.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>

#include "swmm5.h"
#include "toolkitAPI.h"
#include "../src/error.h"

// NOTE: Test Input File
#define DATA_PATH_INP "swmm_api_test.inp"
#define DATA_PATH_RPT "swmm_api_test.rpt"
#define DATA_PATH_OUT "swmm_api_test.out"

using namespace std;

// Custom test to check functions


// Non-Fixuture Unit Tests
BOOST_AUTO_TEST_SUITE(test_toolkitapi)

// Test Model Not Open
BOOST_AUTO_TEST_CASE(model_not_open) {
    int error;
    double val;

    //Subcatchment
    error = swmm_getSubcatchParam(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setSubcatchParam(0, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    
    //Node
    error = swmm_getNodeParam(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setNodeParam(0, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    //Link
    error = swmm_getLinkParam(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setLinkParam(0, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
}

BOOST_AUTO_TEST_SUITE_END()


// Defining Fixtures

/* Fixture Open Close
 1. Opens Model
 *. testing interactions
 2. Closes Model 
*/
struct FixtureOpenClose{
    FixtureOpenClose() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
    }
    ~FixtureOpenClose() {
        swmm_close();
    }
};

/* Fixture Before Start
 1. Opens Model
 *. can choose to start simulation 
 2. Starts Simulation
 3. Runs Simlation
 4. Ends simulation
 5. Closes Model 
*/
struct FixtureBeforeStart{
    FixtureBeforeStart() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
    }
    ~FixtureBeforeStart() {
        swmm_start(0);
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

/* Fixture Before Step
 1. Opens Model
 2. Starts Simulation
 *. can choose iterate over simulation if simulation started, 
    must call swmm_end() 
 3. Closes Model 
*/
struct FixtureBeforeStep{
    FixtureBeforeStep() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
        swmm_start(0);
    }
    ~FixtureBeforeStep() {
        swmm_close();
    }
};

/* Fixture Before Close
 1. Opens Model
 2. Starts Simulation
 3. Runs Simlation
 4. Ends simulation
 * can choose to interact after simulation end
 5. Closes Model 
*/
struct FixtureBeforeClose{
    FixtureBeforeClose() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
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



BOOST_AUTO_TEST_SUITE(test_toolkitapi_fixture)

// Testing for Simulation Started Error
BOOST_FIXTURE_TEST_CASE(sim_started_check, FixtureBeforeStep) {
    int error;
    //Subcatchment
    error = swmm_setSubcatchParam(0, 0, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);

    //Node
    error = swmm_setNodeParam(0, 0, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);

    //Link
    error = swmm_setLinkParam(0, SM_OFFSET1, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
    error = swmm_setLinkParam(0, SM_OFFSET2, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
    error = swmm_setLinkParam(0, SM_INITFLOW, 1);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
    error = swmm_setLinkParam(0, SM_FLOWLIMIT, 1);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
    error = swmm_setLinkParam(0, SM_INLETLOSS, 1);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
    error = swmm_setLinkParam(0, SM_OUTLETLOSS, 1);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
    error = swmm_setLinkParam(0, SM_AVELOSS, 1);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
}

// Testing for invalid object index
BOOST_FIXTURE_TEST_CASE(object_bounds_check, FixtureOpenClose) {
    int error;
    double val;
    //Subcatchment
    error = swmm_getSubcatchParam(100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_setSubcatchParam(100, 0, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    //Node
    error = swmm_getNodeParam(100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_setNodeParam(100, 0, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    //Link
    error = swmm_getLinkParam(100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_setLinkParam(100, 0, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
}

// Testing for invalid parameter key
BOOST_FIXTURE_TEST_CASE(key_bounds_check, FixtureOpenClose) {
    int error;
    double val;
    //Subcatchment
    error = swmm_getSubcatchParam(0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setSubcatchParam(0, 100, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

    //Node
    error = swmm_getNodeParam(0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setNodeParam(0, 100, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

    //Link
    error = swmm_getLinkParam(0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLinkParam(0, 100, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
}

// Testing for subcatchment get/set
BOOST_FIXTURE_TEST_CASE(getset_subcatch, FixtureOpenClose) {

    int error, subc_ind;
    double val;
    
    subc_ind = 4;
    
    // Get/Set Subcatchment SM_WIDTH
    error = swmm_getSubcatchParam(subc_ind, SM_WIDTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 500, 0.0001);
    error = swmm_setSubcatchParam(subc_ind, SM_WIDTH, 600);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getSubcatchParam(subc_ind, SM_WIDTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 600, 0.0001);

    // Get/Set Subcatchment SM_AREA
    error = swmm_getSubcatchParam(subc_ind, SM_AREA, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 15, 0.0001);
    error = swmm_setSubcatchParam(subc_ind, SM_AREA, 20);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getSubcatchParam(subc_ind, SM_AREA, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 20, 0.0001);

    // Get/Set Subcatchment SM_FRACIMPERV
    error = swmm_getSubcatchParam(subc_ind, SM_FRACIMPERV, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - .50, 0.0001);
    error = swmm_setSubcatchParam(subc_ind, SM_FRACIMPERV, .70);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getSubcatchParam(subc_ind, SM_FRACIMPERV, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - .50, 0.0001);
    
    // Get/Set Subcatchment SM_SLOPE
    error = swmm_getSubcatchParam(subc_ind, SM_SLOPE, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.0001, 0.000001);
    error = swmm_setSubcatchParam(subc_ind, SM_SLOPE, 0.0005);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getSubcatchParam(subc_ind, SM_SLOPE, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.0005, 0.000001);

    // Get/Set Subcatchment SM_CURBLEN
    error = swmm_getSubcatchParam(subc_ind, SM_CURBLEN, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.0001);
    error = swmm_setSubcatchParam(subc_ind, SM_CURBLEN, 1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getSubcatchParam(subc_ind, SM_CURBLEN, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);
}

// Testing for node get/set
BOOST_FIXTURE_TEST_CASE(getset_node, FixtureOpenClose) {

    int error, node_ind;
    double val;
    
    node_ind = 3;
    
    // Get/Set Node SM_INVERTEL
    error = swmm_getNodeParam(node_ind, SM_INVERTEL, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 990, 0.0001);
    error = swmm_setNodeParam(node_ind, SM_INVERTEL, 991);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getNodeParam(node_ind, SM_INVERTEL, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 991, 0.0001);

    // Get/Set Node SM_FULLDEPTH
    error = swmm_getNodeParam(node_ind, SM_FULLDEPTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 3, 0.0001);
    error = swmm_setNodeParam(node_ind, SM_FULLDEPTH, 5);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getNodeParam(node_ind, SM_FULLDEPTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 5, 0.0001);

    // Get/Set Node SM_SURCHDEPTH
    error = swmm_getNodeParam(node_ind, SM_SURCHDEPTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.0001);
    error = swmm_setNodeParam(node_ind, SM_SURCHDEPTH, 20);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getNodeParam(node_ind, SM_SURCHDEPTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 20, 0.0001);
    
    // Get/Set Node SM_PONDAREA
    error = swmm_getNodeParam(node_ind, SM_PONDAREA, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.000001);
    error = swmm_setNodeParam(node_ind, SM_PONDAREA, 5000);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getNodeParam(node_ind, SM_PONDAREA, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 5000, 0.000001);

    // Get/Set Node SM_INITDEPTH
    error = swmm_getNodeParam(node_ind, SM_INITDEPTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.0001);
    error = swmm_setNodeParam(node_ind, SM_INITDEPTH, 1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getNodeParam(node_ind, SM_INITDEPTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);
}

// Testing for link get/set
BOOST_FIXTURE_TEST_CASE(getset_link, FixtureOpenClose) {

    int error, link_ind;
    double val;
    
    link_ind = 6;
    
    // Get/Set Link SM_OFFSET1
    error = swmm_getLinkParam(link_ind, SM_OFFSET1, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.0001);
    error = swmm_setLinkParam(link_ind, SM_OFFSET1, 1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLinkParam(link_ind, SM_OFFSET1, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);

    // Get/Set Link SM_OFFSET2
    error = swmm_getLinkParam(link_ind, SM_OFFSET2, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.0001);
    error = swmm_setLinkParam(link_ind, SM_OFFSET2, 1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLinkParam(link_ind, SM_OFFSET2, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);

    // Get/Set Link SM_INITFLOW
    error = swmm_getLinkParam(link_ind, SM_INITFLOW, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.0001);
    error = swmm_setLinkParam(link_ind, SM_INITFLOW, 1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLinkParam(link_ind, SM_INITFLOW, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);
    
    // Get/Set Link SM_FLOWLIMIT
    error = swmm_getLinkParam(link_ind, SM_FLOWLIMIT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.0001);
    error = swmm_setLinkParam(link_ind, SM_FLOWLIMIT, 1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLinkParam(link_ind, SM_FLOWLIMIT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);

    // Get/Set Link SM_INLETLOSS
    error = swmm_getLinkParam(link_ind, SM_INLETLOSS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.0001);
    error = swmm_setLinkParam(link_ind, SM_INLETLOSS, 1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLinkParam(link_ind, SM_INLETLOSS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);

    // Get/Set Link SM_OUTLETLOSS
    error = swmm_getLinkParam(link_ind, SM_OUTLETLOSS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.0001);
    error = swmm_setLinkParam(link_ind, SM_OUTLETLOSS, 1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLinkParam(link_ind, SM_OUTLETLOSS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);

    // Get/Set Link SM_AVELOSS
    error = swmm_getLinkParam(link_ind, SM_AVELOSS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.0001);
    error = swmm_setLinkParam(link_ind, SM_AVELOSS, 1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLinkParam(link_ind, SM_AVELOSS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);
}

BOOST_AUTO_TEST_SUITE_END()