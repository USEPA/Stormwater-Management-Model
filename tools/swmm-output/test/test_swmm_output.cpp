/*
 *   test_swmm_output.cpp
 *
 *   Created: 11/2/2017
 *   Author: Michael E. Tryby
 *           US EPA - ORD/NRMRL
 *
 *   Unit testing for SWMM outputapi using Google Test.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "gtest/gtest.h"
#include "../src/swmm_output.h"


// NOTE: Project Home needs to be updated to run unit test
#define PROJECT_HOME "C:/Users/mtryby/Workspace/GitRepo/michaeltryby/Stormwater-Management-Model"
// NOTE: Reference data for the unit tests is currently tied to SWMM 5.1.7
#define DATA_PATH "/tools/swmm-output/test/data/Example1.out"

namespace {

TEST(SMO_init, InitTest) {
    SMO_Handle p_handle = NULL;

    int error = SMO_init(&p_handle);
    ASSERT_EQ(0, error);
    ASSERT_TRUE(p_handle != NULL);
}

TEST(SMO_open, OpenTest) {
    std::string path = std::string(PROJECT_HOME) + std::string(DATA_PATH);
    SMO_Handle p_handle = NULL;
    SMO_init(&p_handle);

    int error = SMO_open(p_handle, path.c_str());
    ASSERT_EQ(0, error);
    SMO_close(&p_handle);
}

class SWMM_OutputFixture : public testing::Test {
protected:
    // SetUp for OutputapiTest fixture
    virtual void SetUp() {
        std::string path = std::string(PROJECT_HOME) + std::string(DATA_PATH);

        error = SMO_init(&p_handle);
        SMO_clearError(p_handle);
        error = SMO_open(p_handle, path.c_str());
    }

    // TearDown for OutputapiTest fixture
    virtual void TearDown() {
        SMO_free((void**)&array);
        error = SMO_close(&p_handle);
    }

    int error = 0;
    SMO_Handle p_handle = NULL;

    float* array = NULL;
    int array_dim = 0;
};

TEST_F(SWMM_OutputFixture, getVersionTest) {
    int version;

    error = SMO_getVersion(p_handle, &version);
    ASSERT_EQ(0, error);

    EXPECT_EQ(51000, version);
}

TEST_F(SWMM_OutputFixture, getProjectSizeTest) {
    int* i_array = NULL;
    // subcatchs, nodes, links, pollutants
    int ref_array[4] = {8,14,13,2};

    error = SMO_getProjectSize(p_handle, &i_array, &array_dim);
    ASSERT_EQ(0, error);

    EXPECT_EQ(4, array_dim);
    for (int i = 0; i < array_dim; i++)
        EXPECT_EQ(ref_array[i], i_array[i]);

    SMO_free((void**)&i_array);
}

TEST_F(SWMM_OutputFixture, getFlowUnitsTest) {
    int units = -1;

    error = SMO_getFlowUnits(p_handle, &units);
    ASSERT_EQ(0, error);
    EXPECT_EQ(0, units);
}

TEST_F(SWMM_OutputFixture, getPollutantUnitsTest) {
    int* i_array = NULL;
    int ref_array[2] = {0, 1};

    error = SMO_getPollutantUnits(p_handle, &i_array, &array_dim);
    ASSERT_EQ(0, error);

    EXPECT_EQ(2, array_dim);
    for (int i = 0; i < array_dim; i++)
        EXPECT_EQ(ref_array[i], i_array[i]);

    SMO_free((void**)&i_array);
}

TEST_F(SWMM_OutputFixture, getStartDateTest) {
    double date = -1;

    error = SMO_getStartDate(p_handle, &date);
    ASSERT_EQ(0, error);

    EXPECT_EQ(35796., date);
}

TEST_F(SWMM_OutputFixture, getTimesTest) {
    int time = -1;

    error = SMO_getTimes(p_handle, SMO_reportStep, &time);
    ASSERT_EQ(0, error);

    EXPECT_EQ(3600, time);

    error = SMO_getTimes(p_handle, SMO_numPeriods, &time);
    ASSERT_EQ(0, error);

    EXPECT_EQ(36, time);
}

TEST_F(SWMM_OutputFixture, getElementNameTest) {
    char* c_array = NULL;
    int index = 1;

    error = SMO_getElementName(p_handle, SMO_node, index, &c_array, &array_dim);
    ASSERT_EQ(0, error);

    EXPECT_STREQ("10", c_array);
    EXPECT_EQ(2, array_dim);

    SMO_free((void**)&c_array);

}

//TEST_F(OutputapiTest, getSubcatchSeriesTest) {
//    float ref_array[11] = {0.0,
//            1.2438242,
//            2.5639679,
//            4.524055,
//            2.5115132,
//            0.69808137,
//            0.040894926,
//            0.011605669,
//            0.00509294,
//            0.0027438672,
//            10};
//
//    error = SMO_getSubcatchSeries(p_handle, 1, runoff_rate, 0, 10, &array, &array_dim);
//    ASSERT_EQ(0, error);
//
//    EXPECT_EQ(11, array_dim);
//    for (int i = 0; i < array_dim; i++)
//        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
//}

TEST_F(SWMM_OutputFixture, getSubcatchResultTest) {
    float ref_array[10] = {0.5,
            0.0,
            0.0,
            0.125,
            1.2438242,
            0.0,
            0.0,
            0.0,
            33.481991,
            6.6963983};

    error = SMO_getSubcatchResult(p_handle, 1, 1, &array, &array_dim);
    ASSERT_EQ(0, error);

    EXPECT_EQ(10, array_dim);
    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}

TEST_F(SWMM_OutputFixture, getNodeResultTest) {
    float ref_array[8] = {0.296234,
            995.296204,
            0.0,
            1.302650,
            1.302650,
            0.0,
            15.361463,
            3.072293};

    error = SMO_getNodeResult(p_handle, 2, 2, &array, &array_dim);
    ASSERT_EQ(0, error);

    EXPECT_EQ(8, array_dim);
    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}

TEST_F(SWMM_OutputFixture, getLinkResultTest) {
    float ref_array[7] = {4.631762,
            1.0,
            5.8973422,
            314.15927,
            1.0,
            19.070757,
            3.8141515};

    error = SMO_getLinkResult(p_handle, 3, 3, &array, &array_dim);
    ASSERT_EQ(0, error);

    EXPECT_EQ(7, array_dim);
    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}

TEST_F(SWMM_OutputFixture, getSystemResultTest) {
    float ref_array[14] = {70.0,
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
            0.0};

    error = SMO_getSystemResult(p_handle, 4, 4, &array, &array_dim);
    ASSERT_EQ(0, error);

    EXPECT_EQ(14, array_dim);
    for (int i = 0; i < array_dim; i++)
        EXPECT_FLOAT_EQ(ref_array[i], array[i]);
}
}


int main(int argc, char **argv) {
    SWMM_OutputFixture::SetUpTestCase();
    printf("Running main() from gtest_main.cc\n");
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
