/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.1.13
 Module:       test_solver.hpp
 Description:  tests for SWMM solver library API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/21/2020
 ******************************************************************************
*/

#ifndef TEST_SOLVER_HPP
#define TEST_SOLVER_HPP

extern "C" {
#include "swmm5.h"
#include "toolkit.h"
}

// Add shared data paths here
#define DATA_PATH_INP "test_example1.inp"
#define DATA_PATH_INP_POLLUT_NODE "node_constantinflow_constanteffluent.inp"
#define DATA_PATH_INP_POLLUT_LINK "link_constantinflow.inp"
#define DATA_PATH_INP_LINK_DIR "link_flow_dir.inp"
#define DATA_PATH_RPT "tmp.rpt"
#define DATA_PATH_OUT "tmp.out"


// Add shared fixtures here
struct FixtureOpenClose{
    FixtureOpenClose() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
    }

    FixtureOpenClose(const char *input_file) {
        swmm_open(input_file, DATA_PATH_RPT, DATA_PATH_OUT);
    }

    ~FixtureOpenClose() {
        swmm_close();
    }
};

struct FixtureBeforeStep{
    FixtureBeforeStep() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
        swmm_start(0);
    }
    ~FixtureBeforeStep() {
        swmm_close();
    }
};

struct FixtureBeforeEnd{
    FixtureBeforeEnd() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
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

struct FixtureBeforeStep_Flow_Dir{
    FixtureBeforeStep_Flow_Dir() {
        swmm_open(DATA_PATH_INP_LINK_DIR, DATA_PATH_RPT, DATA_PATH_OUT);
        swmm_start(0);
    }
    ~FixtureBeforeStep_Flow_Dir() {
        swmm_close();
    }
};


// Declare shared test predicates here
boost::test_tools::predicate_result check_cdd_double(std::vector<double>& test,
    std::vector<double>& ref, long cdd_tol);

boost::test_tools::predicate_result check_string(std::string test, std::string ref);


#endif //TEST_SOLVER_HPP
