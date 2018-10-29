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
BOOST_AUTO_TEST_SUITE(test_toolkitapi)

// Test Model Not Open
BOOST_AUTO_TEST_CASE(model_not_open) {
    int error, index;
    double val;
    double input_val = 0;
    double *result_array;
    char id[] = "test";

    //Project
    error = swmm_getObjectIndex(SM_NODE, id, &index);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    //Gage
    error = swmm_getGagePrecip(0, &result_array);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_setGagePrecip(0, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);


    // //Subcatchment
    error = swmm_getSubcatchParam(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_setSubcatchParam(0, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_getSubcatchResult(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);


    // //Node
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


    // //Link
    error = swmm_getLinkParam(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_setLinkParam(0, 0, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_getLinkResult(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_setLinkSetting(0, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);


    // //Pollutant
    error = swmm_getSubcatchPollut(0, 0, &result_array);
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
}


// Testing for invalid object index
BOOST_FIXTURE_TEST_CASE(object_bounds_check, FixtureOpenClose) {
    int error;
    double val;
    double input_val = 0;
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


// Testing for Project Settings after Open
BOOST_FIXTURE_TEST_CASE(project_info, FixtureOpenClose){

    int error, index;
    char id[] = "14";

    //Project
    error = swmm_getObjectIndex(SM_NODE, id, &index);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(index, 3);
}


// Testing for subcatchment get/set
BOOST_FIXTURE_TEST_CASE(getset_subcatch, FixtureOpenClose) {

    int error, subc_ind;
    double val;
    char id[] = "5";

    error = swmm_getObjectIndex(SM_SUBCATCH, id, &subc_ind);
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


    // // Get/Set Subcatchment SM_FRACIMPERV
    // error = swmm_getSubcatchParam(subc_ind, SM_FRACIMPERV, &val);
    // BOOST_REQUIRE(error == ERR_NONE);
    // BOOST_CHECK_SMALL(val - .50, 0.0001);

    // error = swmm_setSubcatchParam(subc_ind, SM_FRACIMPERV, .70);
    // BOOST_REQUIRE(error == ERR_NONE);

    // error = swmm_getSubcatchParam(subc_ind, SM_FRACIMPERV, &val);
    // BOOST_REQUIRE(error == ERR_NONE);
    // BOOST_CHECK_SMALL(val - .50, 0.0001);


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

    char id[] = "14";

    error = swmm_getObjectIndex(SM_NODE, id, &node_ind);
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
    char id[] = "10";

    error = swmm_getObjectIndex(SM_LINK, id, &link_ind);
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


// Testing for After Start Errors
BOOST_FIXTURE_TEST_CASE(sim_after_start_check, FixtureBeforeStep){
    int error;
    double val, input_val;
    input_val = 0;

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
}


// Testing Results Getters (During Simulation)
BOOST_FIXTURE_TEST_CASE(get_result_during_sim, FixtureBeforeStep){
    int error, step_ind;
    int subc_ind, nde_ind, lnk_ind;
    double val;
    double elapsedTime = 0.0;

    char subid[] = "1";
    char ndeid[] = "19";
    char lnkid[] = "14";

    error = swmm_getObjectIndex(SM_SUBCATCH, subid, &subc_ind);
    BOOST_REQUIRE(error == ERR_NONE);

    error = swmm_getObjectIndex(SM_NODE, ndeid, &nde_ind);
    BOOST_REQUIRE(error == ERR_NONE);

    error = swmm_getObjectIndex(SM_LINK, lnkid, &lnk_ind);
    BOOST_REQUIRE(error == ERR_NONE);

    step_ind = 0;

    std::vector<double> test_vec;
    std::vector<double> subc_vec{0.8,
                                 0.0,
                                 0.125,
                                 0.0,
                                 4.3365,
                                 0.0};

    std::vector<double> node_vec{0.4793,
                                 0.4793,
                                 0.0,
                                 0.0,
                                 0.0,
                                 0.1730,
                                 1010.1730,
                                 0.4793};

    std::vector<double> link_vec{1.1245,
                                 0.3023,
                                 80.15,
                                 0.0,
                                 0.0,
                                 1.0,
                                 1.0,
                                 0.0};

    do
    {
        error = swmm_step(&elapsedTime);

        if (step_ind == 200) // (Jan 1, 1998 3:20am)
        {
            // Subcatchment
            for (SM_SubcResult property = SM_SUBCRAIN; property <= SM_SUBCSNOW;
                property = SM_SubcResult(property + 1))
            {
                error = swmm_getSubcatchResult(subc_ind, property, &val);
                BOOST_REQUIRE(error == ERR_NONE);
                test_vec.push_back(val);
            }
            BOOST_CHECK(check_cdd(test_vec, subc_vec, 3));

            test_vec.clear();


            // Node
            for (SM_NodeResult property = SM_TOTALINFLOW; property <= SM_LATINFLOW;
                property = SM_NodeResult(property + 1))
            {
                error = swmm_getNodeResult(nde_ind, property, &val);
                BOOST_REQUIRE(error == ERR_NONE);
                test_vec.push_back(val);
            }
            BOOST_CHECK(check_cdd(test_vec, node_vec, 3));

            test_vec.clear();


            // Link
            for (SM_LinkResult property = SM_LINKFLOW; property <= SM_FROUDE;
                property = SM_LinkResult(property + 1))
            {
                error = swmm_getLinkResult(lnk_ind, property, &val);
                BOOST_REQUIRE(error == ERR_NONE);
                test_vec.push_back(val);
            }
            BOOST_CHECK(check_cdd(test_vec, link_vec, 3));

        }
        step_ind+=1;
    }while (elapsedTime != 0 && !error);
    BOOST_REQUIRE(error == ERR_NONE);
    swmm_end();
}


// Testing Results Getters (Before End Simulation)
BOOST_FIXTURE_TEST_CASE(get_results_after_sim, FixtureBeforeEnd){
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

    freeArray((void**)&subc_stats);
}

BOOST_AUTO_TEST_SUITE_END()
