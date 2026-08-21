#include "peer_registry.h"

#include <string.h>

static ap_peer_connection_t *registry_connections = NULL;
static size_t registry_connection_count = 0;


/* -------------------------------------------------- */
/* Registry init                                      */
/* -------------------------------------------------- */

void ap_peer_registry_init(
    ap_peer_connection_t *connections,
    size_t connection_count)
{
    registry_connections = connections;
    registry_connection_count = connection_count;

    if (registry_connections != NULL)
    {
        memset(
            registry_connections,
            0,
            registry_connection_count *
            sizeof(*registry_connections)
        );
    }
}


/* -------------------------------------------------- */
/* Add connection                                     */
/* -------------------------------------------------- */

ap_result_t ap_peer_registry_add(ap_peer_connection_t **connection)
{
    if (connection == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    *connection = NULL;

    if (registry_connections == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_NOT_INITIALIZED
        );
    }

    for (size_t i = 0; i < registry_connection_count; i++)
    {
        /*
         * The Connection Object ID is assigned by the
         * configuration and remains associated with this slot.
         *
         * A NULL backend connection means that the slot is free.
         */
        if (registry_connections[i].connection_object_id != 0 &&
            registry_connections[i].connection == NULL)
        {
            *connection = &registry_connections[i];
            ap_peer_decoder_init(&registry_connections[i].decoder);

            return AP_OK;
        }
    }

    return AP_RESULT_MAKE(
        AP_RESULT_SOURCE_PLUGIN,
        AP_COMPONENT_REGISTRY,
        AP_PLUGIN_PEER_SERVER,
        AP_ERROR_FULL
    );
}


/* -------------------------------------------------- */
/* Remove connection                                  */
/* -------------------------------------------------- */
ap_result_t ap_peer_registry_remove(ap_peer_connection_t *connection)
{
    if (connection == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    if (connection->connection_object_id == 0)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_REGISTRY,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_NOT_FOUND
        );
    }

    connection->connection = NULL;
    connection->object_count = 0;

    memset(
        connection->object_ids,
        0,
        sizeof(connection->object_ids)
    );
    ap_peer_decoder_init(&connection->decoder);

    return AP_OK;
}

/* -------------------------------------------------- */
/* Find connection                                    */
/* -------------------------------------------------- */
ap_peer_connection_t *ap_peer_registry_find(ap_object_id_t connection_object_id)
{
    if (registry_connections == NULL || connection_object_id == 0)
    {
        return NULL;
    }

    for (size_t i = 0; i < registry_connection_count; i++)
    {
        if (registry_connections[i].connection_object_id == connection_object_id)
        {
            return &registry_connections[i];
        }
    }

    return NULL;
}


/* -------------------------------------------------- */
/* Check registered object                            */
/* -------------------------------------------------- */
bool ap_peer_registry_is_registered(
    const ap_peer_connection_t *connection,
    ap_object_id_t object_id)
{
    if (connection == NULL || connection->connection_object_id == 0)
    {
        return false;
    }

    for (size_t i = 0;
         i < connection->object_count;
         i++)
    {
        if (connection->object_ids[i] == object_id)
            return true;
    }

    return false;
}