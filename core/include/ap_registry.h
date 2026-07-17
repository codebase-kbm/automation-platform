#ifndef AP_REGISTRY_H
#define AP_REGISTRY_H

#include "ap_common.h"
#include "ap_signal.h"

#ifdef __cplusplus
extern "C" {
#endif

void ap_registry_init(void);

bool ap_registry_register(const ap_signal_t *signal);

const ap_signal_t *ap_registry_find(uint32_t id);

#ifdef __cplusplus
}
#endif

#endif /* AP_REGISTRY_H */