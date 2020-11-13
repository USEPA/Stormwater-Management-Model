


#include <boost/test/unit_test.hpp>

#include "test_solver.hpp"

#define ERR_NONE 0
#define ERR_API_MEMORY 115

using namespace std;


BOOST_AUTO_TEST_SUITE()


// Testing Results Getters (Before End Simulation)
BOOST_FIXTURE_TEST_CASE(get_subcatch_stats, FixtureBeforeEnd){
    int error;
    int subc_ind;

    char subid[] = "1";

    // Subcatchment
    SM_SubcatchStats *subc_stats = NULL;

    error = swmm_getObjectIndex(SM_SUBCATCH, subid, &subc_ind);
    BOOST_REQUIRE(error == ERR_NONE);

    error = swmm_getSubcatchStats(subc_ind, &subc_stats);
    BOOST_CHECK_EQUAL(error, ERR_NONE);

    BOOST_CHECK_SMALL(subc_stats->runon - 0.0, 0.0001);
    BOOST_CHECK_SMALL(subc_stats->infil - 42088, 1.0);
    BOOST_CHECK_SMALL(subc_stats->runoff - 53781, 1.0);
    BOOST_CHECK_SMALL(subc_stats->maxFlow - 4.6561, 0.0001);
    BOOST_CHECK_SMALL(subc_stats->precip - 2.65, 0.0001);
    BOOST_CHECK_SMALL(subc_stats->evap - 0.0, 0.0001);

    swmm_freeMemory(subc_stats);
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

BOOST_AUTO_TEST_SUITE_END()
