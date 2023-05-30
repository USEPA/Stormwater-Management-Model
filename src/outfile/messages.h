/*!
 * \file messages.h
 * \brief Provides a simple interface for managing runtime error messages.
 * \author Michael E. Tryby (US EPA - ORD/NRMRL)
 * \author Caleb A. Buahin (US EPA - ORD/NRMRL) (Last Editor)
 * \date Created on: Oct 20, 2017
 * \date Last Edited: May 28, 2023
*  \remarks
 * \see
 * \bug
 * \warning
 * \todo
 * \note
 */

#ifndef SRC_MESSAGES_H_
#define SRC_MESSAGES_H_


/*!
 * \def MAXMSG
 * \brief Maximum number of characters in error message
*/
#define MAXMSG 56

/*!
* \def WARN10
* \brief Warning 10: model run issued warnings
*/
#define WARN10 "Warning: model run issued warnings"

/*!
* \def ERR411
* \brief Error 411: memory allocation failure
*/
#define ERR411 "Error 411: memory allocation failure"

/*!
* \def ERR421
* \brief Error 421: invalid parameter code
*/
#define ERR421 "Input Error 421: invalid parameter code"

/*!
* \def ERR422
* \brief Error 422: reporting period index out of range
*/
#define ERR422 "Input Error 422: reporting period index out of range"

/*!
* \def ERR423
* \brief Error 423: element index out of range
*/
#define ERR423 "Input Error 423: element index out of range"

/*!
* \def ERR424
* \brief Error 424: no memory allocated for results
*/
#define ERR424 "Input Error 424: no memory allocated for results"

/*!
* \def ERR434
* \brief Error 434: unable to open binary output file
*/
#define ERR434 "File Error 434: unable to open binary output file"

/*!
* \def ERR435
* \brief Error 435: invalid file - not created by SWMM
*/
#define ERR435 "File Error 435: invalid file - not created by SWMM"

/*!
* \def ERR436
* \brief Error 436: invalid file - contains no results
*/
#define ERR436 "File Error 436: invalid file - contains no results"

/*!
* \def ERR440
* \brief Error 440: an unspecified error has occurred
*/
#define ERR440 "ERROR 440: an unspecified error has occurred"


#endif /* SRC_MESSAGES_H_ */
