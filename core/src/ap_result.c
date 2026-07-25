#include "ap_result.h"

const char *ap_result_string(ap_result_t result)
{
    switch (result)
    {
        case AP_OK:
            return "OK";

        case AP_ERROR:
            return "Generic error";

        case AP_ERROR_INVALID_ARGUMENT:
            return "Invalid argument";

        case AP_ERROR_INVALID_ID:
            return "Invalid ID";

        case AP_ERROR_INVALID_TYPE:
            return "Invalid type";

        case AP_ERROR_NOT_FOUND:
            return "Not found";

        case AP_ERROR_ALREADY_EXISTS:
            return "Already exists";

        case AP_ERROR_FULL:
            return "Resource full";

        case AP_ERROR_OUT_OF_MEMORY:
            return "Out of memory";

        case AP_ERROR_TIMEOUT:
            return "Timeout";

        case AP_ERROR_NOT_SUPPORTED:
            return "Not supported";

        case AP_ERROR_INIT_FAILED:
            return "Initialization failed";

        case AP_ERROR_CONNECTION:
            return "Connection failed";

        case AP_ERROR_AUTHENTICATION:
            return "Authentication failed";

        case AP_ERROR_OPERATION_FAILED:
            return "Operation failed";

        default:
            return "Unknown";
    }
}