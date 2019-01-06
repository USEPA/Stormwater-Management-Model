// NOTE: Travis installs libboost test version 1.5.4
//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "toolkitAPI_lid"
#include "test_toolkitapi_lid.hpp"

using namespace std;

// Custom test to check the minimum number of correct decimal digits between
// the test and the ref vectors.
boost::test_tools::predicate_result check_cdd(std::vector<double>& test,
    std::vector<double>& ref, long cdd_tol)
{
    double tmp, min_cdd = 10.0;

    // TODO: What if the vectors aren't the same length?

    std::vector<double>::iterator test_it;
    std::vector<double>::iterator ref_it;

    for (test_it = test.begin(); test_it < test.end(); ++test_it) {
        for (ref_it = ref.begin(); ref_it < ref.end(); ++ref_it) {

             if (*test_it != *ref_it) {
                // Compute log absolute error
                tmp = abs(*test_it - *ref_it);
                if (tmp < 1.0e-7)
                    tmp = 1.0e-7;

                else if (tmp > 2.0)
                    tmp = 1.0;

                tmp = - log10(tmp);
                if (tmp < 0.0)
                    tmp = 0.0;

                if (tmp < min_cdd)
                    min_cdd = tmp;
            }
        }
    }

    return floor(min_cdd) <= cdd_tol;
}

// Non-Fixuture Unit Tests
BOOST_AUTO_TEST_SUITE(test_lid_toolkitapi)

// Test Model Not Open
BOOST_AUTO_TEST_CASE(model_not_open) {
    
    int error;
    int intValue = 0;
    double dbValue = 0;
    char chrValue = '0';
    
    //Lid Control
    error = swmm_getLidCOverflow(0, &chrValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN); 
    error = swmm_setLidCOverflow(0, chrValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN); 
    error = swmm_getLidCParam(0, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN); 
    error = swmm_setLidCParam(0, 0, 0, dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN); 
    
    //Lid Unit
    error = swmm_getLidUCount(0, &intValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_getLidUParam(0, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setLidUParam(0, 0, 0, dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_getLidUOption(0, 0, 0, &intValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    error = swmm_setLidUOption(0, 0, 0, intValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);  
    error = swmm_getLidUFluxRates(0, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);  
    error = swmm_getLidUResult(0, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);  
        
    //Lid Group
    error = swmm_getLidGResult(0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);  
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_lid_toolkitapi_fixture)

// Testing for Simulation Started Error
BOOST_FIXTURE_TEST_CASE(sim_started_check, FixtureBeforeStep_LID) {
    int error;
    int intValue = 0;
    double dbValue = 0;
    char chrValue = '0';
    
    //Lid Control
    error = swmm_getLidCOverflow(0, &chrValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE); 
    error = swmm_setLidCOverflow(0, chrValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE); 
    error = swmm_getLidCParam(0, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE); 
    error = swmm_setLidCParam(0, 0, 0, dbValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE); 
    
    //Lid Unit
    error = swmm_getLidUCount(0, &intValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
    error = swmm_getLidUParam(0, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
    error = swmm_setLidUParam(0, 0, 0, dbValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
    error = swmm_getLidUOption(0, 0, 0, &intValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
    error = swmm_setLidUOption(0, 0, 0, intValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE);  
    error = swmm_getLidUFluxRates(0, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE);  
    error = swmm_getLidUResult(0, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE);  
        
    //Lid Group
    error = swmm_getLidGResult(0, 14, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_NONE); 
}


// Testing for invalid object index
BOOST_FIXTURE_TEST_CASE(object_bounds_check, FixtureOpenClose_LID) {
    int error;
    int intValue = 0;
    double dbValue = 0;
    char chrValue = '0';
    
    //Lid Control
    error = swmm_getLidCOverflow(1, &chrValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX); 
    error = swmm_setLidCOverflow(1, chrValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX); 
    error = swmm_getLidCParam(1, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX); 
    error = swmm_setLidCParam(1, 0, 0, dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX); 
    
    //Lid Unit
    error = swmm_getLidUCount(2, &intValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getLidUParam(2, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_setLidUParam(2, 0, 0, dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_getLidUOption(2, 0, 0, &intValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    error = swmm_setLidUOption(2, 0, 0, intValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);  
    error = swmm_getLidUFluxRates(2, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);  
    error = swmm_getLidUResult(2, 0, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);  
        
    //Lid Group
    error = swmm_getLidGResult(2, 14, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX); 
}



// Testing for invalid parameter key
BOOST_FIXTURE_TEST_CASE(key_bounds_check, FixtureOpenClose_LID) {
    int error;
    int intValue = 0;
    double dbValue = 0;
    char chrValue = '0';
    
    //Lid Control    
    error = swmm_getLidCParam(0, 0, 100, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS); 
    error = swmm_setLidCParam(0, 0, 100, dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS); 
    error = swmm_getLidCParam(0, 100, 0, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS); 
    error = swmm_setLidCParam(0, 100, 0, dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS); 
    
    //Lid Unit
    error = swmm_getLidUParam(0, 0, 100, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLidUParam(0, 0, 100, dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_getLidUOption(0, 0, 100, &intValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    error = swmm_setLidUOption(0, 0, 100, intValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);  
    error = swmm_getLidUFluxRates(0, 0, 100, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);  
    error = swmm_getLidUResult(0, 0, 100, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);  
        
    //Lid Group
    error = swmm_getLidGResult(0, 100, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS); 
}

// Testing for undefined lid usage
BOOST_FIXTURE_TEST_CASE(undefined_lid_check, FixtureOpenClose_LID) {
    int error;
    double dbValue = 0;

    //Lid Group
    error = swmm_getLidGResult(1, 14, &dbValue);
    BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);  
}

// Testing for Project Settings after Open
BOOST_FIXTURE_TEST_CASE(project_lid_info, FixtureOpenClose_LID){
    int error, index;
    string id = string("BC");

    //Project
    index = swmm_getObjectIndex(SM_LID, (char *)id.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(index, 0);
}

// Testing for Lid Control Bio Cell parameters get/set
BOOST_FIXTURE_TEST_CASE(getset_lidcontrol_BC, FixtureOpenClose_LID_BC) {
    int error, bioCellInd, subcatchInd;
    double dbVal = 0;
    char charVal = '0';

    string bioCell = string("BC");
    string subcatch = string("wBC");

    bioCellInd = swmm_getObjectIndex(SM_LID, (char *)bioCell.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

    subcatchInd = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

    // SURFACE LAYER CHECK
    error = swmm_getLidCParam(bioCellInd, SM_SURFACE, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 6, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_SURFACE, SM_THICKNESS, 100);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_SURFACE, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 100, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_SURFACE, SM_VOIDFRAC, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.25, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_SURFACE, SM_VOIDFRAC, 0.9);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_SURFACE, SM_VOIDFRAC, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.9, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_SURFACE, SM_ROUGHNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.1, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_SURFACE, SM_ROUGHNESS, 0.2);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_SURFACE, SM_ROUGHNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.2, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_SURFACE, SM_SURFSLOPE, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 1, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_SURFACE, SM_SURFSLOPE, 2);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_SURFACE, SM_SURFSLOPE, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 2, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_SURFACE, SM_ALPHA, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 1.05359, 0.0001);

    // Soil layer get/set
    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 12, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_SOIL, SM_THICKNESS, 100);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 100, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_POROSITY, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.5, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_SOIL, SM_POROSITY, 0.3);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_POROSITY, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.3, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_FIELDCAP, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.2, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_SOIL, SM_FIELDCAP, 0.3);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_FIELDCAP, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.3, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_WILTPOINT, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.1, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_SOIL, SM_WILTPOINT, 0.3);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_WILTPOINT, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.3, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_KSAT, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.5, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_SOIL, SM_KSAT, 0.1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_KSAT, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.1, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_KSLOPE, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 10, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_SOIL, SM_KSLOPE, 20);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_KSLOPE, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 20, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_SUCTION, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 3.5, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_SOIL, SM_SUCTION, 7);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_SOIL, SM_SUCTION, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 7, 0.0001);

    // Storage layer get/set check
    error = swmm_getLidCParam(bioCellInd, SM_STOR, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 12, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_STOR, SM_THICKNESS, 100);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_STOR, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 100, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_STOR, SM_VOIDFRAC, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.75, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_STOR, SM_VOIDFRAC, 0.15);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_STOR, SM_VOIDFRAC, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.15, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_STOR, SM_KSAT, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.5, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_STOR, SM_KSAT, 0.75);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_STOR, SM_KSAT, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.75, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_STOR, SM_CLOGFACTOR, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_STOR, SM_CLOGFACTOR, 0.75);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_STOR, SM_CLOGFACTOR, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.75, 0.0001);

    // Drain layer get/set
    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_COEFF, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_DRAIN, SM_COEFF, 1.0);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_COEFF, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 1.0, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_EXPON, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.5, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_DRAIN, SM_EXPON, 2);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_EXPON, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 2, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_OFFSET, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 6, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_DRAIN, SM_OFFSET, 20);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_OFFSET, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 20, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_DELAY, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 6, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_DRAIN, SM_DELAY, 7);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_DELAY, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 7, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_HOPEN, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_DRAIN, SM_HOPEN, 7);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_HOPEN, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 7, 0.0001);

    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_HCLOSE, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0, 0.0001);
    error = swmm_setLidCParam(bioCellInd, SM_DRAIN, SM_HCLOSE, 7);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(bioCellInd, SM_DRAIN, SM_HCLOSE, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 7, 0.0001);

    // Check for immediate overflow option
    error = swmm_getLidCOverflow(bioCellInd, &charVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_REQUIRE(charVal == FALSE);
    error = swmm_setLidCParam(bioCellInd, SM_SURFACE, SM_ROUGHNESS, 0);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCOverflow(bioCellInd, &charVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_REQUIRE(charVal == TRUE);
}

// Testing for Lid Control Green Roof parameters get/set
BOOST_FIXTURE_TEST_CASE(getset_lidcontrol_GR, FixtureOpenClose_LID_GR) {
    int error, greenRoofInd, subcatchInd;
    double dbVal = 0;
    char charVal = '0';

    string greenRoof = string("GR");
    string subcatch = string("wGR");
    
    greenRoofInd = swmm_getObjectIndex(SM_LID, (char *)greenRoof.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
 
    subcatchInd = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
    
    // SURFACE LAYER CHECK
    error = swmm_getLidCParam(greenRoofInd, SM_SURFACE, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 6, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_SURFACE, SM_THICKNESS, 100);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_SURFACE, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 100, 0.0001);
    
    error = swmm_getLidCParam(greenRoofInd, SM_SURFACE, SM_VOIDFRAC, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.25, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_SURFACE, SM_VOIDFRAC, 0.9);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_SURFACE, SM_VOIDFRAC, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.9, 0.0001);
    
    error = swmm_getLidCParam(greenRoofInd, SM_SURFACE, SM_ROUGHNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.1, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_SURFACE, SM_ROUGHNESS, 0.2);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_SURFACE, SM_ROUGHNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.2, 0.0001);

    error = swmm_getLidCParam(greenRoofInd, SM_SURFACE, SM_SURFSLOPE, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 1, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_SURFACE, SM_SURFSLOPE, 2);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_SURFACE, SM_SURFSLOPE, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 2, 0.0001);
    
    error = swmm_getLidCParam(greenRoofInd, SM_SURFACE, SM_ALPHA, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 1.05359, 0.0001);
    
    // Soil layer get/set
    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 12, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_SOIL, SM_THICKNESS, 100);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 100, 0.0001);

    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_POROSITY, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.5, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_SOIL, SM_POROSITY, 0.3);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_POROSITY, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.3, 0.0001);

    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_FIELDCAP, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.2, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_SOIL, SM_FIELDCAP, 0.3);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_FIELDCAP, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.3, 0.0001);

    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_WILTPOINT, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.1, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_SOIL, SM_WILTPOINT, 0.3);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_WILTPOINT, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.3, 0.0001);

    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_KSAT, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.5, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_SOIL, SM_KSAT, 0.1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_KSAT, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.1, 0.0001);

    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_KSLOPE, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 10, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_SOIL, SM_KSLOPE, 20);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_KSLOPE, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 20, 0.0001);

    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_SUCTION, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 3.5, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_SOIL, SM_SUCTION, 7);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_SOIL, SM_SUCTION, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 7, 0.0001);

    // Drainmat layer get/set
    error = swmm_getLidCParam(greenRoofInd, SM_DRAINMAT, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 3, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_DRAINMAT, SM_THICKNESS, 11);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_DRAINMAT, SM_THICKNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 11, 0.0001);

    error = swmm_getLidCParam(greenRoofInd, SM_DRAINMAT, SM_VOIDFRAC, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.5, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_DRAINMAT, SM_VOIDFRAC, 0.1);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_DRAINMAT, SM_VOIDFRAC, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.1, 0.0001);

    error = swmm_getLidCParam(greenRoofInd, SM_DRAINMAT, SM_ROUGHNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.1, 0.0001);
    error = swmm_setLidCParam(greenRoofInd, SM_DRAINMAT, SM_ROUGHNESS, 0.2);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCParam(greenRoofInd, SM_DRAINMAT, SM_ROUGHNESS, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 0.2, 0.0001);

    error = swmm_getLidCParam(greenRoofInd, SM_DRAINMAT, SM_ALPHA, &dbVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(dbVal - 1.05359 - 0, 0.0001);

    // Check for immediate overflow option
    error = swmm_getLidCOverflow(greenRoofInd, &charVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_REQUIRE(charVal == FALSE);
    error = swmm_setLidCParam(greenRoofInd, SM_SURFACE, SM_ROUGHNESS, 0);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getLidCOverflow(greenRoofInd, &charVal);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_REQUIRE(charVal == TRUE);
}


    /*
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
*/
    
/*

// Testing for After Start Errors
BOOST_FIXTURE_TEST_CASE(sim_after_start_check, FixtureBeforeStep){

}


// Testing Results Getters (During Simulation)
BOOST_FIXTURE_TEST_CASE(get_result_during_sim, FixtureBeforeStep){

}


// Testing Results Getters (Before End Simulation)
BOOST_FIXTURE_TEST_CASE(get_results_after_sim, FixtureBeforeEnd){

}
*/
BOOST_AUTO_TEST_SUITE_END()
