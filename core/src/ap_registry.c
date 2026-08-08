#include "ap_registry.h"

#include <stdlib.h>

static const ap_object_t **objects = NULL;
static uint32_t object_count = 0;
static uint32_t object_capacity = 0;

#define AP_REGISTRY_INITIAL_CAPACITY 16u

/* -------------------------------------------------- */
/* Registry init                                      */
/* -------------------------------------------------- */

void ap_registry_init(void)
{
    for (uint32_t i = 0;
         i < object_count;
         i++)
    {
        free((void *)objects[i]);
    }

    free(objects);

    objects = NULL;
    object_count = 0;
    object_capacity = 0;
}

/* -------------------------------------------------- */
/* Get object                                         */
/* -------------------------------------------------- */

const ap_object_t *ap_registry_get(
    ap_object_id_t id)
{
    for (uint32_t i = 0;
         i < object_count;
         i++)
    {
        if (objects[i]->id == id)
            return objects[i];
    }

    return NULL;
}

/* -------------------------------------------------- */
/* Register object                                    */
/* -------------------------------------------------- */

ap_result_t ap_registry_register(
    const ap_object_t *object)
{
    if (object == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_NONE,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    /* Duplicate ID? */
    if (ap_registry_get(object->id) != NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_NONE,
            AP_ERROR_ALREADY_EXISTS
        );
    }

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
        {
            return AP_RESULT_MAKE(
                AP_RESULT_SOURCE_CORE,
                AP_COMPONENT_REGISTRY,
                AP_PLUGIN_NONE,
                AP_ERROR_OUT_OF_MEMORY
            );
        }

        objects = new_objects;
        object_capacity = new_capacity;
    }

    objects[object_count++] = object;

    return AP_OK;
}

/* -------------------------------------------------- */
/* Get or create object                              */
/* -------------------------------------------------- */

ap_result_t ap_registry_get_or_create(
    ap_object_id_t id,
    ap_value_type_t value_type,
    const ap_object_t **object)
{
    const ap_object_t *existing;
    ap_object_t *created;

    if (object == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_NONE,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    *object = NULL;

    existing = ap_registry_get(id);

    if (existing != NULL)
    {
        if (existing->value_type != value_type)
        {
            return AP_RESULT_MAKE(
                AP_RESULT_SOURCE_CORE,
                AP_COMPONENT_REGISTRY,
                AP_PLUGIN_NONE,
                AP_ERROR_TYPE_MISMATCH
            );
        }

        *object = existing;

        return AP_OK;
    }

    created = calloc(
        1,
        sizeof(*created)
    );

    if (created == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_NONE,
            AP_ERROR_OUT_OF_MEMORY
        );
    }

    created->id = id;
    created->object_type = AP_OBJECT_SIGNAL;
    created->value_type = value_type;

    if (ap_registry_register(created) != AP_OK)
    {
        free(created);

        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_NONE,
            AP_ERROR_OPERATION_FAILED
        );
    }

    *object = created;

    return AP_OK;
}