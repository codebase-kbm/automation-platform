#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <libxml/tree.h>

#include "payload_helpers.h"
#include "xml_helpers.h"
#include "ap_object.h"
#include "ap_plugin_compiler.h"

#define AP_PEER_SERVER_CONFIG_VERSION 1u

static int compile_connection(
    xmlNodePtr connection,
    ap_plugin_config_buffer_t *buffer)
{
    xmlChar *object_id = NULL;
    xmlChar *endpoint = NULL;

    uint32_t object_id_value;

    int result = -1;

    if (connection == NULL ||
        buffer == NULL)
    {
        return -1;
    }

    object_id = get_attribute(
        connection,
        "object"
    );

    endpoint = get_attribute(
        connection,
        "endpoint"
    );

    if (object_id == NULL ||
        endpoint == NULL)
    {
        goto cleanup;
    }

    if (parse_u32(
            object_id,
            &object_id_value) != 0)
    {
        goto cleanup;
    }

    /*
     * The endpoint is intentionally kept as a string.
     *
     * 0.0.0.0 means wildcard.
     * Address validation/resolution belongs to
     * the runtime/backend, not the config compiler.
     */
    if (payload_write_u32_le(
            buffer,
            object_id_value) != 0 ||
        payload_write_string(
            buffer,
            (const char *)endpoint) != 0)
    {
        goto cleanup;
    }

    result = 0;

cleanup:

    xmlFree(object_id);
    xmlFree(endpoint);

    return result;
}


static int ap_peer_server_config_compile(
    xmlNodePtr plugin,
    ap_plugin_config_buffer_t *buffer)
{
    xmlChar *port = NULL;

    uint16_t port_value;
    uint8_t connection_count = 0;
    size_t connection_count_offset;

    if (plugin == NULL ||
        buffer == NULL ||
        buffer->data == NULL)
    {
        return -1;
    }

    port = get_attribute(
        plugin,
        "port"
    );

    if (port == NULL)
    {
        goto error;
    }

    if (parse_u16(
            port,
            &port_value) != 0)
    {
        goto error;
    }

    /*
     * A TCP listen port of zero is not useful for the
     * configured peer server.
     */
    if (port_value == 0)
    {
        goto error;
    }

    buffer->length = 0;

    if (payload_write_u8(
            buffer,
            (uint8_t)AP_PLUGIN_PEER_SERVER) != 0 ||
        payload_write_u8(
            buffer,
            AP_PEER_SERVER_CONFIG_VERSION) != 0 ||
        payload_write_u16_le(
            buffer,
            port_value) != 0)
    {
        goto error;
    }

    /*
     * Reserve one byte for the connection count.
     */
    connection_count_offset = buffer->length;

    if (payload_write_u8(
            buffer,
            0) != 0)
    {
        goto error;
    }

    for (xmlNodePtr node = plugin->children;
         node != NULL;
         node = node->next)
    {
        if (node->type != XML_ELEMENT_NODE)
            continue;

        if (xmlStrcmp(
                node->name,
                BAD_CAST "connection") != 0)
        {
            continue;
        }

        if (connection_count == UINT8_MAX)
        {
            goto error;
        }

        if (compile_connection(
                node,
                buffer) != 0)
        {
            goto error;
        }

        connection_count++;
    }

    /*
     * A peer server without configured connection objects
     * is technically valid. Runtime connections can still
     * be accepted if a wildcard configuration is used.
     */
    buffer->data[connection_count_offset] =
        connection_count;

    xmlFree(port);

    return 0;

error:

    xmlFree(port);

    buffer->length = 0;

    fprintf(
        stderr,
        "ERROR: PEER_SERVER config line %ld: invalid\n",
        xmlGetLineNo(plugin)
    );

    return -1;
}


static const ap_plugin_compiler_t ap_peer_server_config_plugin =
{
    .type = AP_PLUGIN_PEER_SERVER,
    .name = "peer_server",
    .compile = ap_peer_server_config_compile
};


AP_PLUGIN_COMPILER_REGISTER(
    ap_peer_server_config_plugin
);