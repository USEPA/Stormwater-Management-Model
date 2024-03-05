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
#include <fstream>

#include "swmm5.h"
#include "swmm_output_enums.h"
#include "swmm_output.h"
#include "helper.h"

/*!
* \def ORIGINAL_INPUT_FILE 
* \brief The original input file for the SWMM model.
*/
#define ORIGINAL_INPUT_FILE "./hotstart/site_drainage_model.inp"

/*!
* \def SAVE_HOTSTART_INPUT_FILE
* \brief The input file for the SWMM model to save the hotstart file.
*/
#define SAVE_HOTSTART_INPUT_FILE "./hotstart/site_drainage_model_save_hotstart.inp"

/*!
* \def HOTSTART_FILE_V1
* \brief The first hotstart file to save for the SWMM model.
*/
#define HOTSTART_FILE_V1 "./hotstart/hotstart_v1.hsf"

/*!
* \def HOTSTART_FILE_V2
* \brief The second hotstart file to save for the SWMM model.
*/
#define HOTSTART_FILE_V2 "./hotstart/hotstart_v2.hsf"

/*!
* \def HOTSTART_FILE_V3
* \brief The third hotstart file to save for the SWMM model.
*/
#define HOTSTART_FILE_END "./hotstart/hotstart_end.hsf"

/*!
* \def RUN_HOTSTART_INPUT_FILE_v1
* \brief The input file for the SWMM model to use the first hotstart file.
*/
#define RUN_HOTSTART_INPUT_FILE_v1 "./hotstart/site_drainage_model_use_hotstart_v1.inp"

/*!
* \def RUN_HOTSTART_INPUT_FILE_v2
* \brief The input file for the SWMM model to use the second hotstart file.
*/
#define RUN_HOTSTART_INPUT_FILE_v2 "./hotstart/site_drainage_model_use_hotstart_v2.inp"

/*!
* \def RUN_HOTSTART_INPUT_FILE_v3
* \brief The input file for the SWMM model to use the third hotstart file.
*/
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
BOOST_AUTO_TEST_CASE(
	test_save_hotstart, 
	*boost::unit_test::label("Test save multiple hotstart files")
) {
	int error = 0;
	std::string filepath = std::string(ORIGINAL_INPUT_FILE);
    std::string extension = ".inp";

    size_t startPos = filepath.find(extension);
    std::string reportFilepath = std::string(filepath).replace(startPos, extension.length(), ".rpt");
	std::string outputFilepath = std::string(filepath).replace(startPos, extension.length(), ".out");

	error = swmm_run(filepath.c_str(), reportFilepath.c_str(), outputFilepath.c_str());
	BOOST_REQUIRE(error == 0);

	// Check to make sure output exists
	std::ifstream output_file(outputFilepath);
	BOOST_REQUIRE(output_file.good());

	filepath = std::string(SAVE_HOTSTART_INPUT_FILE);
	startPos = filepath.find(extension);
	reportFilepath = std::string(filepath).replace(startPos, filepath.length(), ".rpt");
	outputFilepath = std::string(filepath).replace(startPos, extension.length(), ".out");
	error = swmm_run(SAVE_HOTSTART_INPUT_FILE, reportFilepath.c_str(), outputFilepath.c_str());
	BOOST_REQUIRE(error == 0);

	// Check to make sure hostart file exists
	std::ifstream hotstartSaveOutputFile(outputFilepath);
	BOOST_REQUIRE(hotstartSaveOutputFile.good());

	// Check to make sure hotstart file exists
	std::ifstream hotstartFileV1(std::string(HOTSTART_FILE_V1));
	BOOST_REQUIRE(hotstartFileV1.good());

	std::ifstream hotstartFileV2(std::string(HOTSTART_FILE_V2));
	BOOST_REQUIRE(hotstartFileV1.good());

	std::ifstream hotstartFileEnd(std::string(HOTSTART_FILE_END));
	BOOST_REQUIRE(hotstartFileEnd.good());
}

/*!
* \brief Test the hotstart saving feature of the SWMM solver.
* \test
* The test case runs the SWMM model with the hotstart file and compares
* the results with the original input file at critical locations.
*/
BOOST_AUTO_TEST_CASE(
	test_run_hotstart_first,
	*boost::unit_test::depends_on("test_solver_hotstart/test_save_hotstart"),
	*boost::unit_test::label("Test using first saved hotstart")
) {
	int error = 0;
	std::string originalFilepath = std::string(ORIGINAL_INPUT_FILE);
	std::string filepath = std::string(RUN_HOTSTART_INPUT_FILE_v1);
	std::string extension = ".inp";

	size_t startPos = filepath.find(extension);
	std::string reportFilepath = std::string(filepath).replace(startPos, extension.length(), ".rpt");
	std::string outputFilepath = std::string(filepath).replace(startPos, extension.length(), ".out");
	error = swmm_run(filepath.c_str(), reportFilepath.c_str(), outputFilepath.c_str());
	BOOST_REQUIRE(error == 0);
	
	std::string originalOutputFilepath = std::string(originalFilepath).replace(startPos, extension.length(), ".out");
	
	SWMMOutputFile output_file(originalOutputFilepath);

	int *origElementCount;
	int origLength;
	error = SMO_getProjectSize(output_file.m_handle, &origElementCount, &origLength);
	BOOST_REQUIRE(error == 0);

	int num_periods;
	double startDate;
	error = SMO_getTimes(output_file.m_handle, SMO_time::SMO_numPeriods, &num_periods);

	SWMMOutputFile hotstart_output_file(output_filepath);

}


BOOST_AUTO_TEST_SUITE_END()