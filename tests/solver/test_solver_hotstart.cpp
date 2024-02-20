/*
 *   test_solver_hotstart.cpp
 *
 *   Created: 02/01/2024
 *   Updated: 02/01/2024
 * 
 *   Unit testing for SWMM solver api hotstart saving.
 */

#define BOOST_TEST_MODULE "solver hotstart"
#define BOOST_TEST_DYN_LINK
#include <boost/test/included/unit_test.hpp>

#include "swmm5.h"
#include "swmm_output.h"

#define ORIGINAL_INPUT_FILE "./hotstart/site_drainage_model.inp"
#define SAVE_HOTSTART_INPUT_FILE "./hotstart/site_drainage_model_save_hotstart.inp"
#define RUN_HOTSTART_INPUT_FILE_v1 "./hotstart/site_drainage_model_use_hotstart_v1.inp"
#define RUN_HOTSTART_INPUT_FILE_v2 "./hotstart/site_drainage_model_use_hotstart_v2.inp"
#define RUN_HOTSTART_INPUT_FILE_v3 "./hotstart/site_drainage_model_use_hotstart_v3.inp"


/*!
  \brief Test the hotstart saving feature of the SWMM solver.
  \sa swmm_run
*/
BOOST_AUTO_TEST_SUITE(test_solver_hotstart, *boost::unit_test::label("Test SWMM Solver Hotstart"))

/*!
* \brief Test the hotstart saving feature of the SWMM solver.
* \test
* The test case runs the SWMM model with the original input file and saves the hotstart file.
* Then, the test case runs the SWMM model with the hotstart file and checks for errors.
* \sa swmm_run
*/
BOOST_AUTO_TEST_CASE(test_save_hotstart) {
	int error = 0;
	std::string filepath = std::string(ORIGINAL_INPUT_FILE);
    std::string extension = ".inp";

    size_t start_pos = filepath.find(extension);
    std::string report_filepath = std::string(filepath).replace(start_pos, extension.length(), ".rpt");
	std::string output_filepath = std::string(filepath).replace(start_pos, extension.length(), ".out");

	error = swmm_run(filepath.c_str(), report_filepath.c_str(), output_filepath.c_str());
	BOOST_REQUIRE(error == 0);

	filepath = std::string(SAVE_HOTSTART_INPUT_FILE);
	start_pos = filepath.find(extension);
	report_filepath = std::string(filepath).replace(start_pos, filepath.length(), ".rpt");
	output_filepath = std::string(filepath).replace(start_pos, extension.length(), ".out");
	error = swmm_run(SAVE_HOTSTART_INPUT_FILE, report_filepath.c_str(), output_filepath.c_str());
	BOOST_REQUIRE(error == 0);

}
BOOST_AUTO_TEST_SUITE_END()