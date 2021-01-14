/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.1.13
 Module:       test_lid_results.cpp
 Description:  tests for SWMM LID API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/21/2020
 ******************************************************************************
*/



#include <math.h>
#include <boost/test/unit_test.hpp>

#include "test_lid.hpp"

#define ERR_NONE 0

using namespace std;


BOOST_AUTO_TEST_SUITE(test_lid_toolkitapi_fixture)

    // Testing Results Getters (During Simulation) BC
    BOOST_FIXTURE_TEST_CASE(result_check_LID_BC, Fixture_LID_Results)
    {
        open_swmm_model(0);
        int error, sub_index, lid_index;
        double elapsed_time = 0.0;
        int int_value = 0;
        double db_value = 0.0;

        vector<double> revised_subcatchment_runoff;

        string lid = string("BC");
        string subcatch = string("wBC");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Lid Control
        // Surface layer
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 6);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.25);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);

        // Soil layer
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_THICKNESS, 12);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_POROSITY, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSAT, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSLOPE, 10.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_SUCTION, 3.5);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 3.5, 0.0001);

        // Storage layer
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_THICKNESS, 12);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_KSAT, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        // Drain layer
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_COEFF, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_EXPON, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_OFFSET, 6.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_DELAY, 6.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HOPEN, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        // Lid Unit
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 50.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 10.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 25.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMPERV, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 100);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        swmm_start(0);
        do
        {
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);
            error = swmm_getLidCOverflow(lid_index, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_EQUAL(int_value, 0);

            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 12, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 3.5, 0.0001);

            error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 12, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 100);
            error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 1);
            error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_step(&elapsed_time);
            BOOST_REQUIRE(error == ERR_NONE);
            error = swmm_getSubcatchResult(sub_index, SM_SUBCRUNOFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            revised_subcatchment_runoff.push_back(round(db_value * 100000.0) / 100000.0);
        }while (elapsed_time != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
        swmm_end();

        BOOST_CHECK_EQUAL_COLLECTIONS(revised_subcatchment_runoff.begin(), revised_subcatchment_runoff.end(),
                              subcatchment_runoff.begin(), subcatchment_runoff.end());
    }

    // Testing Results Getters (During Simulation) GR
    BOOST_FIXTURE_TEST_CASE(result_check_LID_GR, Fixture_LID_Results)
    {
        open_swmm_model(1);
        int error, sub_index, lid_index;
        double elapsed_time = 0.0;
        int int_value = 0;
        double db_value = 0.0;
        vector<double> revised_subcatchment_runoff;

        string lid = string("GR");
        string subcatch = string("wGR");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Lid Control
        // Surface layer
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 6);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.25);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);

        // Soil layer
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_THICKNESS, 12);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_POROSITY, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSAT, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSLOPE, 10.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_SUCTION, 3.5);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 3.5, 0.0001);

        // Drainmat layer get/set
        error = swmm_setLidCParam(lid_index, SM_DRAINMAT, SM_THICKNESS, 3);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAINMAT, SM_VOIDFRAC, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAINMAT, SM_ROUGHNESS, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 3, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.49 - 0, 0.0001);

        // Lid Unit
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 50.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 10.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 25.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMPERV, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 100);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        swmm_start(0);
        do
        {
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);
            error = swmm_getLidCOverflow(lid_index, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_EQUAL(int_value, 0);

            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 12, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 3.5, 0.0001);

            error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 3, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_ROUGHNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_ALPHA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.49 - 0, 0.0001);

            error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 100);
            error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 1);
            error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_step(&elapsed_time);
            BOOST_REQUIRE(error == ERR_NONE);
            error = swmm_getSubcatchResult(sub_index, SM_SUBCRUNOFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);

            revised_subcatchment_runoff.push_back(round(db_value * 100000.0) / 100000.0);
        }while (elapsed_time != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
        swmm_end();

        BOOST_CHECK_EQUAL_COLLECTIONS(revised_subcatchment_runoff.begin(), revised_subcatchment_runoff.end(),
                              subcatchment_runoff.begin(), subcatchment_runoff.end());
    }

    // Testing Results Getters (During Simulation) IT
    BOOST_FIXTURE_TEST_CASE(result_check_LID_IT, Fixture_LID_Results)
    {
        open_swmm_model(2);
        int error, sub_index, lid_index;
        double elapsed_time = 0.0;
        int int_value = 0;
        double db_value = 0.0;
        vector<double> revised_subcatchment_runoff;

        string lid = string("IT");
        string subcatch = string("wIT");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Lid Control
        // Surface layer
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 6);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.25);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);

        // Storage layer
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_THICKNESS, 12);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_KSAT, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        // Drain layer
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_COEFF, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_EXPON, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_OFFSET, 6.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_DELAY, 6.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HOPEN, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        // Lid Unit
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 50.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 10.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 25.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMPERV, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 100);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        swmm_start(0);
        do
        {
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);
            error = swmm_getLidCOverflow(lid_index, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_EQUAL(int_value, 0);

            error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 12, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 100);
            error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 1);
            error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_step(&elapsed_time);
            BOOST_REQUIRE(error == ERR_NONE);
            error = swmm_getSubcatchResult(sub_index, SM_SUBCRUNOFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);

            revised_subcatchment_runoff.push_back(round(db_value * 100000.0) / 100000.0);
        }while (elapsed_time != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
        swmm_end();

        BOOST_CHECK_EQUAL_COLLECTIONS(revised_subcatchment_runoff.begin(), revised_subcatchment_runoff.end(),
                              subcatchment_runoff.begin(), subcatchment_runoff.end());
    }

    // Testing Results Getters (During Simulation) PP
    BOOST_FIXTURE_TEST_CASE(result_check_LID_PP, Fixture_LID_Results)
    {
        open_swmm_model(3);
        int error, sub_index, lid_index;
        double elapsed_time = 0.0;
        int int_value = 0;
        double db_value = 0.0;
        vector<double> revised_subcatchment_runoff;

        string lid = string("PP");
        string subcatch = string("wPP");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Lid Control
        // Surface layer
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 6);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.25);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);

        // Pavement layer get/set check
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_THICKNESS, 6);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_VOIDFRAC, 0.15);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_IMPERVFRAC, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_KSAT, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_CLOGFACTOR, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_REGENDAYS, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_REGENDEGREE, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.15, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_IMPERVFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_REGENDAYS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_REGENDEGREE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        // Soil layer
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_THICKNESS, 12);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_POROSITY, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSAT, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSLOPE, 10.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_SUCTION, 3.5);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 3.5, 0.0001);

        // Storage layer
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_THICKNESS, 12);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_KSAT, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        // Drain layer
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_COEFF, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_EXPON, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_OFFSET, 6.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_DELAY, 6.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HOPEN, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        // Lid Unit
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 50.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 10.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 25.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMPERV, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 100);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        swmm_start(0);
        do
        {
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);
            error = swmm_getLidCOverflow(lid_index, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_EQUAL(int_value, 0);

            error = swmm_getLidCParam(lid_index, SM_PAVE, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_PAVE, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.15, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_PAVE, SM_IMPERVFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_PAVE, SM_KSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 100, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_PAVE, SM_CLOGFACTOR, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_PAVE, SM_REGENDAYS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_PAVE, SM_REGENDEGREE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 12, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 3.5, 0.0001);

            error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 12, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 100);
            error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 1);
            error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_step(&elapsed_time);
            BOOST_REQUIRE(error == ERR_NONE);
            error = swmm_getSubcatchResult(sub_index, SM_SUBCRUNOFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);

            revised_subcatchment_runoff.push_back(round(db_value * 100000.0) / 100000.0);
        }while (elapsed_time != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
        swmm_end();

        BOOST_CHECK_EQUAL_COLLECTIONS(revised_subcatchment_runoff.begin(), revised_subcatchment_runoff.end(),
                              subcatchment_runoff.begin(), subcatchment_runoff.end());
    }

    // Testing Results Getters (During Simulation) RB
    BOOST_FIXTURE_TEST_CASE(result_check_LID_RB, Fixture_LID_Results)
    {
        open_swmm_model(4);
        int error, sub_index, lid_index;
        double elapsed_time = 0.0;
        int int_value = 0;
        double db_value = 0.0;
        vector<double> revised_subcatchment_runoff;

        string lid = string("RB");
        string subcatch = string("wRB");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Lid Control
        // Storage layer
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_THICKNESS, 48);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_KSAT, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 48, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        // Drain layer
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_COEFF, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_EXPON, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_OFFSET, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_DELAY, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HOPEN, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        // Lid Unit
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 12.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 10.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 25.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMPERV, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 100);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        swmm_start(0);
        do
        {
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 48, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 100);
            error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 1);
            error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 12.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_step(&elapsed_time);
            BOOST_REQUIRE(error == ERR_NONE);
            error = swmm_getSubcatchResult(sub_index, SM_SUBCRUNOFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);

            revised_subcatchment_runoff.push_back(round(db_value * 100000.0) / 100000.0);
        }while (elapsed_time != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
        swmm_end();

        BOOST_CHECK_EQUAL_COLLECTIONS(revised_subcatchment_runoff.begin(), revised_subcatchment_runoff.end(),
                              subcatchment_runoff.begin(), subcatchment_runoff.end());
    }

    // Testing Results Getters (During Simulation) RG
    BOOST_FIXTURE_TEST_CASE(result_check_LID_RG, Fixture_LID_Results)
    {
        open_swmm_model(5);
        int error, sub_index, lid_index;
        double elapsed_time = 0.0;
        int int_value = 0;
        double db_value = 0.0;
        vector<double> revised_subcatchment_runoff;

        string lid = string("RG");
        string subcatch = string("wRG");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Lid Control
        // Surface layer
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 6);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.25);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);

        // Soil layer
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_THICKNESS, 12);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_POROSITY, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSAT, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSLOPE, 10.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_SUCTION, 3.5);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 3.5, 0.0001);

        // Storage layer
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_THICKNESS, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_KSAT, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        // storage layer thickness was originally zero
        // void frac is default 1.0
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        // Lid Unit
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 50.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 10.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 25.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMPERV, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 100);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        swmm_start(0);
        do
        {
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);
            error = swmm_getLidCOverflow(lid_index, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_EQUAL(int_value, 0);

            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 12, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 3.5, 0.0001);

            error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            // storage layer thickness was originally zero
            // void frac is default 1.0
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0., 0.0001);

            error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 100);
            error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 1);
            error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_step(&elapsed_time);
            BOOST_REQUIRE(error == ERR_NONE);
            error = swmm_getSubcatchResult(sub_index, SM_SUBCRUNOFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);

            revised_subcatchment_runoff.push_back(round(db_value * 100000.0) / 100000.0);
        }while (elapsed_time != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
        swmm_end();

        BOOST_CHECK_EQUAL_COLLECTIONS(revised_subcatchment_runoff.begin(), revised_subcatchment_runoff.end(),
                              subcatchment_runoff.begin(), subcatchment_runoff.end());
    }

    // Testing Results Getters (During Simulation) SWALE
    BOOST_FIXTURE_TEST_CASE(result_check_LID_SWALE, Fixture_LID_Results)
    {
        open_swmm_model(6);
        int error, sub_index, lid_index;
        double elapsed_time = 0.0;
        int int_value = 0;
        double db_value = 0.0;
        vector<double> revised_subcatchment_runoff;

        string lid = string("SWALE");
        string subcatch = string("wSWALE");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Lid Control
        // Surface layer
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 12);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.25);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SIDESLOPE, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SIDESLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);

        // Lid Unit
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 10);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 500.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 100.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 25.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMPERV, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 10);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 500.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        swmm_start(0);
        do
        {
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 12, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SIDESLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);
            // always true for veg. swale
            error = swmm_getLidCOverflow(lid_index, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_EQUAL(int_value, 1);

            error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 10);
            error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 1);
            error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 500.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 100.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_step(&elapsed_time);
            BOOST_REQUIRE(error == ERR_NONE);
            error = swmm_getSubcatchResult(sub_index, SM_SUBCRUNOFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);

            revised_subcatchment_runoff.push_back(round(db_value * 100000.0) / 100000.0);
        }while (elapsed_time != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
        swmm_end();

        BOOST_CHECK_EQUAL_COLLECTIONS(revised_subcatchment_runoff.begin(), revised_subcatchment_runoff.end(),
                              subcatchment_runoff.begin(), subcatchment_runoff.end());
    }

    // Testing Results Getters (During Simulation) RD
    BOOST_FIXTURE_TEST_CASE(result_check_LID_RD, Fixture_LID_Results)
    {
        open_swmm_model(7);
        int error, sub_index, lid_index;
        double elapsed_time = 0.0;
        int int_value = 0;
        double db_value = 0.0;
        vector<double> revised_subcatchment_runoff;

        string lid = string("RD");
        string subcatch = string("wRD");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Lid Control
        // Surface layer
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 6);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.25);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);

        // Drain layer
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_COEFF, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_EXPON, 0.5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_OFFSET, 6.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_DELAY, 6.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HOPEN, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        // if storage thickness is zero (no storage layer since it is roof disconnection)
        // the offset is changed to zero
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        // Lid Unit
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 50.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 10.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 25.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMPERV, 0.0);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 100);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        swmm_start(0);
        do
        {
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 1.49, 0.0001);
            error = swmm_getLidCOverflow(lid_index, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_EQUAL(int_value, 0);

            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
            // if storage thickness is zero (no storage layer since it is roof disconnection)
            // the offset is changed to zero
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 6.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 100);
            error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_REQUIRE(int_value == 1);
            error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 50.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 10.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 25.0, 0.0001);
            error = swmm_getLidUParam(sub_index, 0, SM_FROMPERV, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

            error = swmm_step(&elapsed_time);
            BOOST_REQUIRE(error == ERR_NONE);
            error = swmm_getSubcatchResult(sub_index, SM_SUBCRUNOFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);

            revised_subcatchment_runoff.push_back(round(db_value * 100000.0) / 100000.0);
        }while (elapsed_time != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
        swmm_end();

        BOOST_CHECK_EQUAL_COLLECTIONS(revised_subcatchment_runoff.begin(), revised_subcatchment_runoff.end(),
                              subcatchment_runoff.begin(), subcatchment_runoff.end());
    }

BOOST_AUTO_TEST_SUITE_END()
