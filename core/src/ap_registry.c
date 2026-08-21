#include "ap_registry.h"

#include <stdlib.h>
#include <string.h>

static const ap_object_t **objects = NULL;
static uint32_t object_count = 0;
static uint32_t object_capacity = 0;

#define AP_REGISTRY_INITIAL_CAPACITY 255u
#define AP_OBJECT_STATUS_CONTEXT_SIZE 6

static ap_object_t *ap_registry_get_mutable(
    ap_object_id_t id)
{
    for (uint32_t i = 0;
         i < object_count;
         i++)
    {
        if (objects[i]->id == id)
            return (ap_object_t *)objects[i];
    }

    return NULL;
}

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
/* Create object                                      */
/* -------------------------------------------------- */

ap_result_t ap_registry_create_object(
    ap_object_id_t id,
    ap_object_type_t object_type,
    ap_value_type_t value_type,
    const ap_object_t **object)
{
    ap_object_t *created;
    ap_result_t result;

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

    if (ap_registry_get(id) != NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_NONE,
            AP_ERROR_ALREADY_EXISTS
        );
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
    created->object_type = object_type;
    created->value_type = value_type;
    created->status.code = AP_ERROR_NONE;

    result = ap_registry_register(created);

    if (result != AP_OK)
    {
        free(created);
        return result;
    }

    *object = created;

    return AP_OK;
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
            ((ap_object_t *)existing)->status.code = AP_ERROR_TYPE_MISMATCH;
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

    created = calloc(1,sizeof(*created));

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
    created->status.code = AP_ERROR_NONE;

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

ap_result_t ap_registry_set_objectstatus(
    ap_object_id_t id,
    ap_error_code_t code)
{
    ap_object_t *object =
        ap_registry_get_mutable(id);

    if (object == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_NONE,
            AP_ERROR_NOT_FOUND
        );
    }

    object->status.code = (uint8_t)code;

    return AP_OK;
}

ap_result_t ap_registry_set_objectstatuscontext(
    ap_object_id_t id,
    const uint8_t *context,
    uint8_t size)
{
    ap_object_t *object;

    if (context == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_NONE,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    if (size > AP_OBJECT_STATUS_CONTEXT_SIZE)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_NONE,
            AP_ERROR_INVALID_SIZE
        );
    }

    object = ap_registry_get_mutable(id);

    if (object == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_NONE,
            AP_ERROR_NOT_FOUND
        );
    }

    memcpy(
        object->status.context,
        context,
        size
    );

    return AP_OK;
}

ap_result_t ap_registry_get_objectstatus(
    ap_object_id_t id,
    ap_error_code_t *code)
{
    const ap_object_t *object;

    if (code == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            0,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    object = ap_registry_get(id);

    if (object == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            0,
            AP_ERROR_NOT_FOUND
        );
    }

    *code = (ap_error_code_t)object->status.code;

    return AP_OK;
}

ap_result_t ap_registry_get_objectstatuscontext(
    ap_object_id_t id,
    uint8_t *context,
    uint8_t size)
{
    const ap_object_t *object;

    if (context == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            0,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    if (size < sizeof(object->status.context))
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            0,
            AP_ERROR_INVALID_SIZE
        );
    }

    object = ap_registry_get(id);

    if (object == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_CORE,
            AP_COMPONENT_REGISTRY,
            0,
            AP_ERROR_NOT_FOUND
        );
    }

    memcpy(
        context,
        object->status.context,
        sizeof(object->status.context)
    );

    return AP_OK;
}