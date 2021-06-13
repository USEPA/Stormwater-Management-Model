//
//  test_pollutant.hpp
//
//  Created: Aug 14, 2020
//
//  Author: Abhiram Mullapudi
//        
//
#ifndef TEST_POLLUTANT_HPP
#define TEST_POLLUTANT_HPP

extern "C" {
#include "swmm5.h"
#include "toolkit.h"
}

#define ERR_NONE 0

// Add shared data paths here
#define DATA_PATH_INP "test_example1.inp"
#define DATA_PATH_INP_POLLUT_NODE "pollutants/node_constantinflow_constanteffluent.inp"
#define DATA_PATH_INP_POLLUT_LINK "pollutants/link_constantinflow.inp"
#define DATA_PATH_RPT "tmp.rpt"
#define DATA_PATH_OUT "tmp.out"


struct FixtureBeforeStep{
    FixtureBeforeStep() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
        swmm_start(0);
    }
    ~FixtureBeforeStep() {
        swmm_close();
    }
};

struct FixtureBeforeStep_Pollut_Node{
    FixtureBeforeStep_Pollut_Node() {
        swmm_open(DATA_PATH_INP_POLLUT_NODE, DATA_PATH_RPT, DATA_PATH_OUT);
        swmm_start(0);
    }
    ~FixtureBeforeStep_Pollut_Node() {
        swmm_close();
    }
};

struct FixtureBeforeStep_Pollut_Link{
    FixtureBeforeStep_Pollut_Link() {
        swmm_open(DATA_PATH_INP_POLLUT_LINK, DATA_PATH_RPT, DATA_PATH_OUT);
        swmm_start(0);
    }
    ~FixtureBeforeStep_Pollut_Link() {
        swmm_close();
    }
};



#endif
