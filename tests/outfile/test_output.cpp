/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.1.13
 Module:       test_output.cpp
 Description:  tests for output library functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/21/2020
 ******************************************************************************
 */

#define BOOST_TEST_MODULE "output"
#include <boost/test/included/unit_test.hpp>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "swmm_output.h"

// NOTE: Reference data for the unit tests is currently tied to SWMM 5.1.7
#define DATA_PATH "./test_example1.out"

using namespace std;

// Checks for minimum number of correct decimal digits
boost::test_tools::predicate_result check_cdd_float(std::vector<float>& test,
    std::vector<float>& ref, long cdd_tol){

    float tmp, min_cdd = 10.0;

    // TODO: What if the vectors aren't the same length?

    std::vector<float>::iterator test_it;
    std::vector<float>::iterator ref_it;

    for (test_it = test.begin(), ref_it = ref.begin();
        (test_it < test.end()) && (ref_it < ref.end());
        ++test_it, ++ref_it)
    {
        if (*test_it != *ref_it)
        {
            // Compute log absolute error
            tmp = abs(*test_it - *ref_it);
            if (tmp < 1.0e-7f)
                tmp = 1.0e-7f;

            else if (tmp > 2.0f)
                tmp = 1.0f;

            tmp = -log10(tmp);
            if (tmp < 0.0f)
                tmp = 0.0f;

            if (tmp < min_cdd)
                min_cdd = tmp;
        }
    }
    return floor(min_cdd) >= cdd_tol;
}

boost::test_tools::predicate_result check_string(std::string test,
    std::string ref) {

    if (ref.compare(test) == 0)
        return true;
    else
        return false;
}

BOOST_AUTO_TEST_SUITE(test_output_auto)

BOOST_AUTO_TEST_CASE(InitTest) {
    SMO_Handle p_handle = NULL;

    int error = SMO_init(&p_handle);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(p_handle != NULL);

    SMO_close(p_handle);

}

BOOST_AUTO_TEST_CASE(CloseTest) {
    SMO_Handle p_handle = NULL;
    SMO_init(&p_handle);

    int error = SMO_close(p_handle);
    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_CASE(InitOpenCloseTest) {
    std::string path     = std::string(DATA_PATH);
    SMO_Handle p_handle = NULL;
    SMO_init(&p_handle);

    int error = SMO_open(p_handle, path.c_str());
    BOOST_REQUIRE(error == 0);

    SMO_close(p_handle);
}

BOOST_AUTO_TEST_SUITE_END()

struct Fixture {
    Fixture() {
        std::string path = std::string(DATA_PATH);

        error = SMO_init(&p_handle);
        SMO_clearError(p_handle);
        error = SMO_open(p_handle, path.c_str());

        array     = NULL;
        array_dim = 0;
    }
    ~Fixture() {
        SMO_freeMemory((void*)array);
        error = SMO_close(p_handle);
    }

    std::string path;
    int         error;
    SMO_Handle  p_handle;

    float* array;
    int    array_dim;
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
    const int ref_dim            = 5;
    int       ref_array[ref_dim] = {8, 14, 13, 1, 2};

    std::vector<int> ref;
    ref.assign(ref_array, ref_array + ref_dim);

    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

    SMO_freeMemory((void*)i_array);
}

BOOST_FIXTURE_TEST_CASE(test_getUnits, Fixture) {
    int*      i_array            = NULL;

    error = SMO_getUnits(p_handle, &i_array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<int> test;
    test.assign(i_array, i_array + array_dim);

    // unit system, flow units, pollut units
    const int        ref_dim            = 4;
    const int        ref_array[ref_dim] = {SMO_US, SMO_CFS, SMO_MG, SMO_UG};

    std::vector<int> ref;
    ref.assign(ref_array, ref_array + ref_dim);

    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

    SMO_freeMemory((void*)i_array);
}

// BOOST_FIXTURE_TEST_CASE(test_getFlowUnits, Fixture) {
//     int units = -1;
//
//     error = SMO_getFlowUnits(p_handle, &units);
//     BOOST_REQUIRE(error == 0);
//     BOOST_CHECK_EQUAL(0, units);
// }
//
// BOOST_FIXTURE_TEST_CASE(test_getPollutantUnits, Fixture) {
//     int* i_array = NULL;
//
//     error = SMO_getPollutantUnits(p_handle, &i_array, &array_dim);
//     BOOST_REQUIRE(error == 0);
//
//     std::vector<int> test;
//     test.assign(i_array, i_array + array_dim);
//
//     const int ref_dim            = 2;
//     int       ref_array[ref_dim] = {0, 1};
//
//     std::vector<int> ref;
//     ref.assign(ref_array, ref_array + ref_dim);
//
//     BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(),
//                                   test.end());
//
//     SMO_freeMemory((void**)&i_array);
//     BOOST_CHECK(i_array == NULL);
// }

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
    int   index   = 1;

    error = SMO_getElementName(p_handle, SMO_node, index, &c_array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::string test(c_array);
    std::string ref("10");
    BOOST_CHECK(check_string(test, ref));

    SMO_freeMemory((void*)c_array);
}

BOOST_FIXTURE_TEST_CASE(test_getSubcatchSeries, Fixture) {
    error = SMO_getSubcatchSeries(p_handle, 1, SMO_runoff_rate, 0, 10, &array,
                                  &array_dim);
    BOOST_REQUIRE(error == 0);

    const int ref_dim            = 10;
    float     ref_array[ref_dim] = {
        0.0f, 1.2438242f, 2.5639679f, 4.524055f, 2.5115132f, 0.69808137f,
        0.040894926f, 0.011605669f, 0.00509294f, 0.0027438672f};

    std::vector<float> ref_vec;
    ref_vec.assign(ref_array, ref_array + 10);

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd_float(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getSubcatchResult, Fixture) {
    error = SMO_getSubcatchResult(p_handle, 1, 1, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    const int ref_dim            = 10;
    float     ref_array[ref_dim] = {
        0.5f, 0.0f, 0.0f, 0.125f, 1.2438242f,
        0.0f, 0.0f, 0.0f, 33.481991f, 6.6963983f};

    std::vector<float> ref_vec;
    ref_vec.assign(ref_array, ref_array + ref_dim);

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd_float(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getNodeResult, Fixture) {
    error = SMO_getNodeResult(p_handle, 2, 2, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    const int ref_dim        = 8;
    float ref_array[ref_dim] = {
        0.296234f, 995.296204f, 0.0f, 1.302650f, 1.302650f, 0.0f,
        15.361463f, 3.072293f};

    std::vector<float> ref_vec;
    ref_vec.assign(ref_array, ref_array + ref_dim);

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd_float(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getLinkResult, Fixture) {
    error = SMO_getLinkResult(p_handle, 3, 3, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    const int ref_dim        = 7;
    float ref_array[ref_dim] = {
        4.631762f, 1.0f, 5.8973422f, 314.15927f, 1.0f, 19.070757f,
        3.8141515f};

    std::vector<float> ref_vec;
    ref_vec.assign(ref_array, ref_array + ref_dim);

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd_float(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getSystemResult, Fixture) {
    error = SMO_getSystemResult(p_handle, 4, 4, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    const int ref_dim            = 14;
    float     ref_array[ref_dim] = {
        70.0f, 0.1f, 0.0f, 0.19042271f, 14.172027f, 0.0f, 0.0f, 0.0f,
        0.0f, 14.172027f, 0.55517411f, 13.622702f, 2913.0793f, 0.0f};

    std::vector<float> ref_vec;
    ref_vec.assign(ref_array, ref_array + ref_dim);

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd_float(test_vec, ref_vec, 3));
}

BOOST_AUTO_TEST_SUITE_END()
