#include "ap_plugin_compiler_manager.h"
#include <string.h>
#include <stddef.h>


extern const ap_plugin_compiler_t * const __start_ap_plugin_compilers[];
extern const ap_plugin_compiler_t * const __stop_ap_plugin_compilers[];


const ap_plugin_compiler_t *
ap_plugin_compiler_manager_find(
    ap_module_type_t type)
{
    const ap_plugin_compiler_t * const *plugin;

    for (
        plugin = __start_ap_plugin_compilers;
        plugin < __stop_ap_plugin_compilers;
        plugin++)
    {
        if ((*plugin)->type == type)
            return *plugin;
    }

    return NULL;
}

const ap_plugin_compiler_t *
ap_plugin_compiler_manager_find_by_name(
    const char *name)
{
    const ap_plugin_compiler_t * const *plugin;

    if (name == NULL)
        return NULL;

    for (
        plugin = __start_ap_plugin_compilers;
        plugin < __stop_ap_plugin_compilers;
        plugin++)
    {
        if ((*plugin)->name == NULL)
            continue;

        if (strcmp((*plugin)->name, name) == 0)
            return *plugin;
    }

    return NULL;
}