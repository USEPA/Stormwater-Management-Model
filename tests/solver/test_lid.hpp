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


#ifndef TEST_LID_HPP
#define TEST_LID_HPP


#include <vector>

#include "swmm5.h"
#include "toolkit.h"

#define ERR_NONE 0

// NOTE: Test LID Input File
#define DATA_PATH_INP_LID_BC "lid/test_w_wo_BC_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_BC "lid/w_wo_BC_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_BC "lid/w_wo_BC_2Subcatchments.out"

#define DATA_PATH_INP_LID_BC_REVISED "lid/revised/test_w_wo_BC_2Subcatchments_revised.inp"
#define DATA_PATH_RPT_LID_BC_REVISED "lid/revised/w_wo_BC_2Subcatchments_revised.rpt"
#define DATA_PATH_OUT_LID_BC_REVISED "lid/revised/w_wo_BC_2Subcatchments_revised.out"

#define DATA_PATH_INP_LID_GR "lid/test_w_wo_GR_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_GR "lid/w_wo_GR_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_GR "lid/w_wo_GR_2Subcatchments.out"

#define DATA_PATH_INP_LID_GR_REVISED "lid/revised/test_w_wo_GR_2Subcatchments_revised.inp"
#define DATA_PATH_RPT_LID_GR_REVISED "lid/revised/w_wo_GR_2Subcatchments_revised.rpt"
#define DATA_PATH_OUT_LID_GR_REVISED "lid/revised/w_wo_GR_2Subcatchments_revised.out"

#define DATA_PATH_INP_LID_IT "lid/test_w_wo_IT_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_IT "lid/w_wo_IT_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_IT "lid/w_wo_IT_2Subcatchments.out"

#define DATA_PATH_INP_LID_IT_REVISED "lid/revised/test_w_wo_IT_2Subcatchments_revised.inp"
#define DATA_PATH_RPT_LID_IT_REVISED "lid/revised/w_wo_IT_2Subcatchments_revised.rpt"
#define DATA_PATH_OUT_LID_IT_REVISED "lid/revised/w_wo_IT_2Subcatchments_revised.out"

#define DATA_PATH_INP_LID_PP "lid/test_w_wo_PP_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_PP "lid/w_wo_PP_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_PP "lid/w_wo_PP_2Subcatchments.out"

#define DATA_PATH_INP_LID_PP_REVISED "lid/revised/test_w_wo_PP_2Subcatchments_revised.inp"
#define DATA_PATH_RPT_LID_PP_REVISED "lid/revised/w_wo_PP_2Subcatchments_revised.rpt"
#define DATA_PATH_OUT_LID_PP_REVISED "lid/revised/w_wo_PP_2Subcatchments_revised.out"

#define DATA_PATH_INP_LID_RB "lid/test_w_wo_RB_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_RB "lid/w_wo_RB_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_RB "lid/w_wo_RB_2Subcatchments.out"

#define DATA_PATH_INP_LID_RB_REVISED "lid/revised/test_w_wo_RB_2Subcatchments_revised.inp"
#define DATA_PATH_RPT_LID_RB_REVISED "lid/revised/w_wo_RB_2Subcatchments_revised.rpt"
#define DATA_PATH_OUT_LID_RB_REVISED "lid/revised/w_wo_RB_2Subcatchments_revised.out"

#define DATA_PATH_INP_LID_RG "lid/test_w_wo_RG_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_RG "lid/w_wo_RG_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_RG "lid/w_wo_RG_2Subcatchments.out"

#define DATA_PATH_INP_LID_RG_REVISED "lid/revised/test_w_wo_RG_2Subcatchments_revised.inp"
#define DATA_PATH_RPT_LID_RG_REVISED "lid/revised/w_wo_RG_2Subcatchments_revised.rpt"
#define DATA_PATH_OUT_LID_RG_REVISED "lid/revised/w_wo_RG_2Subcatchments_revised.out"

#define DATA_PATH_INP_LID_SWALE "lid/test_w_wo_SWALE_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_SWALE "lid/w_wo_SWALE_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_SWALE "lid/w_wo_SWALE_2Subcatchments.out"

#define DATA_PATH_INP_LID_SWALE_REVISED "lid/revised/test_w_wo_SWALE_2Subcatchments_revised.inp"
#define DATA_PATH_RPT_LID_SWALE_REVISED "lid/revised/w_wo_SWALE_2Subcatchments_revised.rpt"
#define DATA_PATH_OUT_LID_SWALE_REVISED "lid/revised/w_wo_SWALE_2Subcatchments_revised.out"

#define DATA_PATH_INP_LID_RD "lid/test_w_wo_RD_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_RD "lid/w_wo_RD_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_RD "lid/w_wo_RD_2Subcatchments.out"

#define DATA_PATH_INP_LID_RD_REVISED "lid/revised/test_w_wo_RD_2Subcatchments_revised.inp"
#define DATA_PATH_RPT_LID_RD_REVISED "lid/revised/w_wo_RD_2Subcatchments_revised.rpt"
#define DATA_PATH_OUT_LID_RD_REVISED "lid/revised/w_wo_RD_2Subcatchments_revised.out"

using namespace std;


// Fixture helper function
void open_swmm_lid(int lid_type, bool revised);



/* Fixture Open Close
 1. Opens Model
 *. testing interactions
 2. Closes Model
*/
struct FixtureOpenClose_LID
{
    FixtureOpenClose_LID() { }

    void open_swmm_model(int lid_type)
    {
        open_swmm_lid(lid_type, false);
    }

    ~FixtureOpenClose_LID()
    {
        swmm_close();
    }
};

/* Fixture Before Start
 1. Opens Model
 *. can choose to start simulation
 2. Starts Simulation
 3. Runs Simlation
 4. Ends simulation
 5. Closes Model
*/
struct FixtureBeforeStart_LID {
    FixtureBeforeStart_LID() { }

    void open_swmm_model(int lid_type)
    {
        open_swmm_lid(lid_type, false);
    }

    ~FixtureBeforeStart_LID() {
        swmm_start(0);
        int error;
        double elapsedTime = 0.0;
        do
        {
            error = swmm_step(&elapsedTime);
        }while (elapsedTime != 0 && !error);
        if (!error) swmm_end();
        if (!error) swmm_report();

        swmm_close();
    }
};

/* Fixture Before Step Default
 1. Opens Model
 2. Starts Simulation
 *. can choose iterate over simulation if simulation started,
    must call swmm_end()
 3. Closes Model
*/
struct FixtureBeforeStep_LID {
    FixtureBeforeStep_LID(){ }

    void open_swmm_model(int lid_type)
    {
        open_swmm_lid(lid_type, false);
        swmm_start(0);
    }

    ~FixtureBeforeStep_LID() {
        swmm_close();
    }
};

/* Fixture for comparing results using LID API
1. Opens model
2. Starts simulation
3. Save data results
4. Closes model
5. Open revised model
6. Starts simulation
*. update parameters to match original model
7. Closes model
*/
struct Fixture_LID_Results
{
    Fixture_LID_Results()
    {
    }

    void open_swmm_model(int lid_type)
    {
        int error, sub_index, lid_index;
        double elapsed_time = 0.0;
        double db_value = 0.0;
        string subcatch;
        string lid;

        switch(lid_type)
        {
            case 0:
                subcatch = string("wBC");
                lid = string("BC");
                break;
            case 1:
                subcatch = string("wGR");
                lid = string("GR");
                break;
            case 2:
                subcatch = string("wIT");
                lid = string("IT");
                break;
            case 3:
                subcatch = string("wPP");
                lid = string("PP");
                break;
            case 4:
                subcatch = string("wRB");
                lid = string("RB");
                break;
            case 5:
                subcatch = string("wRG");
                lid = string("RG");
                break;
            case 6:
                subcatch = string("wSWALE");
                lid = string("SWALE");
                break;
            case 7:
                subcatch = string("wRD");
                lid = string("RD");
                break;
            default:
                subcatch = string("wBC");
                lid = string("BC");
                break;
        }

        open_swmm_lid(lid_type, false);
        swmm_start(0);
        sub_index = swmm_getObjectIndex(SM_SUBCATCH, (char *)subcatch.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);
        lid_index = swmm_getObjectIndex(SM_LID, (char *)lid.c_str(), &error);
        BOOST_REQUIRE(error == ERR_NONE);

        do
        {
            error = swmm_step(&elapsed_time);
            BOOST_REQUIRE(error == ERR_NONE);
            error = swmm_getSubcatchResult(sub_index, SM_SUBCRUNOFF, &db_value);
            BOOST_REQUIRE(error == ERR_NONE);
            subcatchment_runoff.push_back(round(db_value * 100000.0) / 100000.0);
            } while (elapsed_time != 0 && !error);
        BOOST_CHECK_EQUAL(ERR_NONE, error);
        swmm_end();
        swmm_close();

        open_swmm_lid(lid_type, true);
    }
    ~Fixture_LID_Results() {
        swmm_close();
    }

    vector<double> subcatchment_runoff;
};

/* Fixture Before End
 1. Opens Model
 2. Starts Simulation
 3. Runs Simlation
 * can choose to interact after simulation end
 4. Ends simulation
 5. Closes Model
*/
struct FixtureBeforeEnd_LID
{
    FixtureBeforeEnd_LID() { }

    void open_swmm_model(int lid_type)
    {
        open_swmm_lid(lid_type, false);
        swmm_start(0);

        int error;
        double elapsedTime = 0.0;
        do
        {
            error = swmm_step(&elapsedTime);
        }while (elapsedTime != 0 && !error);
        BOOST_CHECK_EQUAL(ERR_NONE, error);
    }

    ~FixtureBeforeEnd_LID() {
        swmm_end();
        swmm_close();
    }
};

/* Fixture Before Close
 1. Opens Model
 2. Starts Simulation
 3. Runs Simlation
 4. Ends simulation
 * can choose to interact after simulation end
 5. Closes Model
*/
struct FixtureBeforeClose_LID
{
    FixtureBeforeClose_LID() {}

    void open_swmm_model(int lid_type)
    {
        open_swmm_lid(lid_type, false);
        swmm_start(0);

        int error;
        double elapsedTime = 0.0;
        do
        {
            error = swmm_step(&elapsedTime);
        }while (elapsedTime != 0 && !error);
        BOOST_CHECK_EQUAL(ERR_NONE, error);
        swmm_end();
    }

    ~FixtureBeforeClose_LID() {
        swmm_close();
    }
};


#endif //TEST_LID_HPP
