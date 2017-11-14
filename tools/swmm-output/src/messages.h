/*
 * messages.h - SWMM
 *
 *  Created on: Oct 20, 2017
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/NRMRL
 */

#ifndef SRC_MESSAGES_H_
#define SRC_MESSAGES_H_

#define MAXMSG 56

/*------------------- Error Messages --------------------*/
#define WARN10 "Warning: model run issued warnings"

#define ERR411 "Error 411: memory allocation failure"

#define ERR421 "Input Error 421: invalid parameter code"
#define ERR422 "Input Error 422: reporting period index out of range"
#define ERR423 "Input Error 423: element index out of range"
#define ERR424 "Input Error 424: no memory allocated for results"

#define ERR434 "File Error 434: unable to open binary output file"
#define ERR435 "File Error 435: invalid file - not created by SWMM"
#define ERR436 "File Error 436: invalid file - contains no results"

#define ERR440 "ERROR 440: an unspecified error has occurred"

#endif /* SRC_MESSAGES_H_ */
