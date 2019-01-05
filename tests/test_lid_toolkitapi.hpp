#include <boost/test/included/unit_test.hpp>
 
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>

#include "swmm5.h"
#include "toolkitAPI.h"
#include "../src/error.h"

// NOTE: Test LID Input File
#define DATA_PATH_INP_LID_BC "lid/w_wo_BC_2Subcatchmentst.inp"
#define DATA_PATH_RPT_LID_BC "lid/w_wo_BC_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_BC "lid/w_wo_BC_2Subcatchments.out"

#define DATA_PATH_INP_LID_GR "lid/w_wo_GR_2Subcatchmentst.inp"
#define DATA_PATH_RPT_LID_GR "lid/w_wo_GR_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_GR "lid/w_wo_GR_2Subcatchments.out"

#define DATA_PATH_INP_LID_IT "lid/w_wo_IT_2Subcatchmentst.inp"
#define DATA_PATH_RPT_LID_IT "lid/w_wo_IT_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_IT "lid/w_wo_IT_2Subcatchments.out"

#define DATA_PATH_INP_LID_PP "lid/w_wo_PP_2Subcatchmentst.inp"
#define DATA_PATH_RPT_LID_PP "lid/w_wo_PP_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_PP "lid/w_wo_PP_2Subcatchments.out"

#define DATA_PATH_INP_LID_RB "lid/w_wo_RB_2Subcatchmentst.inp"
#define DATA_PATH_RPT_LID_RB "lid/w_wo_RB_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_RB "lid/w_wo_RB_2Subcatchments.out"

#define DATA_PATH_INP_LID_RG "lid/w_wo_RG_2Subcatchmentst.inp"
#define DATA_PATH_RPT_LID_RG "lid/w_wo_RG_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_RG "lid/w_wo_RG_2Subcatchments.out"

#define DATA_PATH_INP_LID_SWALE "lid/w_wo_SWALE_2Subcatchmentst.inp"
#define DATA_PATH_RPT_LID_SWALE "lid/w_wo_SWALE_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_SWALE "lid/w_wo_SWALE_2Subcatchments.out"

using namespace std;

void openSwmmLid(int lidType){
    switch(lidType) {
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
        default:
            break;
    }
} 

// Defining Fixtures

/* Fixture Open Close
 1. Opens Model
 *. testing interactions
 2. Closes Model 
*/
struct FixtureOpenClose_LID{
    FixtureOpenClose(int lidType) {
        openSwmmLid(lidType);
    }
    ~FixtureOpenClose() {
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
struct FixtureBeforeStart_LID{
    FixtureBeforeStart(int lidType) {
        openSwmmLid(lidType);
    }
    ~FixtureBeforeStart() {
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

/* Fixture Before Step
 1. Opens Model
 2. Starts Simulation
 *. can choose iterate over simulation if simulation started, 
    must call swmm_end() 
 3. Closes Model 
*/
struct FixtureBeforeStep_LID{
    FixtureBeforeStep(int lidType) {
        openSwmmLid(lidType);
        swmm_start(0);
    }
    ~FixtureBeforeStep() {
        swmm_close();
    }
};


/* Fixture Before End
 1. Opens Model
 2. Starts Simulation
 3. Runs Simlation
 * can choose to interact after simulation end
 4. Ends simulation
 5. Closes Model 
*/
struct FixtureBeforeEnd_LID{
    FixtureBeforeEnd(int lidType) {
        openSwmmLid(lidType);
        swmm_start(0);

        int error;
        double elapsedTime = 0.0;
        do
        {
            error = swmm_step(&elapsedTime);
        }while (elapsedTime != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
    }
    ~FixtureBeforeEnd() {
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
struct FixtureBeforeClose_LID{
    FixtureBeforeClose(int lidType) {
        openSwmmLid(lidType);
        swmm_start(0);

        int error;
        double elapsedTime = 0.0;
        do
        {
            error = swmm_step(&elapsedTime);
        }while (elapsedTime != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
        swmm_end();
    }
    ~FixtureBeforeClose() {
        swmm_close();
    }
};

