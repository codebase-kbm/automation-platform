#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/tree.h>

#include "ap_object.h"
#include "ap_plugin_compiler.h"


#define AP_MQTT_CONFIG_VERSION 1u

#define AP_MQTT_MAP_FLAG_SUBSCRIBE  0x01u
#define AP_MQTT_MAP_QOS_SHIFT       1u
#define AP_MQTT_MAP_QOS_MASK        0x06u
#define AP_MQTT_MAP_FLAG_RETAIN     0x08u


static int write_u8(
    ap_plugin_config_buffer_t *buffer,
    uint8_t value)
{
    if (buffer->length + 1 > buffer->capacity)
        return -1;

    buffer->data[buffer->length++] = value;

    return 0;
}


static int write_u16_le(
    ap_plugin_config_buffer_t *buffer,
    uint16_t value)
{
    if (buffer->length + 2 > buffer->capacity)
        return -1;

    buffer->data[buffer->length++] =
        (uint8_t)(value & 0xffu);

    buffer->data[buffer->length++] =
        (uint8_t)((value >> 8) & 0xffu);

    return 0;
}


static int write_u32_le(
    ap_plugin_config_buffer_t *buffer,
    uint32_t value)
{
    if (buffer->length + 4 > buffer->capacity)
        return -1;

    buffer->data[buffer->length++] =
        (uint8_t)(value & 0xffu);

    buffer->data[buffer->length++] =
        (uint8_t)((value >> 8) & 0xffu);

    buffer->data[buffer->length++] =
        (uint8_t)((value >> 16) & 0xffu);

    buffer->data[buffer->length++] =
        (uint8_t)((value >> 24) & 0xffu);

    return 0;
}


static int write_string(
    ap_plugin_config_buffer_t *buffer,
    const char *string)
{
    size_t length;

    if (string == NULL)
        return -1;

    length = strlen(string);

    if (length > UINT8_MAX)
        return -1;

    if (write_u8(buffer, (uint8_t)length) != 0)
        return -1;

    if (buffer->length + length > buffer->capacity)
        return -1;

    memcpy(
        &buffer->data[buffer->length],
        string,
        length
    );

    buffer->length += length;

    return 0;
}


static xmlNodePtr find_child(
    xmlNodePtr parent,
    const char *name)
{
    for (xmlNodePtr node = parent->children;
         node != NULL;
         node = node->next)
    {
        if (node->type != XML_ELEMENT_NODE)
            continue;

        if (xmlStrcmp(
                node->name,
                BAD_CAST name) == 0)
        {
            return node;
        }
    }

    return NULL;
}


static xmlChar *get_attribute(
    xmlNodePtr node,
    const char *name)
{
    return xmlGetProp(
        node,
        BAD_CAST name
    );
}


static int parse_u16(
    const xmlChar *value,
    uint16_t *result)
{
    char *end;
    unsigned long number;

    if (value == NULL || result == NULL)
        return -1;

    number = strtoul(
        (const char *)value,
        &end,
        10
    );

    if (*end != '\0' ||
        number > UINT16_MAX)
    {
        return -1;
    }

    *result = (uint16_t)number;

    return 0;
}


static int parse_u32(
    const xmlChar *value,
    uint32_t *result)
{
    char *end;
    unsigned long number;

    if (value == NULL || result == NULL)
        return -1;

    number = strtoul(
        (const char *)value,
        &end,
        10
    );

    if (*end != '\0' ||
        number > UINT32_MAX)
    {
        return -1;
    }

    *result = (uint32_t)number;

    return 0;
}


static int parse_bool(
    const xmlChar *value,
    uint8_t *result)
{
    if (value == NULL || result == NULL)
        return -1;

    if (xmlStrcasecmp(
            value,
            BAD_CAST "true") == 0 ||
        xmlStrcmp(
            value,
            BAD_CAST "1") == 0)
    {
        *result = 1;
        return 0;
    }

    if (xmlStrcasecmp(
            value,
            BAD_CAST "false") == 0 ||
        xmlStrcmp(
            value,
            BAD_CAST "0") == 0)
    {
        *result = 0;
        return 0;
    }

    return -1;
}


static int parse_value_type(
    const xmlChar *value,
    uint8_t *result)
{
    if (value == NULL || result == NULL)
        return -1;

    if (xmlStrcmp(value, BAD_CAST "bool") == 0)
        *result = AP_VALUE_BOOL;

    else if (xmlStrcmp(value, BAD_CAST "int32") == 0)
        *result = AP_VALUE_INT32;

    else if (xmlStrcmp(value, BAD_CAST "float") == 0)
        *result = AP_VALUE_FLOAT;

    else if (xmlStrcmp(value, BAD_CAST "string") == 0)
        *result = AP_VALUE_STRING;

    else
        return -1;

    return 0;
}


static int compile_mapping(
    xmlNodePtr node,
    ap_plugin_config_buffer_t *buffer)
{
    xmlChar *topic = NULL;
    xmlChar *object_id = NULL;
    xmlChar *type = NULL;
    xmlChar *direction = NULL;
    xmlChar *qos = NULL;
    xmlChar *retain = NULL;

    uint32_t object_id_value;
    uint8_t value_type;
    uint8_t flags = 0;
    uint16_t qos_value = 0;
    uint8_t retain_value = 0;

    int result = -1;

    topic = get_attribute(node, "topic");
    object_id = get_attribute(node, "object_id");
    type = get_attribute(node, "type");
    direction = get_attribute(node, "direction");
    qos = get_attribute(node, "qos");
    retain = get_attribute(node, "retain");

    if (topic == NULL ||
        object_id == NULL ||
        type == NULL ||
        direction == NULL)
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

    if (xmlStrcmp(
            direction,
            BAD_CAST "publish") == 0)
    {
        /* default */
    }
    else if (xmlStrcmp(
                 direction,
                 BAD_CAST "subscribe") == 0)
    {
        flags |= AP_MQTT_MAP_FLAG_SUBSCRIBE;
    }
    else
    {
        goto cleanup;
    }

    if (qos != NULL)
    {
        if (parse_u16(qos, &qos_value) != 0 ||
            qos_value > 2)
        {
            goto cleanup;
        }
    }

    if (retain != NULL)
    {
        if (parse_bool(
                retain,
                &retain_value) != 0)
        {
            goto cleanup;
        }

        if (retain_value)
            flags |= AP_MQTT_MAP_FLAG_RETAIN;
    }

    flags |= (uint8_t)(
        (qos_value << AP_MQTT_MAP_QOS_SHIFT)
        & AP_MQTT_MAP_QOS_MASK
    );

    if (write_string(
            buffer,
            (const char *)topic) != 0 ||
        write_u32_le(
            buffer,
            object_id_value) != 0 ||
        write_u8(
            buffer,
            value_type) != 0 ||
        write_u8(
            buffer,
            flags) != 0)
    {
        goto cleanup;
    }

    result = 0;

cleanup:

    xmlFree(topic);
    xmlFree(object_id);
    xmlFree(type);
    xmlFree(direction);
    xmlFree(qos);
    xmlFree(retain);

    return result;
}


static int ap_mqtt_config_compile(
    xmlNodePtr module,
    ap_plugin_config_buffer_t *buffer)
{
    xmlNodePtr broker;
    xmlNodePtr auth;
    xmlNodePtr client;
    xmlNodePtr mappings;

    xmlChar *host = NULL;
    xmlChar *port = NULL;
    xmlChar *user = NULL;
    xmlChar *password = NULL;
    xmlChar *client_id = NULL;

    uint16_t port_value;
    uint8_t mapping_count = 0;
    size_t mapping_count_offset;

    if (module == NULL ||
        buffer == NULL ||
        buffer->data == NULL)
    {
        return -1;
    }

    broker = find_child(module, "broker");
    auth = find_child(module, "auth");
    client = find_child(module, "client");
    mappings = find_child(module, "mappings");

    if (broker == NULL ||
        auth == NULL ||
        client == NULL ||
        mappings == NULL)
    {
        return -1;
    }

    host = get_attribute(broker, "host");
    port = get_attribute(broker, "port");

    user = get_attribute(auth, "user");
    password = get_attribute(auth, "password");

    client_id = get_attribute(client, "id");

    if (host == NULL ||
        port == NULL ||
        user == NULL ||
        password == NULL ||
        client_id == NULL)
    {
        goto error;
    }

    if (parse_u16(port, &port_value) != 0)
		goto error;

	buffer->length = 0;

	if (write_u8(
			buffer,
			(uint8_t)AP_MODULE_MQTT) != 0 ||
		write_u8(
			buffer,
			AP_MQTT_CONFIG_VERSION) != 0 ||
        write_string(
            buffer,
            (const char *)host) != 0 ||
        write_u16_le(
            buffer,
            port_value) != 0 ||
        write_string(
            buffer,
            (const char *)user) != 0 ||
        write_string(
            buffer,
            (const char *)password) != 0 ||
        write_string(
            buffer,
            (const char *)client_id) != 0)
    {
        goto error;
    }

    /*
     * Mapping count is written after
     * the connection configuration.
     */
    mapping_count_offset = buffer->length;

    if (write_u8(buffer, 0) != 0)
        goto error;

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
            goto error;

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
    xmlFree(user);
    xmlFree(password);
    xmlFree(client_id);

    return 0;

error:

    xmlFree(host);
    xmlFree(port);
    xmlFree(user);
    xmlFree(password);
    xmlFree(client_id);

    buffer->length = 0;

    return -1;
}


static const ap_plugin_compiler_t ap_mqtt_config_plugin =
{
    .type = AP_MODULE_MQTT,
    .name = "mqtt",
    .compile = ap_mqtt_config_compile
};

AP_PLUGIN_COMPILER_REGISTER(
    ap_mqtt_config_plugin
);