#ifndef AP_RESULT_H
#define AP_RESULT_H

#include "ap_common.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Result codes returned by Automation Platform functions.
 */
typedef enum
{
    AP_OK = 0,

    /* Generic errors */
    AP_ERROR,

    /* Invalid parameters */
    AP_ERROR_INVALID_ARGUMENT,
    AP_ERROR_INVALID_ID,
    AP_ERROR_INVALID_TYPE,

    /* Resource errors */
    AP_ERROR_NOT_FOUND,
    AP_ERROR_ALREADY_EXISTS,
    AP_ERROR_FULL,
    AP_ERROR_OUT_OF_MEMORY,

    /* Runtime errors */
    AP_ERROR_TIMEOUT,
	AP_ERROR_NOT_INITIALIZED,
    AP_ERROR_NOT_SUPPORTED,
	AP_ERROR_INIT_FAILED,
	AP_ERROR_CONNECTION,
	AP_ERROR_AUTHENTICATION,
	AP_ERROR_OPERATION_FAILED,
	
	/* Socket errors */
	AP_ERROR_SOCKET_CREATE,
	AP_ERROR_SOCKET_ACCEPT,
	AP_ERROR_SOCKET_OPTION,
	AP_ERROR_SOCKET_BIND,
	AP_ERROR_SOCKET_LISTEN,
	AP_ERROR_SOCKET_RESOLVE,
	AP_ERROR_SOCKET_CONNECT,
	AP_ERROR_SOCKET_SEND,
	AP_ERROR_SOCKET_RECEIVE,
	AP_ERROR_SOCKET_PROTOCOL

} ap_result_t;

/**
 * @brief Returns a human-readable string for a result code.
 */
const char *ap_result_string(ap_result_t result);

#ifdef __cplusplus
}
#endif

#endif /* AP_RESULT_H */