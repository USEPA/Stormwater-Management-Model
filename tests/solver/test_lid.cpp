/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.1.13
 Module:       test_lid.cpp
 Description:  tests for SWMM LID API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/21/2020
 ******************************************************************************
*/


#define BOOST_TEST_MODULE "lid"


#include <boost/test/included/unit_test.hpp>

#include "test_lid.hpp"


#define ERR_NONE 0
#define ERR_API_OUTBOUNDS 501
#define ERR_API_INPUTNOTOPEN 502
#define ERR_API_SIM_NRUNNING 503
#define ERR_API_OBJECT_INDEX 505
#define ERR_API_UNDEFINED_LID 511

using namespace std;


void open_swmm_lid(int lid_type, bool revised=false)
{
    if (revised)
    {
        switch(lid_type)
        {
            case 0:
                swmm_open((char *)DATA_PATH_INP_LID_BC_REVISED, (char *)DATA_PATH_RPT_LID_BC_REVISED, (char *)DATA_PATH_OUT_LID_BC_REVISED);
                break;
            case 1:
                swmm_open((char *)DATA_PATH_INP_LID_GR_REVISED, (char *)DATA_PATH_RPT_LID_GR_REVISED, (char *)DATA_PATH_OUT_LID_GR_REVISED);
                break;
            case 2:
                swmm_open((char *)DATA_PATH_INP_LID_IT_REVISED, (char *)DATA_PATH_RPT_LID_IT_REVISED, (char *)DATA_PATH_OUT_LID_IT_REVISED);
                break;
            case 3:
                swmm_open((char *)DATA_PATH_INP_LID_PP_REVISED, (char *)DATA_PATH_RPT_LID_PP_REVISED, (char *)DATA_PATH_OUT_LID_PP_REVISED);
                break;
            case 4:
                swmm_open((char *)DATA_PATH_INP_LID_RB_REVISED, (char *)DATA_PATH_RPT_LID_RB_REVISED, (char *)DATA_PATH_OUT_LID_RB_REVISED);
                break;
            case 5:
                swmm_open((char *)DATA_PATH_INP_LID_RG_REVISED, (char *)DATA_PATH_RPT_LID_RG_REVISED, (char *)DATA_PATH_OUT_LID_RG_REVISED);
                break;
            case 6:
                swmm_open((char *)DATA_PATH_INP_LID_SWALE_REVISED, (char *)DATA_PATH_RPT_LID_SWALE_REVISED, (char *)DATA_PATH_OUT_LID_SWALE_REVISED);
                break;
            case 7:
                swmm_open((char *)DATA_PATH_INP_LID_RD_REVISED, (char *)DATA_PATH_RPT_LID_RD_REVISED, (char *)DATA_PATH_OUT_LID_RD_REVISED);
                break;
            default:
                break;
        }
    }
    else
    {
        switch(lid_type)
        {
            case 0:
                swmm_open((char *)DATA_PATH_INP_LID_BC, (char *)DATA_PATH_RPT_LID_BC, (char *)DATA_PATH_OUT_LID_BC);
                break;
            case 1:
                swmm_open((char *)DATA_PATH_INP_LID_GR, (char *)DATA_PATH_RPT_LID_GR, (char *)DATA_PATH_OUT_LID_GR);
                break;
            case 2:
                swmm_open((char *)DATA_PATH_INP_LID_IT, (char *)DATA_PATH_RPT_LID_IT, (char *)DATA_PATH_OUT_LID_IT);
                break;
            case 3:
                swmm_open((char *)DATA_PATH_INP_LID_PP, (char *)DATA_PATH_RPT_LID_PP, (char *)DATA_PATH_OUT_LID_PP);
                break;
            case 4:
                swmm_open((char *)DATA_PATH_INP_LID_RB, (char *)DATA_PATH_RPT_LID_RB, (char *)DATA_PATH_OUT_LID_RB);
                break;
            case 5:
                swmm_open((char *)DATA_PATH_INP_LID_RG, (char *)DATA_PATH_RPT_LID_RG, (char *)DATA_PATH_OUT_LID_RG);
                break;
            case 6:
                swmm_open((char *)DATA_PATH_INP_LID_SWALE, (char *)DATA_PATH_RPT_LID_SWALE, (char *)DATA_PATH_OUT_LID_SWALE);
                break;
            case 7:
                swmm_open((char *)DATA_PATH_INP_LID_RD, (char *)DATA_PATH_RPT_LID_RD, (char *)DATA_PATH_OUT_LID_RD);
                break;
            default:
                break;
        }
    }
}


// Non-Fixuture Unit Tests
BOOST_AUTO_TEST_SUITE(test_lid_toolkitapi)

    // Test Model Not Open
    BOOST_AUTO_TEST_CASE(model_not_open)
    {
        int error;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        //Lid Control
        error = swmm_getLidCOverflow(0, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
        error = swmm_getLidCParam(0, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
        error = swmm_setLidCParam(0, SM_SURFACE, SM_THICKNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

        //Lid Unit
        error = swmm_getLidUCount(0, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
        error = swmm_getLidUParam(0, 0, SM_UNITAREA, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
        error = swmm_setLidUParam(0, 0, SM_UNITAREA, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
        error = swmm_getLidUOption(0, 0, SM_INDEX, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
        error = swmm_setLidUOption(0, 0, SM_INDEX, int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
        error = swmm_getLidUFluxRates(0, 0, SM_SURFACE, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
        error = swmm_getLidUResult(0, 0, SM_INFLOW, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

        //Lid Group
        error = swmm_getLidGResult(0, SM_INFLOW, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);
    }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_lid_toolkitapi_fixture)

    // Testing for Simulation Started Error
    BOOST_FIXTURE_TEST_CASE(sim_started_check, FixtureBeforeStep_LID)
    {
        open_swmm_model(0);
        int error;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        //Lid Control
        error = swmm_getLidCOverflow(0, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_getLidCParam(0, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);

        //Lid Surface
        error = swmm_setLidCParam(0, SM_SURFACE, SM_THICKNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_SURFACE, SM_VOIDFRAC, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_SURFACE, SM_ROUGHNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidCParam(0, SM_SURFACE, SM_SURFSLOPE, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_SURFACE, SM_SIDESLOPE, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_SURFACE, SM_ALPHA, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

        //Lid Soil
        error = swmm_setLidCParam(0, SM_SOIL, SM_THICKNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_SOIL, SM_POROSITY, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_SOIL, SM_FIELDCAP, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_SOIL, SM_WILTPOINT, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_SOIL, SM_SUCTION, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_SOIL, SM_KSAT, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_SOIL, SM_KSLOPE, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_SOIL, SM_CLOGFACTOR, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

        //Lid Storage
        error = swmm_setLidCParam(0, SM_STOR, SM_THICKNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_STOR, SM_VOIDFRAC, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_STOR, SM_KSAT, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_STOR, SM_CLOGFACTOR, db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidCParam(0, SM_STOR, SM_ROUGHNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

        //Lid Pavement
        error = swmm_setLidCParam(0, SM_PAVE, SM_THICKNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_PAVE, SM_VOIDFRAC, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_PAVE, SM_IMPERVFRAC, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_PAVE, SM_KSAT, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_PAVE, SM_CLOGFACTOR, db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidCParam(0, SM_PAVE, SM_REGENDAYS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_PAVE, SM_REGENDEGREE, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_PAVE, SM_WILTPOINT, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

        //Lid Drain
        error = swmm_setLidCParam(0, SM_DRAIN, SM_COEFF, db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidCParam(0, SM_DRAIN, SM_EXPON, db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidCParam(0, SM_DRAIN, SM_OFFSET, db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidCParam(0, SM_DRAIN, SM_DELAY, db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidCParam(0, SM_DRAIN, SM_HOPEN, db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidCParam(0, SM_DRAIN, SM_HCLOSE, db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidCParam(0, SM_DRAIN, SM_CLOGFACTOR, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

        //Lid DrainMat
        error = swmm_setLidCParam(0, SM_DRAINMAT, SM_THICKNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_DRAINMAT, SM_VOIDFRAC, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidCParam(0, SM_DRAINMAT, SM_ROUGHNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidCParam(0, SM_DRAINMAT, SM_CLOGFACTOR, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

        //Lid Unit
        error = swmm_getLidUCount(0, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_getLidUParam(0, 0, SM_UNITAREA, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidUParam(0, 0, SM_UNITAREA, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_getLidUOption(0, 0, SM_INDEX, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidUOption(0, 0, SM_INDEX, int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidUOption(0, 0, SM_NUMBER, int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidUOption(0, 0, SM_TOPERV, int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
        error = swmm_setLidUOption(0, 0, SM_DRAINSUB, int_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_setLidUOption(0, 0, SM_DRAINNODE, int_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_getLidUFluxRates(0, 0, SM_SURFACE, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_getLidUResult(0, 0, SM_INFLOW, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);

        //Lid Group
        error = swmm_getLidGResult(0, SM_PERVAREA, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
    }


    // Testing for invalid object index
    BOOST_FIXTURE_TEST_CASE(object_bounds_check, FixtureOpenClose_LID)
    {
        open_swmm_model(0);
        int error;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        //Lid Control
        error = swmm_getLidCOverflow(1, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_getLidCParam(1, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_setLidCParam(1, SM_SURFACE, SM_THICKNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

        //Lid Unit
        error = swmm_getLidUCount(2, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_getLidUParam(2, 0, SM_UNITAREA, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_setLidUParam(2, 0, SM_UNITAREA, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_getLidUOption(2, 0, SM_INDEX, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_setLidUOption(2, 0, SM_INDEX, int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_getLidUFluxRates(2, 0, SM_SURFACE, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_getLidUResult(2, 0, SM_INFLOW, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

        //Lid Group
        error = swmm_getLidGResult(2, SM_PERVAREA, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    }

    // Testing for invalid parameter key
    BOOST_FIXTURE_TEST_CASE(key_bounds_check, FixtureOpenClose_LID)
    {
        open_swmm_model(0);
        int error;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        //Lid Control
        error = swmm_getLidCParam(0, SM_SURFACE, static_cast<SM_LidLayerProperty>(100), &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_setLidCParam(0, SM_SURFACE, static_cast<SM_LidLayerProperty>(100), db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_getLidCParam(0, static_cast<SM_LidLayer>(100), SM_THICKNESS, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_setLidCParam(0, static_cast<SM_LidLayer>(100), SM_THICKNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

        //Lid Unit
        error = swmm_getLidUParam(0, 0, static_cast<SM_LidUProperty>(100), &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_setLidUParam(0, 0, static_cast<SM_LidUProperty>(100), db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_getLidUOption(0, 0, static_cast<SM_LidUOptions>(100), &int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_setLidUOption(0, 0, static_cast<SM_LidUOptions>(100), int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_getLidUFluxRates(0, 0, static_cast<SM_LidLayer>(100), &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_getLidUResult(0, 0, static_cast<SM_LidResult>(100), &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

        //Lid Group
        error = swmm_getLidGResult(0, static_cast<SM_LidResult>(100), &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    }

    // Testing for undefined lid usage
    BOOST_FIXTURE_TEST_CASE(undefined_lid_check, FixtureOpenClose_LID)
    {
        open_swmm_model(0);
        int error;
        double db_value = 0;

        //Lid Unit
        error = swmm_getLidUResult(0, 1, SM_INFLOW, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);
        error = swmm_getLidUFluxRates(0, 1, SM_SURFACE, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);

        //Lid Group
        error = swmm_getLidGResult(1, SM_PERVAREA, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);
    }

    // Testing for Project Settings after Open
    BOOST_FIXTURE_TEST_CASE(project_lid_info, FixtureOpenClose_LID)
    {
        open_swmm_model(0);
        int error, index, sub_index, int_value;

        string id = string("BC");
        string subcatch = string("wBC");

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        //Project
        index = swmm_getObjectIndex(SM_LID, (char *)id.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(index, 0);

        error = swmm_getLidUCount(sub_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);
    }

    // Testing for Lid Control Bio Cell parameters get/set
    BOOST_FIXTURE_TEST_CASE(getset_lidcontrol_BC, FixtureOpenClose_LID)
    {
        open_swmm_model(0);
        int error, lid_index, sub_index;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        string bioCell = string("BC");
        string subcatch = string("wBC");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)bioCell.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Lid Control
        // Surface layer get/set check
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.9);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.9, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.05359, 0.0001);

        // Soil layer get/set
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_POROSITY, 0.3);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.3, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, 0.3);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.3, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, 0.3);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.3, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSAT, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSLOPE, 20);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 20, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 3.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_SUCTION, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        // Storage layer get/set check
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, 0.15);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.15, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_KSAT, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        // Drain layer get/set
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_COEFF, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_EXPON, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_OFFSET, 20);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 20, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_DELAY, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HOPEN, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        // Check for immediate overflow option
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 0);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);

        // Lid Unit
        error = swmm_getLidUCount(sub_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);

        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 1000);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1000, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 75, 0.0001);

        error = swmm_getLidUOption(sub_index, 0, SM_INDEX, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, lid_index);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 100);
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 11);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 11);

        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINSUB, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINNODE, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);
    }

    // Testing for Lid Control Green Roof parameters get/set
    BOOST_FIXTURE_TEST_CASE(getset_lidcontrol_GR, FixtureOpenClose_LID)
    {
        open_swmm_model(1);
        int error, lid_index, sub_index;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        string greenRoof = string("GR");
        string subcatch = string("wGR");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)greenRoof.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Surface layer get/set check
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.9);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.9, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.05359, 0.0001);

        // Soil layer get/set
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_POROSITY, 0.3);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.3, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, 0.3);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.3, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, 0.3);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.3, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSAT, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSLOPE, 20);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 20, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 3.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_SUCTION, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        // Drainmat layer get/set
        error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 3, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAINMAT, SM_THICKNESS, 11);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 11, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAINMAT, SM_VOIDFRAC, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAINMAT, SM_ROUGHNESS, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAINMAT, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.05359 - 0, 0.0001);

        // Check for immediate overflow option
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 0);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);

        // Lid Unit
        error = swmm_getLidUCount(sub_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);

        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 1000);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1000, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 75, 0.0001);

        error = swmm_getLidUOption(sub_index, 0, SM_INDEX, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, lid_index);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 100);
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 11);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 11);

        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINSUB, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINNODE, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);
    }

    // Testing for Lid Control Infiltration Trench parameters get/set
    BOOST_FIXTURE_TEST_CASE(getset_lidcontrol_IT, FixtureOpenClose_LID)
    {
        open_swmm_model(2);
        int error, lid_index, sub_index;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        string infilTrench = string("IT");
        string subcatch = string("wIT");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)infilTrench.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Surface layer get/set check
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.9);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.9, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.05359, 0.0001);

        // Storage layer get/set check
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, 0.15);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.15, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_KSAT, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        // Drain layer get/set
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_COEFF, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_EXPON, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_OFFSET, 20);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 20, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_DELAY, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HOPEN, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        // Check for immediate overflow option
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 0);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);

        // Lid Unit
        error = swmm_getLidUCount(sub_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);

        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 1000);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1000, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 75, 0.0001);

        error = swmm_getLidUOption(sub_index, 0, SM_INDEX, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, lid_index);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 100);
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 11);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 11);

        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINSUB, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINNODE, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);
    }

    // Testing for Lid Control Permeable Pavement parameters get/set
    BOOST_FIXTURE_TEST_CASE(getset_lidcontrol_PP, FixtureOpenClose_LID)
    {
        open_swmm_model(3);
        int error, lid_index, sub_index;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        string permPave = string("PP");
        string subcatch = string("wPP");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)permPave.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Surface layer get/set check
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.9);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.9, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.05359, 0.0001);

        // Pavement layer get/set check
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.15, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_VOIDFRAC, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_IMPERVFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_IMPERVFRAC, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_IMPERVFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_KSAT, 75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 75, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(db_value - 0, 0);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_CLOGFACTOR, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_REGENDAYS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(db_value - 0, 0);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_REGENDAYS, 1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_REGENDAYS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_REGENDEGREE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(db_value - 0, 0);
        error = swmm_setLidCParam(lid_index, SM_PAVE, SM_REGENDEGREE, 1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_PAVE, SM_REGENDEGREE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1, 0.0001);

        // Storage layer get/set check
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, 0.15);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.15, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_KSAT, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        // Drain layer get/set
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_COEFF, 1.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_EXPON, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_OFFSET, 20);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 20, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_DELAY, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HOPEN, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        // Check for immediate overflow option
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 0);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);

        // Lid Unit
        error = swmm_getLidUCount(sub_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);

        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 1000);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1000, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 75, 0.0001);

        error = swmm_getLidUOption(sub_index, 0, SM_INDEX, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, lid_index);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 100);
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 11);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 11);

        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINSUB, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINNODE, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);
    }

    // Testing for Lid Control Rain Barrel parameters get/set
    BOOST_FIXTURE_TEST_CASE(getset_lidcontrol_RB, FixtureOpenClose_LID)
    {
        open_swmm_model(4);
        int error, lid_index, sub_index;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        string lid = string("RB");
        string subcatch = string("wRB");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Storage layer get/set check
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 48, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        // default to value of 1.0 for rain barrels
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, 0.15);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1, 0.0001);

        // default to value of 0.0 for rain barrels
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_KSAT, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        // Drain layer get/set
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_COEFF, 2.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2.0, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_EXPON, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_OFFSET, 20);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 20, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_DELAY, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HOPEN, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        // Check for immediate overflow option
        // Always available to immediate overflow
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);

        // Lid Unit
        error = swmm_getLidUCount(sub_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);

        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 1000);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1000, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 75, 0.0001);

        error = swmm_getLidUOption(sub_index, 0, SM_INDEX, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, lid_index);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 100);
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 11);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 11);

        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINSUB, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINNODE, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);
    }

    // Testing for Lid Control Rain Garden parameters get/set
    BOOST_FIXTURE_TEST_CASE(getset_lidcontrol_RG, FixtureOpenClose_LID)
    {
        open_swmm_model(5);
        int error, lid_index, sub_index;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        string lid = string("RG");
        string subcatch = string("wRG");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Surface layer get/set check
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.9);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.9, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.05359, 0.0001);

        // Soil layer get/set
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_POROSITY, 0.3);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_POROSITY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.3, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, 0.3);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_FIELDCAP, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.3, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, 0.3);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_WILTPOINT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.3, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSAT, 0.1);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_KSLOPE, 20);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_KSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 20, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 3.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SOIL, SM_SUCTION, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SOIL, SM_SUCTION, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        // Storage layer get/set check
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        // storage layer thickness was originally zero
        // void frac is default 1.0
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, 0.15);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.15, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_KSAT, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_KSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, 0.75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_STOR, SM_CLOGFACTOR, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.75, 0.0001);

        // Check for immediate overflow option
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 0);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);

        // Lid Unit
        error = swmm_getLidUCount(sub_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);

        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 1000);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1000, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 75, 0.0001);

        error = swmm_getLidUOption(sub_index, 0, SM_INDEX, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, lid_index);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 100);
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 11);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 11);

        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINSUB, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINNODE, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);
    }

    // Testing for Lid Control Swale parameters get/set
    BOOST_FIXTURE_TEST_CASE(getset_lidcontrol_SWALE, FixtureOpenClose_LID)
    {
        open_swmm_model(6);
        int error, lid_index, sub_index;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        string lid = string("SWALE");
        string subcatch = string("wSWALE");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        // Surface layer get/set check
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 12, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.9);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.9, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.05359, 0.0001);

        // Check for immediate overflow option
        // Always available to immediate overflow
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 1);

        // Lid Unit
        error = swmm_getLidUCount(sub_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);

        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 500, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 1000);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1000, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 75, 0.0001);

        error = swmm_getLidUOption(sub_index, 0, SM_INDEX, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, lid_index);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 10);
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 11);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 11);

        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINSUB, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINNODE, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);
    }

    // Testing for Lid Control Roof Disconnection parameters get/set
    BOOST_FIXTURE_TEST_CASE(getset_lidcontrol_RD, FixtureOpenClose_LID)
    {
        open_swmm_model(7);
        int error, lid_index, sub_index;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        string lid = string("RD");
        string subcatch = string("wRD");

        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_CHECK_EQUAL(error, ERR_NONE);

        // Surface layer get/set check
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, 100);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 100, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.25, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, 0.9);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_VOIDFRAC, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.9, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.1, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0.2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_SURFSLOPE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_SURFACE, SM_ALPHA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1.05359, 0.0001);

        // Drain layer get/set
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_COEFF, 2.0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_COEFF, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2.0, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.5, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_EXPON, 2);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_EXPON, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 2, 0.0001);

        // if storage thickness is zero
        // the offset is changed to zero
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_OFFSET, 20);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_OFFSET, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0.0, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 6, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_DELAY, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_DELAY, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HOPEN, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HOPEN, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, 7);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCParam(lid_index, SM_DRAIN, SM_HCLOSE, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 7, 0.0001);

        // Check for immediate overflow option
        // No option available for immediate overflow
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 0);
        error = swmm_setLidCParam(lid_index, SM_SURFACE, SM_ROUGHNESS, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidCOverflow(lid_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(int_value == 0);

        // Lid Unit
        error = swmm_getLidUCount(sub_index, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);

        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 50, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_UNITAREA, 1000);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_UNITAREA, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 1000, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 10, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FWIDTH, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FWIDTH, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 0, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_INITSAT, 5);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_INITSAT, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 5, 0.0001);

        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 25, 0.0001);
        error = swmm_setLidUParam(sub_index, 0, SM_FROMIMPERV, 75);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUParam(sub_index, 0, SM_FROMIMPERV, &db_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(db_value - 75, 0.0001);

        error = swmm_getLidUOption(sub_index, 0, SM_INDEX, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, lid_index);

        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 100);
        error = swmm_setLidUOption(sub_index, 0, SM_NUMBER, 11);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_NUMBER, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 11);

        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 1);
        error = swmm_setLidUOption(sub_index, 0, SM_TOPERV, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_TOPERV, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINSUB, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINSUB, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);

        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, -1);
        error = swmm_setLidUOption(sub_index, 0, SM_DRAINNODE, 0);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getLidUOption(sub_index, 0, SM_DRAINNODE, &int_value);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(int_value, 0);
    }

    // Testing for After Start Errors
    BOOST_FIXTURE_TEST_CASE(sim_after_start_index_check, FixtureBeforeStep_LID)
    {
        open_swmm_model(0);
        int error;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        //Lid Control
        error = swmm_getLidCOverflow(1, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_getLidCParam(1, SM_SURFACE, SM_THICKNESS, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_setLidCParam(1, SM_SURFACE, SM_THICKNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

        //Lid Unit
        error = swmm_getLidUCount(2, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_getLidUParam(2, 0, SM_UNITAREA, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_setLidUParam(2, 0, SM_UNITAREA, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_getLidUOption(2, 0, SM_INDEX, &int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_setLidUOption(2, 0, SM_INDEX, int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_getLidUFluxRates(2, 0, SM_SURFACE, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
        error = swmm_getLidUResult(2, 0, SM_INFLOW, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

        //Lid Group
        error = swmm_getLidGResult(2, SM_PERVAREA, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
    }

    // Testing for invalid parameter key
    BOOST_FIXTURE_TEST_CASE(sim_after_start_key_check, FixtureBeforeStep_LID)
    {
        open_swmm_model(0);
        int error;
        int int_value = 0;
        double db_value = 0;
        char chr_value = '0';

        //Lid Control
        error = swmm_getLidCParam(0, SM_SURFACE, static_cast<SM_LidLayerProperty>(100), &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_setLidCParam(0, SM_SURFACE, static_cast<SM_LidLayerProperty>(100), db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_getLidCParam(0, static_cast<SM_LidLayer>(100), SM_THICKNESS, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_setLidCParam(0, static_cast<SM_LidLayer>(100), SM_THICKNESS, db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

        //Lid Unit
        error = swmm_getLidUParam(0, 0, static_cast<SM_LidUProperty>(100), &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_getLidUOption(0, 0, static_cast<SM_LidUOptions>(100), &int_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_getLidUFluxRates(0, 0, static_cast<SM_LidLayer>(100), &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
        error = swmm_getLidUResult(0, 0, static_cast<SM_LidResult>(100), &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);

        //Lid Group
        error = swmm_getLidGResult(0, static_cast<SM_LidResult>(100), &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_OUTBOUNDS);
    }

    // Testing for undefined lid usage
    BOOST_FIXTURE_TEST_CASE(sim_after_start_undefined_check, FixtureBeforeStep_LID)
    {
        open_swmm_model(0);
        int error;
        double db_value = 0;

        //Lid Unit
        error = swmm_getLidUFluxRates(0, 1, SM_SURFACE, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);
        error = swmm_getLidUResult(0, 1, SM_INFLOW, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);

        //Lid Group
        error = swmm_getLidGResult(1, SM_PERVAREA, &db_value);
        BOOST_CHECK_EQUAL(error, ERR_API_UNDEFINED_LID);
    }

BOOST_AUTO_TEST_SUITE_END()
