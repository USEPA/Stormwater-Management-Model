#include <boost/test/included/unit_test.hpp>
#include "swmm_output.h"

#include <vector>

/*!
* \brief Compare two vectors of doubles for equality.
* \param v1 The first vector of doubles.
* \param v2 The second vector of doubles.
* \param rtol The relative tolerance.
* \param atol The absolute tolerance.
* \return A predicate result.
* \note This function is used to compare two vectors of doubles for equality.
* The function uses the relative and absolute tolerances to determine if the
* two vectors are equal. The function returns a predicate result that is used
* by the BOOST_CHECK_EQUAL macro to determine if the two vectors are equal.
*/
boost::test_tools::predicate_result all_close(
	const std::vector<double>& v1,
	const std::vector<double>& v2,
	double rtol = 1.0E-05,
	double atol = 1.0E-08)
{
	double diff = 0.0;

	for (size_t i = 0; i < v1.size(); i++)
	{
		diff = std::abs(v1[i] - v2[i]);
		if (diff > (atol + rtol * std::abs(v2[i])))
		{
			boost::test_tools::predicate_result result(false);
			result.message() << "The vectors are not equal at index " << i << ".";
			return result;
		}
	}

	return true;
};


/*!
* \brief A structure to hold the SWMM output file.
* \note This structure is used to hold the SWMM output file.
* The structure is used to open the SWMM output file and close the SWMM output file.
* The structure is used to hold the error code and the handle to the SWMM output file.
* The structure is used to open the SWMM output file and close the SWMM output file.
*/
struct SWMMOutputFile
{
	/*!
	* \brief The constructor for the SWMM output file.
	* \param outputFilepath The path to the SWMM output file.
	* \note This constructor is used to open the SWMM output file.
	* The constructor initializes the error code and the handle to the SWMM output file.
	* The constructor opens the SWMM output file.
	*/
	SWMMOutputFile(const std::string& outputFilepath)
		:m_errorCode(0),
		 m_handle(nullptr)
	{
		m_errorCode = SMO_init(&m_handle);
		SMO_clearError(m_handle);
		m_errorCode = SMO_open(m_handle, outputFilepath.c_str());
	}

	/*!
	* \brief The destructor for the SWMM output file.
	*/
	~SWMMOutputFile()
	{
		SMO_close(&m_handle);
	}

	/*!
	* \brief The error code for the SWMM output file.
	*/
	int  m_errorCode;

	/*!
	* \brief The handle to the SWMM output file.
	*/
	SMO_Handle m_handle;
};


