/*
 *   test_toolkitAPI_pollut.cpp
 *
 *   Created: 07/20/2018
 *   Author: Katherine M. Ratliff
 *
 *   Unit testing mechanics for the pollutant API using Boost Test.
 */

// NOTE: Travis installs libboost test version 1.5.4
//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "toolkitAPI_pollut"
#include "test_toolkitapi.hpp"


BOOST_AUTO_TEST_SUITE(test_toolkitapi_pollut)

// Testing Rain Gage Setter 
BOOST_FIXTURE_TEST_CASE(get_pollut_values, FixtureBeforeStep){
    int error, step_ind;
    int subc_ind;
    double* pollut_array;
    double elapsedTime = 0.0;
    // Pollutant IDs
    int TSS = 0;
    int Lead = 1;
    
    std::string subid = std::string("1");

    subc_ind = swmm_getObjectIndex(SM_SUBCATCH, (char *)subid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

    step_ind = 0;

    do
    {
        if (step_ind == 360) // (Jan 1, 1998 6:00am)
        {
            // buildup
            error = swmm_getSubcatchPollut(subc_ind, SM_BUILDUP, &pollut_array);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(pollut_array[TSS] - 31.906912, 0.0001);
            BOOST_CHECK_SMALL(pollut_array[Lead] - 0.0, 0.0001);
            
        }

        // Route Model Forward
        error = swmm_step(&elapsedTime);
        step_ind+=1;
    }while (elapsedTime != 0 && !error);
    BOOST_REQUIRE(error == ERR_NONE);

    freeArray((void**) &pollut_array);

    swmm_end();
}


BOOST_AUTO_TEST_SUITE_END()