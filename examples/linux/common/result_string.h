#ifndef RESULT_STRING_H
#define RESULT_STRING_H

#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

const char *ap_result_string(ap_result_t result);
//const char *ap_result_string(ap_error_code_t error);

#ifdef __cplusplus
}
#endif

#endif /* RESULT_STRING_H */