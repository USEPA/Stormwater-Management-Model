// Additional error codes and error functions for the toolkit api

enum  ToolkitErrorType {
    ERR_TKAPI_OUTBOUNDS          = 2000, 
    ERR_TKAPI_INPUTNOTOPEN       = 2001,
    ERR_TKAPI_SIM_NRUNNING       = 2002,
    ERR_TKAPI_WRONG_TYPE         = 2003,
    ERR_TKAPI_OBJECT_INDEX       = 2004,
    ERR_TKAPI_POLLUT_INDEX       = 2005,
    ERR_TKAPI_INFLOWTYPE         = 2006,
    ERR_TKAPI_TSERIES_INDEX      = 2007,
    ERR_TKAPI_PATTERN_INDEX      = 2008,
    ERR_TKAPI_LIDUNIT_INDEX      = 2009,
    ERR_TKAPI_UNDEFINED_LID      = 2010,
    ERR_TKAPI_MEMORY             = 2011,
    ERR_TKAPI_NO_INLET           = 2012,
    ERR_TKAPI_SIM_RUNNING        = 2013,

    TKMAXERRMSG                  = 3000
};
