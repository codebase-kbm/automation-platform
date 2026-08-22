#ifndef AP_PLUGIN_COMPILER_MANAGER_H
#define AP_PLUGIN_COMPILER_MANAGER_H

#include "ap_plugin_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif


const ap_plugin_compiler_t *
ap_plugin_compiler_manager_find(
    ap_plugin_type_t type
);

const ap_plugin_compiler_t *
ap_plugin_compiler_manager_find_by_name(
    const char *name
);


#ifdef __cplusplus
}
#endif

#endif /* AP_PLUGIN_COMPILER_MANAGER_H */