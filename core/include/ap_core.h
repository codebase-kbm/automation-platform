#ifndef AP_CORE_H
#define AP_CORE_H

#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Core lifecycle */
ap_result_t ap_core_init(void);
ap_result_t ap_core_process(void);
void ap_core_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* AP_CORE_H */