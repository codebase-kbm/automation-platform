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

#define AP_INFLUX_CONFIG_VERSION 1u

static int compile_mapping(
    xmlNodePtr mapping,
    ap_plugin_config_buffer_t *buffer)
{
    xmlChar *object_id = NULL;
    xmlChar *type = NULL;
    xmlChar *name = NULL;

    uint32_t object_id_value;
    uint8_t value_type;

    int result = -1;

    if (mapping == NULL ||
        buffer == NULL)
    {
        return -1;
    }

    object_id = get_attribute(
        mapping,
        "object_id"
    );

    type = get_attribute(
        mapping,
        "type"
    );

    name = get_attribute(
        mapping,
        "name"
    );

    if (object_id == NULL ||
        type == NULL ||
        name == NULL)
    {
        goto cleanup;
    }

    if (parse_u32(
            object_id,
            &object_id_value) != 0)
    {
        goto cleanup;
    }

    if (parse_value_type(
            type,
            &value_type) != 0)
    {
        goto cleanup;
    }

    /*
     * Influx currently supports only
     * value types that can be written
     * as numeric/boolean fields.
     */
    if (value_type != AP_VALUE_BOOL &&
        value_type != AP_VALUE_INT32 &&
        value_type != AP_VALUE_FLOAT)
    {
        goto cleanup;
    }

    if (payload_write_u32_le(
            buffer,
            object_id_value) != 0 ||
        payload_write_u8(
            buffer,
            value_type) != 0 ||
        payload_write_string(
            buffer,
            (const char *)name) != 0)
    {
        goto cleanup;
    }

    result = 0;

cleanup:

    xmlFree(object_id);
    xmlFree(type);
    xmlFree(name);

    return result;
}

static int ap_influx_config_compile(
    xmlNodePtr plugin,
    ap_plugin_config_buffer_t *buffer)
{
    xmlNodePtr connection;
    xmlNodePtr mappings;

    xmlChar *host = NULL;
    xmlChar *port = NULL;
    xmlChar *org = NULL;
    xmlChar *token = NULL;
    xmlChar *bucket = NULL;

    uint16_t port_value;
    uint8_t mapping_count = 0;
    size_t mapping_count_offset;

    if (plugin == NULL ||
        buffer == NULL ||
        buffer->data == NULL)
    {
        return -1;
    }

    connection = find_child(
        plugin,
        "connection"
    );

    mappings = find_child(
        plugin,
        "mappings"
    );

    if (connection == NULL ||
        mappings == NULL)
    {
        goto error;
    }

    host = get_attribute(connection,"host");
    port = get_attribute(connection,"port");
    org = get_attribute(connection,"org");
    token = get_attribute(connection,"token");
    bucket = get_attribute(connection,"bucket");

    if (host == NULL ||
        port == NULL ||
        org == NULL ||
        token == NULL ||
        bucket == NULL)
    {
        goto error;
    }

    if (parse_u16(
            port,
            &port_value) != 0)
    {
        goto error;
    }

    buffer->length = 0;

    if (payload_write_u8(
            buffer,
            (uint8_t)AP_PLUGIN_INFLUX) != 0 ||
        payload_write_u8(
            buffer,
            AP_INFLUX_CONFIG_VERSION) != 0 ||
        payload_write_string(
            buffer,
            (const char *)host) != 0 ||
        payload_write_u16_le(
            buffer,
            port_value) != 0 ||
        payload_write_string(
            buffer,
            (const char *)org) != 0 ||
        payload_write_string(
            buffer,
            (const char *)token) != 0 ||
        payload_write_string(
            buffer,
            (const char *)bucket) != 0)
    {
        goto error;
    }

    /*
     * Reserve one byte for the mapping count.
     */
    mapping_count_offset = buffer->length;

    if (payload_write_u8(buffer,0) != 0)
    {
        goto error;
    }

    for (xmlNodePtr node = mappings->children;
         node != NULL;
         node = node->next)
    {
        if (node->type != XML_ELEMENT_NODE)
            continue;

        if (xmlStrcmp(
                node->name,
                BAD_CAST "mapping") != 0)
        {
            continue;
        }

        if (mapping_count == UINT8_MAX)
        {
            goto error;
        }

        if (compile_mapping(
                node,
                buffer) != 0)
        {
            goto error;
        }

        mapping_count++;
    }

    buffer->data[mapping_count_offset] =
        mapping_count;

    xmlFree(host);
    xmlFree(port);
    xmlFree(org);
    xmlFree(token);
    xmlFree(bucket);

    return 0;

error:

    xmlFree(host);
    xmlFree(port);
    xmlFree(org);
    xmlFree(token);
    xmlFree(bucket);

    buffer->length = 0;

    fprintf(
        stderr,
        "ERROR: INFLUX config line %ld: invalid\n",
        xmlGetLineNo(plugin)
    );

    return -1;
}

static const ap_plugin_compiler_t ap_influx_config_plugin =
{
    .type = AP_PLUGIN_INFLUX,
    .name = "influx",
    .compile = ap_influx_config_compile
};

AP_PLUGIN_COMPILER_REGISTER(
    ap_influx_config_plugin
);
