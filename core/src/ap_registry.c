#include "ap_registry.h"
#include "ap_config.h"

static const ap_object_t *objects[AP_MAX_VARIABLES];
static uint32_t object_count;

void ap_registry_init(void)
{
    object_count = 0;
}

ap_result_t ap_registry_register(const ap_object_t *object)
{
    if (object == NULL)
        return AP_ERROR_INVALID_ARGUMENT;

    /* Duplicate ID? */
    if (ap_registry_find(object->id) != NULL)
        return AP_ERROR_ALREADY_EXISTS;

    if (object_count >= AP_MAX_VARIABLES)
        return AP_ERROR_FULL;

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