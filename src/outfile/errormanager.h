/*!
 * \file errormanager.h
 * \brief Header file for error handling
 * \date Created On: Aug 25, 2017
 * \date Last Edited: May 26, 2023
 * \author Michael E. Tryby (US EPA - ORD/NRMRL)
 * \author Caleb Buahin (US EPA - ORD/CESER) (Last Editor)
 */
 #ifndef ERRORMANAGER_H_
#define ERRORMANAGER_H_


#define ERR_MAXMSG 256 /*! \brief Maximum length of error message */

/*!
* \typedef p_msg_lookup
* \brief Function pointer for error message lookup
* \param errorcode Error code
* \param msg Error message
* \param msg_len Length of error message
*/
typedef void (*p_msg_lookup)(int, char*, int);

/*!
* \struct error_s
* \brief Error manager structure
*/
struct error_s {
	/*! \brief error status code*/
	int error_status;
	/*! \brief error message */
	p_msg_lookup message_lookup;
};

/*! 
* \typedef error_handle_t as error_s
*/
typedef struct error_s error_handle_t;

/*!
* \brief Creates a new error manager
* \param p_error_message Pointer to function that returns error message
* \return Pointer to error manager
*/
error_handle_t* new_errormanager(p_msg_lookup message_lookup);

/*!
* \brief Destroys an error manager
* \param error_handle Pointer to error manager
*/
void dst_errormanager(error_handle_t* error_handle);

/*!
* \brief Sets error code
* \param error_handle Pointer to error manager
* \param errorcode Error code
* \return Error code
*/
int set_error(error_handle_t* error_handle, int errorcode);

/*!
* \brief Gets error code
* \param error_handle Pointer to error manager
* \return Error code
*/
char* check_error(error_handle_t* error_handle);

/*!
* \brief Clears error code
* \param error_handle Pointer to error manager
*/
void clear_error(error_handle_t* error_handle);


#endif /* ERRORMANAGER_H_ */
