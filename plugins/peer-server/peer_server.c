#include <stdlib.h>
#include <string.h>

#include "ap_config_payload_reader.h"
#include "ap_registry.h"
#include "peer_server.h"
#include "peer_registry.h"
#include "tcp.h"

static ap_peer_server_config_t peer_server_config;
static ap_peer_connection_t *peer_connections = NULL;

static void *peer_tcp_context = NULL;

/* -------------------------------------------------- */
/* Config cleanup                                     */
/* -------------------------------------------------- */

static void ap_peer_server_config_clear(
    ap_peer_server_config_t *config)
{
    if (config == NULL)
        return;

    if (config->connections != NULL)
    {
        for (uint8_t i = 0;
             i < config->connection_count;
             i++)
        {
            free(config->connections[i].endpoint);
        }

        free(config->connections);
    }

    memset(
        config,
        0,
        sizeof(*config)
    );
}

/* -------------------------------------------------- */
/* Plugin configuration                               */
/* -------------------------------------------------- */

static ap_result_t ap_peer_server_plugin_load(
    const ap_config_object_t *object)
{
    ap_config_payload_reader_t reader;

    uint8_t plugin_type;
    uint8_t version;
    uint8_t connection_count;

    if (object == NULL ||
        object->payload == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    reader.data =
        object->payload;

    reader.length =
        object->header.payload_length;

    reader.offset = 0;

    /*
     * Plugin type
     */
    if (ap_config_read_u8(
            &reader,
            &plugin_type) != 0 ||
        ap_config_read_u8(
            &reader,
            &version) != 0)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    if (plugin_type != AP_PLUGIN_PEER_SERVER)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    if (version != AP_PEER_SERVER_CONFIG_VERSION)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    /*
     * Reset previous configuration.
     */
    ap_peer_server_config_clear(
        &peer_server_config
    );

    /*
     * Listen port.
     */
    if (ap_config_read_u16_le(
            &reader,
            &peer_server_config.port) != 0 ||
        ap_config_read_u8(
            &reader,
            &connection_count) != 0)
    {
        ap_peer_server_config_clear(
            &peer_server_config
        );

        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    peer_server_config.connection_count =
        connection_count;

    /*
     * Allocate connection configuration.
     */
    if (connection_count > 0)
    {
        peer_server_config.connections =
            calloc(
                connection_count,
                sizeof(*peer_server_config.connections)
            );

        if (peer_server_config.connections == NULL)
        {
            ap_peer_server_config_clear(
                &peer_server_config
            );

            return AP_RESULT_MAKE(
                AP_RESULT_SOURCE_PLUGIN,
                AP_COMPONENT_CONFIG_READER,
                AP_PLUGIN_PEER_SERVER,
                AP_ERROR_OUT_OF_MEMORY
            );
        }
    }

    /*
     * Read connection configuration.
     */
    for (uint8_t i = 0;
         i < connection_count;
         i++)
    {
        ap_peer_server_connection_config_t *connection =
            &peer_server_config.connections[i];

        if (ap_config_read_u32_le(
                &reader,
                &connection->object_id) != 0 ||
            ap_config_read_string(
                &reader,
                &connection->endpoint) != 0)
        {
            ap_peer_server_config_clear(
                &peer_server_config
            );

            return AP_RESULT_MAKE(
                AP_RESULT_SOURCE_PLUGIN,
                AP_COMPONENT_CONFIG_READER,
                AP_PLUGIN_PEER_SERVER,
                AP_ERROR_INVALID_ARGUMENT
            );
        }

        /*
        * The connection object must exist.
        *
        * Unlike normal signal mappings, the peer server
        * does not create a signal object here. The object
        * represents the configured connection itself.
        */
        const ap_object_t *connection_object;

        ap_result_t result =
            ap_registry_create_object(
                connection->object_id,
                AP_OBJECT_CONNECTION,
                AP_VALUE_NONE,
                &connection_object
            );

        if (result != AP_OK)
        {
            ap_peer_server_config_clear(
                &peer_server_config
            );

            return result;
        }
    }

    /*
     * The payload must be consumed completely.
     */
    if (reader.offset != reader.length)
    {
        ap_peer_server_config_clear(
            &peer_server_config
        );

        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_CONFIG_READER,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    return AP_OK;
}

/* -------------------------------------------------- */
/* Plugin init                                        */
/* -------------------------------------------------- */

static ap_result_t ap_peer_server_plugin_init(void)
{
ap_result_t result;

    if (peer_server_config.connection_count == 0)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_INIT,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_INVALID_ARGUMENT
        );
    }

    peer_connections =
        calloc(
            peer_server_config.connection_count,
            sizeof(*peer_connections)
        );

    if (peer_connections == NULL)
    {
        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_INIT,
            AP_PLUGIN_PEER_SERVER,
            AP_ERROR_OUT_OF_MEMORY
        );
    }

    ap_peer_registry_init(
        peer_connections,
        peer_server_config.connection_count
    );

    for (uint8_t i = 0;
         i < peer_server_config.connection_count;
         i++)
    {
        peer_connections[i].connection_object_id =
            peer_server_config.connections[i].object_id;
    }

    result = ap_tcp_backend.create(
        &peer_tcp_context
    );

    if (result != AP_OK)
    {
        free(peer_connections);
        peer_connections = NULL;

        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_INIT,
            AP_PLUGIN_PEER_SERVER,
            AP_RESULT_ERROR(result)
        );
    }

    result = ap_tcp_backend.listen(
        peer_tcp_context,
        "0.0.0.0",
        peer_server_config.port,
        peer_server_config.connection_count
    );

    if (result != AP_OK)
    {
        ap_tcp_backend.destroy(
            peer_tcp_context
        );

        peer_tcp_context = NULL;

        free(peer_connections);
        peer_connections = NULL;

        return AP_RESULT_MAKE(
            AP_RESULT_SOURCE_PLUGIN,
            AP_COMPONENT_INIT,
            AP_PLUGIN_PEER_SERVER,
            AP_RESULT_ERROR(result)
        );
    }

    return AP_OK;
}

/* -------------------------------------------------- */
/* Plugin process                                     */
/* -------------------------------------------------- */

static ap_result_t ap_peer_server_plugin_process(void)
{
    void *tcp_connection = NULL;
    uint8_t remote_address[4];
    uint16_t remote_port;

    ap_result_t result =
        ap_tcp_backend.accept(
            peer_tcp_context,
            &tcp_connection,
            remote_address,
            &remote_port
        );

    if (result != AP_OK)
        return result;

    /*
     * No pending connection.
     */
    if (tcp_connection == NULL)
        return AP_OK;

    /*
     * Find a configured and currently free
     * Peer connection slot.
     */
    ap_peer_connection_t *connection = NULL;

    result = ap_peer_registry_add(&connection);
    if (result != AP_OK)
    {
        ap_tcp_backend.disconnect(
            peer_tcp_context,
            tcp_connection
        );

        return result;
    }
    connection->connection = tcp_connection;

    return AP_OK;
}


/* -------------------------------------------------- */
/* Plugin shutdown                                    */
/* -------------------------------------------------- */

static void ap_peer_server_plugin_shutdown(void)
{
    free(peer_connections);
    peer_connections = NULL;

    ap_peer_server_config_clear(
        &peer_server_config
    );
}


/* -------------------------------------------------- */
/* Plugin definition                                  */
/* -------------------------------------------------- */

const ap_plugin_t ap_peer_server_plugin =
{
    .type = AP_PLUGIN_PEER_SERVER,
    .name = "peer_server",

    .dependencies = NULL,
    .dependency_count = 0,

    .load = ap_peer_server_plugin_load,
    .init = ap_peer_server_plugin_init,
    .process = ap_peer_server_plugin_process,
    .shutdown = ap_peer_server_plugin_shutdown
};

AP_PLUGIN_REGISTER(
    ap_peer_server_plugin
);