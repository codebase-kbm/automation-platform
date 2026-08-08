#include "ap_plugin_manager.h"

#include <stddef.h>
#include <stdint.h>

#include "ap_config_reader.h"
#include "ap_object.h"
#include "ap_plugin_type.h"


extern const ap_plugin_t * const __start_ap_plugins[];
extern const ap_plugin_t * const __stop_ap_plugins[];

static uint16_t plugin_instances_loaded = 0;
static uint16_t plugin_types_registered = 0;

/* -------------------------------------------------- */
/* Plugin lookup                                      */
/* -------------------------------------------------- */

const ap_plugin_t * ap_plugin_manager_find(ap_plugin_type_t type)
{
    const ap_plugin_t * const *plugin;

    for (
        plugin = __start_ap_plugins;
        plugin < __stop_ap_plugins;
        plugin++
    )
    {
        if ((*plugin)->type == type)
            return *plugin;
    }

    return NULL;
}


/* -------------------------------------------------- */
/* Manager init                                       */
/* -------------------------------------------------- */

ap_result_t ap_plugin_manager_init(void)
{
    ap_config_object_t object;
    plugin_types_registered = (uint16_t)(__stop_ap_plugins - __start_ap_plugins);
    

    while (ap_config_reader_next(&object) == AP_OK)
    {
        if (object.header.object_type != AP_OBJECT_PLUGIN)
            continue;

        if (object.payload == NULL)
            return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_PLUGIN_MANAGER,AP_PLUGIN_NONE,AP_ERROR_INVALID_ARGUMENT);

        ap_plugin_type_t type = (ap_plugin_type_t)object.payload[0];
        const ap_plugin_t *plugin = ap_plugin_manager_find(type);

        if (plugin == NULL)
            return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_PLUGIN_MANAGER,AP_PLUGIN_NONE,AP_ERROR_NOT_FOUND);

        if (plugin->load != NULL)
        {
            ap_result_t result = plugin->load(&object);
            if (result != AP_OK)
                return result;
        }

        if (plugin->init != NULL)
        {
            ap_result_t result = plugin->init();
            if (result != AP_OK)
                return result;
        }
            plugin_instances_loaded++;
    }

    return AP_OK;
}


/* -------------------------------------------------- */
/* Runtime processing                                 */
/* -------------------------------------------------- */

ap_result_t ap_plugin_manager_process(void)
{
    const ap_plugin_t * const *plugin;

    for (
        plugin = __start_ap_plugins;
        plugin < __stop_ap_plugins;
        plugin++
    )
    {
        if ((*plugin)->process == NULL)
            continue;

        ap_result_t result =
            (*plugin)->process();

        if (result != AP_OK)
            return result;
    }

    return AP_OK;
}


/* -------------------------------------------------- */
/* Shutdown                                           */
/* -------------------------------------------------- */

void
ap_plugin_manager_shutdown(void)
{
    const ap_plugin_t * const *plugin;

    for (
        plugin = __start_ap_plugins;
        plugin < __stop_ap_plugins;
        plugin++
    )
    {
        if ((*plugin)->shutdown != NULL)
            (*plugin)->shutdown();
    }
}

uint16_t ap_plugin_manager_get_plugin_types_registered(void)
{
    return plugin_types_registered;
}

uint16_t ap_plugin_manager_get_plugin_instances_loaded(void)
{
    return plugin_instances_loaded;
}