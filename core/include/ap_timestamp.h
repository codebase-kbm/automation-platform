#ifndef AP_TIMESTAMP_H
#define AP_TIMESTAMP_H

#include "ap_common.h"

#ifdef __cplusplus
extern "C" {
#endif
void ap_timestamp_init(void);

uint64_t ap_timestamp_now(void);

#ifdef __cplusplus
}
#endif

#endif