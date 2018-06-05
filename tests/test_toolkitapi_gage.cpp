/*
 *   test_toolkitAPI_gage.cpp
 *
 *   Created: 03/10/2018
 *   Author: Bryant E. McDonnell
 *           EmNet LLC
 *
 *   Unit testing mechanics for the gage API using Boost Test.
 */

// NOTE: Travis installs libboost test version 1.5.4
//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "toolkitAPI_gage"
#include "test_toolkitapi.hpp"


BOOST_AUTO_TEST_SUITE(test_toolkitapi_gage_fixture)

// Testing Rain Gage Setter 
BOOST_FIXTURE_TEST_CASE(get_set_gage_rate, FixtureBeforeStep){
    int error, step_ind;
    int rg_ind, subc_ind;
    double rain;
    double* precip_array;
    double elapsedTime = 0.0;
    
    double start_rainfall_rate = 0;// in/hr
    double new_rainfall_rate = 4;  // in/hr
    
    std::string rgid = std::string("RG1");
    std::string subid = std::string("1");

    rg_ind = swmm_getObjectIndex(SM_GAGE, (char *)rgid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);
    subc_ind = swmm_getObjectIndex(SM_SUBCATCH, (char *)subid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

    step_ind = 0;
    do
    {
        if (step_ind == 0) // (Jan 1, 1998 12:00am)
        {
            error = swmm_setGagePrecip(rg_ind, 0);
            BOOST_REQUIRE(error == ERR_NONE);
        }
        if (step_ind == 360) // (Jan 1, 1998 6:00am)
        {
            // Gage
            error = swmm_getGagePrecip(rg_ind, &precip_array);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(precip_array[SM_RAINFALL] - start_rainfall_rate, 0.0001);
            BOOST_CHECK_SMALL(precip_array[SM_SNOWFALL] - 0.0, 0.0001);
            BOOST_CHECK_SMALL(precip_array[SM_TOTALPRECIP] - start_rainfall_rate, 0.0001);

            // Subcatchment
            error = swmm_getSubcatchResult(subc_ind, SM_SUBCRAIN, &rain);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(rain - start_rainfall_rate, 0.0001);

            // Setting Rainfall Rate
            error = swmm_setGagePrecip(rg_ind, new_rainfall_rate);
            BOOST_REQUIRE(error == ERR_NONE);
        }

        if (step_ind == 720) // (Jan 1, 1998 12:00pm)
        {
            // Gage
            error = swmm_getGagePrecip(rg_ind, &precip_array);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(precip_array[SM_RAINFALL] - new_rainfall_rate, 0.0001);
            BOOST_CHECK_SMALL(precip_array[SM_SNOWFALL] - 0.0, 0.0001);
            BOOST_CHECK_SMALL(precip_array[SM_TOTALPRECIP] - new_rainfall_rate, 0.0001);

            // Subcatchment
            error = swmm_getSubcatchResult(subc_ind, SM_SUBCRAIN, &rain);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(rain - new_rainfall_rate, 0.0001);

            // Setting Rainfall Rate
            error = swmm_setGagePrecip(rg_ind, start_rainfall_rate);
            BOOST_REQUIRE(error == ERR_NONE);
        }

        // Route Model Forward
        error = swmm_step(&elapsedTime);
        step_ind+=1;
    }while (elapsedTime != 0 && !error);
    BOOST_REQUIRE(error == ERR_NONE);

    // Final Checks from Subcatchment Stats Totalizers
    SM_SubcatchStats subcstats;
    
    error = swmm_getSubcatchStats(subc_ind, &subcstats);
    BOOST_CHECK_EQUAL(error, ERR_NONE);
    // 4 in/hr * 6hrs = 24inches
    // Time to call FEMA!
    BOOST_CHECK_SMALL(subcstats.precip - 24, 0.0001);
    swmm_freeSubcatchStats(&subcstats);
    freeArray((void**) &precip_array);

    swmm_end();
}


BOOST_AUTO_TEST_SUITE_END()