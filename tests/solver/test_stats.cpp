/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.1.13
 Module:       test_stats.cpp
 Description:  tests for SWMM stats access functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/21/2020
 ******************************************************************************
*/


#include <boost/test/unit_test.hpp>

#include "test_solver.hpp"

#define ERR_NONE 0
#define ERR_API_MEMORY 512
#define ERR_API_WRONG_TYPE 504

using namespace std;


BOOST_AUTO_TEST_SUITE(test_toolkit_stats)


// Testing Results Getters (Before End Simulation)
BOOST_FIXTURE_TEST_CASE(get_node_stats, FixtureBeforeEnd){

    int error, index;
    char id[] = "24";

    SM_NodeStats *s = NULL;

    error = swmm_getObjectIndex(SM_NODE, id, &index);
    BOOST_REQUIRE(error == ERR_NONE);

    // Test argument checks
    error = swmm_getNodeStats(index, s);
    BOOST_CHECK_EQUAL(error, ERR_API_MEMORY);
}


BOOST_FIXTURE_TEST_CASE(get_storage_stats, FixtureBeforeEnd){
    int error, index;
    char id[] = "24";

    SM_StorageStats *s = NULL;

    error = swmm_getObjectIndex(SM_NODE, id, &index);
    BOOST_REQUIRE(error == ERR_NONE);

    // Test argument checks
    error = swmm_getStorageStats(index, s);
    BOOST_CHECK_EQUAL(error, ERR_API_WRONG_TYPE);

    //TODO: Example 1 has no storage nodes
}


BOOST_FIXTURE_TEST_CASE(get_outfall_stats, FixtureBeforeEnd){

    int error;
    int outfall_index;

    char outfall_id[] = "18";

    SM_OutfallStats *s = NULL;

    error = swmm_getObjectIndex(SM_NODE, outfall_id, &outfall_index);
    BOOST_REQUIRE(error == ERR_NONE);

    // Test argument checks
    error = swmm_getOutfallStats(outfall_index, s);
    BOOST_CHECK_EQUAL(error, ERR_API_MEMORY);

    int num_pollut;
    error = swmm_countObjects(SM_POLLUT, &num_pollut);
    BOOST_REQUIRE(error == ERR_NONE);


    // Allocate memory for SM_OutfallStats
    s = (SM_OutfallStats *)calloc(1, sizeof(SM_OutfallStats));
    if (num_pollut > 0)
            s->totalLoad = (double *)calloc(num_pollut, sizeof(double));
    BOOST_REQUIRE(s != NULL);

    error = swmm_getOutfallStats(outfall_index, s);
    BOOST_REQUIRE(error == ERR_NONE);

    // Free memory allocated for SM_OutfallStats
    if (s != NULL)
        free(s->totalLoad);
    free(s);
}


BOOST_FIXTURE_TEST_CASE(get_link_stats, FixtureBeforeEnd){
    int error;
    int index;

    char id[] = "15";

    SM_LinkStats *s = NULL;

    error = swmm_getObjectIndex(SM_LINK, id, &index);
    BOOST_REQUIRE(error == ERR_NONE);

    // Test argument checks
    error = swmm_getLinkStats(index, s);
    BOOST_CHECK_EQUAL(error, ERR_API_MEMORY);

}


BOOST_FIXTURE_TEST_CASE(get_pump_stats, FixtureBeforeEnd){
    int error;
    int index;

    char id[] = "15";

    SM_PumpStats *s = NULL;

    error = swmm_getObjectIndex(SM_LINK, id, &index);
    BOOST_REQUIRE(error == ERR_NONE);

    // Test argument checks
    error = swmm_getPumpStats(index, s);
    BOOST_CHECK_EQUAL(error, ERR_API_WRONG_TYPE);

    //TODO: Example 1 has no pumps
}


BOOST_FIXTURE_TEST_CASE(get_subcatch_stats, FixtureBeforeEnd){
    int error;
    int index;

    char id[] = "1";

    // Subcatchment
    SM_SubcatchStats *subc_stats = NULL;

    error = swmm_getObjectIndex(SM_SUBCATCH, id, &index);
    BOOST_REQUIRE(error == ERR_NONE);

    error = swmm_getSubcatchStats(index, subc_stats);
    BOOST_CHECK_EQUAL(error, ERR_API_MEMORY);

    // BOOST_CHECK_SMALL(subc_stats->runon - 0.0, 0.0001);
    // BOOST_CHECK_SMALL(subc_stats->infil - 42088, 1.0);
    // BOOST_CHECK_SMALL(subc_stats->runoff - 53781, 1.0);
    // BOOST_CHECK_SMALL(subc_stats->maxFlow - 4.6561, 0.0001);
    // BOOST_CHECK_SMALL(subc_stats->precip - 2.65, 0.0001);
    // BOOST_CHECK_SMALL(subc_stats->evap - 0.0, 0.0001);
}

BOOST_FIXTURE_TEST_CASE(get_routing_totals, FixtureBeforeEnd){
    int error;

    // Subcatchment
    SM_RoutingTotals *s = NULL;

    error = swmm_getSystemRoutingTotals(s);
    BOOST_CHECK_EQUAL(error, ERR_API_MEMORY);

    SM_RoutingTotals __rtots;
    SM_RoutingTotals *_rtots = &__rtots;

    error = swmm_getSystemRoutingTotals(_rtots);
    BOOST_REQUIRE(error == ERR_NONE);

    BOOST_CHECK(_rtots != NULL);
}


BOOST_FIXTURE_TEST_CASE(get_runoff_totals, FixtureBeforeEnd){
    int error;

    // Subcatchment
    SM_RunoffTotals *s = NULL;

    error = swmm_getSystemRunoffTotals(s);
    BOOST_CHECK_EQUAL(error, ERR_API_MEMORY);

    SM_RunoffTotals __rtots;
    SM_RunoffTotals *_rtots = &__rtots;

    error = swmm_getSystemRunoffTotals(_rtots);
    BOOST_REQUIRE(error == ERR_NONE);

    BOOST_CHECK(_rtots != NULL);
}

BOOST_AUTO_TEST_SUITE_END()
