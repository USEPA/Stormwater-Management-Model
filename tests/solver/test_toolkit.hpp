//
//  test_toolkit.hpp
//
//  Created: July 7, 2020
//
//  Author: Michael E. Tryby
//         US EPA ORD/CESER
//

#ifndef TEST_TOOLKIT_HPP
#define TEST_TOOLKIT_HPP

#include "swmm5.h"
#include "toolkitAPI.h"


// Add shared data paths here
#define DATA_PATH_INP "swmm_api_test.inp"
#define DATA_PATH_RPT "swmm_api_test.rpt"
#define DATA_PATH_OUT "swmm_api_test.out"


// Add shared fixtures here
struct FixtureBeforeStep{
    FixtureBeforeStep() {
        swmm_open((char *)DATA_PATH_INP, (char *)DATA_PATH_RPT, (char *)DATA_PATH_OUT);
        swmm_start(0);
    }
    ~FixtureBeforeStep() {
        swmm_close();
    }
};


// Declare shared test predicates
boost::test_tools::predicate_result check_cdd_double(std::vector<double>& test,
    std::vector<double>& ref, long cdd_tol);

boost::test_tools::predicate_result check_string(std::string test, std::string ref);


#endif //TEST_TOOLKIT_HPP
