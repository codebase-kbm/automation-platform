#include "ap_plugin_manager.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "ap_config_reader.h"
#include "ap_object.h"
#include "ap_plugin_type.h"
#include "ap_registry.h"


typedef struct
{
    const ap_plugin_t *plugin;
    ap_object_id_t object_id;
    bool initialized;

} ap_plugin_instance_t;


extern const ap_plugin_t * const __start_ap_plugins[];
extern const ap_plugin_t * const __stop_ap_plugins[];


static uint16_t plugin_instances_loaded = 0;
static uint16_t plugin_types_registered = 0;

static ap_plugin_instance_t *plugin_instances = NULL;
static uint16_t plugin_instance_count = 0;
static uint16_t plugin_instance_capacity = 0;


/* -------------------------------------------------- */
/* Plugin lookup                                      */
/* -------------------------------------------------- */

const ap_plugin_t *ap_plugin_manager_find(
    ap_plugin_type_t type)
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
/* Add plugin instance                                */
/* -------------------------------------------------- */

static ap_result_t ap_plugin_manager_add_instance(
    const ap_plugin_t *plugin,
    ap_object_id_t object_id,
    ap_plugin_instance_t **instance)
{
    if (plugin == NULL || instance == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_PLUGIN_MANAGER,
            AP_PLUGIN_NONE,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    if (plugin_instance_count >= plugin_instance_capacity)
    {
        uint16_t new_capacity =
            (plugin_instance_capacity == 0)
                ? 8u
                : (uint16_t)(plugin_instance_capacity * 2u);

        ap_plugin_instance_t *new_instances =
            realloc(
                plugin_instances,
                new_capacity * sizeof(*plugin_instances)
            );

        if (new_instances == NULL)
        {
            return AP_RESULT_MAKE(
                AP_RESULT_SOURCE_CORE,
                AP_COMPONENT_PLUGIN_MANAGER,
                AP_PLUGIN_NONE,
                AP_ERROR_OUT_OF_MEMORY
            );
        }

        plugin_instances = new_instances;
        plugin_instance_capacity = new_capacity;
    }

    ap_plugin_instance_t *new_instance =
        &plugin_instances[plugin_instance_count++];

    new_instance->plugin = plugin;
    new_instance->object_id = object_id;
    new_instance->initialized = false;

    *instance = new_instance;

    return AP_OK;
}


/* -------------------------------------------------- */
/* Manager init                                       */
/* -------------------------------------------------- */

ap_result_t ap_plugin_manager_init(void)
{
    ap_config_object_t object;

    plugin_types_registered =
        (uint16_t)(
            __stop_ap_plugins -
            __start_ap_plugins
        );

    plugin_instances_loaded = 0;

    while (ap_config_reader_next(&object) == AP_OK)
    {
        if (object.header.object_type != AP_OBJECT_PLUGIN)
            continue;

        /*
         * Create the runtime object for this
         * configured plugin instance.
         */
        ap_object_t *plugin_object =
            calloc(1, sizeof(*plugin_object));

        if (plugin_object == NULL)
        {
            return AP_RESULT_MAKE(
                AP_RESULT_SOURCE_CORE,
                AP_COMPONENT_PLUGIN_MANAGER,
                AP_PLUGIN_NONE,
                AP_ERROR_OUT_OF_MEMORY
            );
        }

        plugin_object->id =
            object.header.object_id;

        plugin_object->object_type =
            AP_OBJECT_PLUGIN;

        plugin_object->value_type =
            AP_VALUE_NONE;

        plugin_object->status.code =
            AP_ERROR_NONE;

        ap_result_t result =
            ap_registry_register(plugin_object);

        if (result != AP_OK)
        {
            free(plugin_object);
            return result;
        }

        /*
         * Validate plugin configuration.
         */
        if (object.payload == NULL)
        {
            ap_registry_set_objectstatus(
                plugin_object->id,
                AP_ERROR_INVALID_ARGUMENT
            );

            continue;
        }

        ap_plugin_type_t type =
            (ap_plugin_type_t)object.payload[0];

        const ap_plugin_t *plugin =
            ap_plugin_manager_find(type);

        if (plugin == NULL)
        {
            ap_registry_set_objectstatus(
                plugin_object->id,
                AP_ERROR_NOT_FOUND
            );

            continue;
        }

        /*
         * Load plugin instance.
         */
        if (plugin->load != NULL)
        {
            result = plugin->load(&object);

            if (result != AP_OK)
            {
                ap_registry_set_objectstatus(
                    plugin_object->id,
                    AP_RESULT_ERROR(result)
                );

                continue;
            }
        }

        /*
         * Keep the instance after successful load.
         *
         * load() may already have allocated resources
         * which must be released during shutdown.
         */
        ap_plugin_instance_t *instance = NULL;

        result = ap_plugin_manager_add_instance(
            plugin,
            plugin_object->id,
            &instance
        );

        if (result != AP_OK)
        {
            ap_registry_set_objectstatus(
                plugin_object->id,
                AP_RESULT_ERROR(result)
            );

            continue;
        }

        /*
         * Initialize plugin instance.
         */
        if (plugin->init != NULL)
        {
            result = plugin->init();

            if (result != AP_OK)
            {
                ap_registry_set_objectstatus(
                    plugin_object->id,
                    AP_RESULT_ERROR(result)
                );

                continue;
            }
        }

        instance->initialized = true;
        plugin_instances_loaded++;
    }

    return AP_OK;
}


/* -------------------------------------------------- */
/* Runtime processing                                 */
/* -------------------------------------------------- */

ap_result_t ap_plugin_manager_process(void)
{
    for (uint16_t i = 0;
         i < plugin_instance_count;
         i++)
    {
        ap_plugin_instance_t *instance =
            &plugin_instances[i];

        if (!instance->initialized)
            continue;

        if (instance->plugin->process == NULL)
            continue;

        ap_result_t result =
            instance->plugin->process();

        if (result != AP_OK)
        {
            ap_registry_set_objectstatus(
                instance->object_id,
                AP_RESULT_ERROR(result)
            );
        }
    }

    return AP_OK;
}


/* -------------------------------------------------- */
/* Shutdown                                           */
/* -------------------------------------------------- */

void ap_plugin_manager_shutdown(void)
{
    for (uint16_t i = 0;
         i < plugin_instance_count;
         i++)
    {
        ap_plugin_instance_t *instance =
            &plugin_instances[i];

        if (instance->plugin->shutdown != NULL)
            instance->plugin->shutdown();
    }

    free(plugin_instances);

    plugin_instances = NULL;
    plugin_instance_count = 0;
    plugin_instance_capacity = 0;
    plugin_instances_loaded = 0;
}


/* -------------------------------------------------- */
/* Runtime information                                */
/* -------------------------------------------------- */

uint16_t ap_plugin_manager_get_plugin_types_registered(void)
{
    return plugin_types_registered;
}


uint16_t ap_plugin_manager_get_plugin_instances_loaded(void)
{
    return plugin_instances_loaded;
}