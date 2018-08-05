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
  ->  Unit tests using the fixtures
      - Error Test Checks
      - Parameter Get / Set Tests
         1. Subcatchments
         2. Nodes
         3. Links
      - Result Get Tests
 */

// NOTE: Travis installs libboost test version 1.5.4
//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "toolkitAPI"
#include "test_toolkitapi.hpp"

// Non-Fixuture Unit Tests
BOOST_AUTO_TEST_SUITE(test_toolkitapi)

// Test Model Not Open
BOOST_AUTO_TEST_CASE(model_not_open) {
    int error, index;
    int intVal=0;
    double val;
    double input_val = 0;
    double *precip_array;
    char chrVal = '0';
    std::string id = std::string("test");

    //Project
    index = swmm_getObjectIndex(SM_NODE, (char *)id.c_str(), &error);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    //Gage
    error = swmm_getGagePrecip(0, &precip_array);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setGagePrecip(0, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    //Subcatchment
    error = swmm_getSubcatchParam(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setSubcatchParam(0, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_getSubcatchResult(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    //Node
    error = swmm_getNodeParam(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setNodeParam(0, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setNodeInflow(0, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_getNodeResult(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setOutfallStage(0, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    //Link
    error = swmm_getLinkParam(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setLinkParam(0, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_getLinkResult(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setLinkSetting(0, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    // Lid Unit
    error = swmm_getLidUCount(1, &intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_getLidUParam(1, 0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setLidUParam(1, 0, 0, 10000);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_getLidUOption(1, 0, 0, &intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setLidUOption(1, 0, 0, intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    
    // Lid Control
    error = swmm_setLidCOverflow(0, chrVal);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_getLidCOverflow(0, &chrVal);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setLidCParam(0, 0, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_getLidCParam(0, 0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    }

BOOST_AUTO_TEST_SUITE_END()

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

    //Lid Control
    error = swmm_setLidCParam(0, 0, 0, 1.0);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
}

// Testing for invalid object index
BOOST_FIXTURE_TEST_CASE(object_bounds_check, FixtureOpenClose) {
    int error;
    int intVal;
    double val;
    double input_val = 0;
    double *precip_array;
    char chrVal = '0';

    //Gage
    error = swmm_getGagePrecip(100, &precip_array);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

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
    error = swmm_setOutfallStage(100, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    //Link
    error = swmm_getLinkParam(100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_setLinkParam(100, 0, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    // Lid Unit
    error = swmm_getLidUCount(100, &intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getLidUParam(100, 0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getLidUParam(1, 100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_LIDUNIT_INDEX);
    error = swmm_getLidUParam(0, 100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);
    error = swmm_setLidUParam(100, 0, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_setLidUParam(1, 100, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_LIDUNIT_INDEX);
    error = swmm_setLidUParam(0, 100, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);
    error = swmm_getLidUOption(100, 0, 0, &intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getLidUOption(1, 100, 0, &intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_LIDUNIT_INDEX);
    error = swmm_getLidUOption(0, 100, 0, &intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);
    error = swmm_setLidUOption(100, 0, 0, intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_setLidUOption(1, 100, 0, intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_LIDUNIT_INDEX);
    error = swmm_setLidUOption(0, 100, 0, intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);
    error = swmm_getLidUFluxRates(100, 0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getLidUFluxRates(1, 100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_LIDUNIT_INDEX);
    error = swmm_getLidUFluxRates(0, 100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);
    error = swmm_getLidUResult(100, 0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getLidUResult(1, 100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_LIDUNIT_INDEX);
    error = swmm_getLidUResult(0, 100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);

    // Lid Control
    error = swmm_getLidCOverflow(100, &chrVal);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_setLidCOverflow(100, chrVal);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getLidCParam(100, 0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_setLidCParam(100, 0, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getLidGResult(100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
}

// Testing for invalid parameter key
BOOST_FIXTURE_TEST_CASE(key_bounds_check, FixtureOpenClose) {
    int error;
    int intVal;
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

    // Lid Unit
    error = swmm_getLidUParam(1, 0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLidUParam(1, 0, 100, val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_getLidUOption(1, 0, 100, &intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLidUOption(1, 0, 100, intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_getLidUFluxRates(1, 0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_getLidUResult(1, 0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

    // Lid Control
    error = swmm_getLidCParam(0, 0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_getLidCParam(0, 0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLidCParam(0, 0, 100, val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLidCParam(0, 100, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_getLidGResult(1, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
}

// Testing for Project Settings after Open
BOOST_FIXTURE_TEST_CASE(project_info, FixtureOpenClose){
    
    int error, index;
    std::string id = std::string("14");

    //Project
    index = swmm_getObjectIndex(SM_NODE, (char *)id.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(index, 3);   
}

// Testing for subcatchment get/set
BOOST_FIXTURE_TEST_CASE(getset_subcatch, FixtureOpenClose) {

    int error, subc_ind;
    double val;
    std::string id = std::string("5");

    subc_ind = swmm_getObjectIndex(SM_SUBCATCH, (char *)id.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
    
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

    std::string id = std::string("14");

    node_ind = swmm_getObjectIndex(SM_NODE, (char *)id.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

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
    std::string id = std::string("10");

    link_ind = swmm_getObjectIndex(SM_LINK, (char *)id.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

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

// Testing for Lid Control get/set
BOOST_FIXTURE_TEST_CASE(getset_lidcontrol, FixtureOpenClose) {

    int error, lidc_ind, lidc_greenRoof_ind;
    double val = 0;
    char charVal = '0';

    std::string id = std::string("LID");
    std::string greenRoof = std::string("green_LID");

    lidc_ind = swmm_getObjectIndex(SM_LID, (char *)id.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

    lidc_greenRoof_ind = swmm_getObjectIndex(SM_LID, (char *)greenRoof.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

    // Get/Set Link SM_OFFSET1
    error = swmm_getLidCOverflow(lidc_ind, &charVal);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_setLidCOverflow(lidc_ind, charVal);
    BOOST_REQUIRE(error == ERR_NONE);

    // SURFACE LAYER CHECK
    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_THICKNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 10, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SURFACE, SM_THICKNESS, 100);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_THICKNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 100, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_VOIDFRAC, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.5, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SURFACE, SM_VOIDFRAC, 0.9);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_VOIDFRAC, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.9, 0.0001);
    
    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_ROUGHNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.013, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SURFACE, SM_ROUGHNESS, 0.2);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_ROUGHNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.2, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_SURFSLOPE, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SURFACE, SM_SURFSLOPE, 2);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_SURFSLOPE, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 2, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_SIDESLOPE, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 5, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SURFACE, SM_SIDESLOPE, 1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_SIDESLOPE, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);
    
    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_ALPHA, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1.05359, 0.0001);
//    alpha is a calculated variable using surface slope and surface roughness
//    error = swmm_setLidCParam(lidc_ind, SM_SURFACE, SM_ALPHA, 2);
//    BOOST_REQUIRE(error == ERR_NONE);
//    error = swmm_getLidCParam(lidc_ind, SM_SURFACE, SM_ALPHA, &val);
//    BOOST_REQUIRE(error == ERR_NONE);
//    BOOST_CHECK_SMALL(val, 0.0001);

    // Pavement layer get/set  
    error = swmm_getLidCParam(lidc_ind, SM_PAVE, SM_THICKNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 20, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_PAVE, SM_THICKNESS, 100);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_PAVE, SM_THICKNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 100, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_PAVE, SM_VOIDFRAC, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.15, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_PAVE, SM_VOIDFRAC, 0.75);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_PAVE, SM_VOIDFRAC, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.75, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_PAVE, SM_IMPERVFRAC, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.5, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_PAVE, SM_IMPERVFRAC, 0.75);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_PAVE, SM_IMPERVFRAC, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.75, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_PAVE, SM_KSAT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 100, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_PAVE, SM_KSAT, 75);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_PAVE, SM_KSAT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 75, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_PAVE, SM_CLOGFACTOR, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(val - 8, 0);
    error = swmm_setLidCParam(lidc_ind, SM_PAVE, SM_CLOGFACTOR, 0.75);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_PAVE, SM_CLOGFACTOR, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.75, 0.0001);
    
    // Storage layer get/set check
    error = swmm_getLidCParam(lidc_ind, SM_STOR, SM_THICKNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 40, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_STOR, SM_THICKNESS, 100);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_STOR, SM_THICKNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 100, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_STOR, SM_VOIDFRAC, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.75, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_STOR, SM_VOIDFRAC, 0.15);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_STOR, SM_VOIDFRAC, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.15, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_STOR, SM_KSAT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.5, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_STOR, SM_KSAT, 0.75);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_STOR, SM_KSAT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.75, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_STOR, SM_CLOGFACTOR, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.2, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_STOR, SM_CLOGFACTOR, 0.75);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_STOR, SM_CLOGFACTOR, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.75, 0.0001);

    // Soil layer get/set 
    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_THICKNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 30, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SOIL, SM_THICKNESS, 100);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_THICKNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 100, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_POROSITY, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.5, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SOIL, SM_POROSITY, 0.3);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_POROSITY, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.3, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_FIELDCAP, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.2, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SOIL, SM_FIELDCAP, 0.3);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_FIELDCAP, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.3, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_WILTPOINT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.1, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SOIL, SM_WILTPOINT, 0.3);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_WILTPOINT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.3, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_KSAT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 5, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SOIL, SM_KSAT, 10);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_KSAT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 10, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_KSLOPE, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 10, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SOIL, SM_KSLOPE, 20);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_KSLOPE, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 20, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_SUCTION, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 3.5, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_SOIL, SM_SUCTION, 7);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_SOIL, SM_SUCTION, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 7, 0.0001);
    
    // Drainmat layer get/set 
    error = swmm_getLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_THICKNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 3, 0.0001);
    error = swmm_setLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_THICKNESS, 11);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_THICKNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 11, 0.0001);

    error = swmm_getLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_VOIDFRAC, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.5, 0.0001);
    error = swmm_setLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_VOIDFRAC, 0.1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_VOIDFRAC, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.1, 0.0001);

    error = swmm_getLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_ROUGHNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.1, 0.0001);
    error = swmm_setLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_ROUGHNESS, 0.2);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_ROUGHNESS, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.2, 0.0001);

    error = swmm_getLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_ALPHA, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.745, 0.0001);
    // alpha is a calculated variable using surface slope and drainmat roughness
    //error = swmm_setLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_ALPHA, 0.2);
    //BOOST_REQUIRE(error == ERR_NONE);
    //error = swmm_getLidCParam(lidc_greenRoof_ind, SM_DRAINMAT, SM_ALPHA, &val);
    //BOOST_REQUIRE(error == ERR_NONE);
    //BOOST_CHECK_SMALL(val - 0.2, 0.0001);

    // Drain layer get/set
    error = swmm_getLidCParam(lidc_ind, SM_DRAIN, SM_COEFF, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.5, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_DRAIN, SM_COEFF, 1.0);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_DRAIN, SM_COEFF, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1.0, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_DRAIN, SM_EXPON, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0.5, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_DRAIN, SM_EXPON, 2);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_DRAIN, SM_EXPON, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 2, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_DRAIN, SM_OFFSET, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 6, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_DRAIN, SM_OFFSET, 20);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_DRAIN, SM_OFFSET, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 20, 0.0001);

    error = swmm_getLidCParam(lidc_ind, SM_DRAIN, SM_DELAY, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 6, 0.0001);
    error = swmm_setLidCParam(lidc_ind, SM_DRAIN, SM_DELAY, 7);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(lidc_ind, SM_DRAIN, SM_DELAY, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 7, 0.0001);
}

// Testing for Lid Unit get/set
BOOST_FIXTURE_TEST_CASE(getset_lidunit, FixtureOpenClose) {

    int error, subc_ind;
    int intVal = 0;
    double val = 0;
    char charVal = '0';

    std::string subid = std::string("2");

    subc_ind = swmm_getObjectIndex(SM_SUBCATCH, (char *)subid.c_str(), &error);

    error = swmm_getLidUCount(subc_ind, &intVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(intVal, 2);

    error = swmm_getLidUParam(subc_ind, 0, SM_UNITAREA, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 10000, 0.0001);
    error = swmm_setLidUParam(subc_ind, 0, SM_UNITAREA, 50);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidUParam(subc_ind, 0, SM_UNITAREA, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 50, 0.0001);

    error = swmm_getLidUParam(subc_ind, 1, SM_FWIDTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 10, 0.0001);
    error = swmm_setLidUParam(subc_ind, 1, SM_FWIDTH, 5);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidUParam(subc_ind, 1, SM_FWIDTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 5, 0.0001);

    error = swmm_getLidUParam(subc_ind, 1, SM_BWIDTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 1, 0.0001);
    error = swmm_setLidUParam(subc_ind, 1, SM_BWIDTH, 5);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidUParam(subc_ind, 1, SM_BWIDTH, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 5, 0.0001);

    error = swmm_getLidUParam(subc_ind, 1, SM_INITSAT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 0, 0.0001);
    error = swmm_setLidUParam(subc_ind, 1, SM_INITSAT, 5);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidUParam(subc_ind, 1, SM_INITSAT, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 5, 0.0001);

    error = swmm_getLidUParam(subc_ind, 1, SM_FROMIMPERV, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 10, 0.0001);
    error = swmm_setLidUParam(subc_ind, 1, SM_FROMIMPERV, 75);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidUParam(subc_ind, 1, SM_FROMIMPERV, &val);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(val - 75, 0.0001);

    error = swmm_getLidUOption(subc_ind, 0, SM_INDEX, &intVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(intVal, 0);
    error = swmm_setLidUOption(subc_ind, 0, SM_INDEX, 1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidUOption(subc_ind, 0, SM_INDEX, &intVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(intVal, 1);

    error = swmm_getLidUOption(subc_ind, 0, SM_NUMBER, &intVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(intVal, 4);
    error = swmm_setLidUOption(subc_ind, 0, SM_NUMBER, 11);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidUOption(subc_ind, 0, SM_NUMBER, &intVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(intVal, 11);

    error = swmm_getLidUOption(subc_ind, 0, SM_TOPERV, &intVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(intVal, 1);
    error = swmm_setLidUOption(subc_ind, 0, SM_TOPERV, 0);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidUOption(subc_ind, 0, SM_TOPERV, &intVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(intVal, 0);

    error = swmm_getLidUOption(subc_ind, 0, SM_DRAINSUB, &intVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(intVal, -1);
    error = swmm_setLidUOption(subc_ind, 0, SM_DRAINSUB, 0);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidUOption(subc_ind, 0, SM_DRAINSUB, &intVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(intVal, 0);

    error = swmm_getLidUOption(subc_ind, 0, SM_DRAINNODE, &intVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(intVal, 1);
    error = swmm_setLidUOption(subc_ind, 0, SM_DRAINNODE, 0);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidUOption(subc_ind, 0, SM_DRAINNODE, &intVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(intVal, 0);
}

// Testing for After Start Errors
BOOST_FIXTURE_TEST_CASE(sim_after_start_check, FixtureBeforeStep) {
    int error;
    int intVal;
    double val, input_val = 0;

    // Subcatchment
    error = swmm_getSubcatchResult(100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getSubcatchResult(0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

    // Node
    error = swmm_getNodeResult(100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getNodeResult(0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setNodeInflow(100, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    //Link
    error = swmm_getLinkResult(100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getLinkResult(0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLinkSetting(100, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

   // Lid Unit
    error = swmm_getLidUParam(1, 0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLidUParam(1, 0, 100, val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_getLidUOption(1, 0, 100, &intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLidUOption(1, 0, 100, intVal);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_getLidUFluxRates(1, 0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_getLidUResult(1, 0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

    // Lid Control
    error = swmm_getLidCParam(0, 0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_getLidCParam(0, 0, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLidCParam(0, 0, 100, val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLidCParam(0, 100, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_getLidGResult(1, 100, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
}

// Testing Results Getters (During Simulation)
BOOST_FIXTURE_TEST_CASE(get_result_during_sim, FixtureBeforeStep){
    int error, step_ind;
    int subc_ind, sub_lid, nde_ind, lnk_ind;
    double val;
    double elapsedTime = 0.0;

    std::string subid = std::string("1");
    std::string subid_lid = std::string("2");
    std::string ndeid = std::string("19");
    std::string lnkid = std::string("14");

    subc_ind = swmm_getObjectIndex(SM_SUBCATCH, (char *)subid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
    nde_ind = swmm_getObjectIndex(SM_NODE, (char *)ndeid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
    lnk_ind = swmm_getObjectIndex(SM_LINK, (char *)lnkid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
    sub_lid = swmm_getObjectIndex(SM_SUBCATCH, (char *)subid_lid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

    step_ind = 0;
    do
    {
        error = swmm_step(&elapsedTime);
        
        if (step_ind == 200) // (Jan 1, 1998 3:20am)
        {
            // Subcatchment
            error = swmm_getSubcatchResult(subc_ind, SM_SUBCRAIN, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.8, 0.0001);
            error = swmm_getSubcatchResult(subc_ind, SM_SUBCEVAP, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0, 0.0001);
            error = swmm_getSubcatchResult(subc_ind, SM_SUBCINFIL, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.125, 0.0001);
            error = swmm_getSubcatchResult(subc_ind, SM_SUBCRUNON, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0, 0.0001);
            error = swmm_getSubcatchResult(subc_ind, SM_SUBCRUNOFF, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 4.3365, 0.0001);
            error = swmm_getSubcatchResult(subc_ind, SM_SUBCSNOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0, 0.0001);

            // Node
            error = swmm_getNodeResult(nde_ind, SM_TOTALINFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.4793, 0.0001);
            error = swmm_getNodeResult(nde_ind, SM_TOTALOUTFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.4793, 0.0001);
            error = swmm_getNodeResult(nde_ind, SM_LOSSES, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0, 0.0001);
            error = swmm_getNodeResult(nde_ind, SM_NODEVOL, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0, 0.0001);
            error = swmm_getNodeResult(nde_ind, SM_NODEFLOOD, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0, 0.0001);
            error = swmm_getNodeResult(nde_ind, SM_NODEDEPTH, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.1730, 0.0001);
            error = swmm_getNodeResult(nde_ind, SM_NODEHEAD, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 1010.1730, 0.0001);
            error = swmm_getNodeResult(nde_ind, SM_LATINFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.4793, 0.0001);

            // Link
            error = swmm_getLinkResult(lnk_ind, SM_LINKFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 1.1245, 0.0001);
            error = swmm_getLinkResult(lnk_ind, SM_LINKDEPTH, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.3023, 0.0001);
            error = swmm_getLinkResult(lnk_ind, SM_LINKVOL, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 80.15, 0.01);
            error = swmm_getLinkResult(lnk_ind, SM_USSURFAREA, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0, 0.0001);
            error = swmm_getLinkResult(lnk_ind, SM_DSSURFAREA, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0, 0.0001);
            error = swmm_getLinkResult(lnk_ind, SM_SETTING, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 1.0, 0.0001);
            error = swmm_getLinkResult(lnk_ind, SM_TARGETSETTING, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 1.0, 0.0001);
            error = swmm_getLinkResult(lnk_ind, SM_FROUDE, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0, 0.0001);
        
        }
        
        if (step_ind == 600) // (Jan 1, 1998 10:00am)
        {
            // Lid Group
            error = swmm_getLidGResult(sub_lid, SM_PERVAREA, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 50000, 0.0001);
            error = swmm_getLidGResult(sub_lid, SM_FLOWTOPERV, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.294977, 0.0001);
            error = swmm_getLidGResult(sub_lid, SM_OLDDRAINFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0, 0.0001);
            error = swmm_getLidGResult(sub_lid, SM_NEWDRAINFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0, 0.0001);

            // Lid Unit
            error = swmm_getLidUFluxRates(sub_lid, 0, SM_SURFACE, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0, 0.0001);
            error = swmm_getLidUFluxRates(sub_lid, 0, SM_SOIL, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - (-0.000003), 0.000001);
            error = swmm_getLidUFluxRates(sub_lid, 0, SM_STORAGE, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0, 0.0001);
            error = swmm_getLidUFluxRates(sub_lid, 0, SM_PAVE, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0, 0.0001);

            error = swmm_getLidUResult(sub_lid, 0, SM_INFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.5766, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_EVAP, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_INFIL, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0012, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_SURFFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_DRAINFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0347, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_INITVOL, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.2500, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_FINALVOL, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.7973, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_SURFDEPTH, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_PAVEDEPTH, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_SOILMOIST, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.2247, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_STORDEPTH, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.5343, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_DRYTIME, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 15300, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_OLDDRAINFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0, 0.0001);
            error = swmm_getLidUResult(sub_lid, 0, SM_NEWDRAINFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0, 0.0001);
        }

        step_ind+=1;
    }while (elapsedTime != 0 && !error);
    BOOST_REQUIRE(error == ERR_NONE);
    swmm_end();
}

// Testing Results Getters (Before End Simulation)
BOOST_FIXTURE_TEST_CASE(get_results_after_sim, FixtureBeforeEnd){
    int error;
    int rg_ind, subc_ind, nde_ind, lnk_ind;
    
    
    std::string rgid = std::string("RG1");
    std::string subid = std::string("1");
    std::string ndeid = std::string("19");
    std::string lnkid = std::string("14");

    rg_ind = swmm_getObjectIndex(SM_GAGE, (char *)rgid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
    subc_ind = swmm_getObjectIndex(SM_SUBCATCH, (char *)subid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
    nde_ind = swmm_getObjectIndex(SM_NODE, (char *)ndeid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
    lnk_ind = swmm_getObjectIndex(SM_LINK, (char *)lnkid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

    // Subcatchment
    SM_SubcatchStats subc_stats;
    error = swmm_getSubcatchStats(subc_ind, &subc_stats);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
    BOOST_CHECK_SMALL(subc_stats.runon - 0.0, 0.0001);
    BOOST_CHECK_SMALL(subc_stats.infil - 1.1594, 0.0001);
    BOOST_CHECK_SMALL(subc_stats.runoff - 1.4815, 0.0001);
    BOOST_CHECK_SMALL(subc_stats.maxFlow - 4.6561, 0.0001);
    BOOST_CHECK_SMALL(subc_stats.precip - 2.65, 0.0001);
    BOOST_CHECK_SMALL(subc_stats.evap - 0.0, 0.0001);
    swmm_freeSubcatchStats(&subc_stats);

}
BOOST_AUTO_TEST_SUITE_END()