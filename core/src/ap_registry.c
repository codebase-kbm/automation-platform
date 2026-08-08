#include "ap_registry.h"

#include <stdlib.h>

static const ap_object_t **objects = NULL;
static uint32_t object_count = 0;
static uint32_t object_capacity = 0;

#define AP_REGISTRY_INITIAL_CAPACITY 16u


void ap_registry_init(void)
{
    free(objects);

    objects = NULL;
    object_count = 0;
    object_capacity = 0;
}


ap_result_t ap_registry_register(const ap_object_t *object)
{
    if (object == NULL)
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_REGISTRY,AP_PLUGIN_NONE,AP_ERROR_INVALID_ARGUMENT);

    /* Duplicate ID? */
    if (ap_registry_find(object->id) != NULL)
        return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_REGISTRY,AP_PLUGIN_NONE,AP_ERROR_ALREADY_EXISTS);

    if (object_count >= object_capacity)
    {
        uint32_t new_capacity =
            (object_capacity == 0)
                ? AP_REGISTRY_INITIAL_CAPACITY
                : object_capacity * 2u;

        const ap_object_t **new_objects =
            realloc(
                objects,
                new_capacity * sizeof(*objects)
            );

        if (new_objects == NULL)
            return AP_RESULT_MAKE(AP_RESULT_SOURCE_CORE,AP_COMPONENT_REGISTRY,AP_PLUGIN_NONE,AP_ERROR_OUT_OF_MEMORY);

        objects = new_objects;
        object_capacity = new_capacity;
    }

    objects[object_count++] = object;

    return AP_OK;
}


const ap_object_t *ap_registry_find(ap_object_id_t id)
{
    for (uint32_t i = 0; i < object_count; i++)
    {
        if (objects[i]->id == id)
            return objects[i];
    }

    return NULL;
}