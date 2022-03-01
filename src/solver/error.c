//-----------------------------------------------------------------------------
//   error.c
//
//   Project: EPA SWMM5
//   Version: 5.2
//   Date:    11/01/21   (Build 5.2.0)
//   Author:  L. Rossman
//
//   Error messages
//
//   Update History
//   ==============
//   Build 5.1.008:
//   - Text of Error 217 for control rules modified.
//   Build 5.1.010:
//   - Text of Error 318 for rainfall data files modified.
//   Build 5.1.015:
//   - Added new Error 140 for storage nodes.
//   Build 5.2.0:
//   - Re-designed error message system.
//   - Added new Error 235 for invalid infiltration parameters.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <string.h>
#include "error.h"

char  ErrString[256];

char* error_getMsg(int errCode, char* msg)
{
    switch (errCode)
    {

#define ERR(code,string) case code: strcpy(msg, string); break;
#include "error.txt"
#undef ERR

    default:
        strcpy(msg, "");
    }
    return (msg);
};

int  error_setInpError(int errcode, char* s)
{
    strcpy(ErrString, s);
    return errcode;
}
