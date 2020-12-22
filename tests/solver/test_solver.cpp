/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.1.13
 Module:       test_solver.cpp
 Description:  tests for SWMM solver library API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/21/2020
 ******************************************************************************
*/

#define BOOST_TEST_MODULE solver


#include <math.h>

#include <boost/test/included/unit_test.hpp>

#include "test_solver.hpp"


// Custom test to check the minimum number of correct decimal digits
boost::test_tools::predicate_result check_cdd_double(std::vector<double>& test,
    std::vector<double>& ref, long cdd_tol)
{
    double tmp, min_cdd = 10.0;

    // TODO: What if the vectors aren't the same length?

    std::vector<double>::iterator test_it;
    std::vector<double>::iterator ref_it;

    for (test_it = test.begin(), ref_it = ref.begin();
        (test_it < test.end()) && (ref_it < ref.end());
        ++test_it, ++ref_it)
    {
        if (*test_it != *ref_it) {
            // Compute log absolute error
            tmp = abs(*test_it - *ref_it);
            if (tmp < 1.0e-7)
                tmp = 1.0e-7;

            else if (tmp > 2.0)
                tmp = 1.0;

            tmp = -log10(tmp);
            if (tmp < 0.0)
                tmp = 0.0;

            if (tmp < min_cdd)
                min_cdd = tmp;
        }
    }

    return floor(min_cdd) >= cdd_tol;
}

boost::test_tools::predicate_result check_string(std::string test, std::string ref)
{
    if (ref.compare(test) == 0)
        return true;
    else
        return false;
}
