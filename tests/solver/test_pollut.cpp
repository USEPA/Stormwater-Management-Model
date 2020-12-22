/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.1.13
 Module:       test_pollut.cpp
 Description:  tests for SWMM pollutant API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/21/2020
 ******************************************************************************
*/

 #include <boost/test/unit_test.hpp>

 #include "test_solver.hpp"


#define ERR_NONE 0

BOOST_AUTO_TEST_SUITE(test_toolkitapi_pollut)

// Testing Rain Gage Setter
BOOST_FIXTURE_TEST_CASE(get_pollut_values, FixtureBeforeStep){
    int error, step_ind;
    int subc_ind;
    int node_ind;
    int link_ind;
    double *buildup_array;
    double *ponded_array;
    double *runoff_qual;
    double *runoff_load;
    double *node_qual;
    double *link_qual;
    double *link_load;
    int length;
    double elapsedTime = 0.0;
    // Pollutant IDs
    int TSS = 0;
    int Lead = 1;

    std::string subid = std::string("1");
    std::string nodeid = std::string("9");
    std::string linkid = std::string("1");

    subc_ind = swmm_getObjectIndex(SM_SUBCATCH, (char *)subid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

    node_ind = swmm_getObjectIndex(SM_NODE, (char *)nodeid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

    link_ind = swmm_getObjectIndex(SM_LINK, (char *)linkid.c_str(), &error);
    BOOST_REQUIRE(error == ERR_NONE);

    step_ind = 0;

    do
    {
        if (step_ind == 360) // (Jan 1, 1998 6:00am)
        {
            // subcatchment buildup
            error = swmm_getSubcatchPollut(subc_ind, SM_BUILDUP, &buildup_array, &length);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(buildup_array[TSS] - 31.906912, 0.0001);
            BOOST_CHECK_SMALL(buildup_array[Lead] - 0.0, 0.0001);

            // subcatchment ponded concentration
            error = swmm_getSubcatchPollut(subc_ind, SM_CPONDED, &ponded_array, &length);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(ponded_array[TSS] - 0.0, 0.0001);
            BOOST_CHECK_SMALL(ponded_array[Lead] - 0.0, 0.0001);

            // subcatchment runoff pollutant concentration
            error = swmm_getSubcatchPollut(subc_ind, SM_SUBCQUAL, &runoff_qual, &length);
            BOOST_CHECK_SMALL(runoff_qual[TSS] - 14.118948, 0.0001);
            BOOST_CHECK_SMALL(runoff_qual[Lead] - 2.823790, 0.0001);

            // subcatchment runoff total pollutant loading
            error = swmm_getSubcatchPollut(subc_ind, SM_SUBCTOTALLOAD, &runoff_load, &length);
            BOOST_CHECK_SMALL(runoff_load[TSS] - 0.00242786, 0.0001);
            BOOST_CHECK_SMALL(runoff_load[Lead] - 4.856e-10, 0.0001);

            // node pollutant concentration
            error = swmm_getNodePollut(node_ind, SM_NODEQUAL, &node_qual, &length);
            BOOST_CHECK_SMALL(node_qual[TSS] - 14.121316, 0.0001);
            BOOST_CHECK_SMALL(node_qual[Lead] - 2.824263, 0.0001);

            // link pollutant concentration
            error = swmm_getLinkPollut(node_ind, SM_LINKQUAL, &link_qual, &length);
            BOOST_CHECK_SMALL(link_qual[TSS] - 14.124621, 0.0001);
            BOOST_CHECK_SMALL(link_qual[Lead] - 2.824924, 0.0001);

            // link pollutant total load
            error = swmm_getLinkPollut(node_ind, SM_TOTALLOAD, &link_load, &length);
            BOOST_CHECK_SMALL(link_load[TSS] - 38.496695, 0.01);
            BOOST_CHECK_SMALL(link_load[Lead] - 0.00769934, 0.0001);
        }

        if (step_ind == 720) // (Jan 1, 1998 12:00pm)
        {
            // subcatchment buildup
            error = swmm_getSubcatchPollut(subc_ind, SM_BUILDUP, &buildup_array, &length);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(buildup_array[TSS] - 32.354460, 0.0001);
            BOOST_CHECK_SMALL(buildup_array[Lead] - 0.0, 0.0001);

            // subcatchment ponded concentration
            error = swmm_getSubcatchPollut(subc_ind, SM_CPONDED, &ponded_array, &length);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(ponded_array[TSS] - 0.0, 0.0001);
            BOOST_CHECK_SMALL(ponded_array[Lead] - 0.0, 0.0001);

            // subcatchment runoff pollutant concentration
            error = swmm_getSubcatchPollut(subc_ind, SM_SUBCQUAL, &runoff_qual, &length);
            BOOST_CHECK_SMALL(runoff_qual[TSS] - 0.0, 0.0001);
            BOOST_CHECK_SMALL(runoff_qual[Lead] - 0.0, 0.0001);

            // subcatchment runoff total pollutant loading
            error = swmm_getSubcatchPollut(subc_ind, SM_SUBCTOTALLOAD, &runoff_load, &length);
            BOOST_CHECK_SMALL(runoff_load[TSS] - 0.00248221, 0.0001);
            BOOST_CHECK_SMALL(runoff_load[Lead] - 4.964e-10, 0.0001);

            // node pollutant concentration
            error = swmm_getNodePollut(node_ind, SM_NODEQUAL, &node_qual, &length);
            BOOST_CHECK_SMALL(node_qual[TSS] - 0.0, 0.0001);
            BOOST_CHECK_SMALL(node_qual[Lead] - 0.0, 0.0001);

            // link pollutant concentration
            error = swmm_getLinkPollut(node_ind, SM_LINKQUAL, &link_qual, &length);
            BOOST_CHECK_SMALL(link_qual[TSS] - 4.380e-11, 0.0001);
            BOOST_CHECK_SMALL(link_qual[Lead] - 8.759e-12, 0.0001);

            // link pollutant total load
            error = swmm_getLinkPollut(node_ind, SM_TOTALLOAD, &link_load, &length);
            BOOST_CHECK_SMALL(link_load[TSS] - 39.780193, 0.01);
            BOOST_CHECK_SMALL(link_load[Lead] - 0.00795604, 0.0001);
        }

        // Route Model Forward
        error = swmm_step(&elapsedTime);
        step_ind+=1;
    }while (elapsedTime != 0 && !error);
    BOOST_REQUIRE(error == ERR_NONE);

    swmm_freeMemory(buildup_array);
    swmm_freeMemory(ponded_array);

    swmm_end();
}


BOOST_AUTO_TEST_SUITE_END()
