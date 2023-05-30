/*!
* \file errormanager.c
* \brief Provides a simple interface for managing runtime error messages.
* \author Michael E. Tryby (US EPA - ORD/NRMRL)
* \author Caleb Buahin (US EPA - ORD/CESER)(Last Editor)
* \date Created on: Aug 25, 2017
* \date Last edited: May 26, 2023
* \remarks
* \see
* \bug
* \warning
* \todo
* \note
*/
#include <stdlib.h>
#include <string.h>
#include "errormanager.h"

/*!
* \brief Creates a new error manager handle.
* \param message_lookup Pointer to function that returns error message.
* \return Pointer to error manager.
*/
error_handle_t* new_errormanager(p_msg_lookup message_lookup)
{
	error_handle_t* error_handle = NULL;
	error_handle = (error_handle_t*)calloc(1, sizeof(error_handle_t));

	error_handle->message_lookup = message_lookup;

	return error_handle;
}

/*!
* \brief Destroys an error manager handle.
* \param error_handle Pointer to error manager.
*/
void dst_errormanager(error_handle_t* error_handle)
{
	free(error_handle);
}

/*!
* \brief Sets an error code in the handle.
*/
int set_error(error_handle_t* error_handle, int errorcode)
{
	// If the error code is 0 no action is taken and 0 is returned.
	// This is a feature not a bug.
	if (errorcode)
		error_handle->error_status = errorcode;

	return errorcode;
}

/*!
* \brief Gets the error code from the handle.
*/
char* check_error(error_handle_t* error_handle)
{
	char* temp = NULL;

	if (error_handle->error_status != 0) {
		temp = (char*) calloc(ERR_MAXMSG, sizeof(char));

		if (temp)
			error_handle->message_lookup(error_handle->error_status, temp, ERR_MAXMSG);
	}
	return temp;
}

/*!
* \brief Clears the error from the handle.
*/
void clear_error(error_handle_t* error_handle)
{
	error_handle->error_status = 0;
}