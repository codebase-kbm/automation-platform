#ifndef AP_PLUGIN_MANAGER_H
#define AP_PLUGIN_MANAGER_H

#include "ap_plugin.h"
#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

ap_result_t ap_plugin_manager_init(void);

ap_result_t ap_plugin_manager_register(
    const ap_plugin_t *plugin
);

const ap_plugin_t *ap_plugin_manager_find(
    ap_module_type_t type
);

ap_result_t ap_plugin_manager_process(void);

void ap_plugin_manager_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* AP_PLUGIN_MANAGER_H */