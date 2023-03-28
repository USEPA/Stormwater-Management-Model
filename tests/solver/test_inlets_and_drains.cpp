/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.2.3
 Module:       test_inlets_and_drains.cpp
 Description:  tests for SWMM inlets and drains API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 3/27/2023
 ******************************************************************************
*/

 #include <boost/test/unit_test.hpp>

 #include "test_solver.hpp"


#define ERR_NONE 0

BOOST_AUTO_TEST_SUITE(test_toolkitapi_inlets_and_drains)


BOOST_FIXTURE_TEST_CASE(getset_inlet_params, FixtureBeforeStep_Inlets){
    int error, link_ind;
    double value;
    char id[] = "Street1";

    error = swmm_getObjectIndex(SM_LINK, id, &link_ind);
    BOOST_REQUIRE(error == ERR_NONE);

    // test num inlets getter
    error = swmm_getInletParam(link_ind, SM_INLETNUMINLETS, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(value - 1.0, 0.0001);
    
    // test clogging factor getter
    error = swmm_getInletParam(link_ind, SM_INLETCLOGFACTOR, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(value - 50.0, 0.0001);

    // test clogging factor setter
    error = swmm_setInletParam(link_ind, SM_INLETCLOGFACTOR, 78.0);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getInletParam(link_ind, SM_INLETCLOGFACTOR, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(value - 78.0, 0.0001);

     // test flow limit getter
    error = swmm_getInletParam(link_ind, SM_INLETFLOWLIMIT, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(value - 2.2, 0.0001);

    // test flow limit setter
    error = swmm_setInletParam(link_ind, SM_INLETFLOWLIMIT, 5.8);
    BOOST_REQUIRE(error == ERR_NONE);
    error = swmm_getInletParam(link_ind, SM_INLETFLOWLIMIT, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(value - 5.8, 0.0001);

    // test local depression height getter
    error = swmm_getInletParam(link_ind, SM_INLETLOCALDEPRESS, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(value - 0.5, 0.0001);
    
    // test local depression width getter
    error = swmm_getInletParam(link_ind, SM_INLETLOCALWIDTH, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_SMALL(value - 2.0, 0.0001);

}

BOOST_FIXTURE_TEST_CASE(get_result_during_sim, FixtureBeforeStep_Inlets){
    int error, step_ind;
    int lnk_ind;
    double val;
    double elapsedTime = 0.0;

    char lnkid[] = "Street1";

    error = swmm_getObjectIndex(SM_LINK, lnkid, &lnk_ind);
    BOOST_REQUIRE(error == ERR_NONE);
     
    step_ind = 0;

    do
    {
        error = swmm_step(&elapsedTime);

        if (step_ind == 250) // peak flow in Street1
        {
            // test inlet flow factor getter
            error = swmm_getInletResult(lnk_ind, SM_INLETFLOWFACTOR, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 0.0118, 0.0001);

            // test inlet flow capture getter
            error = swmm_getInletResult(lnk_ind, SM_INLETFLOWCAPTURE, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 3.3864, 0.0001);

            // test inlet backflow getter
            error = swmm_getInletResult(lnk_ind, SM_INLETBACKFLOW, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 2.5670, 0.0001);

            // test inlet backflow ratio getter
            error = swmm_getInletResult(lnk_ind, SM_INLETBACKFLOWRATIO, &val);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(val - 1.0, 0.0001);
        }
        
        step_ind+=1;
    }while (elapsedTime != 0 && !error);
    // BOOST_REQUIRE(error == 1);
    swmm_end();
}


BOOST_AUTO_TEST_SUITE_END()