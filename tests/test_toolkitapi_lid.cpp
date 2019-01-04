/*
 *   test_toolkitAPI_lid.cpp
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
#include "test_toolkitapi_lid.hpp"

// Non-Fixuture Unit Tests
BOOST_AUTO_TEST_SUITE(test_toolkitapi_lid)

// Test Model Not Open
BOOST_AUTO_TEST_CASE(model_not_open) 
{
    int error, index;
    int int_val=0;
    double double_val;
    char chr_val = '0';
    double *result_array;
    std::string id = std::string("test");

    //Lid Control
//    error = swmm_getGagePrecip(0, &result_array);
//    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
//    error = swmm_setGagePrecip(0, input_val);
//    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    
    //Lid Group
    
    //Lid Unit
    error = swmm_getLidUCount(0, &int_val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
}

BOOST_AUTO_TEST_SUITE_END()

/* BOOST_AUTO_TEST_SUITE(test_toolkitapi_lid_fixture)

// Testing for Simulation Started Error
BOOST_FIXTURE_TEST_CASE(sim_started_check, FixtureBeforeStep) {
    int error;
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
    char chrVal = '0';
    double *result_array;

    //Gage
    error = swmm_getGagePrecip(100, &result_array);
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

    //Pollutant
    error = swmm_getSubcatchPollut(100, 0, &result_array);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
}

// Testing for invalid parameter key
BOOST_FIXTURE_TEST_CASE(key_bounds_check, FixtureOpenClose) {
    int error;
    int intVal;
    double val;
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
    BOOST_CHECK_SMALL(val - 0, 0.0001);
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
    BOOST_CHECK_SMALL(subc_stats.infil - 42088, 1.0);
    BOOST_CHECK_SMALL(subc_stats.runoff - 53781, 1.0);
    BOOST_CHECK_SMALL(subc_stats.maxFlow - 4.6561, 0.0001);
    BOOST_CHECK_SMALL(subc_stats.precip - 2.65, 0.0001);
    BOOST_CHECK_SMALL(subc_stats.evap - 0.0, 0.0001);

}
BOOST_AUTO_TEST_SUITE_END() */
