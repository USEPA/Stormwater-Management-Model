/*
 *   test_output.cpp
 *
 *   Created: 11/2/2017
 *   Author: Michael E. Tryby
 *           US EPA - ORD/NRMRL
 *
 *   Unit testing for SWMM outputapi using Boost Test.
 */

// NOTE: Travis installs libboost test version 1.5.4
#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "output"
#include <boost/test/included/unit_test.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>

#include "swmm_output.h"


// NOTE: Project Home needs to be updated to run unit test
//#define PROJECT_HOME "C:/Users/mtryby/Workspace/GitRepo/michaeltryby/Stormwater-Management-Model"
// NOTE: Reference data for the unit tests is currently tied to SWMM 5.1.7
#define DATA_PATH "./Example1.out"

using namespace std;

// Custom test to check the minimum number of correct decimal digits between 
// the test and the ref vectors.  
boost::test_tools::predicate_result check_cdd(std::vector<float>& test, 
    std::vector<float>& ref, long cdd_tol)
{
    float tmp, min_cdd = 100.0;
    
    // TODO: What is the vectors aren't the same length? 

    std::vector<float>::iterator test_it;
    std::vector<float>::iterator ref_it;

    for (test_it = test.begin(); test_it < test.end(); ++test_it) {
        for (ref_it = ref.begin(); ref_it < ref.end(); ++ref_it) {
             
             if (*test_it != *ref_it) {
                tmp = - log10f(abs(*test_it - *ref_it));
                if (tmp < min_cdd) min_cdd = tmp;
            }
        }
    }

    if (min_cdd == 100.0)
        return true; 
    else
        return std::lround(min_cdd) <= cdd_tol;
}

boost::test_tools::predicate_result check_string(std::string test, std::string ref)
{
    if (ref.compare(test) == 0)
        return true;
    else
        return false;
}

BOOST_AUTO_TEST_SUITE (test_output_auto)

BOOST_AUTO_TEST_CASE(InitTest) {
    SMO_Handle p_handle = NULL;

    int error = SMO_init(&p_handle);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(p_handle != NULL);
}

BOOST_AUTO_TEST_CASE(OpenTest) {
    std::string path = std::string(DATA_PATH);
    SMO_Handle p_handle = NULL;
    SMO_init(&p_handle);

    int error = SMO_open(p_handle, path.c_str());
    BOOST_REQUIRE(error == 0);

    SMO_close(&p_handle);
}

BOOST_AUTO_TEST_CASE(CloseTest) {
    SMO_Handle p_handle = NULL;
    int error = SMO_init(&p_handle);

    error = SMO_close(&p_handle);
    BOOST_REQUIRE(error == -1);
    BOOST_CHECK(p_handle != NULL);
}

BOOST_AUTO_TEST_SUITE_END()

struct Fixture{
    Fixture() {
        std::string path = std::string(DATA_PATH);

        error = SMO_init(&p_handle);
        SMO_clearError(p_handle);
        error = SMO_open(p_handle, path.c_str());

        array = NULL;
        array_dim = 0;
    }
    ~Fixture() {
        SMO_free((void**)&array);
        error = SMO_close(&p_handle);
    }
   
    std::string path;
    int error;
    SMO_Handle p_handle;

    float* array;
    int array_dim;
};

BOOST_AUTO_TEST_SUITE(test_output_fixture)

BOOST_FIXTURE_TEST_CASE(test_getVersion, Fixture) {
    int version;

    error = SMO_getVersion(p_handle, &version);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK_EQUAL(51000, version);
}

BOOST_FIXTURE_TEST_CASE(test_getProjectSize, Fixture) {
    int* i_array = NULL;

    error = SMO_getProjectSize(p_handle, &i_array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<int> test;
    test.assign(i_array, i_array + array_dim);
    // subcatchs, nodes, links, pollutants
    std::vector<int> ref({8,14,13,2});

    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

    SMO_free((void**)&i_array);
}

BOOST_FIXTURE_TEST_CASE(test_getFlowUnits, Fixture) {
    int units = -1;

    error = SMO_getFlowUnits(p_handle, &units);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(0, units);
}

BOOST_FIXTURE_TEST_CASE(test_getPollutantUnits, Fixture) {
    int* i_array = NULL;

    error = SMO_getPollutantUnits(p_handle, &i_array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<int> test;
    test.assign(i_array, i_array + array_dim);
    std::vector<int> ref({0, 1});

    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

    SMO_free((void**)&i_array);
}

BOOST_FIXTURE_TEST_CASE(test_getStartDate, Fixture) {
    double date = -1;

    error = SMO_getStartDate(p_handle, &date);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK_EQUAL(35796., date);
}

BOOST_FIXTURE_TEST_CASE(test_getTimes, Fixture) {
    int time = -1;

    error = SMO_getTimes(p_handle, SMO_reportStep, &time);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK_EQUAL(3600, time);

    error = SMO_getTimes(p_handle, SMO_numPeriods, &time);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK_EQUAL(36, time);
}

BOOST_FIXTURE_TEST_CASE(test_getElementName, Fixture) {
    char* c_array = NULL;
    int index = 1;

    error = SMO_getElementName(p_handle, SMO_node, index, &c_array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::string test(c_array);
    std::string ref("10");
    BOOST_CHECK(check_string(test, ref));

    SMO_free((void**)&c_array);

}

BOOST_FIXTURE_TEST_CASE(test_getSubcatchSeries, Fixture) {

    error = SMO_getSubcatchSeries(p_handle, 1, SMO_runoff_rate, 0, 10, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

   std::vector<float> ref_vec({0.0,
                               1.2438242,
                               2.5639679,
                               4.524055,
                               2.5115132,
                               0.69808137,
                               0.040894926,
                               0.011605669,
                               0.00509294,
                               0.0027438672,
                               10.0});


    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getSubcatchResult, Fixture) {
    
    error = SMO_getSubcatchResult(p_handle, 1, 1, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<float> ref_vec({0.5,
                                0.0,
                                0.0,
                                0.125,
                                1.2438242,
                                0.0,
                                0.0,
                                0.0,
                                33.481991,
                                6.6963983});

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getNodeResult, Fixture) {

    error = SMO_getNodeResult(p_handle, 2, 2, &array, &array_dim);
    BOOST_REQUIRE(error == 0);
    
    std::vector<float> ref_vec({0.296234,
                                995.296204,
                                0.0,
                                1.302650,
                                1.302650,
                                0.0,
                                15.361463,
                                3.072293});

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getLinkResult, Fixture) {
    
    error = SMO_getLinkResult(p_handle, 3, 3, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<float> ref_vec({4.631762,
                                1.0,
                                5.8973422,
                                314.15927,
                                1.0,
                                19.070757,
                                3.8141515});

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getSystemResult, Fixture) {
    
    error = SMO_getSystemResult(p_handle, 4, 4, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<float> ref_vec({70.0,
                                0.1,
                                0.0,
                                0.19042271,
                                14.172027,
                                0.0,
                                0.0,
                                0.0,
                                0.0,
                                14.172027,
                                0.55517411,
                                13.622702,
                                2913.0793,
                                0.0});

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd(test_vec, ref_vec, 3));
}

BOOST_AUTO_TEST_SUITE_END()
