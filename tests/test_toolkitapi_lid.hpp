#include <boost/test/included/unit_test.hpp>
 
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>

#include "swmm5.h"
#include "toolkitAPI.h"
#include "../src/error.h"

// NOTE: Test LID Input File
#define DATA_PATH_INP_LID_BC "lid/w_wo_BC_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_BC "lid/w_wo_BC_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_BC "lid/w_wo_BC_2Subcatchments.out"

#define DATA_PATH_INP_LID_GR "lid/w_wo_GR_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_GR "lid/w_wo_GR_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_GR "lid/w_wo_GR_2Subcatchments.out"

#define DATA_PATH_INP_LID_IT "lid/w_wo_IT_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_IT "lid/w_wo_IT_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_IT "lid/w_wo_IT_2Subcatchments.out"

#define DATA_PATH_INP_LID_PP "lid/w_wo_PP_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_PP "lid/w_wo_PP_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_PP "lid/w_wo_PP_2Subcatchments.out"

#define DATA_PATH_INP_LID_RB "lid/w_wo_RB_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_RB "lid/w_wo_RB_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_RB "lid/w_wo_RB_2Subcatchments.out"

#define DATA_PATH_INP_LID_RG "lid/w_wo_RG_2Subcatchments.inp"
#define DATA_PATH_RPT_LID_RG "lid/w_wo_RG_2Subcatchments.rpt"
#define DATA_PATH_OUT_LID_RG "lid/w_wo_RG_2Subcatchments.out"

#define DATA_PATH_INP_LID_SWALE "lid/w_wo_SWALE_2Subcatchments.inp"
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


/* Fixture Before Step
 1. Opens Model
 2. Starts Simulation
 *. can choose iterate over simulation if simulation started, 
    must call swmm_end() 
 3. Closes Model 
*/
struct FixtureBeforeStep_LID {
    FixtureBeforeStep_LID(int lidType=0){
        openSwmmLid(lidType);
        swmm_start(0);
    }
    ~FixtureBeforeStep_LID() {
        swmm_close();
    }
};