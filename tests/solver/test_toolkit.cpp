/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.1.13
 Module:       test_toolkit.cpp
 Description:  tests for SWMM toolkit API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/21/2020
 ******************************************************************************
 */


#include <boost/test/unit_test.hpp>

#include "test_solver.hpp"

#define ERR_NONE 0
#define ERR_API_OUTBOUNDS 501
#define ERR_API_INPUTNOTOPEN 502
#define ERR_API_SIM_NRUNNING 503
#define ERR_API_WRONG_TYPE 504
#define ERR_API_OBJECT_INDEX 505

using namespace std;


// Non-Fixuture Unit Tests
BOOST_AUTO_TEST_SUITE(test_toolkit)

// Test Model Not Open
BOOST_AUTO_TEST_CASE(model_not_open) {
    int error, index;
    double val;
    double input_val = 0;
    double *result_array;
    int length;
    char id[] = "test";

    //Project
    error = swmm_getObjectIndex(SM_NODE, id, &index);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    //Gage
    error = swmm_getGagePrecip(0, SM_TOTALPRECIP, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_setGagePrecip(0, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);


    //Subcatchment
    error = swmm_getSubcatchParam(0, SM_WIDTH, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_setSubcatchParam(0, SM_WIDTH, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_getSubcatchResult(0, SM_SUBCRAIN, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);


    //Node
    error = swmm_getNodeParam(0, SM_INVERTEL, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_setNodeParam(0, SM_INVERTEL, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_setNodeInflow(0, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_getNodeResult(0, SM_TOTALINFLOW, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_setOutfallStage(0, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);


    //Link
    error = swmm_getLinkParam(0, SM_OFFSET1, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_setLinkParam(0, SM_OFFSET1, val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_getLinkResult(0, SM_LINKFLOW, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_setLinkSetting(0, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);


    //Pollutant
    error = swmm_getSubcatchPollut(0, SM_BUILDUP, &result_array, &length);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_getLinkPollut(0, SM_LINKQUAL, &result_array, &length);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_getNodePollut(0, SM_NODEQUAL, &result_array, &length);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_toolkitapi_fixture)

// Testing for Simulation Started Error
BOOST_FIXTURE_TEST_CASE(sim_started_check, FixtureBeforeStep) {
    int error;

    //Subcatchment
    error = swmm_setSubcatchParam(0, SM_WIDTH, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);


    //Node
    error = swmm_setNodeParam(0, SM_INVERTEL, 1);
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
    int length;

    //Gage
    error = swmm_getGagePrecip(100, SM_TOTALPRECIP, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);


    //Subcatchment
    error = swmm_getSubcatchParam(100, SM_WIDTH, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_setSubcatchParam(100, SM_WIDTH, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);


    //Node
    error = swmm_getNodeParam(100, SM_INVERTEL, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_setNodeParam(100, SM_INVERTEL, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_setOutfallStage(100, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);


    //Link
    error = swmm_getLinkParam(100, SM_OFFSET1, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_setLinkParam(100, SM_OFFSET1, 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);


    //Pollutant
    error = swmm_getSubcatchPollut(100, SM_BUILDUP, &result_array, &length);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getLinkPollut(100, SM_LINKQUAL, &result_array, &length);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getNodePollut(100, SM_NODEQUAL, &result_array, &length);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
}


// Testing for invalid parameter key
BOOST_FIXTURE_TEST_CASE(key_bounds_check, FixtureOpenClose) {
    int error;
    double val;
    char* error_msg=new char[256];

    //Error codes
    swmm_getAPIError(341, &error_msg);
    BOOST_CHECK_EQUAL(error_msg, "\n  ERROR 341: cannot open scratch RDII interface file.");
    delete[] error_msg;

    //Subcatchment
    error = swmm_getSubcatchParam(0, static_cast<SM_SubcProperty>(100), &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

    error = swmm_setSubcatchParam(0, static_cast<SM_SubcProperty>(100), 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);


    //Node
    error = swmm_getNodeParam(0, static_cast<SM_NodeProperty>(100), &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

    error = swmm_setNodeParam(0, static_cast<SM_NodeProperty>(100), 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);


    //Link
    error = swmm_getLinkParam(0, static_cast<SM_LinkProperty>(100), &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

    error = swmm_setLinkParam(0, static_cast<SM_LinkProperty>(100), 1);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
}


// Testing for Project Settings after Open
BOOST_FIXTURE_TEST_CASE(project_info, FixtureOpenClose){

    int error, index, year, month, day, hour, minute, second;
    char id[] = "14";

    //Project
    error = swmm_getObjectIndex(SM_NODE, id, &index);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(index, 3);

    //Simulation times

    error = swmm_getSimulationDateTime(SM_STARTDATE,
                                       &year,
                                       &month,
                                       &day,
                                       &hour,
                                       &minute,
                                       &second
                                     );
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(year, 1998);
    BOOST_CHECK_EQUAL(month, 1);
    BOOST_CHECK_EQUAL(day, 1);
    BOOST_CHECK_EQUAL(hour, 0);
    BOOST_CHECK_EQUAL(minute, 0);
    BOOST_CHECK_EQUAL(second, 0);

    error = swmm_getSimulationDateTime(SM_ENDDATE,
                                       &year,
                                       &month,
                                       &day,
                                       &hour,
                                       &minute,
                                       &second
                                     );
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(year, 1998);
    BOOST_CHECK_EQUAL(month, 1);
    BOOST_CHECK_EQUAL(day, 2);
    BOOST_CHECK_EQUAL(hour, 12);
    BOOST_CHECK_EQUAL(minute, 0);
    BOOST_CHECK_EQUAL(second, 0);

    error = swmm_getSimulationDateTime(SM_REPORTDATE,
                                       &year,
                                       &month,
                                       &day,
                                       &hour,
                                       &minute,
                                       &second
                                     );
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(year, 1998);
    BOOST_CHECK_EQUAL(month, 1);
    BOOST_CHECK_EQUAL(day, 1);
    BOOST_CHECK_EQUAL(hour, 0);
    BOOST_CHECK_EQUAL(minute, 0);
    BOOST_CHECK_EQUAL(second, 0);

    // Adjust start time
    year = 1997;
    month = 2;
    day = 2;
    hour = 1;
    minute = 1;
    second = 15;

    swmm_setSimulationDateTime(SM_STARTDATE, year, month,
                               day, hour, minute, second);
    BOOST_REQUIRE(error == ERR_NONE);

    error = swmm_getSimulationDateTime(SM_STARTDATE,
                                       &year,
                                       &month,
                                       &day,
                                       &hour,
                                       &minute,
                                       &second
                                     );
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(year, 1997);
    BOOST_CHECK_EQUAL(month, 2);
    BOOST_CHECK_EQUAL(day, 2);
    BOOST_CHECK_EQUAL(hour, 1);
    BOOST_CHECK_EQUAL(minute, 1);
    BOOST_CHECK_EQUAL(second, 15);

    swmm_setSimulationDateTime(SM_REPORTDATE, year, month,
                               day, hour, minute, second);
    BOOST_REQUIRE(error == ERR_NONE);

    error = swmm_getSimulationDateTime(SM_REPORTDATE,
                                       &year,
                                       &month,
                                       &day,
                                       &hour,
                                       &minute,
                                       &second
                                     );
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(year, 1997);
    BOOST_CHECK_EQUAL(month, 2);
    BOOST_CHECK_EQUAL(day, 2);
    BOOST_CHECK_EQUAL(hour, 1);
    BOOST_CHECK_EQUAL(minute, 1);
    BOOST_CHECK_EQUAL(second, 15);

    year += 1;

    swmm_setSimulationDateTime(SM_ENDDATE, year, month,
                               day, hour, minute, second);
    BOOST_REQUIRE(error == ERR_NONE);

    error = swmm_getSimulationDateTime(SM_ENDDATE,
                                       &year,
                                       &month,
                                       &day,
                                       &hour,
                                       &minute,
                                       &second
                                     );
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(year, 1998);
    BOOST_CHECK_EQUAL(month, 2);
    BOOST_CHECK_EQUAL(day, 2);
    BOOST_CHECK_EQUAL(hour, 1);
    BOOST_CHECK_EQUAL(minute, 1);
    BOOST_CHECK_EQUAL(second, 15);

}



#define DATA_PATH_INP_METRIC "test_ex1_metric.inp"

struct FixtureMetric : FixtureOpenClose {
    FixtureMetric() : FixtureOpenClose(DATA_PATH_INP_METRIC) {}
};

// Testing for Simulation parameters after Open
BOOST_FIXTURE_TEST_CASE(project_info_metric, FixtureMetric) {
    // int paramType[10] =    {SM_ROUTESTEP, SM_MINROUTESTEP, SM_LENGTHSTEP,
    //                         SM_STARTDRYDAYS, SM_COURANTFACTOR, SM_MINSURFAREA,
    //                         SM_MINSLOPE, SM_HEADTOL,
    //                         SM_SYSFLOWTOL, SM_LATFLOWTOL};
    // double expectedValue[10] = {60., 0.5, 0.01,
    //                             5., 0.75, 1.2,
    //                             0.001/100, 0.015,
    //                             5/100, 5/100};
    int error;
    double value;
    //Simulation parameters
    error = swmm_getSimulationParam(SM_ROUTESTEP, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 60);

    error = swmm_getSimulationParam(SM_MINROUTESTEP, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 0.5);

    error = swmm_getSimulationParam(SM_LENGTHSTEP, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 0.01);

    error = swmm_getSimulationParam(SM_STARTDRYDAYS, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 5);

    error = swmm_getSimulationParam(SM_COURANTFACTOR, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 0.75);

    error = swmm_getSimulationParam(SM_MINSLOPE, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 0.001/100);

    error = swmm_getSimulationParam(SM_MINSURFAREA, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 1.2);

    error = swmm_getSimulationParam(SM_HEADTOL, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 0.015);
}


#define DATA_PATH_INP_METRIC_DW "test_ex1_metric_dynwave.inp"

struct FixtureMetricDW : FixtureOpenClose {
    FixtureMetricDW() : FixtureOpenClose(DATA_PATH_INP_METRIC_DW) {}
};

// Testing for Simulation parameters after Open on DW routing
BOOST_FIXTURE_TEST_CASE(project_info_metric_dw, FixtureMetricDW){
    // int paramType[10] =    {SM_ROUTESTEP, SM_MINROUTESTEP, SM_LENGTHSTEP,
    //                         SM_STARTDRYDAYS, SM_COURANTFACTOR, SM_MINSURFAREA,
    //                         SM_MINSLOPE, SM_HEADTOL,
    //                         SM_SYSFLOWTOL, SM_LATFLOWTOL};
    // double expectedValue[10] = {60., 0.5, 0.01,
    //                             5., 0.75, 1.2,
    //                             0.001/100, 0.015,
    //                             5/100, 5/100};
    int error;
    double value;
    //Simulation parameters
    error = swmm_getSimulationParam(SM_ROUTESTEP, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 60);

    error = swmm_getSimulationParam(SM_MINROUTESTEP, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 0.5);

    error = swmm_getSimulationParam(SM_LENGTHSTEP, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 0.01);

    error = swmm_getSimulationParam(SM_STARTDRYDAYS, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 5);

    error = swmm_getSimulationParam(SM_COURANTFACTOR, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 0.75);

    error = swmm_getSimulationParam(SM_MINSLOPE, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 0.001/100);

    error = swmm_getSimulationParam(SM_MINSURFAREA, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 1.2);

    error = swmm_getSimulationParam(SM_HEADTOL, &value);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(value, 0.015);
}

// Testing for count Objects after Open
BOOST_FIXTURE_TEST_CASE(count_objects, FixtureOpenClose){

    int error, count;

    //Count objects
    error = swmm_countObjects(SM_GAGE, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 1);

    error = swmm_countObjects(SM_SUBCATCH, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 8);

    error = swmm_countObjects(SM_NODE, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 14);

    error = swmm_countObjects(SM_LINK, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 13);

    error = swmm_countObjects(SM_POLLUT, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 2);

    error = swmm_countObjects(SM_LANDUSE, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 2);

    error = swmm_countObjects(SM_TIMEPATTERN, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 0);

    error = swmm_countObjects(SM_CURVE, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 0);

    error = swmm_countObjects(SM_TSERIES, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 1);

    error = swmm_countObjects(SM_CONTROL, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 0);


    error = swmm_countObjects(SM_TRANSECT, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 0);

    error = swmm_countObjects(SM_AQUIFER, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 0);

    error = swmm_countObjects(SM_UNITHYD, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 0);

    error = swmm_countObjects(SM_SNOWMELT, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 0);

    error = swmm_countObjects(SM_SHAPE, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 0);

    error = swmm_countObjects(SM_LID, &count);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(count, 0);

    //return error
    error = swmm_countObjects(static_cast<SM_ObjectType>(999), &count);
    BOOST_REQUIRE(error == ERR_API_OUTBOUNDS);
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
    error = swmm_getSubcatchResult(100, SM_SUBCRAIN, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getSubcatchResult(0, static_cast<SM_SubcResult>(100), &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);


    // Node
    error = swmm_getNodeResult(100, SM_TOTALINFLOW, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getNodeResult(0, static_cast<SM_NodeResult>(100), &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

    error = swmm_setNodeInflow(100, input_val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);


    //Link
    error = swmm_getLinkResult(100, SM_LINKFLOW, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getLinkResult(0, static_cast<SM_LinkResult>(100), &val);
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
    std::vector<double> subc_vec =
        {0.8,
         0.0,
         0.125,
         0.0,
         4.3365,
         0.0};

    std::vector<double> node_vec =
        {0.4793,
         0.4793,
         0.0,
         0.0,
         0.0,
         0.1730,
         1010.1730,
         0.4793};

    std::vector<double> link_vec =
        {1.12519,
         0.302433,
         80.1717,
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
            BOOST_CHECK(check_cdd_double(test_vec, subc_vec, 3));

            test_vec.clear();


            // Node
            for (SM_NodeResult property = SM_TOTALINFLOW; property <= SM_LATINFLOW;
                property = SM_NodeResult(property + 1))
            {
                error = swmm_getNodeResult(nde_ind, property, &val);
                BOOST_REQUIRE(error == ERR_NONE);
                test_vec.push_back(val);
            }
            BOOST_CHECK(check_cdd_double(test_vec, node_vec, 3));

            test_vec.clear();


            // Link
            for (SM_LinkResult property = SM_LINKFLOW; property <= SM_FROUDE;
                property = SM_LinkResult(property + 1))
            {
                error = swmm_getLinkResult(lnk_ind, property, &val);
                BOOST_REQUIRE(error == ERR_NONE);
                test_vec.push_back(val);
            }
            BOOST_CHECK(check_cdd_double(test_vec, link_vec, 3));

        }
        step_ind+=1;
    }while (elapsedTime != 0 && !error);
    BOOST_REQUIRE(error == ERR_NONE);
    swmm_end();
}


// Testing Results Getters (Before End Simulation)
// BOOST_FIXTURE_TEST_CASE(get_results_after_sim, FixtureBeforeEnd){
//     int error;
//     int subc_ind;

//     char subid[] = "1";

//     // Subcatchment
//     SM_SubcatchStats *subc_stats = NULL;

//     error = swmm_getObjectIndex(SM_SUBCATCH, subid, &subc_ind);
//     BOOST_REQUIRE(error == ERR_NONE);

//     error = swmm_getSubcatchStats(subc_ind, &subc_stats);
//     BOOST_CHECK_EQUAL(error, ERR_NONE);

//     BOOST_CHECK_SMALL(subc_stats->runon - 0.0, 0.0001);
//     BOOST_CHECK_SMALL(subc_stats->infil - 42088, 1.0);
//     BOOST_CHECK_SMALL(subc_stats->runoff - 53781, 1.0);
//     BOOST_CHECK_SMALL(subc_stats->maxFlow - 4.6561, 0.0001);
//     BOOST_CHECK_SMALL(subc_stats->precip - 2.65, 0.0001);
//     BOOST_CHECK_SMALL(subc_stats->evap - 0.0, 0.0001);

//     swmm_freeMemory(subc_stats);
// }

BOOST_AUTO_TEST_SUITE_END()
